CREATE DATABASE IF NOT EXISTS buyu_golds_db DEFAULT CHARSET=utf8;

USE buyu_golds_db;

CREATE TABLE IF NOT EXISTS seer_bean_account_table
(
  account_id INT NOT NULL AUTO_INCREMENT,
  seer_bean_num INT NOT NULL,
  is_allow_transfer TINYINT NOT NULL,
  status TINYINT NOT NULL,
  PRIMARY KEY(account_id)
)ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS seer_bean_channel_table
(
  channel_id INT NOT NULL,
  channel_name VARCHAR(48) NOT NULL,
  channel_type TINYINT NOT NULL,
  security_code CHAR(8) NOT NULL,
  status TINYINT NOT NULL,
  init_trans_id INT NOT NULL DEFAULT 0,
  PRIMARY KEY(channel_id)
)ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS seer_bean_transaction_table
(
  transaction_id INT NOT NULL AUTO_INCREMENT,
  transaction_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  account_id INT NOT NULL,
  dest_account_id INT NOT NULL,
  channel_id INT NOT NULL,
  external_trans_id INT NOT NULL,
  product_id INT NOT NULL,
  product_count SMALLINT NOT NULL,
  seer_bean_num INT NOT NULL,
  seer_bean_balance INT NOT NULL,
  PRIMARY KEY(transaction_id),
  KEY(transaction_time),
  KEY(account_id),
  KEY(dest_account_id),
  KEY(channel_id),
  KEY(external_trans_id),
  KEY(seer_bean_num),
  FOREIGN KEY(account_id) REFERENCES seer_bean_account_table(account_id),
  FOREIGN KEY(dest_account_id) REFERENCES seer_bean_account_table(account_id),
  FOREIGN KEY(channel_id) REFERENCES seer_bean_channel_table(channel_id)
)ENGINE=InnoDB;

