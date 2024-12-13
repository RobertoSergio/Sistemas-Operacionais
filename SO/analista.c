#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>


sem_t *sem_block;

int main()
{
    pid_t pid = getpid();
	FILE* pidfile = fopen("pidanalista.txt", "w");
	fprintf(pidfile, "%d", pid);
	fclose(pidfile); 

    //sleep
    raise(SIGSTOP);
    

    // Quando acordado:
        // - Bloqueia LNG
        // - Lê LNG e imprime seus 10 primeiros valores
        // - Apaga os 10 valores que imprimiu
        // - Desbloqueia LNG
        // - Dorme novamente


    sem_block = sem_open("/sem_block", O_RDWR);
    
    if(sem_block != SEM_FAILED) sem_wait(sem_block);
    
    //
    
    if(sem_block != SEM_FAILED) sem_post(sem_block);

    return 0; 

}