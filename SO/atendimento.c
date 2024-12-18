#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

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

    sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);
    sem_t *sem_bloque = sem_open("/sem_block", O_RDWR);

    if (sem_atend == SEM_FAILED ){
        perror("Erro ao abrir sem atend");
        exit(1);
    }

    if (sem_bloque == SEM_FAILED) {
        perror("Erro ao abrir semáforo block");
        exit(1);
    }

    int satis =0;
    int insatis=0;

    Fila* fila = (Fila*)args;
    FILE* lng = fopen("LNG.txt", "w");

    if (!lng) {
        perror("Erro ao abrir arquivo LNG");
        return NULL;
    }
    int contador = 0;
    while (1) {        
        sem_wait(&fila->sem_lock);
        Cliente* cliente = fila->inicio;

        if (cliente) {
            fila->inicio = cliente->prox;
        }

        sem_post(&fila->sem_fim); // libera 1 espaço na fila
        sem_post(&fila->sem_lock);
        sem_post(sem_atend); // acorda pra atender

        if (cliente) {
            contador = 0;
            printf("Atendente: Atendendo cliente %d...\n", cliente->pid);
            clock_t fim = clock();

            double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);

            printf("TEMPO DECORRIDO: %lf \n", tempo_decorrido);

            sem_wait(sem_bloque);
            fprintf(lng, "PID: %d\n", cliente->pid);
            fflush(lng);
            sem_post(sem_bloque);

            double tempo_espera = tempo_decorrido - cliente->hora_chegada;

            printf("TEMPO ESPERA: %lf \n", tempo_espera);
            printf("PACIENCIA: %d\n", cliente->paciencia* 1000);
            int satisfacao;
            satisfacao = (tempo_espera <= cliente->paciencia * 1000);

            if(satisfacao == 1){
                satis++;
            } else{
                insatis++;
            }

            printf("Cliente %d %s\n", cliente->pid, satisfacao ? "Satisfeito" : "Insatisfeito");
            kill(cliente->pid, SIGCONT);
            // sem_wait(sem_atende);
            free(cliente);
            // sem_post(sem_atende);
        } else {
            printf("Atendente: Nenhum cliente na fila. Aguardando...\n");
            sleep(1);
            contador ++;
            if(contador == 3){
            break;
            }
        }
    }
    fclose(lng);

    double total = satis+insatis;
    printf("Total de clientes atendidos: %.2lf\n", total);
    double taxa_satis = (satis/total)*100;
    printf("Total de clientes satisfeitos: %.2d\n", satis);
    double taxa_insatis = (insatis/total)*100;
    printf("Taxa de satisfação: %.2lf\n", taxa_satis);
    clock_t fim = clock();
    double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);
    printf("Tempo Total do programa: %lf \n", tempo_decorrido);
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
            double paciencia = fila->paciencia;
            criar_cliente(novo_cliente, fila->clock_inicio, paciencia);
            novo_cliente->prox = NULL;
            // usleep(80000);

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
        double paciencia = fila->paciencia;
        // Criar cliente
        criar_cliente(novo_cliente, fila->clock_inicio, paciencia);
        printf("%d\n\n", fila->tamanho);
        // adiciona na fila
        //testar capacidade da fila antes de adicionar
        sem_wait(&fila->sem_fim);

        adicionar_cliente(fila,novo_cliente);
        
        sem_wait(sem_atend);



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