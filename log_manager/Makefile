CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
OBJ := main.o file_logger.o file_manager.o
TARGET := log_manager
LDFLAGS := -pthread

ifdef DEBUGGING
    CFLAGS += -g
endif

ifdef LOG_PATH
	ADDDEFINE := -DLOG_PATH="\"$(LOG_PATH)\""
endif

all: $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INC) $(ADDDEFINE)

clean:
	$(RM) -f $(TARGET) $(OBJ)
