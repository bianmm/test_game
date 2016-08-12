<?php

require_once 'socket_handler.class.php';

class ipay_iap_sys {

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
    const IPAY_PREORDER_CMD_ID = 0xA044;

    public function ipay_preorder($zoneid,$price,$storeid, $ud,$user_id) {
        $req_body = pack('L3A64', $zoneid,$price,$storeid, $ud);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::IPAY_PREORDER_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lzoneid/Lprice/Lstoreid/Ltrans_id/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const IPAY_NOTIFY_CMD_ID = 0xA045;

        public function receipt_verify($trans_id, $sign,$transdata,$user_id) {
        $req_body = pack('LSSa'.(strlen($transdata)+1), $trans_id,strlen($transdata)+1,$transdata);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::IPAY_NOTIFY_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lverify_result/Ctrans_id', $ack_body);
        }
        
        return array_merge($res_header, $res_body);
    }
}

?>
