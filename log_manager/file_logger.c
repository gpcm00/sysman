#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <signal.h>

#include "log_manager.h"
#include "file_logger.h"

#define TIME_STR_LEN	sizeof("00:00:00 - ")

#define FFLAG			O_CREAT | O_APPEND | O_RDWR
#define FMODE			0666

int init_global_logpath(struct global_logpath *lp)
{
    memset(lp->logpath, 0, sizeof(lp->logpath));
    return pthread_mutex_init(&lp->mtx, NULL);
}

void set_global_logpath(struct global_logpath *lp, char *logpath)
{
    pthread_mutex_lock(&lp->mtx);
    strcpy(lp->logpath, logpath);
    pthread_mutex_unlock(&lp->mtx);
}

static int open_logpath(struct global_logpath *lp)
{
    pthread_mutex_lock(&lp->mtx);
    int fd = open(lp->logpath, FFLAG, FMODE);

    if (fd == -1) {
        pthread_mutex_unlock(&lp->mtx);
    }

    return fd;
}

static int close_logpath(struct global_logpath *lp, int fd)
{
    int ret = close(fd);
    pthread_mutex_unlock(&lp->mtx);
    return ret;
}

static size_t inert_time(char* buff)
{
	time_t t = time(NULL);
	struct tm* ftime = localtime(&t);
	if(ftime == NULL) {
		return 0;
	}

	return strftime(buff, TIME_STR_LEN, "%T - ", ftime);
}

static size_t copy_prefix(char *buffer, const char *prefix)
{
	size_t i = 0;
	while (prefix[i] != '\0') {
		buffer[i] = prefix[i];
		i++;
	}
	return i;
}

void* log_thread(void* arg)
{
    const char *prefix = ((struct thread_arg*)arg)->prefix;
	const int fd = ((struct thread_arg*)arg)->fd;
	struct global_logpath *log = ((struct thread_arg*)arg)->log;
	const unsigned prefix_len = strlen(prefix);
	const unsigned initial_offset = TIME_STR_LEN-1 + prefix_len;

	sigset_t block;
	sigemptyset(&block);
	sigaddset(&block, SIGTERM);
	sigaddset(&block, SIGINT);
	if (pthread_sigmask(SIG_BLOCK, &block, NULL) != 0) {
		__log_err("pthread_sigmask(%s) failed!", prefix);
		exit(EXIT_FAILURE);
	}

	char buffer[512];

	int flags = fcntl(fd, F_GETFL);

	while (1) {
		memset(buffer, 0, sizeof(buffer));
		ssize_t nread = read(fd, buffer+initial_offset, sizeof(buffer) - initial_offset);
		if (nread == -1) {
			__log_err("read(%s): %s", prefix, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			__log_err("fcntl(%s): %s", prefix, strerror(errno));
			exit(EXIT_FAILURE);
		}

		nread += inert_time(buffer);
		nread += copy_prefix(buffer + TIME_STR_LEN-1, prefix);

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		int filefd = open_logpath(log);
		if (filefd == -1) {
			__log_err("open(%s): %s", prefix, strerror(errno));
			exit(EXIT_FAILURE);
		}

		while (nread > 0) {
			nread = write(filefd, buffer, nread);
			if (nread == -1) {
				__log_err("write(%s): %s", prefix, strerror(errno));
				exit(EXIT_FAILURE);
			}
			
			nread = read(fd, buffer, sizeof(buffer));
			if (nread == -1 && (errno != EAGAIN || errno != EWOULDBLOCK)) {
				__log_err("read(%s): %s", prefix, strerror(errno));
			} else if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
				nread = 0;
				errno = 0;
			}
		}

		close_logpath(log, filefd);

		if (fcntl(fd, F_SETFL, flags) == -1) {
			__log_err("fcntl(%s): %s", prefix, strerror(errno));
			exit(EXIT_FAILURE);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	
	return NULL;
}

// int main()
// {
// 	struct global_logpath glp;
// 	init_global_logpath(&glp);
// 	set_global_logpath(&glp, "1999_Jan_01.log");

// 	struct thread_arg arg = {
// 		.prefix = "testing prefix: ",
// 		.fd = 0,
// 		.log = &glp
// 	};

// 	log_thread(&arg);

// 	return 0;
// }