#!/bin/sh 
mysql_user=
mysql_passwd=

mysql -u$mysql_user -p$mysql_passwd -e "CREATE DATABASE IF NOT EXISTS db_c_ipay DEFAULT CHARSET utf8; USE db_c_ipay;CREATE TABLE IF NOT EXISTS ipay_log_table (id int(11) unsigned NOT NULL AUTO_INCREMENT,uid int(11) NOT NULL DEFAULT 0,ud varchar(64) NOT NULL,store_id int(11) NOT NULL,price int(11) NOT NULL DEFAULT 0,status int(11) NOT NULL DEFAULT -1,preorder_time timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,trans_time varchar(20) NOT NULL DEFAULT '',golds int(11) NOT NULL DEFAULT 0,beansys_trans_id int(11) NOT NULL DEFAULT 0,deal_time timestamp NULL DEFAULT NULL,transno varchar(11) NOT NULL DEFAULT '',PRIMARY KEY (id),KEY uid (uid)) ENGINE=InnoDB DEFAULT CHARSET=utf8;"
