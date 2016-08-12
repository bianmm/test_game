<?php

$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if ($argc < 2)
{
	die("Usage:php query_account_balance_test.php user_id\n");
}

$account_id = (int)$argv[1];

if($account_id < 10000)
{
    echo("invalid account_id!\n");
    exit();
}

$bean_server_ip = "10.1.1.5"; /***************************/
$bean_server_port = "12222"; /***************************/
$add_bean_command_id = 23001;

//pack data
$pack_length = 18;
$packed_data = pack("LLSLL",$pack_length, 0, $add_bean_command_id, 0 ,$account_id);

//send to server
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);
if(!$socket)
{
    echo("create socket failed!!!\n");
    exit();
}

if(!socket_connect($socket,$bean_server_ip,$bean_server_port))
{
    echo("connect to server failed!!!\n");
    exit();
}

$result = socket_write($socket,$packed_data,strlen($packed_data));
if($result === false)
{
    echo("send data to server failed!!!\n");
    exit();
}

if($result != strlen($packed_data))
{
    echo("unable to send all data!!!\n");
    exit();
}

//recv from server
$recved_string = socket_read($socket,1024,PHP_BINARY_READ);
if($recved_string == false || strlen($recved_string) <= 0)
{
    echo("recved data from server failed!!!!\n");
    exit();
}

socket_close($socket);

$return_array = unpack("Lpackage_length/Lseq/Scommand_id/Lstatus_code/Laccount_id/Lbean_num/Laccount_status", $recved_string);
if($return_array == false)
{
    echo("recved data from server error!!!\n");
    exit();
}

print_r($return_array);

?>
