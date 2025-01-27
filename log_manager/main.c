#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "log_manager.h"

#include "file_logger.h"
#include "file_manager.h"

#define __sizeofprefix(b)  ((b)? sizeof("INFO ") : sizeof("ERROR "))
#define __getprefix(b)     ((b)? "INFO " : "ERROR ")

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

bool running = true;
void terminate(int signum)
{   
    switch(signum) {
	case SIGTERM:
	case SIGINT:
		running = false;
		break;
	default:
	    break;
    }
}

void check(bool state, char *msg)
{
    char *error_msg = (errno == 0)? "Operation failed" : strerror(errno);
    if (state) {
        __log_err("%s: %s", msg, error_msg);
        exit(EXIT_FAILURE);
    }
}

struct channel_info *alloc_channels(size_t sz, char** names)
{
    struct channel_info* ret = 
        (struct channel_info*)malloc(sizeof(struct channel_info) * sz);
    
    if (ret == NULL) {
        return NULL;
    }

    int fd = 0;
    for (uint i = 0; i < sz; i++) {
        ret[i].procname = names[i];

        ret[i].fd.info = fd++;
        ret[i].fd.error = fd++;
    }

    return ret;
}

void populate_thread_arg(struct thread_arg *arg, struct channel_info *channel, 
            struct global_logpath *log, bool info_errornot) 
{
    arg->fd = (info_errornot)? channel->fd.info : channel->fd.error;
    uint prefix_len = strlen(channel->procname) + __sizeofprefix(info_errornot) + 2;

    char *prefix = (char*)malloc(prefix_len);
    memset(prefix, 0, prefix_len);
    strcpy(prefix, __getprefix(info_errornot));
    strcpy(prefix + __sizeofprefix(info_errornot)-1, channel->procname);
    prefix[prefix_len-3] = ':';
    prefix[prefix_len-2] = ' ';

    arg->prefix = prefix;

    arg->log = log;
}

void delete_thread_arg(struct thread_arg *arg) 
{
    free(arg->prefix);
    arg->prefix = NULL;
}

bool sleep_until_midnight()
{
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    current_time->tm_sec = 0;
    current_time->tm_min = 0;
    current_time->tm_hour = 0;
    current_time->tm_mday++;
    
    time_t midnight = mktime(current_time);
    
    time_t diff = midnight - now;
    do {
        diff = sleep(diff);
    } while (diff != 0 && running);

    return running;
}


// first argument is the number of log files kept in the list
// following arguments are the names of the processes that are logging into the file
int main(int argc, char** argv)
{
    openlog("log_manager", LOG_PID, LOG_USER);

    int nproc = argc - 2;
    check(nproc <= 0, "0 processes found");

    size_t nfiles = atol(argv[1]);
    check(nfiles <= 0, "invalid number of files");

    struct channel_info *channel = alloc_channels(nproc, &argv[2]);
    check(channel == NULL, "alloc(channels)");

    struct file_queue fq;
    check(alloc_file_queue(&fq, nfiles) == -1, "alloc(file_queue)");
    
    check(manage_log_files(&fq) == -1, "manage_file");
    
    struct global_logpath log;
    check(init_global_logpath(&log) != 0, "init_logpath");

    char logpath[TOTAL_PATH_LEN];
    get_curent_logpath(&fq, logpath);
    set_global_logpath(&log, logpath);

    char cwd[1024] = {};
    check(getcwd(cwd, sizeof(cwd)) == NULL, "getcwd");
    __log_info("log path initiated relative to %s", cwd);


    // twice nproc because each proc has 2 fd: info and error
    struct thread_arg *args = 
        (struct thread_arg*)malloc(sizeof(struct thread_arg) * 2 * nproc);

    pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * nproc);
        
    for (uint i = 0; i < nproc; i++) {
        populate_thread_arg(&args[2*i], &channel[i], &log, true);
        check(pthread_create(&tid[i], NULL, log_thread, &args[2*i]) != 0, "pthread_create");

        populate_thread_arg(&args[2*i+1], &channel[i], &log, false);
        check(pthread_create(&tid[i], NULL, log_thread, &args[2*i+1]) != 0, "pthread_create");

        __log_info("logging initialized for %s", channel[i].procname);
    }

    struct sigaction term;
    term.sa_handler = terminate;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    check(sigaction(SIGINT, &term, NULL) == -1, "sigaction(SIGINT)");
	check(sigaction(SIGTERM, &term, NULL) == -1, "sigaction(SIGTERM)");
	
    while (running) {
        if (!sleep_until_midnight()) continue;

        int r = manage_log_files(&fq);
        check(r == -1, "manage_log");
        if (r == -2) continue;

        get_curent_logpath(&fq, logpath);
        set_global_logpath(&log, logpath);
        
        __log_info("new log file: %s", logpath);
    }

    __log_info("terminating all threads");

    for (uint i = 0; i < nproc; i++) {
        pthread_cancel(tid[i]);
        pthread_join(tid[i], NULL);
        delete_thread_arg(&args[i]);
    }

    free(args);
    free(tid);
    free_file_queue(&fq);
    free(channel);
    closelog();

    exit(EXIT_SUCCESS);
}