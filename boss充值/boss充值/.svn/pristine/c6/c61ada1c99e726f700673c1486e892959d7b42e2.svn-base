<?php
require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);
log_trace(var_export($_GET, true));
log_trace(var_export($_POST, true));

echo('SUCCESS');
