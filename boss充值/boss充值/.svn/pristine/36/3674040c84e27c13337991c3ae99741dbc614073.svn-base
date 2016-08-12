<?php
    require_once 'socket_handler.class.php';
    $_sock = new socket_handler();
    $ip='192.168.2.5';
    $port=1236;
    $transdata=$_POST['transdata'];
    $sign=$_POST['sign'];

//    $transdata='{"appid":"500000185","count":1,"cporderid":"2","cpprivate":"cpprivateinfo123456","feetype":0,"money":100,"paytype":5,"result":0,"transid":"32011406301831300001","transtime":"2014-06-30 18:31:32","transtype":0,"waresid":1}';
//    $sign='{"appid":"500000185","count":1,"cporderid":"2","cpprivate":"cpprivateinfo123456","feetype":0,"money":100,"paytype":5,"result":0,"transid":"32011406301831300001","transtime":"2014-06-30 18:31:32","transtype":0,"waresid":1}';
    
    $req_body = pack('SSa'.(strlen($sign)+1).'a'.(strlen($transdata)+1), strlen($sign)+1,strlen($transdata)+1, $sign,$transdata);
    $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                       0xA044, 0, 0);
    $send_buf = $req_header . $req_body;
    
    $ret = $_sock->send2($ip, $port, $send_buf);
    if (empty($ret) || strlen($ret) < 18) {
        echo('FAIL');
        return;
    }
    $ack_header = substr($ret, 0, 18);
    $ack_body   = substr($ret, 18);
    $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
    $res_body = array();

    if ($res_header['pkg_len'] > 18) {
        $res_body = unpack('Cverify_result/Ltrans_id', $ack_body);
        if($res_body['verify_result']==0 || $res_body['verify_result']==1 )
        {
            echo('SUCCESS');
            return;
        }
    }else{
        echo('FAIL');
        return;
    }
    echo('FAIL');
    
?>