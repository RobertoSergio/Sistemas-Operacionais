#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

sem_t *sem_block;
int linha_pid =1;

void ler_imprimir (){
    FILE* LNG = fopen("LNG.txt", "r+");
    if (LNG == NULL) {
        perror("Erro ao abrir LNG.txt");
        sem_post(sem_block);
        sem_close(sem_block);
        exit(EXIT_FAILURE);
    }

    fseek(LNG, 0, SEEK_END);  
    long file_size = ftell(LNG); 
    fseek(LNG, 0, SEEK_SET); 

    char *restante = malloc(file_size + 1);
    if (restante == NULL) {
        perror("Erro ao alocar memória");
        sem_post(sem_block);
        sem_close(sem_block);
        exit(EXIT_FAILURE);
    }
    restante[0] = '\0'; //

    // Lê os primeiros 10 valores de LNG.txt e imprime
    //malloc size of LNG.txt
    char buffer[1024];
    int linha = 0;
    while (fgets(buffer, sizeof(buffer), LNG) != NULL) {
        if (linha < 10) {
            printf("Linha %d: %s", linha_pid, buffer); // Imprime as primeiras 10 linhas
            linha_pid++;
        } else {
            strcat(restante, buffer); // Armazena o restante das linhas
        }
        linha++;
    }

    // Limpa o arquivo e escreve o restante dos dados
    fclose(LNG);
    LNG = fopen("LNG.txt", "w");
    if (LNG == NULL) {
        perror("Erro ao reabrir LNG.txt");
        sem_post(sem_block);
        // sem_close(sem_block);
        exit(EXIT_FAILURE);
    }

    fprintf(LNG, "%s", restante);
    fclose(LNG);

    free(restante);
}

int main() {
    sem_t *sem_analista;
    sem_analista = sem_open("/sem_analista", O_RDWR);

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

    // ler_imprimir ();

    // sem_post(sem_block); // Libera arquivo compartilahdo

    // // Pausa novamente
    // raise(SIGSTOP);

    // sem_analista = sem_open("/sem_analista", O_RDWR);

    // sem_wait(sem_analista);

    // if (sem_block == SEM_FAILED) {
    //     perror("Erro ao abrir o semáforo do analista\n");
    //     exit(EXIT_FAILURE);
    // }

    while (1){
        const char *caminho = "LNG.txt";
        struct stat status_arquivo;

        // Obtém informações sobre o arquivo
        if (stat(caminho, &status_arquivo) == 0) {
            if (status_arquivo.st_size > 0) {
                ler_imprimir ();
            } else {
                break;
            }
        } else {
            perror("Erro ao acessar o arquivo");
        }

    }

    sem_post(sem_block); 
    sem_post(sem_analista);

    return 0; 
}
