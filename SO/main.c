#include "fila.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

extern void* recepcao_thread(void* args);
extern void* atendente_thread(void* args);

int main(int argc, char* argv[]) {

    clock_t inicio;
    inicio = clock();
    
	sem_unlink("sem_atend");
	sem_unlink("sem_cliente");
    sem_unlink("sem_demanda");




    if (argc != 2) {
        printf("Erro na entrada, inicialize de maneira válida: ./main numero_de_clientes");
        exit(0); // Retorna erro
    }

    int n_clientes = atoi(argv[1]);

    printf("numero de clientes: %d\n \n", n_clientes);
    
    
    Fila fila;
    inicializar_fila(&fila, n_clientes, inicio);

    pthread_t recepcao, atendente,thread_menu;

    pthread_create(&recepcao, NULL, recepcao_thread, &fila);
    pthread_create(&atendente, NULL, atendente_thread, &fila);
    pthread_create(&thread_menu, NULL, menu, NULL);

    pthread_join(recepcao, NULL);
    pthread_join(atendente, NULL);
    pthread_join(thread_menu, NULL);


    destruir_fila(&fila);
    sem_unlink("sem_atend");
    sem_unlink("sem_block");
    sem_unlink("sem_demanda");

    clock_t final = clock();
    double tempo = converter_clock_micros(inicio, final);
    printf("tempo total da execução: %lf", tempo);


    return 0;
}
