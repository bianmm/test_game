<?php

$mb_server_ip = "192.168.0.12";
$mb_server_port = "12201";
$consume_mb_command_id = 2002;

$special_channel_id = 100;
$security_code = "69064820";
$product_id = 801;
$product_count = 1;

//**************************//
$consume_transaction_id = 1;
$account_id = 1380013800;
$mb_num = 10000;
//**************************//

//get input pararms
$argc = $_SERVER['argc'];
$argv = $_SERVER['argv'];

if(isset($argv))
{
    foreach($argv as $argv_item)
    {
	if(strstr($argv_item,'-u'))
	{
	    $account_id = (int)substr($argv_item,2);
	}
	else if(strstr($argv_item,'-m'))
	{
	    $mb_num = (int)substr($argv_item,2);
	}
    }//foreach
}

//echo("account_id = $account_id,mb_num = $mb_num\n");

//pack data
$account_pwd = "";
$packed_body = pack("La16LLSLL",$account_id,$account_pwd,$account_id,$product_id,$product_count,$mb_num,$consume_transaction_id);
$verify_string = "channelId=$special_channel_id&securityCode=$security_code&data=" . $packed_body;
$verify_code = md5($verify_string);

$pack_length = 10 + 34 + 38;
$packed_data = pack("SSSL",$pack_length,$consume_mb_command_id,0,0);
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

$return_array = unpack("Spackage_length/Scommand_id/Sstatus_code/Lseq_num/Ltransaction_id/Lxxxx/Lmb_account_balance",$recved_string);
if($return_array == false)
{
    echo("recved data from mb server error!!!\n");
    exit();
}

$status_code = $return_array['status_code'];
$mb_account_balance = $return_array['mb_account_balance'];
echo("operation complete!\naccount_id = $account_id,mb_adjust = $mb_num\nstatus_code = $status_code,mb_account_balance = $mb_account_balance\n");

socket_close($socket);

?>
