<?php

require_once 'socket_handler.class.php';

class anypay_iap_sys {

    private $_ip;
    private $_port;
    private $_sock;

    public function __construct($ip, $port) {
        $this->_ip = $ip;
        $this->_port = $port;
        $this->_sock = new socket_handler();
    }

    public function get_version()
    {
        $req_header = pack('LLSLL', 18, 0, 4096, 0, 0);
        $send_buf = $req_header;

        $ret = $this->_sock->send2($this->_ip, $this->_port, $send_buf);
        if (empty($ret) || strlen($ret) < 18) {
            return false;
        }

        $ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
        $res_body = array();
        if ($res_header['pkg_len'] > 18) {
            $res_body = unpack('a256version', $ack_body);
        }

        return array_merge($res_header, $res_body);

    }



    ///////////////////////////////////////////
    const ANYPAY_PREORDER_CMD_ID = 0xA047;

    public function anypay_preorder($product_id,$costs,$ud,$user_id) {
        $req_body = pack('A64LA64', $product_id,$costs,$ud);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::ANYPAY_PREORDER_CMD_ID, 0, $user_id);
        $send_buf = $req_header . $req_body;

        $ret = $this->_sock->send2($this->_ip, $this->_port, $send_buf);
        if (empty($ret) || strlen($ret) < 18) {
            return false;
        }

        $ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
        $res_body = array();
        if ($res_header['pkg_len'] > 18) {
            $res_body = unpack('A64product_id/Lcosts/Lpreorder_id', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const ANYPAY_NOTIFY_CMD_ID = 0xA048;

        public function receipt_verify($transdata,$user_id) {
        $req_body = pack('Sa'.(strlen($transdata)+1),strlen($transdata)+1,$transdata);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::ANYPAY_NOTIFY_CMD_ID, 0, $user_id);
        $send_buf = $req_header . $req_body;

        $ret = $this->_sock->send2($this->_ip, $this->_port, $send_buf);
        if (empty($ret) || strlen($ret) < 18) {
            return false;
        }

        $ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
        $res_body = array();
        if ($res_header['pkg_len'] > 18) {
            $res_body = unpack('cverify_result/Ltrans_id', $ack_body);
        }
        
        return array_merge($res_header, $res_body);
    }
}

?>
