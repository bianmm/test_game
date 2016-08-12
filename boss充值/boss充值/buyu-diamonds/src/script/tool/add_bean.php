<?php

$argv = $_SERVER['argv'];

$account_id = (int)$argv[1];
$bean_num = ((int)$argv[2]) * 100;
$external_transaction_id = (int)$argv[3];

if($account_id < 10000)
{
    echo("invalid account_id!\n");
    exit();
}

if($bean_num <= 0)
{
    echo("invalid bean_num!\n");
    exit();
}

if($external_transaction_id <= 0)
{
    echo("invalid external_transaction_id!\n");
    exit();
}

$bean_server_ip = "10.1.1.5"; /***************************/
$bean_server_port = "12222"; /***************************/
$add_bean_command_id = 22001;

$special_channel_id = 1099;
$security_code = "11112222";

//pack data
$packed_body = pack("L2", $bean_num, $external_transaction_id);
$verify_string = "channelId=$special_channel_id&securityCode=$security_code&data=" . $packed_body;
$verify_code = md5($verify_string);

$pack_length = 18 + 34 + 8;
$packed_data = pack("LLSLL",$pack_length, 0, $add_bean_command_id, 0 ,$account_id);
$packed_data .= pack("Sa32",$special_channel_id,$verify_code);
$packed_data .= $packed_body;

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

$return_array = unpack("Lpackage_length/Lseq/Scommand_id/Lstatus_code/Laccount_id/Ltransaction_id", $recved_string);
if($return_array == false)
{
    echo("recved data from server error!!!\n");
    exit();
}

print_r($return_array);

$status_code = $return_array['status_code'];
echo("operation complete! account_id = $account_id,bean_num = $bean_num,status_code = $status_code\n");

socket_close($socket);

?>
