#!/bin/sh
if [ "$1" = "stop" ] ; then
        ps -ef |grep $LOGNAME | grep ipay_server | awk '{print "kill " $2}'|sh
elif [ "$1" = "restart" ]; then
	killall -HUP ./ipay_server
elif [ "$1" = "start" ]; then
	export LD_LIBRARY_PATH=../lib;
	./ipay_server ../conf/bench.conf ./libipayiap.so
elif [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
	echo "server.sh start|stop|restart"
fi
