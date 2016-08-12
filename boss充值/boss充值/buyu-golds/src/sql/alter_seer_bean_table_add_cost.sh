#!/bin/sh
host=localhost
port=3306
user=root
password=


tb_sentence="set names utf8;use buyu_golds_db;\
alter table seer_bean_account_table add column costs int not null default 0 after seer_bean_num"
mysql -h$host -u$user -p$password -e "$tb_sentence"
