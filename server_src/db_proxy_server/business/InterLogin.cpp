


/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*
*   文件名称：InterLogin.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月09日
*   描    述：
*
================================================================*/
#include "InterLogin.h"
#include "EncDec.h"
#include "../AutoPool.h"
#include "public_define.h"


bool CInterLoginStrategy::doLogin(const std::string &strName, const std::string &strPass, IM::BaseDefine::UserInfo& user, _AcctInfo& acctInfo)
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("login_db");
	if (pDBConn)
	{
		string strEscName = pDBConn->EscapeString( strName.c_str(), strName.length());

		string strSql = "select *,CURRENT_TIMESTAMP AS login_time from IMFxAccount where user_account='" + strEscName + "' or user_phone='" + strEscName+"' limit 1;" ;  //+ " and status=0";
		log("strSql=%s", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{

			int nuId;
			string strResult, strSalt;
			string strPhone="";
			while (pResultSet->Next())
			{
				nuId = pResultSet->GetInt("user_account");
				strResult = pResultSet->GetString("user_password");
				strSalt = pResultSet->GetString("user_salt");
				strPhone = pResultSet->GetString("user_phone");

				acctInfo.id = pResultSet->GetString("id");
				acctInfo.user_account = pResultSet->GetString("user_account");
				acctInfo.user_password = pResultSet->GetString("user_password");
				acctInfo.user_salt = pResultSet->GetString("user_salt");
				acctInfo.user_phone = pResultSet->GetString("user_phone");
				acctInfo.user_blacksign = pResultSet->GetString("user_blacksign");
				acctInfo.login_time = pResultSet->GetString("login_time");
				acctInfo.token = "";
				acctInfo.is_login = "1";
			}

			pResultSet->Clear();
			
			pDBManger->RelDBConn(pDBConn);
			pDBConn = NULL;

			string strInPass = strPass + strSalt;
			char szMd5[33];
			CMd5::MD5_Calculate(strInPass.c_str(), strInPass.length(), szMd5);
			string strOutPass(szMd5);
			if(strOutPass == strResult)
			{

				CDBConn* pDBConn1 = pDBManger->GetDBConn("dffxIMDB_slave");
				if (pDBConn1)
				{
					//string strSql = "select * from IMUser where name='" + strName + "' and status=0";
					string strSql = "select * from IMFxUser where user_uid=" + int2string(nuId) + " and status=0 limit 1";
					pResultSet = pDBConn1->ExecuteQuery(strSql.c_str());

					log("strSql=%s", strSql.c_str());
					
					if(pResultSet)
					{
						if(pResultSet->Next())
						{
							bRet = true;

							user.set_user_id(pResultSet->GetInt("user_uid"));
							user.set_user_level(pResultSet->GetInt("user_level"));
							user.set_user_status(pResultSet->GetInt("user_status"));
							user.set_user_gender(pResultSet->GetInt("user_sex"));
							user.set_user_uid(pResultSet->GetInt("user_uid"));
							user.set_user_nickname(pResultSet->GetString("user_nickname"));
							user.set_user_headlink(pResultSet->GetString("user_headlink"));
							user.set_user_birthday(pResultSet->GetString("user_birthday"));
							user.set_user_phone(pResultSet->GetString("user_phone"));

							user.set_user_type(pResultSet->GetInt("user_type"));
							user.set_user_ischeck(pResultSet->GetInt("user_ischeck"));

							user.set_user_desc(pResultSet->GetString("user_desc"));

							pResultSet->Clear();

							pDBManger->RelDBConn(pDBConn1);
							pDBConn1 = NULL;

						}
						else
						{
							//no result

							pResultSet->Clear();

							pDBManger->RelDBConn(pDBConn1);
							pDBConn1 = NULL;

							//first login,then create user's info
							//create
							createUserByAcctId( nuId, strPhone);
							//select
							CDBConn* pDBConn2 = pDBManger->GetDBConn("dffxIMDB_slave");
							if(pDBConn2)
							{
								pResultSet = pDBConn2->ExecuteQuery(strSql.c_str());
								if(pResultSet)
								{
									if(pResultSet->Next())
									{
										bRet = true;

										user.set_user_id(pResultSet->GetInt("user_uid"));
										user.set_user_level(pResultSet->GetInt("user_level"));
										user.set_user_status(pResultSet->GetInt("user_status"));
										user.set_user_gender(pResultSet->GetInt("user_sex"));
										user.set_user_uid(pResultSet->GetInt("user_uid"));
										user.set_user_nickname(pResultSet->GetString("user_nickname"));
										user.set_user_headlink(pResultSet->GetString("user_headlink"));
										user.set_user_birthday(pResultSet->GetString("user_birthday"));
										user.set_user_phone(pResultSet->GetString("user_phone"));

										user.set_user_type(pResultSet->GetInt("user_type"));
										user.set_user_ischeck(pResultSet->GetInt("user_ischeck"));

										user.set_user_desc(pResultSet->GetString("user_desc"));
									}
									pResultSet->Clear();
									pDBManger->RelDBConn(pDBConn2);
									pDBConn2 = NULL;
								}
								else
								{
									pDBManger->RelDBConn(pDBConn2);
									pDBConn2 = NULL;
								}
							}
							
						}

					}
					else
					{
						pDBManger->RelDBConn(pDBConn1);
						pDBConn1 = NULL;
						log("error:strSql=%s", strSql.c_str());
					}

					
				}
			}
		}
		else
		{
			pDBManger->RelDBConn(pDBConn);
			pDBConn = NULL;
		}

		
	}
	return bRet;
}

bool CInterLoginStrategy::createUserByAcctId(uint32_t acct_id, const std::string& strPhone)
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_master");

	if (pDBConn)
	{
		string strNowtime = int2string(uint32_t(time(NULL)));
		string strAcctId = int2string(acct_id);

		string strSql = "INSERT INTO IMFxUser( `user_uid`, `user_regedittime`, `user_updatetime`, `status`, `user_phone`) VALUES( "
		                + strAcctId + "," + strNowtime + "," + strNowtime + ", 0, '"+strPhone+"' ) ;" ;

		log("strSql=%s", strSql.c_str());

		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(bRet)
		{
			//string strInnerId = int2string(pDBConn->GetInsertId());

			strSql = "INSERT INTO IMFxUserRelationGroup( `group_id`, `group_name`, `user_uid`, `flag`, `sort`) SELECT 1,'我的好友',"
			         + strAcctId + ",1,1 UNION ALL SELECT 0,'黑名单'," + strAcctId + ",2,-1 ;";

			log("strSql=%s", strSql.c_str());

			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			if(!bRet)
			{
				log("failed");
			}
		}
		else
		{
			log("failed");
		}

		pDBManger->RelDBConn(pDBConn);
	}

	return bRet;
}


int CInterLoginStrategy::generate_uuid(char* id)
{
	if (NULL == id)
	{
		return  -1; // invalid param
	}

	uuid_t uid = {0};
	uuid_generate(uid);
	if (uuid_is_null(uid))
	{
		id = NULL;
		return -2; // uuid generate failed
	}
	uuid_unparse(uid, id);

	return 0;
}

bool CInterLoginStrategy::setLoginToken(_AcctInfo & acctInfo, int nClientType)
{
	if(string2int(acctInfo.user_account) < 10000)
		return true;

	char szToken[64]= {0};
	int nRet = generate_uuid( szToken);
	if(nRet<0)
	{
		log("generate_uuid token error:%s", acctInfo.user_account.c_str());
		return false;
	}
	else
	{
		acctInfo.token = szToken;
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "login_token", &pCacheConn);

		if (pCacheConn)
		{
			string account_uid_Key="account_uid_"+acctInfo.user_account;
			std::map<std::string, std::string> map_key_value;
			map_key_value["id"] = acctInfo.id;
			map_key_value["uid"] = acctInfo.user_account;
			map_key_value["user_password"] = acctInfo.user_password;
			map_key_value["user_salt"] = acctInfo.user_salt;
			map_key_value["user_phone"] = acctInfo.user_phone;
			map_key_value["user_blacksign"] = acctInfo.user_blacksign;
			map_key_value["login_time"] = acctInfo.login_time;
			string strToken_key = "token";
			if(CHECK_CLIENT_TYPE_PC(nClientType))
				strToken_key += "_pc";
			else
				strToken_key += "_mobile";
			map_key_value[strToken_key] = acctInfo.token;
			map_key_value["is_login"] = acctInfo.is_login;

			pCacheConn->hmset( account_uid_Key, map_key_value);

			string account_user_account_Key="account_user_account_"+acctInfo.user_account;
			pCacheConn->hset( account_user_account_Key, "uid", acctInfo.user_account);

			string account_user_phone_Key="account_user_phone_"+acctInfo.user_phone;
			pCacheConn->hset( account_user_phone_Key, "uid", acctInfo.user_account);

			return true;
		}
		else
		{
			log("autoCache token error:%s",  acctInfo.user_account.c_str());
			return false;
		}
	}
}

bool CInterLoginStrategy::insertLogLogin(const _AcctInfo & acctInfo, int nClientType, const string& strLoginIp)
{
	if(string2int(acctInfo.user_account) < 10000)
		return true;
    
    CDBConn* pDBConn = NULL;
    CAutoDB autoDB("login_db", &pDBConn);
    bool bRet = false;
    if (pDBConn)
	{
	    string strSql = "INSERT INTO `dffx_db_bs`.`dffx_log_login`(`log_userid`,"\
            "  `log_logintime`,"\
            "  `log_loginip`,"\
            "  `log_clienttype`)"\
            "  VALUES("+acctInfo.user_account+",'"+acctInfo.login_time+"','"+strLoginIp+"',"+int2string(nClientType)+")";
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());	
        
        log("strSql=%s", strSql.c_str());
	}
	else
	{
		log("!!!login_db not found");
	}

    return bRet;
}

bool CInterLoginStrategy::updateInviteRecord(const _AcctInfo & acctInfo)
{
	if(string2int(acctInfo.user_account) < 10000)
		return true;
    
    CDBConn* pDBConn = NULL;
    CAutoDB autoDB("login_db", &pDBConn);
    bool bRet = false;
    if (pDBConn)
	{		
	    string strSql = "UPDATE `dffx_db_bs`.`dffx_user_inviterecord` SET invite_logintime="+ long2string(time(NULL)*1000)+\
			" WHERE invite_invitee="+ acctInfo.user_account +" AND invite_logintime=0";
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());	
        
        log("strSql=%s", strSql.c_str());
	}	
	else
	{
		log("!!!login_db not found");
	}
	
    return bRet;
}

bool CInterLoginStrategy::veryfyVersion( int nClientType, const string& strClientVersion,IM::Server::IMValidateRsp &msgResp)
{	
	CDBConn* pDBConn = NULL;
	CAutoDB autoDB("login_db", &pDBConn);
	bool bRet = true;
	if (pDBConn)
	{		
		string strSql = "SELECT * FROM dffx_db_bs.dffx_common_version t WHERE t.version_clienttype = "+ int2string(nClientType)+" LIMIT 1";
			
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());	
		
		log("strSql=%s", strSql.c_str());
		
		if(pResultSet )
		{
			if(pResultSet->Next())
			{
				msgResp.set_version_max(pResultSet->GetString("version_max"));
				msgResp.set_version_download(pResultSet->GetString("version_download"));
				msgResp.set_version_filesize(pResultSet->GetString("version_filesize"));

				CStrExplode clientVersion((char*)strClientVersion.c_str(), '.');				
				CStrExplode serverVersion((char*)pResultSet->GetString("version_min"), '.');
				if(clientVersion.GetItemCnt() < serverVersion.GetItemCnt())
					bRet = false;
			    for (uint32_t i = 0; bRet && i < clientVersion.GetItemCnt() && i<serverVersion.GetItemCnt(); i++) 
			    {
			    	if(atoi(clientVersion.GetItem(i)) < atoi(serverVersion.GetItem(i)))
					{
						bRet = false;
						break;
			    	}
			    }
				

				//if(strClientVersion < pResultSet->GetString("version_min"))
				//	bRet = false;
			}
			pResultSet->Clear();
		}
	}	
	else
	{
		log("!!!login_db not found");
	}

	return bRet;
}


