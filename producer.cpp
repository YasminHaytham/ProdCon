
#include  <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
#include <random>
#include <ctime>
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


int main(int argc, char *argv[]){
    printf ("Producer process started\n");
    if (argc !=6){
        printf ("Usage: ./producer NAME MEAN STDDEV SLEEP_MS N\n");
        return 1;
    }
    char *name = argv[1];
    float mean = atof(argv[2]);
    float stddev = atof(argv[3]);
    int sleep_ms = atoi(argv[4]);
    int N = atoi(argv[5]);
    printf ("Producer parameters: name=%s, mean=%f, stddev=%f, sleep_ms=%d, N=%d\n", name, mean, stddev, sleep_ms, N);
}