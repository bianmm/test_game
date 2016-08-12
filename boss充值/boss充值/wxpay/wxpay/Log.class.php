<?php
/**
 * @file Log.class.php
 * @brief functions & class for logging
 * @author Yuan B.J. the_guy_1987@hotmail.com
 * @version 2.0
 * @date 2011-03
 */

function log_init($dir, $prefix, $level = Log::TYPE_INFO, $max_size = NULL)
{
    if (!Log::init($dir, $prefix))
    {
        return false;
    }

    if (!empty($max_size))
    {
        if ($max_size <= 0)
        {
            return false;
        }
        else
        {
            Log::set_max_size($max_size);
        }
    }

    log_level($level);
    return true;
}

function log_level()
{
    static $level = Log::TYPE_INFO;

    if (func_num_args() > 0)
    {
        $level = func_get_arg(0);
    }

    return $level;
}

function log_debug()
{
    if (log_level() < Log::TYPE_DEBUG)
    {
        return;
    }

    $args = func_get_args();
    Log::write($args, 'debug');
}

function log_error()
{
    if (log_level() < Log::TYPE_ERROR)
    {
        return;
    }

    $args = func_get_args();
    Log::write($args, 'error');
}

function log_trace()
{
    if (log_level() < Log::TYPE_TRACE)
    {
        return;
    }

    $args = func_get_args();
    Log::write($args, 'trace');
}

function log_info()
{
    if (log_level() < Log::TYPE_INFO)
    {
        return;
    }

    $args = func_get_args();
    Log::write($args, 'info');
}

class Log
{
    private static $dir     = '';
    private static $prefix  = '';
    private static $path_partial = '';
    private static $max_size = 31457280; /// 30m
    private static $serial_no = array();
    private static $log_date = '';

    /**
     * @brief set max size of a single file
     * @access public
     *
     * @param $max_size max size of a sigle file
     */
    public static function set_max_size($max_size)
    {
        self::$max_size = $max_size;
    }

    /**
     * @brief init the Log class
     * @access public
     *
     * @param $dir      where to log
     * @param $prefix   prefix of log file name
     *
     * @return true-success false-failure
     */
    public static function init($dir, $prefix)
    {
        if (empty($dir) || empty($prefix))
        {
            return false;
        }

        // if the dir not exists, mkdir for it
        if ((!file_exists($dir)) && (!mkdir($dir)))
        {
            return false;
        }

        self::$dir = $dir;
        self::$prefix = $prefix;
        self::$path_partial = self::$dir . self::$prefix;
        return true;
    }

    const TYPE_ERROR = 0;
    const TYPE_DEBUG = 1;
    const TYPE_TRACE = 2;
    const TYPE_INFO  = 3;

    /**
     * @brief do log
     * @access public
     *
     * @param $args parameters for logging
     * @param $type type of logging
     *
     * @return true-success false-failure
     */
    public static function write($args, $type)
    {
        if (empty($args))
        {
            return false;
        }

        // gen the file name to log
        if (self::$log_date != date('Y-m-d'))
        {
            self::$log_date = date('Y-m-d');
            self::$serial_no[$type] = 0;
        }

        if (!array_key_exists($type, self::$serial_no))
        {
            self::$serial_no[$type] = 0;
        }

        $path = self::$path_partial
                . '_' . $type
                . '_' . self::$log_date
                . '_' . self::$serial_no[$type] . '.log';

        clearstatcache();

        while (file_exists($path)
               && (filesize($path) >= self::$max_size))
        {
            ++ self::$serial_no[$type];
            $path = self::$path_partial
                    . '_' . $type
                    . '_' . self::$log_date
                    . '_' . self::$serial_no[$type] . '.log';
        }

        $content = @vsprintf(array_shift($args) . "\n", $args);
        return error_log('[' . date('Ymd H:i:s') . ']' . $content, 3, $path);
    }
}
