<?php
require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);
log_trace(var_export($_GET, true));
log_trace(var_export($_POST, true));
log_trace(var_export(file_get_contents("php://input"), true));
log_trace(var_export($_SERVER['HTTP_USER_AGENT'], true));

echo('{"ErrorCode":"1","ErrorDesc":"接收成功"}');
