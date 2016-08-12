#!/bin/sh
if [ "$1" = "stop" ] ; then
	ps -ef |grep "\<mb_account\>" | awk '{print "kill " $2}'|sh
elif [ "$1" = "restart" ]; then
	killall -HUP ./mb_account
elif [ "$1" = "start" ]; then
	./mb_account ../etc/bench.conf ../lib/libmbaccount.so 
elif [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
	echo "daemon.sh start|stop|restart"
fi
