#!/usr/bin/php5

<?php
/*
	同步用户当前在数据库中的金豆数到目标系统（dbproxy,...）
*/

//同步目标地址,根据使用环境需进行修改
$targets = array();
$targets[] = array("10.1.1.5", "22001"); /////////////
$targets[] = array("10.1.1.7", "13088"); ////////////

//DB连接信息,根据使用环境需进行修改
function get_db_conn()
{
	$db_ip = "localhost"; /////////////
	$db_port = 3306;      ///////////// 
	$db_user = "landry";  /////////////
	$db_pwd = "landry";   /////////////
	$db_name = "seer_bean_db"; /////////////

	$db_conn = mysqli_connect($db_ip,$db_user,$db_pwd,$db_name,$db_port);

	return $db_conn;
}

///
$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if ($argc != 2)
{
	//account_id:用户米米号,
	echo "usage: sync_seer_bean_realtime.php account_id\n";
	exit();
}

$account_id = (int)$argv[1]; 

if($account_id <= 0)
{
    echo("failed! invalid user id!\n");
    exit();
}

/* 获取用户当前的金豆数 */
$bean_num = 0; //赛尔豆数量 (1赛尔豆则 $bean_num = 100)

$db_connect = get_db_conn();
if (!$db_connect)
{
	die("get_db_conn() failed\n");
}

$sql = "SELECT seer_bean_num FROM seer_bean_account_table WHERE account_id=$account_id";

if ($result_set = mysqli_query($db_connect,$sql))
{
	if($row = mysqli_fetch_assoc($result_set))
	{
		$bean_num = (int) $row["seer_bean_num"];
	}
	else
	{
		exit("account_id:$account_id in seer_bean_account_table not exist!\n");
	}
}
else
{
	exit("query error,sql:$sql\n");
}

/* 发送报文到同步的目标系统  */
echo("account_id:$account_id, bean_num:$bean_num\n");

$package_len = 18 + 4;
$cmd_id = 0x19a6; //seer

function pack_header($package_len,$cmd_id,$account_id)
{
    return pack("LLSLL",$package_len,0,$cmd_id,0,$account_id);
}

foreach($targets as $target)
{
	echo "ip:".$target[0]." port:".$target[1]."\n";
	
	$packed_data = pack_header($package_len,$cmd_id,$account_id) . pack("L",$bean_num);
	
	//send to server
	$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);
	if(!$socket)
	{
	    echo("failed! create socket failed!!!\n");
	    exit();
	}
	
	if(!socket_connect($socket,$target[0],$target[1]))
	{
	    echo("failed! connect to  server failed!!!\n");
	    exit();
	}
	
	$result = socket_write($socket,$packed_data,strlen($packed_data));
	if($result === false)
	{
	    echo("failed! send data to  server failed!!!\n");
	    exit();
	}
	
	if($result != strlen($packed_data))
	{
	    echo("failed! unable to send all data!!!\n");
	    exit();
	}
	
	//recv from server
	$recved_string = socket_read($socket,1024,PHP_BINARY_READ);
	if($recved_string == false || strlen($recved_string) <= 0)
	{
	    echo("failed! recved data from  server failed!!!!\n");
	    exit();
	}
	
	$return_array = unpack("Lpackage_length/Lseq_num/Scommand_id/Lstatus_code/Laccount_id",$recved_string);
	if($return_array == false)
	{
	    echo("failed! recved data from  server error!!!\n");
	    exit();
	}
	else
	{
		socket_close($socket);
		print_r($return_array);
	}
}

?>
