#ifndef FILA_H
#define FILA_H

#include <pthread.h>

#define tamanho_maximo 100;

typedef struct Cliente {
    int pid;
    int hora_chegada;
    int prioridade;
    int paciencia;
    struct Cliente* prox;
} Cliente;

typedef struct Fila {
    Cliente* inicio;
    int clientes;
    int capacidade;
    int paciencia;
    pthread_mutex_t lock;
} Fila;



// Protótipos
extern void inicializar_fila(Fila* fila, int clientes, int paciencia);
extern void adicionar_cliente(Fila* fila, Cliente* cliente);
extern Cliente* remover_cliente(Fila* fila);

#endif
