mysql_user=root
mysql_passwd=

mysql -u$mysql_user -p$mysql_passwd -e "
CREATE DATABASE IF NOT EXISTS googleplay_db DEFAULT CHARSET utf8; USE googleplay_db;
        CREATE TABLE IF NOT EXISTS googleplay_receipt_log_table (
  id int(11) NOT NULL AUTO_INCREMENT,
  data_sig_hash char(32) NOT NULL DEFAULT '',
  pay_log_id int(11) NOT NULL,
  purchase_data varchar(1024) NOT NULL DEFAULT '',
  data_sig varchar(1024) NOT NULL DEFAULT '',
  time timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  receipt_pdt_id int(11) NOT NULL DEFAULT 0,
  verify_result int(128) NOT NULL DEFAULT -1,
  PRIMARY KEY (id),
  UNIQUE KEY hash (data_sig_hash)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
        CREATE TABLE IF NOT EXISTS googleplay_log_table (
  id int(11) NOT NULL AUTO_INCREMENT,
  uid int(128) NOT NULL DEFAULT 0,
  ud char(64) NOT NULL DEFAULT '',
  product_id int(11) NOT NULL DEFAULT 0,
  preorder_time datetime NOT NULL,
  costs int(128) NOT NULL DEFAULT 0,
  golds int(10) unsigned NOT NULL DEFAULT 0,
  diamonds int(10) unsigned NOT NULL DEFAULT 0,
  receipt_id int(10) unsigned NOT NULL DEFAULT 0,
  trans_time datetime NOT NULL DEFAULT '1970-00-00 00:00:00',
  beansys_trans_id int(10) unsigned NOT NULL DEFAULT 0,
  deal_time datetime NOT NULL DEFAULT '1970-00-00 00:00:00',
  status int(10) NOT NULL DEFAULT 0,
  PRIMARY KEY (id),
  KEY time (trans_time),
  KEY uid_time (uid,trans_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
