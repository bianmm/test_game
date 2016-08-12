#!/bin/sh

# 需要管理的服务器可执行文件的名称，根据实际应用修改
server_name='./buyu_diamond'
server_path=$server_name
server_conf='../etc/bench.conf'

# 当前登录的用户名
logname=$LOGNAME

# 当前登录的用户 ID，如果用户名长度大于 7，则需要使用用户 ID 来搜索进程
logid=`cat /etc/passwd | grep $logname | awk -F ':' '{print $3}'`

export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH

if [ "$1" = "state" ] ; then
    ps -eo user,pid,stat,pcpu,pmem,cmd | grep "$server_name" | grep -v grep
elif [ "$1" = "start" ] ; then
    $server_path  $server_conf ../lib/libbuyudiamond.so
elif [ "$1" = "stop" ] ; then
    ps -ef | grep -E "$logname|$logid" | grep "$server_name" | grep -v grep | awk '{print "echo kill " $2 " " $8 "; kill " $2;}' | sh
else
    echo 'usage: server.sh state|start|stop'
fi
