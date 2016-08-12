#!/bin/sh
export LD_LIBRARY_PATH=../lib/
if [ "$1" = "stop" ] ; then
ps -ef |grep "google_play" | awk '{print "kill " $2}'|sh
elif [ "$1" = "restart" ]; then
killall -HUP ./google_play
elif [ "$1" = "start" ]; then
./google_play ../conf/bench.conf ./libgoogleplay.so
elif [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
echo "daemon.sh start|stop|restart"
fi
