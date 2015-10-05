/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：InternalAuth.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月09日
*   描    述：内部数据库验证策略
*
#pragma once
================================================================*/
#ifndef __INTERLOGIN_H__
#define __INTERLOGIN_H__

#include <iostream>

#include "IM.BaseDefine.pb.h"

#include <uuid/uuid.h>
using namespace std;

typedef struct AcctInfo{
	std::string id;
	std::string user_account;
	std::string user_password;
	std::string user_salt;
	std::string user_phone;
	std::string user_blacksign;
	std::string login_time;
	std::string token;
	std::string is_login;
} _AcctInfo;

class CInterLoginStrategy 
{
public:
	
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user, _AcctInfo& acctInfo);
	bool setLoginToken( _AcctInfo& acctInfo, int nClientType);
	bool insertLogLogin(_AcctInfo& acctInfo, int nClientType, const string& strLoginIp);
private:
	bool createUserByAcctId(uint32_t acct_id,const std::string& strPhone);	
	int generate_uuid(char* id);
	
};

#endif /*defined(__INTERLOGIN_H__) */
