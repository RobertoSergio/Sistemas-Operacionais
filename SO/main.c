#include "fila.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

extern void* recepcao_thread(void* args);
extern void* atendimento_thread(void* args);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Erro na entrada, inicialize de maneira válida: ./main numero_de_clientes");
        exit(0); // Retorna erro
    }

    int n_clientes = atoi(argv[1]);

    printf("numero de clientes: %d\n \n", n_clientes);
    
    
    Fila fila;
    inicializar_fila(&fila, n_clientes);

    pthread_t recepcao, atendimento;

    pthread_create(&recepcao, NULL, recepcao_thread, &fila);
    pthread_create(&atendimento, NULL, atendimento_thread, &fila);

    pthread_join(recepcao, NULL);
    pthread_join(atendimento, NULL);

    return 0;
}
