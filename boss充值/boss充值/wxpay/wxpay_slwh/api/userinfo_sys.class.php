<?php

require_once 'socket_handler.class.php';

class userinfo_sys {

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
    const USERINFO_DEL_USER_CMD_ID = 0x0104;

    /** @brief 删除用户

      */
    public function userinfo_del_user($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_DEL_USER_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHECK_EXISTED_CMD_ID = 0x0005;

    /** @brief 检查米米号用户是否有记录

      */
    public function userinfo_check_existed($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHECK_EXISTED_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_LOGIN_EX_CMD_ID = 0x0006;

    /** @brief 用户登录
      * @param passwd char[16]

      */
    public function userinfo_login_ex($user_id, $passwd) {
        $req_body = pack('a16', $passwd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_LOGIN_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_GAMEFLAG_CMD_ID = 0x0007;

    /** @brief 获取用户注册了哪些游戏

      */
    public function userinfo_get_gameflag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_GAMEFLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ADD_GAME_CMD_ID = 0x0108;

    /** @brief 添加用户注册过的游戏id
      * @param gameid_flag uint32

      */
    public function userinfo_add_game($user_id, $gameid_flag) {
        $req_body = pack('L', $gameid_flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ADD_GAME_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHANGE_PASSWD_CMD_ID = 0x0109;

    /** @brief 用户修改密码
      * @param oldpasswd char[16]
      * @param newpasswd char[16]

      */
    public function userinfo_change_passwd($user_id, $oldpasswd, $newpasswd) {
        $req_body = pack('a16a16', $oldpasswd, $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHANGE_PASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SU_SET_PASSWD_CMD_ID = 0x010A;

    /** @brief 客服修改登录密码
      * @param passwd char[16]

      */
    public function userinfo_su_set_passwd($user_id, $passwd) {
        $req_body = pack('a16', $passwd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SU_SET_PASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SU_SET_PAYPASSWD_CMD_ID = 0x010B;

    /** @brief 客服修改支付密码
      * @param passwd char[16]

      */
    public function userinfo_su_set_paypasswd($user_id, $passwd) {
        $req_body = pack('a16', $passwd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SU_SET_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_LOG_WITH_STAT_CMD_ID = 0x000C;

    /** @brief 用户登入，记录统计日志
      * @param passwd char[16]
      * @param ip uint32
      * @param gameid uint32

      */
    public function userinfo_log_with_stat($user_id, $passwd, $ip, $gameid) {
        $req_body = pack('a16LL', $passwd, $ip, $gameid);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_LOG_WITH_STAT_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_LOG_FOR_BLW_CMD_ID = 0x000D;

    /** @brief 哈奇小镇登录
      * @param passwd char[16]
      * @param ip uint32

      */
    public function userinfo_log_for_blw($user_id, $passwd, $ip) {
        $req_body = pack('a16L', $passwd, $ip);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_LOG_FOR_BLW_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/a64email/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_DEL_GAME_CMD_ID = 0x0110;

    /** @brief 清除用户注册的游戏id
      * @param gameid_flag uint32

      */
    public function userinfo_del_game($user_id, $gameid_flag) {
        $req_body = pack('L', $gameid_flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_DEL_GAME_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_MEEFAN_GAMEFLAG_CMD_ID = 0x0011;

    /** @brief 获取用户米饭标识

      */
    public function userinfo_get_meefan_gameflag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_MEEFAN_GAMEFLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lmee_fans/Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PASSWDEMAIL_CMD_ID = 0x0117;

    /** @brief 设置密保邮箱
      * @param email char[64]

      */
    public function userinfo_set_passwdemail($user_id, $email) {
        $req_body = pack('a64', $email);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PASSWDEMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ISSET_PASSWDEMAIL_CMD_ID = 0x0018;

    /** @brief 是否设置过密保邮箱

      */
    public function userinfo_isset_passwdemail($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ISSET_PASSWDEMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lis_seted/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ISCONFIRM_PASSWDEMAIL_CMD_ID = 0x0019;

    /** @brief 是否确认过密码邮箱

      */
    public function userinfo_isconfirm_passwdemail($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ISCONFIRM_PASSWDEMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lis_seted/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CONFIRM_PASSWDEMAIL_CMD_ID = 0x011A;

    /** @brief 确认密码邮箱

      */
    public function userinfo_confirm_passwdemail($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CONFIRM_PASSWDEMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_PASSWDEMAIL_CMD_ID = 0x001B;

    /** @brief 获取密保邮箱

      */
    public function userinfo_get_passwdemail($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_PASSWDEMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('a64email/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_PASSWD_HISTORY_CMD_ID = 0x001C;

    /** @brief 获取3次历史密码记录

      */
    public function userinfo_get_passwd_history($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_PASSWD_HISTORY_CMD_ID, 0, $user_id);
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
            $res_body = unpack('a16passwd1/Llogtime1/a16passwd2/Llogtime2/a16passwd3/Llogtime3/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PASSWDEMAIL_EX_CMD_ID = 0x011D;

    /** @brief 设置密保邮箱
      * @param email char[64]

      */
    public function userinfo_set_passwdemail_ex($user_id, $email) {
        $req_body = pack('a64', $email);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PASSWDEMAIL_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHANGE_PASSWD_NOCHECK_CMD_ID = 0x0140;

    /** @brief 设置密码，不检查原来的密码
      * @param newpasswd char[16]

      */
    public function userinfo_change_passwd_nocheck($user_id, $newpasswd) {
        $req_body = pack('a16', $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHANGE_PASSWD_NOCHECK_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_USER_BIRTHDAY_SEX_CMD_ID = 0x0141;

    /** @brief 设置用户生日和性别
      * @param birthday uint32
      * @param sex uint32

      */
    public function userinfo_set_user_birthday_sex($user_id, $birthday, $sex) {
        $req_body = pack('LL', $birthday, $sex);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_USER_BIRTHDAY_SEX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_IS_SET_USER_BIRTHDAY_CMD_ID = 0x0042;

    /** @brief 用户是否设置够生日

      */
    public function userinfo_is_set_user_birthday($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_IS_SET_USER_BIRTHDAY_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lis_existed/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_USER_EMAIL_CMD_ID = 0x0143;

    /** @brief 更新用户邮箱
      * @param email char[64]

      */
    public function userinfo_set_user_email($user_id, $email) {
        $req_body = pack('a64', $email);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_USER_EMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_USER_EMAIL_CMD_ID = 0x0044;

    /** @brief 获取登录邮箱

      */
    public function userinfo_get_user_email($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_USER_EMAIL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('a64email/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_SET_PASSWD_FLAG_CMD_ID = 0x0045;

    /** @brief 得到设置密码的FLAG

      */
    public function userinfo_get_set_passwd_flag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_SET_PASSWD_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_FLAG_CHANGE_PASSWD_CMD_ID = 0x015A;

    /** @brief 设置flag1 可以不用检查old passwd.(一次),与userinfo_change_passwd_nocheck 一起使用
      * @param flag uint32

      */
    public function userinfo_set_flag_change_passwd($user_id, $flag) {
        $req_body = pack('L', $flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_FLAG_CHANGE_PASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PASSWD_EX_CMD_ID = 0x015B;

    /** @brief 设置密码
      * @param passwd_type uint32
      * @param passwd_change_way uint32
      * @param newpasswd char[16]

      */
    public function userinfo_set_passwd_ex($user_id, $passwd_type, $passwd_change_way, $newpasswd) {
        $req_body = pack('LLa16', $passwd_type, $passwd_change_way, $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PASSWD_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_CHANGE_PASSWD_FLAG_EX_CMD_ID = 0x015C;

    /** @brief 设置flag1对应位
      * @param passwd_type uint32

      */
    public function userinfo_set_change_passwd_flag_ex($user_id, $passwd_type) {
        $req_body = pack('L', $passwd_type);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_CHANGE_PASSWD_FLAG_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PASSWD_ONLY_CHECK_CHANGE_FLAG_CMD_ID = 0x015D;

    /** @brief 设置密码
      * @param passwd_type uint32
      * @param passwd_change_way uint32
      * @param newpasswd char[16]

      */
    public function userinfo_set_passwd_only_check_change_flag($user_id, $passwd_type, $passwd_change_way, $newpasswd) {
        $req_body = pack('LLa16', $passwd_type, $passwd_change_way, $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PASSWD_ONLY_CHECK_CHANGE_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_LOGIN_BY_MD5_TWO_CMD_ID = 0x005F;

    /** @brief 用户登录用两次MD5后的验证
      * @param passwd_md5_two char[16]
      * @param ip uint32
      * @param gameid uint32

      */
    public function userinfo_login_by_md5_two($user_id, $passwd_md5_two, $ip, $gameid) {
        $req_body = pack('a16LL', $passwd_md5_two, $ip, $gameid);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_LOGIN_BY_MD5_TWO_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_MEE_FANS_CMD_ID = 0x0060;

    /** @brief 用户是否是米饭

      */
    public function userinfo_get_mee_fans($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_MEE_FANS_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lmee_fans/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_MEE_FANS_CMD_ID = 0x0161;

    /** @brief 设置用户米饭标识
      * @param mee_fans uint32

      */
    public function userinfo_set_mee_fans($user_id, $mee_fans) {
        $req_body = pack('L', $mee_fans);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_MEE_FANS_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_QUESTION_CMD_ID = 0x0070;

    /** @brief 获取一代密保问题

      */
    public function userinfo_get_question($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_QUESTION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('a64question/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_QUESTION2_CMD_ID = 0x0171;

    /** @brief 设置二代密保
      * @param set_time uint32
      * @param question_1 char[64]
      * @param question_2 char[64]
      * @param question_3 char[64]
      * @param answer_1 char[64]
      * @param answer_2 char[64]
      * @param answer_3 char[64]

      */
    public function userinfo_set_question2($user_id, $set_time, $question_1, $question_2, $question_3, $answer_1, $answer_2, $answer_3) {
        $req_body = pack('La64a64a64a64a64a64', $set_time, $question_1, $question_2, $question_3, $answer_1, $answer_2, $answer_3);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_QUESTION2_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_QUESTION2_CMD_ID = 0x0072;

    /** @brief 获取二代密保

      */
    public function userinfo_get_question2($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_QUESTION2_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lset_time/a64question_1/a64question_2/a64question_3/a64answer_1/a64answer_2/a64answer_3/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_VERIFY_QUESTION2_CMD_ID = 0x0073;

    /** @brief 验证二代密保
      * @param answer_1 char[64]
      * @param answer_2 char[64]
      * @param answer_3 char[64]

      */
    public function userinfo_verify_question2($user_id, $answer_1, $answer_2, $answer_3) {
        $req_body = pack('a64a64a64', $answer_1, $answer_2, $answer_3);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_VERIFY_QUESTION2_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_QUESTION2_SET_FLAG_CMD_ID = 0x0074;

    /** @brief 用户是否设置过二代密码

      */
    public function userinfo_get_question2_set_flag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_QUESTION2_SET_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lis_seted/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_INFO_EX_CMD_ID = 0x0080;

    /** @brief 获取用户资料

      */
    public function userinfo_get_info_ex($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_INFO_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lflag/Lbirthday/a16telephone/a16mobile/Lmail_number/Saddr_province/Saddr_city/a64addr/a64interest/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_INFO_EX_CMD_ID = 0x0181;

    /** @brief 设置用户资料
      * @param flag uint32
      * @param birthday uint32
      * @param telephone char[16]
      * @param mobile char[16]
      * @param mail_number uint32
      * @param addr_province uint16
      * @param addr_city uint16
      * @param addr char[64]
      * @param interest char[64]

      */
    public function userinfo_set_info_ex($user_id, $flag, $birthday, $telephone, $mobile, $mail_number, $addr_province, $addr_city, $addr, $interest) {
        $req_body = pack('LLa16a16LSSa64a64', $flag, $birthday, $telephone, $mobile, $mail_number, $addr_province, $addr_city, $addr, $interest);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_INFO_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_INFO_ALL_CMD_ID = 0x0082;

    /** @brief 获取用户所有的资料

      */
    public function userinfo_get_info_all($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_INFO_ALL_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lflag1/Lregflag/a16passwd/a16paypasswd/a64email/a64parent_email/a64passwdemail/a64question/a64answer/a128signature/Lflag/Lbirthday/a16telephone/a16mobile/Lmail_number/Saddr_province/Saddr_city/a64addr/a64interest/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PAYINFO_CMD_ID = 0x0183;

    /** @brief 设置支付信息
      * @param parent_email char[64]
      * @param question char[64]
      * @param answer char[64]
      * @param birthday uint32
      * @param telephone char[16]
      * @param mobile char[16]
      * @param mail_number uint32
      * @param addr_province uint16
      * @param addr_city uint16
      * @param addr char[64]
      * @param interest char[64]

      */
    public function userinfo_set_payinfo($user_id, $parent_email, $question, $answer, $birthday, $telephone, $mobile, $mail_number, $addr_province, $addr_city, $addr, $interest) {
        $req_body = pack('a64a64a64La16a16LSSa64a64', $parent_email, $question, $answer, $birthday, $telephone, $mobile, $mail_number, $addr_province, $addr_city, $addr, $interest);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PAYINFO_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHECK_QUESTION_CMD_ID = 0x0084;

    /** @brief 验证一代密保
      * @param question char[64]
      * @param answer char[64]

      */
    public function userinfo_check_question($user_id, $question, $answer) {
        $req_body = pack('a64a64', $question, $answer);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHECK_QUESTION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ISSET_QUESTION_CMD_ID = 0x0085;

    /** @brief 验证一代密保是否设置过

      */
    public function userinfo_isset_question($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ISSET_QUESTION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_SECURITY_FLAG_CMD_ID = 0x0086;

    /** @brief 获取安全flag

      */
    public function userinfo_get_security_flag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_SECURITY_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PEMAIL_BY_QA_CMD_ID = 0x0187;

    /** @brief 根据一代密保信息设置父母邮箱
      * @param parent_email char[64]
      * @param question char[64]
      * @param answer char[64]

      */
    public function userinfo_set_pemail_by_qa($user_id, $parent_email, $question, $answer) {
        $req_body = pack('a64a64a64', $parent_email, $question, $answer);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PEMAIL_BY_QA_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PEMAIL_ISOK_CMD_ID = 0x0188;

    /** @brief 设置flag1父母邮箱

      */
    public function userinfo_set_pemail_isok($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PEMAIL_ISOK_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SYNC_DV_CMD_ID = 0x018A;

    /** @brief 同步信息到论坛（仅台湾版）

      */
    public function userinfo_sync_dv($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SYNC_DV_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHECK_PAYPASSWD_CMD_ID = 0x008B;

    /** @brief 验证支付密码
      * @param paypasswd char[16]

      */
    public function userinfo_check_paypasswd($user_id, $paypasswd) {
        $req_body = pack('a16', $paypasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHECK_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PAYPASSWD_CMD_ID = 0x018C;

    /** @brief 设置支付密码
      * @param paypasswd char[16]

      */
    public function userinfo_set_paypasswd($user_id, $paypasswd) {
        $req_body = pack('a16', $paypasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ISSET_PAYPASSWD_CMD_ID = 0x008D;

    /** @brief 用户是否设置过支付密码

      */
    public function userinfo_isset_paypasswd($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ISSET_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lis_seted/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_QA_CMD_ID = 0x018E;

    /** @brief 设置一代密保
      * @param question char[64]
      * @param answer char[64]

      */
    public function userinfo_set_qa($user_id, $question, $answer) {
        $req_body = pack('a64a64', $question, $answer);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_QA_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CLEAN_PAYPASSWD_CMD_ID = 0x018F;

    /** @brief 清除支付密码

      */
    public function userinfo_clean_paypasswd($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CLEAN_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_PAYPASSWD_WITH_CHECK_CMD_ID = 0x0190;

    /** @brief 更新支付密码，验证原来的支付密码
      * @param oldpasswd char[16]
      * @param newpasswd char[16]

      */
    public function userinfo_set_paypasswd_with_check($user_id, $oldpasswd, $newpasswd) {
        $req_body = pack('a16a16', $oldpasswd, $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PAYPASSWD_WITH_CHECK_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_LOGIN_INFO_CMD_ID = 0x0091;

    /** @brief 获取登录信息

      */
    public function userinfo_get_login_info($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_LOGIN_INFO_CMD_ID, 0, $user_id);
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
            $res_body = unpack('a16passwd/Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_FALG_CHANGE_PAYPASSWD_CMD_ID = 0x0192;

    /** @brief 设置更改密码flag
      * @param flag uint32

      */
    public function userinfo_set_falg_change_paypasswd($user_id, $flag) {
        $req_body = pack('L', $flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_FALG_CHANGE_PAYPASSWD_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_CHANGE_PAYPASSWD_NOCHECK_CMD_ID = 0x0193;

    /** @brief 更改支付密码不需要任何验证
      * @param new_paypasswd char[16]

      */
    public function userinfo_change_paypasswd_nocheck($user_id, $new_paypasswd) {
        $req_body = pack('a16', $new_paypasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHANGE_PAYPASSWD_NOCHECK_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_SET_USER_ENABLE_FLAG_CMD_ID = 0x0194;

    /** @brief 设置用户冻结标识
      * @param enable_flag uint32

      */
    public function userinfo_set_user_enable_flag($user_id, $enable_flag) {
        $req_body = pack('L', $enable_flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_USER_ENABLE_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_USER_ENABLE_FLAG_CMD_ID = 0x0095;

    /** @brief 获取用户冻结标识

      */
    public function userinfo_get_user_enable_flag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_USER_ENABLE_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lenable_flag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ADD_ACT_FLAG_CMD_ID = 0x0196;

    /** @brief 用户激活游戏
      * @param gameid_flag uint32

      */
    public function userinfo_add_act_flag($user_id, $gameid_flag) {
        $req_body = pack('L', $gameid_flag);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ADD_ACT_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_GAME_ACT_FLAG_CMD_ID = 0x0097;

    /** @brief 获取用户激活哪些游戏

      */
    public function userinfo_get_game_act_flag($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_GAME_ACT_FLAG_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgame_act_flag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_REGISTER_EX_CMD_ID = 0x0103;

    /** @brief 用户注册
      * @param passwd char[16]
      * @param email char[64]
      * @param sex uint32
      * @param mobile char[16]
      * @param birthday uint32
      * @param addr_province uint16
      * @param addr_city uint16
      * @param addr char[64]
      * @param signature char[128]
      * @param reg_addr_type uint32
      * @param ip uint32

      */
    public function userinfo_register_ex($user_id, $passwd, $email, $sex, $mobile, $birthday, $addr_province, $addr_city, $addr, $signature, $reg_addr_type, $ip) {
        $req_body = pack('a16a64La16LSSa64a128LL', $passwd, $email, $sex, $mobile, $birthday, $addr_province, $addr_city, $addr, $signature, $reg_addr_type, $ip);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_REGISTER_EX_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_REGISTER_BY_REAL_NAME_CMD_ID = 0x0198;

    /** @brief 用户实名注册
      * @param passwd char[16]
      * @param email char[64]
      * @param sex uint32
      * @param mobile char[16]
      * @param birthday uint32
      * @param addr_province uint16
      * @param addr_city uint16
      * @param addr char[64]
      * @param signature char[128]
      * @param reg_addr_type uint32
      * @param ip uint32
      * @param real_name char[15]
      * @param personal_id char[18]

      */
    public function userinfo_register_by_real_name($user_id, $passwd, $email, $sex, $mobile, $birthday, $addr_province, $addr_city, $addr, $signature, $reg_addr_type, $ip, $real_name, $personal_id) {
        $req_body = pack('a16a64La16LSSa64a128LLa15a18', $passwd, $email, $sex, $mobile, $birthday, $addr_province, $addr_city, $addr, $signature, $reg_addr_type, $ip, $real_name, $personal_id);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_REGISTER_BY_REAL_NAME_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_LOGIN_BY_MD5_TWO_BY_REGION_CMD_ID = 0x0099;

    /** @brief 按区域登录
      * @param passwd_md5_two char[16]
      * @param ip uint32
      * @param gameid uint32
      * @param region uint32

      */
    public function userinfo_login_by_md5_two_by_region($user_id, $passwd_md5_two, $ip, $gameid, $region) {
        $req_body = pack('a16LLL', $passwd_md5_two, $ip, $gameid, $region);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_LOGIN_BY_MD5_TWO_BY_REGION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_ADD_GAME_BY_REGION_CMD_ID = 0x019A;

    /** @brief 按区域添加用户注册游戏标识
      * @param region uint32
      * @param gameid uint32

      */
    public function userinfo_add_game_by_region($user_id, $region, $gameid) {
        $req_body = pack('LL', $region, $gameid);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_ADD_GAME_BY_REGION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


    ///////////////////////////////////////////
    const USERINFO_GET_MEEFAN_GAME_ACT_FLAG_BY_REGION_CMD_ID = 0x009B;

    /** @brief 按区域得到用户激活了哪些游戏产品和是否米饭
      * @param region uint32

      */
    public function userinfo_get_meefan_game_act_flag_by_region($user_id, $region) {
        $req_body = pack('L', $region);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_MEEFAN_GAME_ACT_FLAG_BY_REGION_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lmee_fans/Lgameflag/', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }

    const USERINFO_GET_REGISTER_INFO_CMD_ID = 0x000E;

    public function userinfo_get_register_info($user_id) {
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_REGISTER_INFO_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Cregister_channel/Lregister_ip/Lregister_time', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }

    const USERINFO_GET_HISTORY_PASSWD_CMD_ID = 0x000F;

    public function userinfo_get_history_passwd($user_id, $passwd_type) {
        $req_body = pack('C', $passwd_type);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_HISTORY_PASSWD_CMD_ID, 0, $user_id);
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
            $tmp = unpack('Lcount', substr($ack_body, 0, 4));
            $count = $tmp['count'];
            $res_body['count'] = $count;

            for ($i = 0; $i < $count; $i++) {
                $item = unpack('a16old_passwd/a16new_passwd/Cchangetype/Lip/Ltime',
                        substr($ack_body, 4 + $i * 41, 41));
                $item['old_passwd'] = bin2hex($item['old_passwd']);
                $item['new_passwd'] = bin2hex($item['new_passwd']);
                $res_body[] = $item;
            }
        }

        return array_merge($res_header, $res_body);
    }

    /** @brief 设置密码
      * @param passwd_type uint32
      * @param passwd_change_way uint32
      * @param newpasswd char[16]

      */

    const USERINFO_SET_PASSWD_ONLY_CHECK_CHANGE_FLAG_WITH_IP_CMD_ID = 0x010E;

    public function userinfo_set_passwd_only_check_change_flag_with_ip($user_id, $passwd_type, $passwd_change_way, $ip, $newpasswd) {
        $req_body = pack('LLLa16', $passwd_type, $passwd_change_way, $ip, $newpasswd);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_PASSWD_ONLY_CHECK_CHANGE_FLAG_WITH_IP_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }

    ///////////////////////////////////////////
    const USERINFO_CHANGE_PASSWD_WITH_IP_CMD_ID = 0x010F;

    /** @brief 用户修改密码
      * @param oldpasswd char[16]
      * @param newpasswd char[16]

      */
    public function userinfo_change_passwd_with_ip($user_id, $oldpasswd, $newpasswd, $ip) {
        $req_body = pack('a16a16L', $oldpasswd, $newpasswd, $ip);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_CHANGE_PASSWD_WITH_IP_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }

    const USERINFO_GET_QUESTION2_USER_DEFINE_CMD_ID     = 0x0012;

    public function userinfo_get_question2_user_define($user_id) {
        $req_body = pack('');
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_GET_QUESTION2_USER_DEFINE_CMD_ID, 0, $user_id);
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
            $res_body = unpack('Lset_time/Cis_user_define1/a64question1/a64answer1/Cis_user_define2/a64question2/a64answer2/Cis_user_define3/a64question3/a64answer3', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }

    const USERINFO_SET_QUESTION2_USER_DEFINE_CMD_ID = 0x0113;

    public function userinfo_set_question2_user_define($user_id, $set_time, $is_user_define, $question1, $answer1, $is_user_define2, $question2, $answer2, $is_user_define3, $question3, $answer3)
    {
        $req_body = pack('LCa64a64Ca64a64Ca64a64', $set_time, $is_user_define, $question1, $answer1, $is_user_define2, $question2, $answer2, $is_user_define3, $question3, $answer3);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0,
                self::USERINFO_SET_QUESTION2_USER_DEFINE_CMD_ID, 0, $user_id);
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
            $res_body = unpack('', $ack_body);
        }

        return array_merge($res_header, $res_body);
    }


}

?>
