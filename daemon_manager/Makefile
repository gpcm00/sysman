CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
OBJ := main.o
TARGET := daemon_manager
ADDDEFINE :=

ifdef DEBUGGING
    CFLAGS += -g
endif

ifdef APP_PATH
	ADDDEFINE += -DAPP_PATH="\"$(APP_PATH)\""
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