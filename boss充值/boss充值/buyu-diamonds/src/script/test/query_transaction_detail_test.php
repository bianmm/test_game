<?php

$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if ($argc < 6)
{
	die("Usage:php query_transaction_detail_test.php account_id start_time end_time start_row end_row\n");
}

function get_timestamp($strtime)
{
    $array = explode("-",$strtime);
    $year = $array[0];
    $month = $array[1];

    $array = explode(":",$array[2]);
    $minute = $array[1];
    $second = $array[2];

    $array = explode(" ",$array[0]);
    $day = $array[0];
    $hour = $array[1];

    return mktime($hour,$minute,$second,$month,$day,$year);
}

$account_id = (int)$argv[1];
$start_time = (int)get_timestamp($argv[2]);
$end_time = (int)get_timestamp($argv[3]);
$start_row = (int)$argv[4];
$end_row = (int)$argv[5];

if($account_id == 0)
{
    echo("invalid account_id!\n");
    exit();
}

$bean_server_ip = "10.1.1.5"; /***************************/
$bean_server_port = "12222"; /***************************/
$command_id = 23003;

//pack data
$pack_length = 18 + 12;
$packed_data = pack("LLSLL",$pack_length, 0, $command_id, 0 , $account_id);
$packed_data .= pack("LLSS", $start_time, $end_time, $start_row, $end_row);

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

$return_array = unpack("Lpackage_length/Lseq/Scommand_id/Lstatus_code/Luser_id/Ltotal_row_cnt/Lcurrent_row_cnt", $recved_string);
if($return_array == false)
{
    echo("recved data from server error!!!\n");
    exit();
}

print_r($return_array);

?>
