#!/bin/bash

cd $(dirname $(realpath $0))
cd ../
make clean
make
./daemon_manager &
# start-stop-daemon -S -b -n daemon_manager -a ./daemon_manager

if [ $? -ne 0 ]; then
  echo "Something failed"
  exit 1
fi

ps
echo "killing after 10 seconds"
sleep 10

PID=$!
kill $PID
echo "$PID killed"

ps

./daemon_manager&
cat /var/log/syslog | grep daemon_manager
cat /var/log/syslog | grep log_manager

echo "letting it go now"