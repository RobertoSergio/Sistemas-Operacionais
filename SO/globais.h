#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

extern int satis;                
extern int insatis;               
extern int executando;             
extern pid_t pid_a;  


extern sem_t sem_global;

#endif
