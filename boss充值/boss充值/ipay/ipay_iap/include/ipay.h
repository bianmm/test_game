/*
 * ipay.h
 *
 *  Created on: 2015年3月2日
 *      Author: apple
 */

#ifndef IPAY_IAP_INCLUDE_IPAY_H_
#define IPAY_IAP_INCLUDE_IPAY_H_

bool ipay_sign_verify(const char *data, const char *sign);

bool ipay_sign_verify_sange(const char *data, const char *sign);

bool ipay_sign_verify_frankwu(const char *data, const char *sign);

#endif /* IPAY_IAP_INCLUDE_IPAY_H_ */
