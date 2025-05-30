#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

int main()
{
    int fd[2];
    pipe2(fd, __O_CLOEXEC);

    pid_t pid = fork();
    if (pid == 0) {
        dup2(fd[1], 2);
        char *args[] = {"cpp_messages", "1", NULL};
        execv("cpp_messages", args);
        exit(EXIT_FAILURE);
    } else {
        while (1) {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer));
            ssize_t n = read(fd[0], buffer, sizeof(buffer));
            if (n > 0)
                printf("received: %s\n", buffer);
        }
    }
}