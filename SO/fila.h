#ifndef FILA_H
#define FILA_H

#include <semaphore.h>

#define TAMANHO_MAXIMO 100

// Estrutura do Cliente
typedef struct Cliente {
    int pid;                 
    double hora_chegada;        
    int prioridade;          
    int paciencia;           
    struct Cliente* prox;   
} Cliente;


// Estrutura da Fila
typedef struct Fila {
    Cliente* inicio;    
    long clock_inicio;                
    int capacidade;         
    int tamanho;             
    int paciencia;           
    sem_t sem_lock;          
} Fila;

// Protótipos das funções
extern void inicializar_fila(Fila* fila, int clientes, clock_t inicio);
extern void adicionar_cliente(Fila* fila, Cliente* cliente); 
extern Cliente* remover_cliente(Fila* fila); 
extern void destruir_fila(Fila* fila); 
extern void* menu(void* args); 
extern void criar_cliente(Cliente* cliente, clock_t inicio);
// extern void atribuir_prioridade(Cliente* cliente);
double converter_clock_micros(clock_t inicio, clock_t fim);


#endif
