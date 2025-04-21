#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <pty.h>
#include <wait.h>
#include <sys/types.h>

#ifndef LOG_MANAGER_PATH
#define LOG_MANAGER_PATH 
#endif

#ifndef APP_PATH
#define APP_PATH 
#endif

#define __log_info(...)	syslog(LOG_INFO, __VA_ARGS__)
#define __log_err(...)	syslog(LOG_ERR, __VA_ARGS__)
#define __len(arr)      (sizeof(arr)/sizeof(arr[0]))

#define MAX_NUM_OF_ARGS 5
#define TRIES_THRESHOLD 50

#define __case_state_name(n)    case n: return #n

#define __exit_if_error(e, ...)                        \
    do {                                               \
        if (e == -1) {                                 \
            __log_err(__VA_ARGS__);                    \
            exit(EXIT_FAILURE);                        \
        }                                              \
    } while(0)                                         \

#define RD      0
#define WR      1

#define DEFAULT_INITIAL_PARAM                          \
    .alive = false,                                    \
    .pid = -1,                                         \
    .fd = {                                            \
        {0,0}, {0,0}                                   \
    }                                                  \

typedef enum {
    running = 0,
    terminated,
    sigchild_error,
    suspend_error,
    restarting_error,
} global_state_t;

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

struct process all_processes[] = {
    {
        .path = LOG_MANAGER_PATH"log_manager",
        .args = {
            "log_manager", "2" , "cpp_messages", "c_messages", "send_messages",
        },
        DEFAULT_INITIAL_PARAM,
    },

    {
        .path = APP_PATH"test/cpp_messages",
        .args = {
            "cpp_messages", "9", NULL, NULL, NULL,
        },
        DEFAULT_INITIAL_PARAM,
    },

    {
        .path = APP_PATH"test/c_messages",
        .args = {
            "c_messages", "9", NULL, NULL, NULL,
        },
        DEFAULT_INITIAL_PARAM,
    },

    {
        .path = APP_PATH"test/send_messages.py",
        .args = {
            "send_messages", NULL, NULL, NULL, NULL,
        },
        DEFAULT_INITIAL_PARAM,
    },
};

global_state_t global_state = running;

void terminate(int signum)
{   
    switch(signum) {
	case SIGTERM:
	case SIGINT:
		global_state = terminated;
		break;
	default:
	    break;
    }
}


void kill_child(int signum)
{
	int i, status;
	pid_t pid;
	if(signum == SIGCHLD) {
		for(i = 0; i < __len(all_processes); i++) {
			pid = waitpid(all_processes[i].pid, &status, WNOHANG);
			if(pid == -1) {
				global_state = sigchild_error;
			} else if(pid > 0) {
				all_processes[i].alive = false;
				all_processes[i].pid = -1;
			}
		}
	}
}

const char* get_state_name(global_state_t s)
{
    switch (s)
    {
    __case_state_name(running);
    __case_state_name(terminated);
    __case_state_name(sigchild_error);
    __case_state_name(suspend_error);
    __case_state_name(restarting_error);
    
    default:
        return "unknown state";
    }
}

bool create_process(struct process *proc)
{
    proc->pid = fork();
    if (proc->pid == 0) {
        __exit_if_error(
            dup2(proc->fd.info[WR], 1), 
            "[%s] dup2(info): %s", 
            proc->args[0], strerror(errno)
        );

        __exit_if_error(
            dup2(proc->fd.error[WR], 2), 
            "[%s] dup2(error): %s", 
            proc->args[0], strerror(errno)
        );

        __exit_if_error(
            execv(proc->path, proc->args), 
            "[%s] execv: %s", 
            proc->args[0], strerror(errno)
        );
    } else if (proc->pid == -1) {
        return false;
    }
    return true;
}

bool set_cloexec(int fd) 
{
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        return false;
    }

    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        return false;
    }

    return true;
}

// use int to comply with standard system call failure method (-1 for fail)
int create_pseudopty(int *fd, char* name) 
{
    int res = openpty(&fd[WR], &fd[RD], NULL, NULL, NULL);
    if (res == -1) {
        return -1;
    }

    if (!set_cloexec(fd[RD]) || !set_cloexec(fd[WR])) {
        return -1;
    }

    return 0;
}

void create_pipe(struct process *proc)
{
    __exit_if_error(
        create_pseudopty(proc->fd.info, proc->args[0]),
        "info openpty(%s): %s",
        proc->args[0], strerror(errno)
    );

    __exit_if_error(
        create_pseudopty(proc->fd.error, proc->args[0]),
        "error openpty(%s): %s",
        proc->args[0], strerror(errno)
    );
}

void duplicate_fd(void)
{
    int fd = 0;
    for (size_t i = 1; i < __len(all_processes); i++) {
        __exit_if_error(
            dup2(all_processes[i].fd.info[RD], fd++),
            "logging info - dup2(%s): %s", 
            all_processes[i].args[0], strerror(errno)
        );

        __exit_if_error(
            dup2(all_processes[i].fd.error[RD], fd++),
            "logging error - dup2(%s): %s", 
            all_processes[i].args[0], strerror(errno)
        );
    }
}

bool create_logger(void)
{
    struct process *logger = &all_processes[0];

    logger->pid = fork();
    if (logger->pid == 0) {
        duplicate_fd();
        
        __exit_if_error(
            execv(logger->path, logger->args),
            "execv(logger): %s", strerror(errno)
        );
    } else if (logger->pid == -1) {
        return false;
    }

    logger->alive = true;
    return true;
}

void init_proclist(void) 
{
    for (size_t i = 0; i < __len(all_processes); i++) {
        __exit_if_error(
            access(all_processes[i].path, X_OK),
            "access(%s): %s",
            all_processes[i].args[0], strerror(errno)
        );

        if (i > 0) {
            create_pipe(&all_processes[i]);
        }
    }
}

bool create_all_processes(void)
{
    for (size_t i = 1; i < __len(all_processes); i++) {
        all_processes[i].alive = create_process(&all_processes[i]);
        if (!all_processes[i].alive) {
            __log_err(
                "%s failed: %s",
                all_processes[i].args[0], strerror(errno)
            );

            return false;
        } else {
            __log_info(
                "%s created [%d]", 
                all_processes[i].args[0], all_processes[i].pid
            );
        }   
    }
    return true;
}

void kill_all_processes(void)
{
    for (size_t i = 0; i < __len(all_processes); i++) {
        if (all_processes[i].alive) {
            kill(all_processes[i].pid, SIGTERM);
        }
    }
}

bool restart_process(void)
{
    for (size_t i = 0; i < __len(all_processes); i++) {
        if (!all_processes[i].alive) {
            all_processes[i].alive = create_process(&all_processes[i]);
            if (!all_processes[i].alive) {
                __log_err(
                    "%s failed: %s",
                    all_processes[i].args[0], strerror(errno)
                );
                return false;
            }
            __log_info("%s reinitialized", all_processes[i].args[0]);
        }
    }
    return true;
}

struct sigaction init_sigaction(void (*handler)(int))
{
    struct sigaction ret;
    ret.sa_handler = handler;
	sigemptyset(&ret.sa_mask);
	ret.sa_flags = 0;
    return ret;
}

int main(int argc, char** argv)
{
    openlog("daemon_manager", LOG_PID, LOG_USER);

    struct sigaction term = init_sigaction(terminate);
	struct sigaction chld = init_sigaction(kill_child);
    
    sigset_t block, old;
    sigemptyset(&block);
	sigaddset(&block, SIGCHLD);

	__exit_if_error( 
        sigprocmask(SIG_BLOCK, &block, &old),
        "sigprocmask: %s", strerror(errno)
    );

    __exit_if_error( 
        sigaction(SIGINT, &term, NULL),
        "sigaction(SIGINT): %s", strerror(errno)
    );

    __exit_if_error( 
        sigaction(SIGTERM, &term, NULL),
        "sigaction(SIGTERM): %s", strerror(errno)
    );

    __exit_if_error( 
        sigaction(SIGCHLD, &chld, NULL),
        "sigaction(SIGCHLD): %s", strerror(errno)
    );

    init_proclist();

    create_logger();

    if (!create_all_processes()) {
        kill_all_processes();
        exit(EXIT_FAILURE);
    }

    __log_info("monitoring child processes");

    while (global_state == running) {
        int e = sigsuspend(&old);
        if (e == -1 && errno != EINTR) {
            global_state = suspend_error;
            __log_err("program suspended unpredictability");
            break;
        }

        errno = 0;

        int tries = 0;
        while ((tries++ < TRIES_THRESHOLD) && !restart_process());

        if (tries > TRIES_THRESHOLD) {
            global_state = restarting_error;
            __log_err("failed to restart processes (tries: %d)", tries);
            break;
        }

    }

    kill_all_processes();
    closelog();

    __log_info("terminating with state: %s", get_state_name(global_state));

    exit((global_state == terminated)? EXIT_SUCCESS : EXIT_FAILURE);
}
