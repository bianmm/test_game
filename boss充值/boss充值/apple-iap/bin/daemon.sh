#!/bin/sh
export LD_LIBRARY_PATH=../lib/
if [ "$1" = "stop" ] ; then
ps -ef |grep "apple_iap" | awk '{print "kill " $2}'|sh
elif [ "$1" = "restart" ]; then
killall -HUP ./apple_iap
elif [ "$1" = "start" ]; then
./apple_iap ../conf/bench.conf ./libappleiap.so
elif [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
echo "daemon.sh start|stop|restart"
fi
