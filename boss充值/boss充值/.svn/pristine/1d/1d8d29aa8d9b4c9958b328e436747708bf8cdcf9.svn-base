<?php

require_once 'socket_handler.class.php';

class apple_iap_sys {

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
    const APPLE_PREORDER_CMD_ID = 0xA048;

    /** @brief 通过邮箱名得到米米号
      * @param email char[64]

      */
    public function apple_preorder($user_id, $product_id, $ud) {
        $req_body = pack('LA64', $product_id, $ud);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::APPLE_PREORDER_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Ltrans_id/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const RECEIPT_VERIFY_CMD_ID = 0xA049;

    /** @brief 添加邮箱到米米号映射
      * @param email char[64]

      */
    public function receipt_verify($user_id, $trans_id, $purchase_data, $data_sig) {
        $req_body = pack('Lsa'.strlen($purchase_data).'sa'.strlen($data_sig), $trans_id, strlen($purchase_data), $purchase_data, strlen($data_sig), $data_sig);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::RECEIPT_VERIFY_CMD_ID, 0, $user_id);
        $send_buf = $req_header . $req_body;

        $ret = $this->_sock->send2($this->_ip, $this->_port, $send_buf);
        if (empty($ret) || strlen($ret) < 18) {
            return false;
        }

        $ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);

        return $res_header;
    }
}

?>
