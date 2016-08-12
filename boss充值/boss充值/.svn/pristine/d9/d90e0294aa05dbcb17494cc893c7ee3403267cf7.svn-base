<?php
/**
 * @file socket_handler.class.php
 * @brief 网络套接字处理类
 * @author baron baron@taomee.com
 * @version 1.0
 * @date 2010-04
 */

class socket_handler
{
    /**
     * @brief socket对象
     * @access private
     */
    private $socket = NULL;

    /**
     * @brief 判断目标地址端口是否可用
     *
     * @access public
     * @param $host 目标IP
     * @param $port 目标端口
     *
     * @return true-成功 false-失败
     */
    public function ping($host, $port)
    {
        $result = $this->connect($host, $port);
        $this->close();

        return $result;
    }

    /**
     * @brief 链接目标地址
     *
     * @access public
     * @param $host 目标IP
     * @param $port 目标端口
     *
     * @return true-成功 false-失败
     */
    public function connect($host, $port)
    {
        // 创建socket
        $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

        if (false == $this->socket)
        {
            return false;
        }

        // 链接主机
        $result = @socket_connect($this->socket, $host, $port);

        if (!$result)
        {
            socket_close($this->socket);
            return false;
        }

        return true;
    }

    function __destruct()
    {
        $this->close();
    }

    /**
     * @brief 关闭socket连接
     *
     * @access public
     *
     * @return
     */
    public function close()
    {
        if (is_resource($this->socket))
        {
            socket_close($this->socket);
            $this->socket = NULL;
        }
    }

    /**
     * @brief 发送数据
     *
     * @access public
     * @param $data 欲发送的字符串数据
     *
     * @return true-成功 false-失败
     */
    public function send($data)
    {
        //echo "send length = " . strlen($data) . "\n";
        $dump_str = array();
        for ($i = 0; $i < strlen($data); $i++) {
            $first = dechex(ord($data[$i]) >> 4);
            $second = dechex(ord($data[$i]) & 0xF);
            $dump_str[] = "$first" . "$second";
        }
        //echo implode(' ', $dump_str) . "\n";
        $result = @socket_write($this->socket, $data, strlen($data));

        if (strlen($data) != $result)
        {
            return false;
        }

        return true;
    }

    /**
     * @brief 接收数据
     *
     * @access public
     *
     * @return false-失败 否则-收到的数据
     */
    public function receive()
    {
        $data = @socket_read($this->socket, 1 << 16, PHP_BINARY_READ);

        if (false == $data || strlen($data) <= 0)
        {
            return false;
        }

        return $data;
    }

    /**
     * @brief 一步直接发送数据
     *
     * @access public
     * @param $host 主机IP
     * @param $port 主机PORT
     * @param $data 欲发送的数据
     *
     * @return fasle-成功 否则-收到的数据
     */
    public function send2($host, $port, $data)
    {
        $result = @$this->connect($host, $port);

        if (false === $result)
        {
            return false;
        }

        $result = $this->send($data);

        if (false === $result)
        {
            return false;
        }

        $result = $this->receive();

        if (false === $result)
        {
            return false;
        }

        $this->close();

        return $result;
    }
}
