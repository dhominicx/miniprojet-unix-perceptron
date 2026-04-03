# ==========================================
# Makefile - Projet Perceptron (Séance 3)
# Auteur: BEZERRA PICANCO Dhomini
# ==========================================

# Compilateur
CC = gcc

# Dossiers
SRC_DIR = src
INC_DIR = include

# Options de compilation
# -Wall -Wextra
# -g            : Ajoute des infos de débogage
CFLAGS = -Wall -Wextra -g

# Bibliothèques à lier
LIBS = -lm

# Nom de l'exécutable final
EXE = seance3

# Fichiers sources
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/processus.c \
       $(SRC_DIR)/ipc_tools.c \
       $(SRC_DIR)/reseau.c

# Fichiers objets
OBJS = $(SRCS:.c=.o)

# RÈGLES DE COMPILATION

# Règle par défaut
all: $(EXE)

# Édition de liens
$(EXE): $(OBJS)
	@echo "Création de l'exécutable $(EXE)..."
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS)

# Compilation générique des .c en .o
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	@echo "Nettoyage des fichiers compilés..."
	rm -f $(SRC_DIR)/*.o $(EXE)

# Recompilation complète
fclean: clean

re: fclean all

# Pour éviter des conflits avec des fichiers qui s'appelleraient 'clean' ou 'all'
.PHONY: all clean
