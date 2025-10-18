#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <stdbool.h>

#define MAX_NUM_OF_ARGS 6

struct file_descriptor {
  int info[2];
  int error[2];
};

struct process {
  char *path;
  char *args[MAX_NUM_OF_ARGS];
  bool alive;
  pid_t pid;
  struct file_descriptor fd;
};

#endif