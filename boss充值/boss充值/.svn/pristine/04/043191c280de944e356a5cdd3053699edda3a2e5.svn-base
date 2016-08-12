#! /usr/bin/php5

<?php

//------------------
//conifg
$mobile=13917184345;
$message="seer_bean_mq_too_many!";
$verify = "098f6bcd4621d373cade4e832627b4f6";

$sms_url = "http://10.1.1.5:8080/send_msg?mobile=$mobile&msg=$message&sign=$verify";

$mq_cmd = "/home/landry/tool/mq/get_mq_msg_count";

//------------------
//
$argc = $_SERVER["argc"];
$argv = $_SERVER["argv"];

if ($argc != 2)
{
	echo "usage: ".$argv[0]." mq_name\n";
	exit();
}

$mq_name = $argv[1];

//----------------------
//init log dir
if(!file_exists('logs'))
{
	if(!mkdir('logs'))
	{
		echo("unable to create logs dir!\n");
		exit();
	}
}

//----------------------
//signal set
declare(ticks = 1);

pcntl_signal(SIGTERM,SIG_IGN); 
pcntl_signal(SIGQUIT,SIG_IGN);
pcntl_signal(SIGHUP,SIG_IGN);
pcntl_signal(SIGPIPE,SIG_IGN);


//------------------
//
while (true)
{
	$log_file_name = 'logs/'.date("Y-m-d").".log";
	ini_set ("error_log",$log_file_name);

	$curr_hour = (int) date("G");
	if ($curr_hour >= 7 && $curr_hour <= 23)
	{
		$cmd = "$mq_cmd $mq_name"; 
		$cmd_status = -1;
		unset($cmd_output);

		$result = exec($cmd, $cmd_output, $cmd_status);
		if ($cmd_status != 0)
		{
				error_log("error:result=$result,cmd_output=$cmd_output");
				sleep(600); //10 minutes
				continue;
		}
		else
		{
				$count = (int)$cmd_output[0];
				if ($count >= 4196)
				{
						echo("$sms_url");
						error_log("=======$count");
						$ctx = stream_context_create(array('http' => array('timeout' => 10)));
						$return_msg = @file_get_contents($sms_url, 0, $ctx);
						error_log("return_msg:$return_msg");
						sleep(3600); //1 hour
				}
				else
				{
						sleep(10); //10 seconds
				}
		}
	}
	else
	{
		sleep(600); //10 minutes
	}
}
	
?>
