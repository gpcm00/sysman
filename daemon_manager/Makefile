CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
OBJ := main.o parser.o
TARGET := daemon_manager
ADDDEFINE :=

ifdef DEBUGGING
    CFLAGS += -g
endif

ifdef LOG_MANAGER_PATH
	ADDDEFINE += -DLOG_MANAGER_PATH="\"$(LOG_MANAGER_PATH)\""
endif

all: $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INC) $(ADDDEFINE)

clean:
	$(RM) -f $(TARGET) $(OBJ)