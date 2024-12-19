#include "fila.h"
#include "globais.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

extern void* recepcao_thread(void* args);
extern void* atendente_thread(void* args);

int satis =0;
int insatis=0;
int executando =1;
double taxa_satis = -1.0;
int pula_menu =0;

sem_t semaforo;


int main(int argc, char* argv[]) {
    sem_init(&semaforo, 0, 0);

    clock_t inicio;
    inicio = clock();
    
	sem_unlink("sem_atend");
    sem_unlink("sem_block");
    sem_unlink("sem_demanda");

    sem_t *sem_analista;
    sem_analista = sem_open("/sem_analista", O_CREAT, 0644,0);


    if (argc != 3) {
        printf("Erro na entrada, inicialize de maneira válida: ./main numero_de_clientes");
        exit(1); // Retorna erro
    }

    int n_clientes = atoi(argv[1]);
    double paciencia = strtod(argv[2], NULL);

    printf("numero de clientes: %d\n \n", n_clientes);
    
    printf("Paciência dos clientes: %lf\n \n", paciencia);
    Fila fila;
    inicializar_fila(&fila, n_clientes, inicio, paciencia);

    pthread_t recepcao, atendente,thread_menu;

    pthread_create(&recepcao, NULL, recepcao_thread, &fila);
    usleep(1000);
    pthread_create(&atendente, NULL, atendente_thread, &fila);
    pthread_create(&thread_menu, NULL, menu, &fila);

    sem_wait(&semaforo);  
    sem_wait(&semaforo); 

    pthread_join(recepcao, NULL);
    pthread_join(atendente, NULL);
    
    if (pula_menu==0){
        pthread_join(thread_menu, NULL);
    }

    destruir_fila(&fila);
    sem_unlink("sem_atend");
    sem_unlink("sem_block");
    sem_unlink("sem_demanda");

    clock_t final = clock();
    double tempo = converter_clock_micros(inicio, final);

    sem_wait(sem_analista);
    printf("Tempo total da execução: %.4lfms\n", tempo);

    return 0;
}
