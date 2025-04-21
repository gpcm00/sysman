# sysman
system management for resource constrained devices

the purpose of this program is to initialize multiple daemons and ensure that they remain alive \
also, the output of stdout and stderr are routed to log_manager, which just stores it in a log file\
everyday a new log file is created\
also, log_manager takes in an argument based on how many log files we wish to keep (currently set to 2)

# files
daemon_manager: contains the main program that initializes all daemons 

log_manager: contains the log_manager app

# usage

    ./build.sh
    daemon_manager/daemon_manager

this will run daemon manager with the programs in daemon_manager/test
