#ifndef PARSER_H
#define PARSER_H

#include "common.h"

int parse_processes(char* file, struct process** all_proc);
void destroy_process_list(struct process* proc);

#endif