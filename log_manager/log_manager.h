#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#ifndef LOG_PATH
#define LOG_PATH        "log/"
#endif

#define FILENAME_LEN    sizeof("1900_Jan_01.log")
#define LOG_PATH_LEN    sizeof(LOG_PATH)
#define TOTAL_PATH_LEN  FILENAME_LEN + LOG_PATH_LEN

#define __log_info(...)	syslog(LOG_INFO, __VA_ARGS__)
#define __log_err(...)	syslog(LOG_ERR, __VA_ARGS__)

struct file_descriptors {
    int info;
    int error;
};

struct channel_info {
    char *procname;
    struct file_descriptors fd;
};



#endif