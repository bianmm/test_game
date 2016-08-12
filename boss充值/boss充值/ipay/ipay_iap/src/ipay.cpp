/*
 * ipay.cpp
 *
 *  Created on: 2015年3月2日
 *      Author: apple
 */

#ifndef APPLE_IAP_SRC_IPAY_CPP_
#define APPLE_IAP_SRC_IPAY_CPP_

#include <iostream>
#include <string.h>
#include "CryptHelper.h"
#include "ipay.h"
#include "benchapi.h"

//平台公钥
const std::string platPkey =
		"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDIIZ0uJ2TEUB9DLk8/CBSaK29BbySjo78hT1XAfhQHiO03rwWYRwXA8xAWqoK6EY0hIpM7yPurQm6qYwMmNq2B+jDCwjSXQRDNEmNviDPfPx4InV/BzpWKeGl8dusCQWc747mzuycVT8p/N8xbfv3+MKWv58jsopEhcyO/falFSQIDAQAB";

bool ipay_sign_verify(const char *data, const char *sign) {

	//验签
	EVP_PKEY* pkey = UTILS::CryptHelper::getKeyByPKCS1(platPkey, 0);
	if (!pkey) {
		ERROR_LOG("getKeyByPKCS1 by platPkey  error");
		return false;
	}

	if (0 == UTILS::CryptHelper::verifyMd5WithRsa(data, sign, pkey)) {
		DEBUG_LOG("verify sign success");
	} else {
		DEBUG_LOG("verify sign error");
		return false;
	}

	return true;
}

const std::string platPkey_sange =
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCX9VHFytRun2/Ma6+WgCni86kAn9nK+x7Rhsdpn4BXjIZ4e7SNAjRP6gvD+Dji3hamc4alOaQirQnyGJWUV0i6dnIGdupa31MWqoJ4+ufB/LqNHSBR84tXG8bzF/G7kQSaVRMyO8jVSoKBjz27Uc8mC/tj/jko/rVO6X/+vBXIGwIDAQAB";

bool ipay_sign_verify_sange(const char *data, const char *sign) {
    
    //验签
    EVP_PKEY* pkey = UTILS::CryptHelper::getKeyByPKCS1(platPkey_sange, 0);
    if (!pkey) {
        ERROR_LOG("getKeyByPKCS1 by platPkey_sange  error");
        return false;
    }
    
    if (0 == UTILS::CryptHelper::verifyMd5WithRsa(data, sign, pkey)) {
        DEBUG_LOG("verify sign success sange");
    } else {
        DEBUG_LOG("verify sign error sange");
        return false;
    }
    
    return true;
}

const std::string platPkey_frankwu =
"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDFkqUiX0yF8b1IBOmDb7LSgwgNmZT5WS1bbw7u+BGqRklyjosbIidyEYW9LQR5rfsPCLCWfKKuof/pAIpdn713gVRSymkeZoOOHkGK8TvLL8qBWPnVsNMrDF33Iynty6Z4I0vX/XIUAYajSezXv9Z16uS1k56YBx0Fah2ihhaorQIDAQAB";

bool ipay_sign_verify_frankwu(const char *data, const char *sign) {
    
    //验签
    EVP_PKEY* pkey = UTILS::CryptHelper::getKeyByPKCS1(platPkey_frankwu, 0);
    if (!pkey) {
        ERROR_LOG("getKeyByPKCS1 by platPkey_sange  error");
        return false;
    }
    
    if (0 == UTILS::CryptHelper::verifyMd5WithRsa(data, sign, pkey)) {
        DEBUG_LOG("verify sign success frankwu");
    } else {
        DEBUG_LOG("verify sign error frankwu");
        return false;
    }
    
    return true;
}

#endif /* APPLE_IAP_SRC_IPAY_CPP_ */
