#include "config.h"
#include "processus.h"
#include "reseau.h"
#include "ipc_tools.h"

/* Controle du Signal */
volatile sig_atomic_t peut_lire = 0;

void handle_sigusr1(int sig) { (void)sig; peut_lire = 1; }

/* Couche 1 : Lire pipes -> Calculer -> Ecrire dans le SHM */
void couche_1(){
    int i, j;
    
    // Attachement de la mémoire partagée
    shm_t *mem = (shm_t*) shmat(shm_id, NULL, 0);
    if (mem == (void*) -1) { perror("Erreur shmat C1"); exit(1); }
    mem->fin_traitement = 0;

    // Fermer les bouts non utilisés (lecture only)
    for(i=0; i<TAILLE_entree; i++) close(pipes_in[i][1]); 

    // Poids Couche 1
    double coef_MAT1[TAILLE_entree+1][NB_neurones_C1] = 
        {{ +1.703661357392334, +2.763631108685933, -2.17236310620485}, 
        {+1.560385693578073, +3.973334575425574, -5.687438139503607}, 
        {-2.681166224391079, -3.131516653284507, +3.143949358790173}, 
        {-0.02639163020174042, -6.500059250638189, +7.00651555282157}, 
        {-4.077513418151064, +7.565128560821631, -8.505618677932459}}; 

    double entree[TAILLE_entree];

    while(1){
        // Lire les entrées (bloque jusqu'à ce que le Lecteur envoie)
        int status = 0;
        for(i=0; i<TAILLE_entree; i++) {
            status = read(pipes_in[i][0], &entree[i], sizeof(double));
        }
        if (status <= 0) break; // EOF do Lecteur

        // Attendre l'espace mémoire (P(SEM_VIDE))
        P(sem_id, SEM_VIDE);

        // Calculer la couche 1
        for (j = 0; j < NB_neurones_C1; j++) {
            double somme = coef_MAT1[TAILLE_entree][j]; 
            for (i = 0; i < TAILLE_entree; i++)
                somme += entree[i] * coef_MAT1[i][j];
            mem->activations[j] = act_tanh(somme); // Écrire dans le SHM
        }

        // Signalisation de la présence de données (V(SEM_PLEIN))
        V(sem_id, SEM_PLEIN);
    }
    
    // Finalisation : Dites à Couche 2 que c'est fini
    P(sem_id, SEM_VIDE);
    mem->fin_traitement = 1;
    V(sem_id, SEM_PLEIN);

    shmdt(mem);
    for(i=0; i<TAILLE_entree; i++) close(pipes_in[i][0]);
}

/* Couche 2: Read SHM -> Calculate -> Write Pipes */
void couche_2(){
    int j, k, i;

    // Attachement de la mémoire partagée
    shm_t *mem = (shm_t*) shmat(shm_id, NULL, 0);
    if (mem == (void*) -1) { perror("Erreur shmat C2"); exit(1); }

    // Fermer les pipes non utilisés (WRITE ONLY)
    for(i=0; i<NB_neurones_C2; i++) close(pipes_out[i][0]);

    // Poids Couche 2
    double coef_MAT2[NB_neurones_C1+1][NB_neurones_C2] = 
        {{ +9.001713871164204, -10.52803062971971, -1.602240268200863},
        {-1.12770505333443, +4.930756962081604, -6.057546672046951}, 
        {-3.42697126393812, -5.601264307780169, +5.878238858982915}, 
        {-3.31644629415654, +1.215105079193245, -1.007589578200665 }};

    double C1[NB_neurones_C1];
    double C2_in[NB_neurones_C2], C2_out[NB_neurones_C2];

    while(1){
        // Attendre les données en mémoire (P(SEM_PLEIN))
        P(sem_id, SEM_PLEIN);

        if (mem->fin_traitement) {
            // Si le drapeau de fin est actif, libérer le sémaphore et sortir.
            V(sem_id, SEM_PLEIN); // Facultatif: garder le signal pour assurer une sortie propre s'il y a plus de consommateurs.
            break; 
        }

        // 2. lecture du SHM
        for(j=0; j<NB_neurones_C1; j++) {
            C1[j] = mem->activations[j];
        }

        // Libérer l'espace (V(SEM_VIDE)) - Permet au C1 de calculer la prochaine valeur de l'espace.
        V(sem_id, SEM_VIDE);

        // Calculer la couche 2
        for (k = 0; k < NB_neurones_C2; k++) {
            double somme = coef_MAT2[NB_neurones_C1][k];
            for (j = 0; j < NB_neurones_C1; j++)
                somme += C1[j] * coef_MAT2[j][k];
            C2_in[k] = somme;
        }
        softmax(C2_in, C2_out, NB_neurones_C2);

        // Écrire dans les pipes pour Analyseur
        for(k=0; k<NB_neurones_C2; k++) {
            write(pipes_out[k][1], &C2_out[k], sizeof(double));
        }
    }

    shmdt(mem);
    for(i=0; i<NB_neurones_C2; i++) close(pipes_out[i][1]);
}

/* MONITEUR: Gere les processus et affiche les statistiques */
void moniteur(pid_t pid_lecteur){
    printf("[Moniteur] En attente de stabilisation...\n");
    sleep(1); 
    
    printf("[Moniteur] Envoi de SIGUSR1 au Lecteur (PID %d)...\n", pid_lecteur);
    kill(pid_lecteur, SIGUSR1);

    int correct = 0, total = 0;
    
    // Lire les statistiques de l'Analyseur
    read(pipe_res[0], &correct, sizeof(int));
    read(pipe_res[0], &total, sizeof(int));

    printf("\n=== RÉSULTAT FINAL ===\n");
    printf("Total Échantillons: %d\n", total);
    printf("Corrects : %d\n", correct);
    if (total > 0)
        printf("Taux de réussite: %.2f%%\n", (double)correct/total * 100.0);
    
    close(pipe_res[0]);
}

/* LECTEUR: Lit le CSV et envoie les données */
void lecteur() {
    signal(SIGUSR1, handle_sigusr1); 
    
    // Fermer la lecture des pipes globaux
    for(int i=0; i<TAILLE_entree; i++) close(pipes_in[i][0]);
    close(pipe_ref[0]);

    printf("[Lecteur] En attente du signal...\n");
    pause(); // Attend SIGUSR1
    if (!peut_lire) exit(0);

    // Verifie si le fichier existe
    FILE *file = fopen("data_Iris.csv", "r"); 
    if (!file) {
        perror("Erreur: Impossible d'ouvrir data_Iris.csv");
        exit(1);
    }

    // Charger toutes les données en mémoire
    printf("[Lecteur] Lecture du fichier et calcul des statistiques...\n");

    double data[150][TAILLE_entree];  // Stockage des données brutes
    int classes[150];
    int total_count = 0;

    char line[MAX_LINE_LENGTH];

    // Première lecture: charger les données
    while (fgets(line, MAX_LINE_LENGTH, file) && total_count < 150) {
        double val[TAILLE_entree];
        char *token;
        int class_id = -1;

        // Analyse en utilisant le point-virgule
        token = strtok(line, ";"); if(!token) continue; val[0] = atof(token);
        token = strtok(NULL, ";"); if(!token) continue; val[1] = atof(token);
        token = strtok(NULL, ";"); if(!token) continue; val[2] = atof(token);
        token = strtok(NULL, ";"); if(!token) continue; val[3] = atof(token);

        // Lire la classe (dernière colonne)
        token = strtok(NULL, ";\n\r");
        if(!token) continue;

        class_id = atoi(token);

        // Si la classe est valide (0, 1 ou 2), stocker les données
        if (class_id >= 0 && class_id <= 2) {
            for (int i = 0; i < TAILLE_entree; i++) {
                data[total_count][i] = val[i];
            }
            classes[total_count] = class_id;
            total_count++;
        }
    }
    fclose(file);

    // Calculer la moyenne pour chaque feature
    double mean[TAILLE_entree] = {0};
    for (int j = 0; j < TAILLE_entree; j++) {
        for (int i = 0; i < total_count; i++) {
            mean[j] += data[i][j];
        }
        mean[j] /= total_count;
    }

    // Calculer l'écart-type pour chaque feature
    double std[TAILLE_entree] = {0};
    for (int j = 0; j < TAILLE_entree; j++) {
        for (int i = 0; i < total_count; i++) {
            double diff = data[i][j] - mean[j];
            std[j] += diff * diff;
        }
        std[j] /= total_count;
        std[j] = sqrt(std[j]);
    }

    // Afficher les statistiques calculées
    printf("[Lecteur] Statistiques calculées :\n");
    for (int j = 0; j < TAILLE_entree; j++) {
        printf("  Feature %d - Moyenne: %.6f, Écart-type: %.6f\n", j, mean[j], std[j]);
    }

    // Envoyer les données normalisées vers les pipes
    printf("[Lecteur] Envoi de %d échantillons normalisés...\n", total_count);
    for (int i = 0; i < total_count; i++) {
        // Envoie les données normalisees vers Application (utilise pipes_in global)
        for (int j = 0; j < TAILLE_entree; j++) {
            double norm = (data[i][j] - mean[j]) * std[j];
            if(write(pipes_in[j][1], &norm, sizeof(double)) == -1) {
                perror("Erreur ecriture pipe App"); break;
            }
        }
        // Envoie la classe attendue vers Analyseur (utilise pipe_ref)
        if(write(pipe_ref[1], &classes[i], sizeof(int)) == -1) {
            perror("Erreur ecriture pipe Ref"); break;
        }
    }

    printf("[Lecteur] %d lignes envoyées avec succes\n", total_count);

    // Fermer lecriture pour envoyer EOF
    for(int i=0; i<TAILLE_entree; i++) close(pipes_in[i][1]);
    close(pipe_ref[1]);
}

/* ANALYSEUR */
void analyseur(){
    for(int i=0; i<NB_neurones_C2; i++) close(pipes_out[i][1]);
    close(pipe_ref[1]);
    close(pipe_res[0]);

    int target_class;
    double probs[NB_neurones_C2];
    int correct = 0, total = 0;

    while(read(pipe_ref[0], &target_class, sizeof(int)) > 0) {
        for(int i=0; i<NB_neurones_C2; i++) {
            read(pipes_out[i][0], &probs[i], sizeof(double));
        }
        int predicted = 0;
        if (probs[1] > probs[0]) predicted = 1;
        if (probs[2] > probs[predicted]) predicted = 2;

        if (predicted == target_class) correct++;
        total++;
    }

    write(pipe_res[1], &correct, sizeof(int));
    write(pipe_res[1], &total, sizeof(int));

    for(int i=0; i<NB_neurones_C2; i++) close(pipes_out[i][0]);
    close(pipe_ref[0]);
    close(pipe_res[1]);
}