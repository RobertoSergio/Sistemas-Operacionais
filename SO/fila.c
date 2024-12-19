#include <stdio.h>
#include <stdlib.h>
#include "fila.h"
#include "globais.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include<sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>


int contador =0;
void inicializar_fila(Fila* fila, int clientes, clock_t inicio, double paciencia) {
    fila->inicio = NULL;
    // testar tamanho maximo e quantidade de clientes

    fila->capacidade = TAMANHO_MAXIMO;
    fila->tamanho = clientes;
    sem_init(&fila->sem_lock, 0, 1);
    sem_init(&fila->sem_fim, 0, TAMANHO_MAXIMO); 
    fila->clock_inicio = inicio;
    fila->paciencia = paciencia;
}

void destruir_fila(Fila* fila) {
    Cliente* atual = fila->inicio;
    while (atual) {
        Cliente* proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
    sem_destroy(&fila->sem_lock);
    sem_destroy(&fila->sem_fim);
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


void remover_cliente(Fila* fila, Cliente *cliente) {
    sem_wait(&fila->sem_lock); // Bloqueia o semáforo
    Cliente* cliente_removido = NULL; 

    if (fila->inicio) {
        cliente_removido = fila->inicio;
        fila->inicio = fila->inicio->prox;
        fila->tamanho--;
        free(cliente_removido);
    }else {
        printf("Fila vazia! Nenhum cliente para remover.\n");
    }
    sem_post(&fila->sem_lock); // Libera o semáforo
}



void* menu (void* args){
    Fila* fila = (Fila*)args;
// se nao digitar nada ou algo diferente de q fica aqui, se digitar q sai do while
	while ((getchar()) != 's'); 

    // ANTES DE SAIR

    //1- parar recepcao e atendimento
    executando = 0;
    printf("Parando atendimento e recepção...\n");
    

    // 2- calcular taxa de satisfação
    double total = satis+insatis;
    printf("Total de clientes atendidos após interrupcao: %.0lf\n", total);
    taxa_satis = (satis/total)*100;
    printf("Total de clientes satisfeitos após interrupção: %d\n", satis);
    double taxa_insatis = (insatis/total)*100;
    printf("Taxa de satisfação apos interrupcao: %.2lf por cento\n", taxa_satis);
    clock_t fim = clock();
    double tempo_decorrido = converter_clock_micros(fila->clock_inicio, fim);
    printf("Tempo Total do programa apos interrupcao: %.4lfms \n", tempo_decorrido);


    //3 - chamar analista pra printar
    pid_t pid_analista;
    FILE *pida = fopen("./pidanalista.txt", "r");
    fscanf(pida, "%d", &pid_analista);
    fclose(pida);


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
    sleep(1);
   
    destruir_fila(fila);
    
	return NULL;

}



void criar_cliente(Cliente *cliente, clock_t inicio, double paciencia) {
    pid_t pid_cliente;

    pid_cliente = fork();

    if (pid_cliente == -1) {
        perror("Erro ao criar processo cliente");
        exit(1);
    }

    if (pid_cliente == 0) {
        // Processo filho: executa o cliente
        execl("./cliente", "./cliente", NULL);
        perror("Erro ao executar cliente");
        exit(1);
    }
    
    clock_t fim = clock();
    cliente->pid = pid_cliente;
    cliente->hora_chegada = converter_clock_micros(inicio, fim);
    cliente->prox = NULL;
    cliente->prioridade = rand()%2;
    if(cliente->prioridade == 0 ){
        cliente->paciencia = paciencia;
    }else{
        cliente->paciencia = paciencia/2;
    }

    sem_t *sem_demanda = sem_open("/sem_demanda", O_RDWR);
    if (sem_demanda == SEM_FAILED) {
        perror("Erro ao abrir semáforo de demanda");
        exit(1);
    }

    sem_wait(sem_demanda);
    usleep(300000);

    // Lê o tempo gerado pelo cliente

    sem_post(sem_demanda); // Libera o acesso ao arquivo

}

// estava em microssegundos, atualizamos para milissegundos, mas nao alteramos no restante ainda
double converter_clock_micros(clock_t inicio, clock_t fim){
    double tempo_decorrido;
    tempo_decorrido = (double)(fim - inicio) / CLOCKS_PER_SEC; // Tempo em segundos
    tempo_decorrido *= 1000; // Converte para milissegundos
    return tempo_decorrido;
}
