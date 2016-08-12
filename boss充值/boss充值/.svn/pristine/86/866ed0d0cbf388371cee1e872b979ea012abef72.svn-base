<?php

$mb_server_ip = "192.168.0.12";
$mb_server_port = "12201";
$add_mb_command_id = 2001;

$special_channel_id = 99;
$security_code = "33591074";
$private_data_02 = "manual adjust";

//**************************//
$pay_gate_transaction_id = 149;
$account_id = 35004070;
$mb_num = 3000;
//**************************//

//pack data
$packed_body = pack("L4a64",$account_id,$mb_num,$pay_gate_transaction_id,0,$private_data_02);
$verify_string = "channelId=$special_channel_id&securityCode=$security_code&data=" . $packed_body;
$verify_code = md5($verify_string);

$pack_length = 10 + 34 + 80;
$packed_data = pack("SSSL",$pack_length,$add_mb_command_id,0,0);
$packed_data .= pack("Sa32",$special_channel_id,$verify_code);
$packed_data .= $packed_body;

//send to server
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);
if(!$socket)
{
    echo("create socket failed!!!\n");
    exit();
}

if(!socket_connect($socket,$mb_server_ip,$mb_server_port))
{
    echo("connect to mb server failed!!!\n");
    exit();
}

$result = socket_write($socket,$packed_data,strlen($packed_data));
if($result === false)
{
    echo("send data to mb server failed!!!\n");
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
    echo("recved data from mb server failed!!!!\n");
    exit();
}

$return_array = unpack("Spackage_length/Scommand_id/Sstatus_code/Lseq_num/Qtransaction_id",$recved_string);
if($return_array == false)
{
    echo("recved data from mb server error!!!\n");
    exit();
}

$status_code = $return_array['status_code'];
echo("operation complete! account_id = $account_id,mb_mum = $mb_num,status_code = $status_code\n");

socket_close($socket);

?>
