CREATE DATABASE IF NOT EXISTS mb_account_db DEFAULT CHARSET=utf8;

USE mb_account_db;

CREATE TABLE IF NOT EXISTS account_table
(
  account_id INT NOT NULL,
  account_pwd CHAR(32) NOT NULL,
  mb_num INT NOT NULL DEFAULT 0,
  month_consume_limit SMALLINT NOT NULL DEFAULT 0,
  per_consume_limit SMALLINT NOT NULL DEFAULT 0,
  is_allow_transfer TINYINT NOT NULL DEFAULT 0,
  status TINYINT NOT NULL DEFAULT 0,
  PRIMARY KEY (account_id),
  KEY (status)
)ENGINE=InnoDB CHARSET=utf8;

CREATE TABLE IF NOT EXISTS channel_table
(
  channel_id INT NOT NULL,
  channel_name VARCHAR(48) NOT NULL,
  channel_type TINYINT NOT NULL,
  security_code CHAR(8) NOT NULL,
  status TINYINT NOT NULL DEFAULT 0,
  init_trans_id INT NOT NULL DEFAULT 0,
  PRIMARY KEY (channel_id),
  KEY (channel_type)
)ENGINE=InnoDB CHARSET=utf8;

CREATE TABLE IF NOT EXISTS transaction_table
(
  transaction_id BIGINT NOT NULL AUTO_INCREMENT,
  account_id INT NOT NULL,
  dest_account_id INT NOT NULL,
  transaction_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  channel_id INT NOT NULL,
  pay_gate_trans_id INT NOT NULL,
  product_id INT NOT NULL,
  product_count SMALLINT NOT NULL,
  mb_num INT NOT NULL,
  mb_account_balance INT NOT NULL,
  private_data_01 BIGINT NOT NULL,
  private_data_02 VARCHAR(128) NOT NULL,
  PRIMARY KEY (transaction_id),
  KEY (account_id,transaction_time),
  KEY (transaction_time),
  KEY (channel_id,transaction_time),
  KEY (channel_id,pay_gate_trans_id),
  KEY (product_id,transaction_time),
  FOREIGN KEY (account_id) REFERENCES account_table(account_id),
  FOREIGN KEY (channel_id) REFERENCES channel_table(channel_id)
)ENGINE=InnoDB CHARSET=utf8;
