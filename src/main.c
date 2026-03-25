#include "config.h"
#include "processus.h"

/* Allocation réelle des variables globales */
int pipes_in[TAILLE_entree][2];     // Lecteur -> Application
int pipes_out[NB_neurones_C2][2];   // Application -> Analyseur
int pipe_ref[2];                    // Lecteur -> Analyseur (Classe attendue)
int pipe_res[2];                    // Analyseur -> Moniteur (Résultats finaux)
int shm_id;
int sem_id;

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    int i;
    
    // 1. Shared Memory
    shm_id = shmget(IPC_PRIVATE, sizeof(shm_t), IPC_CREAT | 0666);
    if (shm_id < 0) { perror("Erreur shmget"); exit(1); }

    // Semaphore (2 semaphores: un pour le vide, un pour le plein)
    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    if (sem_id < 0) { perror("Erreur semget"); exit(1); }

    // Initialisation des sémaphores
    // SEM_VIDE = 1 (peut être écrit initialement)
    if(semctl(sem_id, SEM_VIDE, SETVAL, 1) < 0) { perror("Erreur init SEM_VIDE"); exit(1); }
    // SEM_PLEIN = 0 (impossible de lire initialement)
    if(semctl(sem_id, SEM_PLEIN, SETVAL, 0) < 0) { perror("Erreur init SEM_PLEIN"); exit(1); }

    // Pipes entree
    for(i = 0; i < TAILLE_entree; i++) {
        if(pipe(pipes_in[i]) < 0) { perror("Erreur pipe entree"); exit(1); }
    }
    // Pipes sortie
    for(i = 0; i < NB_neurones_C2; i++) {
        if(pipe(pipes_out[i]) < 0) { perror("Erreur pipe sortie"); exit(1); }
    }
    // Pipe reference
    if(pipe(pipe_ref) < 0) { perror("Erreur pipe reference"); exit(1); }
    // Pipe resultat
    if(pipe(pipe_res) < 0) { perror("Erreur pipe resultats"); exit(1); }


    // Couche 1
    if (fork() == 0) {
        // Fermer les pipes inutilisés
        close(pipe_ref[0]); close(pipe_ref[1]);
        close(pipe_res[0]); close(pipe_res[1]);
        for(i=0; i<NB_neurones_C2; i++) { close(pipes_out[i][0]); close(pipes_out[i][1]); }
        
        couche_1();
        exit(0);
    } 

    // Couche 2
    if (fork() == 0) {
        // Fermer pipes inutilisés
        close(pipe_ref[0]); close(pipe_ref[1]);
        close(pipe_res[0]); close(pipe_res[1]);
        for(i=0; i<TAILLE_entree; i++) { close(pipes_in[i][0]); close(pipes_in[i][1]); }

        couche_2();
        exit(0);
    }

    // Analyseur
    if (fork() == 0) {
        // Fermer pipes inutilisés
        for(i=0; i<TAILLE_entree; i++) { close(pipes_in[i][0]); close(pipes_in[i][1]); }
        analyseur();
        exit(0);
    } 

    // Lecteur
    pid_t pid_lect = fork();
    if (pid_lect == 0) {
        // Fermer pipes inutilisés
        for(i=0; i<NB_neurones_C2; i++) { close(pipes_out[i][0]); close(pipes_out[i][1]); }
        close(pipe_res[0]); close(pipe_res[1]);
        lecteur();
        exit(0);
    }

    // Moniteur (pere)
    
    // Fermer pipes inutilisés
    for(i=0; i<TAILLE_entree; i++) { close(pipes_in[i][0]); close(pipes_in[i][1]); }
    for(i=0; i<NB_neurones_C2; i++) { close(pipes_out[i][0]); close(pipes_out[i][1]); }
    close(pipe_ref[0]); close(pipe_ref[1]);
    close(pipe_res[1]); // Lecture

    moniteur(pid_lect);
        
    // Attendre la fin des 4 fils
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}