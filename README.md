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
    daemon_manager/daemon_manager

this will run daemon manager with the programs in daemon_manager/test

# tips

use build.sh and the Makefile in the root folder to understand how to build this code.

i recommend using the Makefile inside daemon_manager and log_manager if you are planning to use this with buildroot.

check all_processes array in daemon_manager/main.c to check how daemon are initialized. i want to add a parser eventually to make this more extensible

 # TODO

 add parser so we can initialized processes based on an input file
