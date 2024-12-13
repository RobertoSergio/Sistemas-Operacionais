#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

void* atendimento_thread(void* args) {
    Fila* fila = (Fila*)args;
    FILE* lng = fopen("LNG.txt", "w");

    if (!lng) {
        perror("Erro ao abrir arquivo LNG");
        return NULL;
    }

    while (1) {
        pthread_mutex_lock(&fila->lock);
        Cliente* cliente = fila->inicio;

        if (cliente) {
            fila->inicio = cliente->prox;
        }
        pthread_mutex_unlock(&fila->lock);

        if (cliente) {
            printf("Atendente: Atendendo cliente %d...\n", cliente->pid);
            usleep(cliente->paciencia * 1000);
            fprintf(lng, "PID: %d\n", cliente->pid);
            fflush(lng);

            int tempo_espera = ((int)time(NULL) - cliente->hora_chegada) * 1000;
            int satisfeito = (tempo_espera <= cliente->paciencia);
            printf("Cliente %d %s\n", cliente->pid, satisfeito ? "Satisfeito" : "Insatisfeito");

            free(cliente);
        } else {
            printf("Atendente: Nenhum cliente na fila. Aguardando...\n");
            sleep(1);
        }
    }
    fclose(lng);
    return NULL;
}

void* recepcao_thread(void* args) {
    Fila* fila = (Fila*)args;
    printf("\n \nA QUANTIDADE DE CLIENTES DA FILA É %d \n \n \n", fila->clientes);
    int n = fila->clientes;
    int x = fila->paciencia;

    sem_t *sem_atend, *sem_block;

    sem_atend= sem_open("/sem_atend", O_CREAT, 0644, 1); 
    sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);

    srand(time(NULL));


    for (int i = 0; i < n || n == 0; i++) {
        Cliente* novo_cliente = (Cliente*)malloc(sizeof(Cliente));
        novo_cliente->pid = i + 1;
        novo_cliente->hora_chegada = (int)time(NULL);
        novo_cliente->prioridade = rand() % 2;
        novo_cliente->paciencia = (novo_cliente->prioridade == 1) ? x / 2 : x;
        novo_cliente->prox = NULL;

        pthread_mutex_lock(&fila->lock);
        if (!fila->inicio) {
            fila->inicio = novo_cliente;
        } else {
            Cliente* atual = fila->inicio;
            while (atual->prox) {
                atual = atual->prox;
            }
            atual->prox = novo_cliente;
        }
        pthread_mutex_unlock(&fila->lock);

        printf("Recepção: Cliente %d criado com prioridade %d e paciência %d ms\n",
               novo_cliente->pid, novo_cliente->prioridade, novo_cliente->paciencia);

        usleep(100000);
    }
    return NULL;
}