#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

void* atendente_thread(void* args) {

/**
 *  pid_analista = fork();

    if (pid_analista == -1) {
        perror("Erro ao criar processo analista");
        exit(1);
    }

    if (pid_analista == 0) {
        // Processo filho: executa o cliente
        execl("./analista", "./analista", NULL);
        perror("Erro ao executar analista");
        exit(1);
    }
 * 
 * 
 */

    int satis =0;
    int insatis=0;

    Fila* fila = (Fila*)args;
    FILE* lng = fopen("LNG.txt", "w");

    if (!lng) {
        perror("Erro ao abrir arquivo LNG");
        return NULL;
    }

    while (fila->tamanho !=0) {
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
            int satisfacao;
            if (cliente->prioridade == 0){
                 satisfacao = (tempo_espera <= cliente->paciencia*1000);
            }
            
            if (cliente->prioridade==1) {
                satisfacao = (tempo_espera <= cliente->paciencia*500);
            }

            if(satisfacao == 1){
                satis++;
            } else{
                insatis++;
            }

            printf("Cliente %d %s\n", cliente->pid, satisfacao ? "Satisfeito" : "Insatisfeito");

            // remover_cliente(fila, cliente);
            
            fila->tamanho--;
            free(cliente);
        } else {
            printf("Atendente: Nenhum cliente na fila. Aguardando...\n");
            sleep(1);
        }
    }
    fclose(lng);

    int total = satis+insatis;
    double taxa_satis = (satis/total)*100;
    double taxa_insatis = (insatis/total)*100;
    printf("Taxa de satisfação: %.2lf", taxa_satis);

    return NULL;
}

void* recepcao_thread(void* args) {

    sem_t *sem_atend, *sem_block, * sem_demanda;

    sem_atend= sem_open("/sem_atend", O_CREAT, 0644, 1); 
    sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);
    sem_demanda = sem_open("/sem_demanda", O_CREAT, 0644,1);

    Fila* fila = (Fila*)args;

    if (fila->tamanho==0) {
        printf("\n \nA QUANTIDADE DE CLIENTES DA FILA É INFINITA \n \n \n");
    }

    printf("\n \nA QUANTIDADE DE CLIENTES DA FILA É %d \n \n \n", fila->tamanho);

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
            usleep(80000);

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

void atender(){
    pid_t pid_analista = fork();

    if (pid_analista == -1) {
        perror("Erro ao criar processo cliente");
        exit(1);
    }

    if (pid_analista == 0) {
        // Processo filho: executa o cliente
        execl("./analista", "./analista", NULL);
        perror("Erro ao executar cliente");
        exit(1);
    }



}