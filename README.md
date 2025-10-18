# sysman
system management for resource constrained devices

i originally made this to initialize services that are supposed to run on the background for a smart lock running a custom made buildroot image

i am only keeping it here because i want to use this code again in future projects

the purpose of this program is to initialize multiple daemons and ensure that they remain alive \
messages to stdout and stderr for all deamons are piped to log_manager, which just stores them in the log file of the day\
everyday a new log file is created and log_manager keeps N logs in the system, automaticaly deleting the extra log files (currently set to 2)

# files
daemon_manager: contains the main program that initializes all daemons 

log_manager: contains the log_manager app

# usage

    ./build.sh
    daemon_manager/daemon_manager proclist.txt

this will run daemon manager with the programs in daemon_manager/test

# tips

use build.sh and the Makefile in the root folder to understand how to build this code.

i recommend using the Makefile inside daemon_manager and log_manager if you are planning to use this with buildroot.

change the proclist.txt file to run the processes you wish to initialize

the folder that you will store in the log files is defined during compilation time. the the Makefile in the root folder and the Makefile in log_manager to see how LOG_PATH in log_manager/log_manger.h is setup.

daemon_manager needs to know log_manager path during compilation. check the Makefile in the root folder and the Makefile in daemon_manager to see how LOG_MANAGER_PATH in daemon_manager/main.c is defined