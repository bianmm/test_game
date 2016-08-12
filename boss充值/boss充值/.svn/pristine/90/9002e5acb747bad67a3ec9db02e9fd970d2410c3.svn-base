<?php

//head
$pack_length = 18 + 4 ;		

// uncorrect length

// program will block

$cmd_id = 0x1114;
//$cmd_id = 0x0114;

//invalid command id.
//$cmd_id = 5;		

//$user_id = 50001 ;
//$user_id = 50077 ;
$user_id = 50005 ;
//$user_id = 50088 ;
//$user_id = 50002 ;

// Invalid id
//$user_id = 10001 ;

$pack_header=pack("LLSLL",$pack_length,0,$cmd_id,0,$user_id);

//body
//$is_vip = 1 ;
$is_vip = 0 ;

$pack_body = pack("L",$is_vip);

//packge
$ip = "10.1.1.5";
$port = 21001;

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

if(strlen($recved_string) == 18)	//18 is the packege header length.
{
	$return_array = unpack("Lpackage_len/Lseq_num/Scommand_id/Lstatus_code/Luser_id",$recved_string);
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
