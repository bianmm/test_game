<?php

/**
 * rsa
 * 需要 openssl 支持
 * @author andsky
 *
 */
class MyRsa extends Rsa
{

    private static $_instance;

    const private_key = '
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDA4E8H2qksOnCSoBkq+HH3Dcu0/iWt3iNcpC/BCg0F8tnMhF1Q
OQ98cRUM8eeI9h+S6g/5UmO4hBKMOP3vg/u7kI0ujrCN1RXpsrTbWaqry/xTDgTM
8HkKkNhRSyDJWJVye0mPgbvVnx76en+K6LLzDaQH8yKI/dbswSq65XFcIwIDAQAB
AoGAU+uFF3LBdtf6kSGNsc+lrovXHWoTNOJZWn6ptIFOB0+SClVxUG1zWn7NXPOH
/WSxejfTOXTqpKb6dv55JpSzmzf8fZphVE9Dfr8pU68x8z5ft4yv314qLXFDkNgl
MeQht4n6mo1426dyoOcCfmWc5r7LQCi7WmOsKvATe3nzk/kCQQDp1gyDIVAbUvwe
tpsxZpAd3jLD49OVHUIy2eYGzZZLK3rA1uNWWZGsjrJQvfGf+mW+/zeUMYPBpk0B
XYqlgHJNAkEA0yhhu/2SPJYxIS9umCry1mj7rwji5O2qVSssChFyOctcbysbNJLH
qoF7wumr9PAjjWFWdmCzzEJyxMMurL3gLwJBAIEoeNrJQL0G9jlktY3wz7Ofsrye
j5Syh4kc8EBbuCMnDfOL/iAI8zyzyOxuLhMmNKLtx140h0kkOS6C430M2JUCQCnM
a5RX/JOrs2v7RKwwjENvIqsiWi+w8C/NzPjtPSw9mj2TTd5ZU9bnrMUHlnd09cSt
yPzD5bOAT9GtRVcCexcCQBxXHRleikPTJC90GqrC2l6erYJaiSOtv6QYIh0SEDVm
1o6Whw4FEHUPqMW0Z5PobPFiEQT+fFR02xU3NJrjYy0=
-----END RSA PRIVATE KEY-----';
/*    const public_key = '
-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDA4E8H2qksOnCSoBkq+HH3Dcu0
/iWt3iNcpC/BCg0F8tnMhF1QOQ98cRUM8eeI9h+S6g/5UmO4hBKMOP3vg/u7kI0u
jrCN1RXpsrTbWaqry/xTDgTM8HkKkNhRSyDJWJVye0mPgbvVnx76en+K6LLzDaQH
8yKI/dbswSq65XFcIwIDAQAB
-----END PUBLIC KEY-----';
  */
  const public_key = '
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyC7NUoq42/BubLLYMLPf
QWQetLlvtRTX1kEpeW4Uhz8ZTgFkDKtt7X5SBLbM34WpDv4upuMsaGIvHuHP/ABP
6b2uavGqp14BNL0dpo/l22VnzI5vDFbVlWC9bC/q5DT4aSnX8gogEedmb11nH/jI
uVCdN14c1UgIX+P9IrF6a7n/4qj6WmIh5DDtVlvFAZD9ROOBxybbgXL4plLrrZjY
er4xmHUNz5W5YujeS0RbD3kfXxWP4k/OQDRp5Se9fE85BF8WZwZIC+37fmrCJbwW
/S2MwV7wxkrtlTrgtFF2jDEF76lG2VVljCL9fLCI0xfCelI5Ots9G7AsZSmOizG0
OQIDAQAB
-----END PUBLIC KEY-----';
    
    function __construct ()
    {
    }


    function __destruct ()
    {
    }

    public static function instance()
    {
        if (self::$_instance == null) {
            self::$_instance = new self;
        }
        return self::$_instance;
    }


    function sign($sourcestr = NULL)
    {

        return base64_encode(parent::sign($sourcestr, self::private_key));

    }

    function verify($sourcestr = NULL, $signature = NULL)
    {
        return parent::verify($sourcestr, $signature, self::public_key);

    }


}
?>
