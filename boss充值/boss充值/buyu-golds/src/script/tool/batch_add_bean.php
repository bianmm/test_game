<?php

require_once('awk.php');

$data_table = awk("source.txt",' ');
if($data_table === false)
{
    echo("failed to get data table!\n");
    exit();
}

$external_trans_id = 1;   //Next external_transaction_id
foreach($data_table as $data_row)
{
    $account_id = (int)$data_row[0];
    $bean_num = (int)$data_row[1]; //注意：这里的值为乘以100之前的值

    $cmd = "php add_bean.php $account_id $bean_num $external_trans_id";
    echo(system($cmd));
    echo("\n");

    $external_trans_id++;
}

echo(">>>>>>>process end,NEXT external_trans_id = $external_trans_id<<<<<<<<\n");

?>
