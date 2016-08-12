<?php

//get query start date and end date or specific date
$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if($argc < 1)
{
    echo('missing parameters!\n');
    exit();
}

foreach($argv as $argv_item)
{
    if(strstr($argv_item,'-d'))
    {
        $specific_date = substr($argv_item,2);
    }
    else if(strstr($argv_item,'-s'))
    {
        $start_date = substr($argv_item,2);
    }
    else if(strstr($argv_item,'-e'))
    {
        $end_date = substr($argv_item,2);
    }
}

if(strlen($start_date) <= 0 || strlen($end_date) <= 0)
{
    echo("parameters error!\n");
    exit();
}

if(strlen($start_date) != 10 || strlen($end_date) != 10)
{
    echo("parameters format error!\n");
    exit();
}

$db_ip = "localhost";
$db_user = "backup";
$db_pwd = "backuppwd";
$db_name = "mb_account_db";
$db_port = 3306;

mysqli_init();
$db_connection = mysqli_connect($db_ip,$db_user,$db_pwd,$db_name,$db_port) or die("connect mysql server failed!\n");

//query total mb num added
$start_date_escaped = mysqli_real_escape_string($db_connection,$start_date);
$end_date_escaped = mysqli_real_escape_string($db_connection,$end_date);

$query_sql = "SELECT SUM(mb_num) AS total_mb_num FROM transaction_table WHERE mb_num > 0 AND transaction_time >= '$start_date_escaped' AND transaction_time < '$end_date_escaped'";
$record_set = mysqli_query($db_connection,$query_sql) or die('query db error!\n');
$data_row = mysqli_fetch_assoc($record_set);
if($data_row)
{
    $total_mb_num = $data_row['total_mb_num'] / 100;
}
else 
{
    echo('fetch data row error!\n');
    exit();
}

//query mb num added group by channel id
$query_sql = "SELECT SUM(mb_num) AS total_mb_num,channel_id FROM transaction_table WHERE mb_num > 0 AND transaction_time >= '$start_date_escaped' AND transaction_time < '$end_date_escaped' GROUP BY channel_id";
$record_set = mysqli_query($db_connection,$query_sql) or die('query db error!\n');
while($data_row = mysqli_fetch_assoc($record_set))
{
    $channel_id = $data_row['channel_id'];
    $channel_mb_table["$channel_id"] = $data_row['total_mb_num'] / 100;
}

//output statisitc
echo("****************************************************\n");
echo("query date : $start_date - $end_date\n");
echo("****************************************************\n");
echo("总共     : $total_mb_num\n");
echo("支付宝   : ${channel_mb_table['11']}\n");
echo("财付通   : ${channel_mb_table['12']}\n");
echo("网银     : ${channel_mb_table['13']}\n");
echo("神州行   : ${channel_mb_table['14']}\n");
echo("如意通   : ${channel_mb_table['15']}\n");
echo("米米卡   : ${channel_mb_table['16']}\n");
echo("短信点播 : ${channel_mb_table['17']}\n");
echo("EBILLING : ${channel_mb_table['19']}\n");
echo("一卡通   : ${channel_mb_table['20']}\n");
echo("空中网   : ${channel_mb_table['21']}\n");
echo("盛大卡   : ${channel_mb_table['22']}\n");
echo("客服增加 : ${channel_mb_table['99']}\n");

//close db connection
mysqli_close($db_connection);

?>
