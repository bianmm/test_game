mysql_user=root
mysql_passwd=

mysql -u$mysql_user -p$mysql_passwd -e "\
set names utf8;use apple_iap_db;\
alter table applepay_log_table add column diamonds int(10) unsigned NOT NULL DEFAULT 0 after golds;"
