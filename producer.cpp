
#include <cstdarg>
#include <cstring>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ctime>
#include <random>
#include <time.h>

const char *SHM_NAME = "/shm";
const char *SEM_MUTEX = "/mutex";
const char *SEM_EMPTY = "/sem_empty";
const char *SEM_FULL = "/sem_full";
const int name_len = 11;

struct Commodity {
  char name[name_len];
  double price;
};

struct Shared_memory {
  int in;
  int out;
  Commodity buffer[];
};

void log_msg(const char *msg, ...) {
  struct timespec ts;
  struct tm tm_info;
  char time_str[32];
  clock_gettime(CLOCK_REALTIME, &ts);
  localtime_r(&ts.tv_sec, &tm_info);
  strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S", &tm_info);
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "[%s.%03ld] ", time_str, ts.tv_nsec / 1000000);
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
}

int main(int argc, char *argv[]) {
  printf("Producer process started\n");
  if (argc != 6) {
    printf("Usage: ./producer NAME MEAN STDDEV SLEEP_MS N\n");
    return 1;
  }
  char name[name_len];
  strncpy(name, argv[1], name_len - 1);
  name[name_len - 1] = '\0';
  for (int i = 0; name[i] != '\0'; i++) {
    name[i] = toupper((unsigned char)name[i]);
  }
  float mean = atof(argv[2]);
  float stddev = atof(argv[3]);
  int sleep_ms = atoi(argv[4]);
  int N = atoi(argv[5]);
  printf(
      "Producer parameters: name=%s, mean=%f, stddev=%f, sleep_ms=%d, N=%d\n",
      name, mean, stddev, sleep_ms, N);

  std::random_device rd;
  std::mt19937 rng(std::random_device{}());
  std::normal_distribution<double> dist(mean, stddev);
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
  log_msg("%s: IPC ready", name);
  while (true) {
    double value = dist(rng);
    log_msg("%s: generating anew value %.2f", name, value);
    log_msg("%s: trying to get mutex on shared buffer", name);
    sem_wait(empty);
    sem_wait(mutex);
    strncpy(shm_ptr->buffer[shm_ptr->in].name, name, name_len - 1);
    shm_ptr->buffer[shm_ptr->in].name[name_len - 1] = '\0';
    shm_ptr->buffer[shm_ptr->in].price = value;
    shm_ptr->in = (shm_ptr-> in + 1) % N;
    log_msg("%s: placing %.2f on shared buffer" , name , value);
    sem_post(mutex);
    sem_post(full);
    log_msg("%s: sleeping for %d ms", name, sleep_ms);
    usleep(sleep_ms * 1000);
  }
}