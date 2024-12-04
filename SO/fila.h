#ifndef FILA_H
#define FILA_H

#include <pthread.h>

typedef struct Cliente {
    int pid;
    int hora_chegada;
    int prioridade;
    int paciencia;
    struct Cliente* prox;
} Cliente;

typedef struct Fila {
    Cliente* inicio;
    pthread_mutex_t lock;
} Fila;

// Protótipos
extern void inicializar_fila(Fila* fila);
extern void adicionar_cliente(Fila* fila, Cliente* cliente);
extern Cliente* remover_cliente(Fila* fila);

#endif
