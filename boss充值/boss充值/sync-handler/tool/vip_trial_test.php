<?php

//head
$pack_length = 18 + 40 ;		

// uncorrect length
//$pack_length = 18 + 39 ;		

// program will block

$cmd_id = 0x2007;
//$cmd_id = 0x0114;

//invalid command id.
//$cmd_id = 5;		

$user_id = 50055; //default user id
if ($_SERVER["argc"]>1)
{
	$user_id = (int)$_SERVER["argv"][1] ;
}

echo "user_id:$user_id\n";

$pack_header=pack("LLSLL",$pack_length,0,$cmd_id,0,$user_id);

//body
$client_ip = ip2long("10.1.1.19") ;

$channel_id = 90;
//$channel_id = -1; //invalid value
$security_code ="123456789";
$verify_code = md5("channelID=$channeld_id&securityCode=$security_code&data=$client_ip");

$pack_body = pack("La32L",$channel_id,$verify_code,$client_ip);

//packge
$ip = "10.1.1.5";
$port = 12208;
//$port = 57009;

$pack_data = $pack_header;
$pack_data .= $pack_body;

//send to server
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);
if(!$socket)
{
    echo("create socket failed!!!\n");
    exit();
}

if(!socket_connect($socket,$ip,$port))
{
    echo("connect to mb server failed!!!\n");
    exit();
}

$result = socket_write($socket,$pack_data,strlen($pack_data));
if($result === false)
{
    echo("send data to mb server failed!!!\n");
    exit();
}

if($result != strlen($pack_data))
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

if(strlen($recved_string) == (18+8))	//18 is the packege header length.
{
	$return_array = unpack("Lpackage_len/Lseq_num/Scommand_id/Lstatus_code/Luser_id/Lop_id/Lresult",$recved_string);
	if($return_array == false)
	{
	    echo("unpack data from mb server error!!!\n");
	    exit();
	}
}
else
{
	    echo("unpack data from mb server error!!!\n");
	    exit();
}

print_r($return_array);

?>
