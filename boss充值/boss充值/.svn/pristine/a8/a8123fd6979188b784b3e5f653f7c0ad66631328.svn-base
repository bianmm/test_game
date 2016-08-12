#!/bin/sh
mysql_user=
mysql_passwd=

mysql -u$mysql_user -p$mysql_passwd -e "set CHARSET utf8; USE db_anypay; CREATE TABLE IF NOT EXISTS anypay_log_table (\
order_id int(11) unsigned NOT NULL AUTO_INCREMENT,\
extern_order_id int(11) unsigned NOT NULL,\
beansys_trans_id int(11) unsigned NOT NULL,\
channel_id char(10)  DEFAULT '',\
pay_channel_id char(10)  DEFAULT '',\
\
product_id char(64) NOT NULL DEFAULT '',\
product_name char(64) DEFAULT '',\
product_count int(11) NOT NULL DEFAULT 1,\
\
costs char(20)  DEFAULT '',\
pay_time char(30)  DEFAULT '',\
pay_status int(11) NOT NULL DEFAULT 0,\
deal_time timestamp NOT NULL DEFAULT 0,\
channel_user_id char(10) DEFAULT '',\
game_user_id int(11) NOT NULL DEFAULT 0,\
preorder_time timestamp  DEFAULT 0,\
ud char(64) NOT NULL DEFAULT '',\
PRIMARY KEY (order_id),\
KEY index_game_user_id (game_user_id)\
) ENGINE=InnoDB DEFAULT CHARSET=utf8;"

