mysql_user=root
mysql_passwd=

mysql -u$mysql_user -p$mysql_passwd -e "\
set names utf8;use db_c_360pay;\
alter table pay_360_log_table add column diamonds int(11) NOT NULL DEFAULT 0 after golds, add column costs int(11) NOT NULL DEFAULT 0 after diamonds;"