#include "parser.h"
#include "common.h"

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef LOG_MANAGER_PATH
#define LOG_MANAGER_PATH 
#endif

#define DEFAULT_INITIAL_PARAM                          \
    .alive = false,                                    \
    .pid = -1,                                         \
    .fd = {                                            \
        {0,0}, {0,0}                                   \
    }    

#define MAX_COMMANDS    (4)

struct file_buffer {
  char* buffer;
  ssize_t size;
};

struct process get_log_manager() 
{
  struct process ret = {
    .path = LOG_MANAGER_PATH"log_manager",
    .args = {
        "log_manager", "1" , NULL, NULL, NULL, NULL,
    },
    DEFAULT_INITIAL_PARAM,
  };
  return ret;
}

static struct file_buffer alloc_buffer(char* file) 
{
  struct file_buffer ret = {NULL, -1};
  struct stat st;
  if (stat(file, &st) == -1) {
    return ret;
  }
  
  int fd = open(file, O_RDONLY);
  if (fd == -1) {
    return ret;  
  }

  ret.buffer = (char*)malloc(st.st_size + 1);
  if (ret.buffer == NULL) {
    goto Exit;
  }
  
  ret.size = st.st_size + 1;
  memset(ret.buffer, 0, ret.size);

  size_t nread = 0;
  do {
    ssize_t n = read(fd, ret.buffer + nread, st.st_size - nread);

    if (n == -1) {
      free(ret.buffer);
      ret.buffer = NULL;
      ret.size = -1;
      goto Exit;
    }

    nread += n;
  } while (nread < st.st_size);

Exit:
  close(fd);
  return ret;
}

static void free_buffer(struct file_buffer* fb) {
  free(fb->buffer);
  fb->buffer = NULL;
  fb->size = -1;
}

static int parse_lines(struct file_buffer fb, char **line) 
{
  int lines = 0;

  line[lines++] = fb.buffer;
  for (int i = 0; i < fb.size && fb.buffer[i] != '\0'; i++) {
    
    if (fb.buffer[i] == '\n' || fb.buffer[i] == '\r')
    {  
      while (fb.buffer[i] == '\n' || fb.buffer[i] == '\r' || fb.buffer[i] == ' ') {
        fb.buffer[i++] = '\0';
      }

      if (lines < MAX_COMMANDS && fb.buffer[i] != '\0') {
        line[lines++] = fb.buffer + i;
      }
    }
  }

  return lines;
}

static char* get_process_name(char* path)
{
  char* prev = path;

  do {
    while (*path != ' ' && *path != '/' && *path != '\0') {
      path++;
    }

    if (*path == '/') {
      prev = ++path;
    }

  } while (*path != '\0' && *path != ' ');
  return prev;
}

static char* get_arg(char* command)
{
  if (command == NULL) {
    return NULL;
  }

  do {
    command++;
  } while (*command != ' ' && *command != '\t' && *command != '\0');

  while (*command == ' ')
  {
    *command = '\0';
    command++;
  }
  
  if (*command == '\0') {
    return NULL;
  }

  return command;
}

static struct process parse_command(char* command) {
  struct process ret;

  ret.path = command;
  ret.args[0] = get_process_name(ret.path);

  for (size_t i = 1; i < MAX_NUM_OF_ARGS; i++) {
    ret.args[i] = get_arg(ret.args[i-1]);
  }

  ret.alive = false;
  ret.fd.info[0] = 0;
  ret.fd.info[1] = 0;
  ret.fd.error[0] = 0;
  ret.fd.error[1] = 0;
  ret.pid = -1;

  return ret;
}

static struct file_buffer fb = {};
int parse_processes(char* file, struct process** all_proc)
{
  fb = alloc_buffer(file);
  if (fb.buffer == NULL) {
    return -1;
  }

  char* lines[MAX_COMMANDS];
  memset(lines, 0, sizeof(lines));

  int l = parse_lines(fb, lines);

  // plus log_manager
  struct process* all_processes = 
            (struct process*)malloc((l+1) * sizeof(struct process));
  
  if (all_processes == NULL) {
    free_buffer(&fb);
    return -1;
  }

  all_processes[0] = get_log_manager();

  for (size_t i = 0; i < l; i++) {
    all_processes[i+1] = parse_command(lines[i]);
    all_processes[0].args[2+i] = all_processes[i+1].args[0];  // add proc name to log_manager
  }

  *all_proc = all_processes;
  return l+1;  
}

void destroy_process_list(struct process* proc)
{
  free(proc);
  free_buffer(&fb);
}

#if 0
int main(int argc, char** argv) {
  struct process* plist = NULL;
  ssize_t size = parse_processes(argv[1], &plist);
  if (size == -1) {
    perror("parse_processes");
    return 1;
  }

  for (size_t i = 0; i < size; i++) {
    printf("proc[%ld]:\n", i);
    printf("\tpath: %s\n", plist[i].path);
    for (int a = 0; a < MAX_NUM_OF_ARGS; a++) {
      if (plist[i].args[a] == NULL) 
        break;
      printf("\targ[%d] = %s\n", a, plist[i].args[a]);
    }
  }

  destroy_process_list(plist);
  return 0;
}
#endif