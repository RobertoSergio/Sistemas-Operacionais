#ifndef FILA_H
#define FILA_H

#include <semaphore.h>

#define TAMANHO_MAXIMO 100

// Estrutura do Cliente
typedef struct Cliente {
    int pid;                 
    double hora_chegada;        
    int prioridade;          
    double paciencia;           
    struct Cliente* prox;   
} Cliente;


// Estrutura da Fila
typedef struct Fila {
    Cliente* inicio;    
    long clock_inicio;           
    int capacidade;         
    int tamanho;             
    double paciencia;           
    sem_t sem_lock;
    sem_t sem_fim;
} Fila;

extern void inicializar_fila(Fila* fila, int clientes, clock_t inicio, double paciencia);
extern void adicionar_cliente(Fila* fila, Cliente* cliente); 
extern void remover_cliente(Fila* fila, Cliente* cliente); 
extern void destruir_fila(Fila* fila); 
void radix_sort(Fila* fila);
extern void* menu(void* args); 
extern void criar_cliente(Cliente* cliente, clock_t inicio, double paciencia);
double converter_clock_micros(clock_t inicio, clock_t fim);
int encontrar_max_prioridade(Cliente* cliente);


#endif
