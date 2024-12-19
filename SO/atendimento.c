#include "fila.h"
#include "globais.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

extern sem_t semaforo;

void* atendente_thread(void* args) {

    pid_t pid_analista = fork();

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


    Fila* fila = (Fila*)args;
    FILE* lng = fopen("LNG.txt", "w");

    if (!lng) {
        perror("Erro ao abrir arquivo LNG");
        return NULL;
    }
    int contador = 0;
    
    while (executando) {        
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
            printf("\nAtendente: Atendendo cliente %d...\n", cliente->pid);
            clock_t fim = clock();

            double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);

            printf("TEMPO DECORRIDO: %lf \n", tempo_decorrido);

            sem_wait(sem_bloque);
            fprintf(lng, "PID: %d\n", cliente->pid);
            fflush(lng);
            sem_post(sem_bloque);

            double tempo_espera = tempo_decorrido - cliente->hora_chegada;

            printf("TEMPO ESPERA: %lf \n", tempo_espera);
            printf("PACIENCIA: %.2lf\n", cliente->paciencia);
            int satisfacao;
            satisfacao = (tempo_espera <= cliente->paciencia);

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
            usleep(500000);
            contador ++;
            if(contador == 3){
                break;
            }
        }
    }
    fclose(lng);

    
    // acordar analista
    // kill(pid_analista, SIGCONT);
    
    // //TODO em vez de 1 = checagem do tamanho do arquivo lng.txt se maior que zero, tenta abrir
    // while(1){
    //     sem_wait(sem_bloque);
    //     const char *caminho = "LNG.txt";
    //     struct stat status_arquivo;

    //     // Obtém informações sobre o arquivo
    //     if (stat(caminho, &status_arquivo) == 0) {
    //         if (status_arquivo.st_size > 0) {
    //             sem_post(sem_bloque);
    //             kill(pid_analista, SIGCONT);
    //         } else {
    //             break;
    //         }
    //     } else {
    //         perror("Erro ao acessar o arquivo no atendimento");
    //     }
    // }

    
    double total = satis+insatis;
    double taxa_satisfacao = (satis/total)*100;
    if (taxa_satis != taxa_satisfacao){
        printf("Total de clientes atendidos na thread: %.2lf\n", total);
    
        printf("Total de clientes satisfeitos na thread: %.2d\n", satis);
        double taxa_insatis = (insatis/total)*100;
        printf("Taxa de satisfação na thread: %.2lf por cento\n", taxa_satisfacao);
        clock_t fim = clock();
        double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);
        printf("Tempo Total do programa na thread: %.4lfms \n", tempo_decorrido);
    }
    
    
    while (1){
        const char *caminho = "LNG.txt";
        struct stat status_arquivo;

        // Obtém informações sobre o arquivo
        if (stat(caminho, &status_arquivo) == 0) {
            if (status_arquivo.st_size > 0) {
                kill(pid_analista, SIGCONT);
            } else {
                break;
            }
        } else {
            perror("Erro ao acessar o arquivo");
        }
    }
    
    sem_close(sem_atend);
    sem_close(sem_bloque);

    pula_menu = 1;
    sem_post(&semaforo);
    
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


    if (fila->tamanho == 0){

        while (executando){
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
        // adiciona na fila
        //testar capacidade da fila antes de adicionar
        sem_wait(&fila->sem_fim);

        adicionar_cliente(fila,novo_cliente);
        
        sem_wait(sem_atend);

        if(!executando){
            break;
        }

    }


    sem_close(sem_atend);
    sem_close(sem_block);
    sem_post(&semaforo);
    return NULL;
}
