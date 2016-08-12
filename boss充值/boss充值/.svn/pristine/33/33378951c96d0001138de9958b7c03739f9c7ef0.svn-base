<?php
    require_once ("socket_handler.class.php");
    $_sock = new socket_handler();
    $ip='192.168.2.207';
    $port=12000;
    
    $transdata = json_encode($_REQUEST);
    if(strlen($transdata) == 0)
    {
        echo('FAIL1');
        return;
    }
    $req_body = pack('Sa'.(strlen($transdata)+1), strlen($transdata)+1,$transdata);
    $req_header = pack('LLSLL', 18 + strlen($req_body), 0,0xA046, 0, 0);
    $send_buf = $req_header . $req_body;
    
    $ret = $_sock->connect($ip, $port);
    $ret = $_sock->send($send_buf);
    if (empty($ret) || strlen($ret) < 18) {
        echo('FAIL1');
        return;
    }
    $ret = $_sock->receive();
    $_sock->close();
    $ack_header = substr($ret, 0, 18);
    $ack_body   = substr($ret, 18);
    $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
    $res_body = array();
    if ($res_header['pkg_len'] > 18) {
        $res_body = unpack('Cverify_result/Ltrans_id', $ack_body);
        if($res_body['verify_result']==0 || $res_body['verify_result']==1 )
        {
            echo('ok');
            return;
        }
    }else{
        echo('FAIL2');
        return;
    }
    echo('FAIL3');
    