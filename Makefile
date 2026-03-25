# ==========================================
# Makefile - Projet Perceptron (Séance 3)
# Auteur: BEZERRA PICANCO Dhomini
# ==========================================

# Compilateur
CC = gcc

# Options de compilation
# -Wall -Wextra
# -g            : Ajoute des infos de débogage
CFLAGS = -Wall -Wextra -g

# Bibliothèques à lier (Linker flags)
# -lm : Bibliothèque mathématique (nécessaire pour exp, tanh, sqrt)
LIBS = -lm

# Nom de l'exécutable final
EXE = seance3

# Liste des fichiers objets nécessaires
OBJS = main.o processus.o ipc_tools.o reseau.o

# --- RÈGLES DE COMPILATION ---

# Règle par défaut
all: $(EXE)

# Édition de liens (Linking): Crée l'exécutable à partir des objets
$(EXE): $(OBJS)
	@echo "Création de l'exécutable $(EXE)..."
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS)

# --- COMPILATION DES FICHIERS SOURCES ---

# Compile main.c
main.o: main.c config.h processus.h
	$(CC) $(CFLAGS) -c main.c

# Compile processus.c (dépend de reseau.h et ipc_tools.h)
processus.o: processus.c config.h processus.h reseau.h ipc_tools.h
	$(CC) $(CFLAGS) -c processus.c

# Compile ipc_tools.c
ipc_tools.o: ipc_tools.c config.h ipc_tools.h
	$(CC) $(CFLAGS) -c ipc_tools.c

# Compile reseau.c
reseau.o: reseau.c config.h reseau.h
	$(CC) $(CFLAGS) -c reseau.c

# Supprime les fichiers temporaires (.o) et l'exécutable
clean:
	@echo "Nettoyage des fichiers compilés..."
	rm -f *.o $(EXE)

# Pour éviter des conflits avec des fichiers qui s'appelleraient 'clean' ou 'all'
.PHONY: all clean
