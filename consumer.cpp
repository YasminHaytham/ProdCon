
#include  <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
#include <iostream>

const char* SHM_NAME = "/shm";
const char* SEM_MUTEX ="/mutex";
const char* SEM_EMPTY ="/sem_empty";
const char *SEM_FULL = "/sem_full";
const int name_len= 11;


struct commodity {
    char name[name_len];
    double price;
};

struct shared_memory {
    int in;
    int out;
    commodity buffer[];
};

int main(){
    printf ("Consumer process started\n");
}