#include "config.h"
#include "ipc_tools.h"

/* Opérations sémaphores P (Wait) et V (Signal) */
void P(int sem_id, int sem_num) {
    struct sembuf operation;
    operation.sem_num = sem_num;
    operation.sem_op = -1;
    operation.sem_flg = 0;
    semop(sem_id, &operation, 1);
}

void V(int sem_id, int sem_num) {
    struct sembuf operation;
    operation.sem_num = sem_num;
    operation.sem_op = 1;
    operation.sem_flg = 0;
    semop(sem_id, &operation, 1);
}