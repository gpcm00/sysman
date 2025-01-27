#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include <pthread.h>

struct global_logpath {
    pthread_mutex_t mtx;
    char logpath[TOTAL_PATH_LEN];
};

struct thread_arg {
    char *prefix;
    int fd;
    struct global_logpath *log;
};

int init_global_logpath(struct global_logpath *lp);
void set_global_logpath(struct global_logpath *lp, char *logpath);
void* log_thread(void* arg);

#endif
