#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>

sem_t *sem_block;

int main() {
    pid_t pid = getpid();
    FILE* pidfile = fopen("pidanalista.txt", "w");
    if (pidfile == NULL) {
        perror("Erro ao criar pidanalista.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(pidfile, "%d", pid);
    fclose(pidfile); 
    
    // Abrindo semáforo
    sem_block = sem_open("/sem_block", O_RDWR);
    if (sem_block == SEM_FAILED) {
        perror("Erro ao abrir o semáforo");
        exit(EXIT_FAILURE);
    }

    // Pausa inicial
    raise(SIGSTOP);

    // Quando acordado:
    // Bloqueia acesso ao arquivo compartilhado
    sem_wait(sem_block);

    FILE* LNG = fopen("LNG.txt", "r+");
    if (LNG == NULL) {
        perror("Erro ao abrir LNG.txt");
        sem_post(sem_block);
        sem_close(sem_block);
        exit(EXIT_FAILURE);
    }

    // Lê os primeiros 10 valores de LNG.txt e imprime
    char buffer[1024];
    char restante[1024] = "";
    int linha = 0;
    while (fgets(buffer, sizeof(buffer), LNG) != NULL) {
        if (linha < 10) {
            printf("Linha %d: %s", linha + 1, buffer); // Imprime as primeiras 10 linhas
        } else {
            strcat(restante, buffer); // Armazena o restante das linhas
        }
        linha++;
    }

    // Limpa o arquivo e escreve o restante dos dados
    freopen("LNG.txt", "w", LNG);
    fprintf(LNG, "%s", restante);

    fclose(LNG);
    sem_post(sem_block); // Libera o semáforo
    printf("Seção crítica concluída. Voltando a dormir...\n");

    // Pausa novamente
    raise(SIGSTOP);

    return 0; 
}
