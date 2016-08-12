<?php

require_once('awk.php');

$data_table = awk("source.txt",' ');
if($data_table === false)
{
    echo("failed to get data table!\n");
    exit();
}

$pay_gate_trans_id = 354;   //Next usable pay_gate_transaction_id
foreach($data_table as $data_row)
{
    $account_id = (int)$data_row[0];
    $mb_num = (int)$data_row[1];

    $cmd = "php add_mb_ex.php $account_id $mb_num $pay_gate_trans_id";
    echo(system($cmd));
    echo("\n");

    $pay_gate_trans_id++;
}

echo(">>>>>>>is's end,pay_gate_trans_id = $pay_gate_trans_id<<<<<<<<\n");

?>
