#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

void* atendente_thread(void* args) {
    Fila* fila = (Fila*)args;
    FILE* lng = fopen("LNG.txt", "w");

    if (!lng) {
        perror("Erro ao abrir arquivo LNG");
        return NULL;
    }

    while (1) {
        sem_wait(&fila->sem_lock);
        Cliente* cliente = fila->inicio;

        if (cliente) {
            fila->inicio = cliente->prox;
        }
        sem_post(&fila->sem_lock);

        if (cliente) {
            printf("Atendente: Atendendo cliente %d...\n", cliente->pid);
            clock_t fim = clock();

            double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);

            printf("TEMPO DECORRIDO: %lf \n", tempo_decorrido);


            fprintf(lng, "PID: %d\n", cliente->pid);
            fflush(lng);


            double tempo_espera = tempo_decorrido - cliente->hora_chegada;

            printf("TEMPO ESPERA: %lf \n", tempo_espera);
            printf("PACIENCIA: %d\n", cliente->paciencia*1000);
            int satisfeito = (tempo_espera <= cliente->paciencia*1000);
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

    sem_t *sem_atend, *sem_block;

    sem_atend= sem_open("/sem_atend", O_CREAT, 0644, 1); 
    sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);

    Fila* fila = (Fila*)args;

    if (fila->tamanho==0) {
        printf("\n \nA QUANTIDADE DE CLIENTES DA FILA É INFINITA \n \n \n");
    }

    printf("\n \nA QUANTIDADE DE CLIENTES DA FILA É %d \n \n \n", fila->tamanho);

    srand(time(NULL));

    //se n = 0 ==> fila->clientes == 0;
    // criar infinitos clientes

    if (fila->tamanho == 0){

        while (1){
            Cliente* novo_cliente = (Cliente*)malloc(sizeof(Cliente));
            if (!novo_cliente) {
                perror("Erro ao alocar memória para novo cliente");
                exit(1);
            }      
            // Criar cliente
            criar_cliente(novo_cliente, fila->clock_inicio);
            novo_cliente->prox = NULL;

            // saleiro para atendimento e adicionar cliente na fila

            if (fila->tamanho <= TAMANHO_MAXIMO){
                adicionar_cliente(fila,novo_cliente);
            }

        }
    }


    for (int i = 0; i < fila->tamanho; i++) {
        Cliente* novo_cliente = (Cliente*)malloc(sizeof(Cliente));
        if (!novo_cliente) {
            perror("Erro ao alocar memória para novo cliente");
            exit(1);
        }    
        
        // Criar cliente
        criar_cliente(novo_cliente, fila->clock_inicio);
        printf("%d\n\n", fila->tamanho);
        // adiciona na fila
        //testar capacidade da fila antes de adicionar
        if (fila->tamanho < TAMANHO_MAXIMO){
            // adicionar cliente na fila
            adicionar_cliente(fila,novo_cliente);
        }

        //TODO fazer um buffer pra nao perder os clientes fora da fila?
        // fila tamanho == 100 => parar criação raise(SIGSTOP) 
        continue;

        // if (!fila->inicio) {
        //     fila->inicio = novo_cliente;
        // } else {
        //     Cliente* atual = fila->inicio;
        //     while (atual->prox) {
        //         atual = atual->prox;
        //     }
        //     atual->prox = novo_cliente;
        // }

        // printf("Recepção: Cliente %d criado com prioridade %d e paciência %d ms\n",
        //        novo_cliente->pid, novo_cliente->prioridade, novo_cliente->paciencia);

    }


    return NULL;
}

