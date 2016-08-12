<?php

define('IP', '192.168.2.5');
define('PORT', 1236);

require_once 'ipay_iap_sys.class.php';

$sys = new ipay_iap_sys(IP, PORT);

print_r($sys->ipay_preorder(1,100,10000, 'woshiudwoshiudwoshiud',123321));
    print_r($sys->receipt_verify(2, '{"appid":"500000185","count":1,"cporderid":"2","cpprivate":"cpprivateinfo123456","feetype":0,"money":100,"paytype":5,"result":0,"transid":"32011406301831300001","transtime":"2014-06-30 18:31:32","transtype":0,"waresid":1}','{"appid":"500000185","count":1,"cporderid":"2","cpprivate":"cpprivateinfo123456","feetype":0,"money":100,"paytype":5,"result":0,"transid":"32011406301831300001","transtime":"2014-06-30 18:31:32","transtype":0,"waresid":1}'
                                 ,123321));

?>
