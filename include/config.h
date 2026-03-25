/* BEZERRA PICANCO Dhomini
IESE4 - UNIX
16/12/2025

Miniprojet Perceptron - Fichier de configuration
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

/* Structure du réseau */
#define NB_neurones_C1 3
#define NB_neurones_C2 3

/* 4 données d'entree et une valeur de sortie */
#define TAILLE_entree   4
#define MAX_LINE_LENGTH 1024

/* Structure de la mémoire partagée */
typedef struct {
    double activations[NB_neurones_C1]; // Sortie de la couche 1
    int fin_traitement;                 // Drapeau pour indiquer à C2 qu'il est terminé
} shm_t;

/* Définitions des sémaphores */
#define SEM_VIDE 0 // Espace disponible pour l'écriture (contrôles C1)
#define SEM_PLEIN 1 // Données disponibles pour la lecture (contrôles C2)

/* Variables globales pour les Pipes (déclarées comme extern) */
extern int pipes_in[TAILLE_entree][2];     // Lecteur -> Application
extern int pipes_out[NB_neurones_C2][2];   // Application -> Analyseur
extern int pipe_ref[2];                    // Lecteur -> Analyseur (Classe attendue)
extern int pipe_res[2];                    // Analyseur -> Moniteur (Résultats finaux)

/* IDs IPC */
extern int shm_id;
extern int sem_id;

#endif