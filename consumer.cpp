
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <semaphore.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

const char *SHM_NAME = "/shm";
const char *SEM_MUTEX = "/mutex";
const char *SEM_EMPTY = "/sem_empty";
const char *SEM_FULL = "/sem_full";
const int name_len = 11;
const int NUM_COMMODITIES = 11;
const char *commodity_names[NUM_COMMODITIES] = {
    "ALUMINIUM", "COPPER",     "COTTON", "CRUDEOIL", "GOLD", "LEAD",
    "MENTHAOIL", "NATURALGAS", "NICKEL", "SILVER",   "ZINC"};

struct Commodity {
  char name[name_len];
  double price;
};

struct Shared_memory {
  int in;
  int out;
  Commodity buffer[];
};

struct Record {
  double history[5] = {};
  int count = 0;
  int next = 0;
  double price = 0.0;
  double avg = 0.0;
  double prev_price = 0.0;
  double prev_avg = 0.0;
};

int get_index(const char *name) {
  int i = 0;
  for (i = 0; i < NUM_COMMODITIES; i++) {
    if (strcmp(name, commodity_names[i]) == 0) {
      return i;
    }
  }
  return -1;
}

int compare(double prev, double curr, int count) {
  if (count <= 1) {
    return 0;
  }
  if (curr > prev) {
    return 1;
  } else if (curr < prev) {
    return -1;
  } else {
    return 0;
  }
}

const char *get_color(int comp) {
  if (comp == 1) {
    return "\033[32m";
  } else if (comp == -1) {
    return "\033[31m";
  } else {
    return "\033[36m";
  }
}

const char *get_arrow(int comp) {
  if (comp == 1) {
    return "↑";
  } else if (comp == -1) {
    return "↓";
  } else {
    return " ";
  }
}

void draw_table(Record records[]) {
  printf("\033[1;1H");
  printf("+------------+-----------+-----------+\n");
  printf("| Commodity  | Price     | AvgPrice  |\n");
  printf("+------------+-----------+-----------+\n");
  for (int i = 0; i < NUM_COMMODITIES; i++) {
    int comp_p =
        compare(records[i].prev_price, records[i].price, records[i].count);
    int comp_a = compare(records[i].prev_avg, records[i].avg, records[i].count);
    const char *color_p = get_color(comp_p);
    const char *color_a = get_color(comp_a);
    const char *arrow_p = get_arrow(comp_p);
    const char *arrow_a = get_arrow(comp_a);
    printf("| %-10s | %s%7.2lf %s\033[0m | %s%7.2lf %s\033[0m |\n",
           commodity_names[i], color_p, records[i].price, arrow_p, color_a,
           records[i].avg, arrow_a);
  }
  printf("+------------+-----------+-----------+\n");
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: ./consumer N\n");
    return 1;
  }
  int N = atoi(argv[1]);
  size_t shm_size = sizeof(Shared_memory) + N * sizeof(Commodity);
  int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
  if (fd == -1) {
    perror("shm_open");
    return 1;
  }
  if (ftruncate(fd, shm_size) == -1) {
    perror("ftruncate");
    return 1;
  }
  void *p = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) {
    perror("mmap");
    return 1;
  }
  Shared_memory *shm_ptr = (Shared_memory *)p;
  sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
  sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, N);
  sem_t *full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
  if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
    perror("sem_open");
    return 1;
  }
  Record records[NUM_COMMODITIES];
  printf("\033[2J");
  draw_table(records);
  while (true) {
    sem_wait(full);
    sem_wait(mutex);
    Commodity c = shm_ptr->buffer[shm_ptr->out];
    shm_ptr->out = (shm_ptr->out + 1) % N;
    sem_post(mutex);
    sem_post(empty);
    int i = get_index(c.name);
    if (i == -1) {
      continue;
    }
    records[i].prev_price = records[i].price;
    records[i].prev_avg = records[i].avg;
    records[i].price = c.price;
    records[i].history[records[i].next] = c.price;
    records[i].next = (records[i].next + 1) % 5;
    if (records[i].count < 5) {
      records[i].count++;
    }
    double sum = 0.0;
    for (int j = 0; j < records[i].count; j++) {
      sum += records[i].history[j];
    }
    records[i].avg = sum / records[i].count;
    draw_table(records);
  }
}