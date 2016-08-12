<?php

define('IP', '192.168.1.91');
define('PORT', 21001);

require_once 'db_sys.class.php';

$sys = new db_sys(IP, PORT);
$rst = $sys->get_user_role(977557);
var_export($rst);
