# !/bin/sh

db_user="backup"
db_pass="backuppwd"
db_name="seer_bean_db"

if [ -n $1 ] ; then
        sft=$[($(date +%s) - $(date +%s -d "$1"))/86400]
else
        sft=0
fi
today="date -d -$sft+day"
yesterday="date -d -$[$sft + 1]+day"
twodaysago="date -d -$[$sft + 2]+day"
threedaysago="date -d -$[$sft + 3]+day"

dateofweek=`date +%u -d -$sft+day`

mt_dir="./maintenance"
mt_file=$mt_dir"/mt_`$today +%Y-%m-%d`.txt"

if [ ! -d $mt_dir ]
then
        mkdir $mt_dir
fi

# 查看服务器的运行状态：处理器及内存占用，进程状态
echo "----> Server state:" > $mt_file
./server.sh state >> $mt_file

echo >> $mt_file

# 如果是周一，则显示周末的内容，否则只显示今天和昨天的内容
if [ $dateofweek = 1 ]
then
        time_from="`$threedaysago +%Y-%m-%d` 00:00:00"
        log_files="../log/*`$threedaysago +%Y%m%d`* ../log/*`$twodaysago +%Y%m%d`* ../log/*`$yesterday +%Y%m%d`* ../log/*`$today +%Y%m%d`* "
else
        time_from="`$yesterday +%Y-%m-%d` 00:00:00"
        log_files="../log/*`$yesterday +%Y%m%d`* ../log/*`$today +%Y%m%d`*"
fi
time_to="`$today +%Y-%m-%d` 23:59:59"

# 检查日志，只检查今天和昨天的日志
echo "----> Error log in $log_files:" >> $mt_file
grep -in "err\|warn\|fail" $log_files >> $mt_file

echo >> $mt_file

# 检查数据库，只检查今天和昨天的数据
echo "----> top 10 transaction summary from $time_from to $time_to:" >> $mt_file
my_sql="SET NAMES UTF8; SELECT SUM(seer_bean_num) AS sumb FROM seer_bean_transaction_table WHERE transaction_time>='$time_from' and transaction_time<='$time_to' GROUP BY account_id ORDER BY sumb LIMIT 10;"
mysql -u$db_user -p$db_pass -e "$my_sql" $db_name >> $mt_file

echo "----> top 10 bean account:" >> $mt_file
my_sql="SET NAMES UTF8; SELECT * FROM seer_bean_account_table ORDER BY seer_bean_num DESC LIMIT 10;"
mysql -u$db_user -p$db_pass -e "$my_sql" $db_name >> $mt_file


