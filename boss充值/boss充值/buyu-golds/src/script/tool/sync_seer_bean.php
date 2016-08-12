#!/usr/bin/php5

<?php
/*
	根据提供的参数，同步用户的赛尔豆数量到目标系统（dbproxy,...）
*/

///同步目标地址,根据使用环境需进行修改
$targets = array();
$targets[] = array("10.1.1.5", "22001"); /////////////
$targets[] = array("10.1.1.7", "13088"); ////////////

///
$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if ($argc != 3)
{
	//user_id:用户米米号,
	//bean_num:赛尔豆数量 (1赛尔豆则 bean_num = 1)
	echo "usage: sync_seer_bean user_id bean_num\n";
	exit();
}

$user_id = (int)$argv[1]; 
$bean_num = (int)$argv[2];

if($user_id <= 0)
{
    echo("failed! invalid user id!\n");
    exit();
}

if($bean_num < 0)
{
    echo("failed! invalid bean_num!\n");
    exit();
}

if($bean_num === 0)
{
    echo("warning: bean_num is zero!\n");
    exit();
}

echo("user_id:$user_id, bean_num:$bean_num\n");

///
$package_len = 18 + 4;
$cmd_id = 0x19a6; //seer

function pack_header($package_len,$cmd_id,$user_id)
{
    return pack("LLSLL",$package_len,0,$cmd_id,0,$user_id);
}

foreach($targets as $target)
{
	echo "ip:".$target[0]." port:".$target[1]."\n";
	
	$packed_data = pack_header($package_len,$cmd_id,$user_id) . pack("L",($bean_num * 100));
	
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
	
	$return_array = unpack("Lpackage_length/Lseq_num/Scommand_id/Lstatus_code/Luser_id",$recved_string);
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
