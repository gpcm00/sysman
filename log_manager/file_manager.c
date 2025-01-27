#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "log_manager.h"
#include "file_manager.h"

static void swap_filename(char *str1, char *str2) {
    for (int i = 0; i < FILENAME_LEN; i++) {
        char tmp = str1[i];
        str1[i] = str2[i];
        str2[i] = tmp;
    }
}

static void make_logpath(char *fullpath, char *file)
{
    strcpy(fullpath, LOG_PATH);
    strcpy(fullpath+LOG_PATH_LEN-1, file);    // minus null byte
}

static int delete_file(char *path)
{
    if (access(path, F_OK) == -1) {
        // file doesn't exist
        // file may never be created if there is no logging
        if (errno == ENOENT) {
            errno = 0;
            return 0;
        }
        return -1;
    }

    if (unlink(path) == -1) {
        return -1;
    }

    return 0;
}
static size_t generate_filename(char *file)
{
	time_t t = time(NULL);
	struct tm* ftime = localtime(&t);
	if(ftime == NULL) {
		return 0;
	}
	return strftime(file, FILENAME_LEN, "%Y_%b_%d.log", ftime);
}

int alloc_file_queue(struct file_queue *queue, size_t nfiles)
{
    queue->filename = (char*)malloc(FILENAME_LEN * nfiles);
    if (queue->filename == NULL) {
        return -1;
    }

    memset(queue->filename, 0, FILENAME_LEN * nfiles);
    queue->head = 0;
    queue->size = nfiles;

    return 0;
}

void free_file_queue(struct file_queue *queue)
{
    memset(queue->filename, 0, FILENAME_LEN * queue->size);
    free(queue->filename);

    queue->filename = NULL;
    queue->head = 0;
    queue->size = 0;
}

void push_filename(struct file_queue *queue, char *filename)
{
    size_t offs = FILENAME_LEN * queue->head;
    swap_filename(queue->filename + offs, filename);

    queue->head = (queue->head + 1) % queue->size;
}

char* previous_filename(struct file_queue *queue)
{
    unsigned offs = (queue->size - 1 + queue->head) % queue->size;
    return &queue->filename[offs * FILENAME_LEN];
}

int manage_log_files(struct file_queue *queue)
{
    char newfile[FILENAME_LEN];
    generate_filename(newfile);
    if (strcmp(newfile, previous_filename(queue)) == 0) {
        return -2;
    }

    char logpath[TOTAL_PATH_LEN] = {0};
    make_logpath(logpath, newfile);
    
    push_filename(queue, newfile);

    if (strlen(newfile) > 0) {
        make_logpath(logpath, newfile);
        return delete_file(logpath);
    }

    return 0; 
}

void get_curent_logpath(struct file_queue *queue, char *logpath) 
{
    make_logpath(logpath, previous_filename(queue));
}

// #include <stdio.h>
// void printval(char *n)
// {
//     for (int i = 0; i < FILENAME_LEN; i++) {
//         printf("%d ", n[i]);
//     }
//     printf("\n");
// }
// int main() 
// {
//     struct file_queue tst;
//     if (alloc_file_queue(&tst, 5) == -1) {
//         perror("malloc");
//         return -1;
//     }

//     char fnames[][FILENAME_LEN] = {
//         "1900_Jan_01.log",
//         "1900_Jan_02.log",
//         "1900_Jan_03.log",
//         "1900_Jan_04.log",
//         "1900_Jan_05.log",
//         "1900_Jan_06.log",
//         "1900_Jan_07.log",
//         "1900_Jan_08.log",
//         "1900_Jan_09.log",
//         "1900_Jan_10.log",
//         "1999_Dec_10.log",
//     };
//     char logpath[TOTAL_PATH_LEN];
//     for (int i = 0; i < 11; i++) {
//         make_logpath(logpath, fnames[i]);
//         printf("creating %s\n", logpath);
//         int fd = open(logpath, FFLAG, FMODE);
//         push_filename(&tst, fnames[i]);
//         printval(fnames[i]);
//         printf("%s\n", fnames[i]);
//         printf("%s\n", &tst.filename[FILENAME_LEN * ((9 + tst.head)%tst.size)]);
//         printf("closing fd - %d\n", fd);
//         close(fd);
//     }
//     int fd = manage_log_files(&tst);
// }