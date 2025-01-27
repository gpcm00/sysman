
.PHONY: all log_manager daemon_manager

all: log_manager daemon_manager

ifdef NEW_LOG_PATH
	mkdir -p $(NEW_LOG_PATH)
    ADD_LOG_PATH := LOG_PATH=$(NEW_LOG_PATH)
else
    ADD_LOG_PATH :=
endif

ifdef NEW_APP_PATH
	ADD_APP_PATH := APP_PATH=$(NEW_APP_PATH)
else
	ADD_APP_PATH :=
endif

ifdef NEW_LOG_MANAGER_PATH
	ADD_LOG_MANAGER_PATH := LOG_MANAGER_PATH=$(NEW_LOG_MANAGER_PATH)
else
	ADD_LOG_MANAGER_PATH :=
endif

log_manager:
	$(MAKE) -C log_manager $(ADD_LOG_PATH)

daemon_manager:
	$(MAKE) -C daemon_manager $(ADD_APP_PATH) $(ADD_LOG_MANAGER_PATH)

clean:
	$(MAKE) -C log_manager clean
	$(MAKE) -C daemon_manager clean
