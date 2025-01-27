#ifndef FILE_LIST_H
#define FILE_LIST_H

struct file_queue {
    char* filename;
    unsigned head;
    unsigned size;
};

int alloc_file_queue(struct file_queue *queue, unsigned long nfiles);
void free_file_queue(struct file_queue *queue);
void push_filename(struct file_queue *queue, char *filename);
char* previous_filename(struct file_queue *queue);
int manage_log_files(struct file_queue *queue);
void get_curent_logpath(struct file_queue *queue, char *logpath);

#endif