# Mini Projet - UNIX - Perceptron

Projet proposé par M. Frederic ROUSSEAU

Ce projet implémente, en langage C, un perceptron multicouche (MLP) pour la classification des fleurs de l'ensemble de données Iris, organisé sous la forme d'une application multiprocessus sous Unix. Le modèle reçoit les caractéristiques physiques des fleurs (sépales et pétales) et produit en sortie la classe prédite (Setosa, Versicolor ou Virginica), servant ainsi de noyau informatique pour illustrer un pipeline complet de traitement des données et d'inférence.

L'application est décomposée en plusieurs processus aux responsabilités bien définies — telles que la lecture des données, l'exécution du modèle, l'analyse des résultats et la surveillance — qui communiquent entre eux à l'aide de mécanismes d'IPC tels que les pipes, les signaux, la mémoire partagée et les sémaphores. Cette architecture a pour objectif d'explorer et de consolider des concepts fondamentaux de la programmation des systèmes, tels que la concurrence, la communication et la synchronisation entre processus, appliqués à un cas concret d'apprentissage automatique.

<p align="center">
  <img width="475" height="276" alt="perceptron" src="https://github.com/user-attachments/assets/4bb3a4a6-4149-4c82-af22-e077359403a6" />
</p>

**Moniteur:** coordonne l'exécution et affiche les résultats  
**Lecteur:** lit et prépare les données d'entrée  
**Application:** exécute le perceptron  
**Analyseur:** compare la sortie à la valeur attendue  
**Couche 1/Couche 2:** exécution séparée des couches du modèle  

Technologies et concepts utilisés
- Langage C
- Programmation multiprocessus (fork)
- Pipes (pipe)
- Signaux (SIGUSR1)
- Mémoire partagée (shm)
- Sémaphores (sem)
- Makefile (compilation modulaire)
- Analyse de fichiers CSV

## Organisation

```
.
│   Makefile
│   README.md
├───include
│       config.h
│       ipc_tools.h
│       processus.h
│       reseau.h
└───src
        ipc_tools.c
        main.c
        processus.c
        reseau.c
```

## Building

```
git clone git@github.com:dhominicx/miniprojet-unix-perceptron.git
cd miniprojet-unix-perceptron
make
```

## Exécution

```
./seance3
```
