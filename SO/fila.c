#include <stdio.h>
#include <stdlib.h>
#include "fila.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include<sys/wait.h>
#include <unistd.h>

int contador =0;

void inicializar_fila(Fila* fila, int clientes, clock_t inicio) {
    fila->inicio = NULL;
    fila->clock_inicio = inicio;
    // testar tamanho maximo e quantidade de clientes
    
    fila->capacidade = TAMANHO_MAXIMO;
    fila->tamanho = clientes;
    sem_init(&fila->sem_lock, 0, 1);  

}

void destruir_fila(Fila* fila) {
    Cliente* atual = fila->inicio;
    while (atual) {
        Cliente* proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
    sem_destroy(&fila->sem_lock);
}

int encontrar_max_prioridade(Cliente* cliente) {
    int max = cliente->prioridade;
    while (cliente) {
        if (cliente->prioridade > max)
            max = cliente->prioridade;
        cliente = cliente->prox;
    }
    return max;
}

void radix_sort(Fila* fila) {
    int exp, max;
    Cliente *bucket[10], *atual;
    Cliente *temp[10] = { NULL };

    sem_wait(&fila->sem_lock);
    
    max = encontrar_max_prioridade(fila->inicio);
    
    for (exp = 1; max / exp > 0; exp *= 10) {
        for (int i = 0; i < 10; i++)
            bucket[i] = NULL;

        atual = fila->inicio;
        while (atual) {
            int index = (atual->prioridade / exp) % 10;
            if (!bucket[index]) {
                bucket[index] = atual;
                temp[index] = atual;
            } else {
                temp[index]->prox = atual;
                temp[index] = atual;
            }
            atual = atual->prox;
        }

        fila->inicio = NULL;
        Cliente* ultimo = NULL;
        for (int i = 9; i >= 0; i--) {
            if (bucket[i]) {
                if (!fila->inicio) {
                    fila->inicio = bucket[i];
                } else {
                    ultimo->prox = bucket[i];
                }
                ultimo = temp[i];
            }
        }
        if (ultimo)
            ultimo->prox = NULL;
    }

    sem_post(&fila->sem_lock);
}


void adicionar_cliente(Fila* fila, Cliente* novo_cliente) {
    sem_wait(&fila->sem_lock); // Bloqueia o semáforo

    if (fila->tamanho == fila->capacidade) {
        printf("Fila cheia! Não é possível adicionar mais clientes.\n");
        sem_post(&fila->sem_lock);
        free(novo_cliente); // Libera memória do cliente descartado
        return;
    }

    // Inserção no final da fila (FIFO)
    novo_cliente->prox = NULL;
    if (!fila->inicio) {
        fila->inicio = novo_cliente;
    } else {
        Cliente* atual = fila->inicio;
        while (atual->prox) {
            atual = atual->prox;
        }
        atual->prox = novo_cliente;
    }


    sem_post(&fila->sem_lock); // Libera o semáforo
    
    if (fila->tamanho % 20 == 0) {
        radix_sort(fila);
    }

}



Cliente* remover_cliente(Fila* fila) {
    sem_wait(&fila->sem_lock); // Bloqueia o semáforo
    Cliente* cliente_removido = NULL; 

    if (fila->inicio) {
        cliente_removido = fila->inicio;
        fila->inicio = fila->inicio->prox;
        fila->tamanho--;
    }else {
        printf("Fila vazia! Nenhum cliente para remover.\n");
    }

    sem_post(&fila->sem_lock); // Libera o semáforo
    return cliente_removido;
}



void* menu (void* args){

// se nao digitar nada ou algo diferente de q fica aqui, se digitar q sai do while
	while ((getchar()) != 's'); 

    
    // Esperando ajustes de semáforos

	//sem_close(sem); // pra liberar o arquivo, porque o writer pode querer escrever = coloca sem em verde
	// sem_unlink("/sem_printer");


    // ANTES DE SAIR

    // o arquivo deve ser esvaziado antes, isto é, o “Analista” deve ser acordado 
    // tantas vezes quantas forem necessárias para imprimir todos os PID de clientes 
    // que já haviam sido atendidos.

    /*
        abrir lista de LNG e imprimir todos os clientes de lá

    
    */


	exit(0);
	return NULL;

}

void criar_cliente(Cliente *cliente, clock_t inicio) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Erro ao criar processo cliente");
        exit(1);
    }

    if (pid == 0) {
        // Processo filho: executa o cliente
        execl("./cliente", "./cliente", NULL);
        perror("Erro ao executar cliente");
        exit(1);
    }

    // Processo pai: registra informações do cliente
    clock_t fim = clock();
    cliente->pid = pid;
    cliente->hora_chegada = converter_clock_micros(inicio, fim);
    cliente->prox = NULL;
    cliente->prioridade = rand()%2;

    // // Espera o cliente finalizar
    // waitpid(pid, NULL, 0);

    // Lê o tempo gerado pelo cliente
    FILE *demanda = fopen("./demanda.txt", "r");
    if (demanda) {
        fscanf(demanda, "%d", &cliente->paciencia);
        printf("paciencia pegada do demanda.txt: %d \n", cliente->paciencia);
        fclose(demanda);
    } else {
        perror("Erro ao abrir demanda.txt");
        cliente->paciencia = -1; // Marca erro
    }
    // atribuir_prioridade(cliente);

    // printf("pid: %d \n", cliente->pid);
    // printf("hora da chegada: %d \n", cliente->hora_chegada);
    // printf("paciencia: %d \n", cliente->paciencia);
    // printf("%d\n", contador++);

}

// void atribuir_prioridade(Cliente* cliente){
//         if (cliente->paciencia == 1) {
//             cliente->prioridade = 3;
//         }

//         if (cliente->paciencia == 5){
//             cliente->prioridade = 2;
//         }
//         cliente->prioridade = 1;
// }

double converter_clock_micros(clock_t inicio, clock_t fim){
    double tempo_decorrido;
    tempo_decorrido = (double)(fim - inicio) / CLOCKS_PER_SEC; // Tempo em segundos
    tempo_decorrido *= 1000000; // Converte para microssegundos
    return tempo_decorrido;
}




// long get_current_time_micros() {
//     struct timeval tv;
//     gettimeofday(&tv, NULL);
//     return (tv.tv_sec * 1000000LL) + tv.tv_usec; // Retorna o tempo em microssegundos
// }


// void iniciar_programa(long start_time) {
//     start_time = get_current_time_micros();  // Salva o tempo inicial
// }

// long tempo_decorrido(long start_time) {
//     long current_time = get_current_time_micros();
//     return current_time - start_time;  // Tempo decorrido em microssegundos
// }