




/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：UserModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月05日
*   描    述：
*
================================================================*/
#include "UserModel.h"
#include "MessageModel.h"
#include "../AutoPool.h"
#include "Common.h"

CUserModel* CUserModel::m_pInstance = NULL;

CUserModel::CUserModel()
{

}

CUserModel::~CUserModel()
{
    
}

CUserModel* CUserModel::getInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new CUserModel();
    }
    return m_pInstance;
}

void CUserModel::getChangedId(uint32_t& nLastTime, list<uint32_t>& lsIds, uint32_t user_id)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
	
	 if (pDBConn)
	{	
		string strSql ;
		if(nLastTime == 0)
		{
			strSql = "SELECT t2.friend_friendid, t1.user_updatetime, t2.`friend_groupid`, t2.`status` AS friend_status "\
		   		"FROM IMFxUser t1, IMFxUserRelationPrivate t2 WHERE t1.user_uid = t2.`friend_friendid` "\
		   		" AND t1.`status` != 3 AND t2.`user_uid` = " + int2string(user_id);
		}
		else
		{
			strSql = "SELECT t2.friend_friendid, t1.user_updatetime, t2.`friend_groupid`, t2.`status` AS friend_status "\
		   		"FROM IMFxUser t1, IMFxUserRelationPrivate t2 WHERE t1.user_uid = t2.`friend_friendid` "\
		   		"AND ( t2.`update_time` >= "+int2string(nLastTime)+" OR t1.`user_updatetime` >= "+int2string(nLastTime)\
		   		+ " ) AND t1.`status` != 3 AND t2.`user_uid` = " + int2string(user_id);
		}
		
		log("strSql= %s", strSql.c_str());

        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
        	while (pResultSet->Next()) {
                uint32_t nId = pResultSet->GetInt("friend_friendid");
                uint32_t nUpdated = pResultSet->GetInt("user_updatetime");
                if(nLastTime < nUpdated)
                {
                    nLastTime = nUpdated;
                }
                lsIds.push_back(nId);
            }
            pResultSet->Clear();
        }
        else
        { 
            log(" no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
}

void CUserModel::getUsers(list<uint32_t> lsIds, list<IM::BaseDefine::UserInfo> &lsUsers, uint32_t nUserId)
{
    if (lsIds.empty()) {
        log("list is empty");
        return;
    }
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if (pDBConn)
    {
        string strClause;
        bool bFirst = true;
        for (auto it = lsIds.begin(); it!=lsIds.end(); ++it)
        {
            if(bFirst)
            {
                bFirst = false;
                strClause += int2string(*it);
            }
            else
            {
                strClause += ("," + int2string(*it));
            }
        }
		
        string  strSql = "SELECT t1.*,t2.`friend_groupid`,t2.`friend_remark`,t2.`status` AS friend_status FROM IMFxUser t1 "\
			"LEFT JOIN IMFxUserRelationPrivate t2 ON t1.`user_uid`=t2.`friend_friendid` AND t2.`user_uid`="+int2string(nUserId)+" WHERE t1.user_uid IN ("\
			+ strClause + ") ORDER BY t1.user_uid";

		log("getUsers from_user=%d sql=%s", nUserId, strSql.c_str());
		
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::UserInfo cUser;
                cUser.set_user_id(pResultSet->GetInt("user_uid"));				
                cUser.set_user_nickname(pResultSet->GetString("user_nickname"));
                cUser.set_user_gender(pResultSet->GetInt("user_sex"));
                cUser.set_user_birthday(pResultSet->GetString("user_birthday"));
                cUser.set_user_headlink(pResultSet->GetString("user_headlink"));
                cUser.set_user_level(pResultSet->GetInt("user_level"));
                cUser.set_user_status(pResultSet->GetInt("user_status"));
				cUser.set_user_uid(pResultSet->GetInt("user_uid"));
				cUser.set_user_phone(pResultSet->GetString("user_phone"));

				cUser.set_user_type(pResultSet->GetInt("user_type"));
				cUser.set_user_ischeck(pResultSet->GetInt("user_ischeck"));

				cUser.set_user_desc(pResultSet->GetString("user_desc"));

				cUser.set_friend_groupid(pResultSet->GetInt("friend_groupid"));
				cUser.set_friend_remark(pResultSet->GetString("friend_remark"));

				cUser.set_friend_status(pResultSet->GetInt("friend_status"));
				
                lsUsers.push_back(cUser);
            }
            pResultSet->Clear();
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
}

void CUserModel::getFriendsList(uint32_t& nLastTime, uint32_t user_id, list<IM::BaseDefine::UserInfo> &lsUsers)
{   
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if (pDBConn)
    {   
    	// if friend_groupid is wrong then update it to default 1
		string  strSql = "UPDATE IMFxUserRelationPrivate t1 SET friend_groupid = 1 WHERE t1.user_uid="+int2string(user_id)+\ 
		" AND NOT EXISTS (SELECT t2.`group_id` FROM IMFxUserRelationGroup t2 WHERE t2.`group_id`=t1.`friend_groupid` AND t2.`user_uid`=t1.`user_uid`)";
		
		pDBConn->ExecuteUpdate(strSql.c_str());
			
    	//AND t2.`status` >= 3
        strSql = "SELECT t1.*,"\
						"  t2.`friend_groupid`,  t2.`friend_remark`, t2.`status` as friend_status FROM "\
						" IMFxUser t1,  IMFxUserRelationPrivate t2 WHERE t1.user_uid = t2.`friend_friendid` AND "\
						"(t2.`update_time`>="+int2string(nLastTime)+" or t1.`user_updatetime` >= "+int2string(nLastTime)\
						+") AND t1.`status` !=3    AND t2.`user_uid` = "+int2string(user_id) +" ORDER BY t1.user_uid";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		
		log("strSql= %s", strSql.c_str());
		
        if(pResultSet)
        {
        	CacheConn* pCacheConn = NULL;
			CAutoCache autoCache( "table_redis", &pCacheConn);
				
            while (pResultSet->Next())
            {
                IM::BaseDefine::UserInfo cUser;
                cUser.set_user_id(pResultSet->GetInt("user_uid"));				
                cUser.set_user_nickname(pResultSet->GetString("user_nickname"));
                cUser.set_user_gender(pResultSet->GetInt("user_sex"));
                cUser.set_user_birthday(pResultSet->GetString("user_birthday"));
                cUser.set_user_headlink(pResultSet->GetString("user_headlink"));
                cUser.set_user_level(pResultSet->GetInt("user_level"));
                cUser.set_user_status(pResultSet->GetInt("user_status"));
				
				cUser.set_user_uid(pResultSet->GetInt("user_uid"));
				cUser.set_user_phone(pResultSet->GetString("user_phone"));

				cUser.set_user_type(pResultSet->GetInt("user_type"));
				cUser.set_user_ischeck(pResultSet->GetInt("user_ischeck"));

				cUser.set_user_desc(pResultSet->GetString("user_desc"));

				cUser.set_friend_groupid(pResultSet->GetInt("friend_groupid"));
				cUser.set_friend_remark(pResultSet->GetString("friend_remark"));

				cUser.set_friend_status(pResultSet->GetInt("friend_status"));

				uint32_t nUpdated = pResultSet->GetInt("user_updatetime");
				
                lsUsers.push_back(cUser);

				if(nLastTime < nUpdated)
                {
                    nLastTime = nUpdated;
                }

				//redis insert
				if(pCacheConn)
				{
					if(cUser.friend_groupid()==0)
					{
						string strBlackKey = "IMFxUserRelationPrivate:BlackSet:userId_"+int2string(user_id);
						pCacheConn->sadd(strBlackKey, pResultSet->GetString("user_uid"));
					}
					else if((int)cUser.friend_status()>=3)
					{
						string strFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(user_id);
						pCacheConn->sadd(strFriendKey, pResultSet->GetString("user_uid"));
					}
				}
            }

			// delete the status=0,-1		
			strSql = "DELETE FROM IMFxUserRelationPrivate WHERE (status=0 or status=-1) AND user_uid="+int2string(user_id);
			pDBConn->ExecuteUpdate(strSql.c_str());
			
            pResultSet->Clear();
        }
        else
        {
            log(" no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
}


bool CUserModel::getUser(uint32_t nUserId, DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if (pDBConn)
    {
        string strSql = "select * from IMUser where id="+int2string(nUserId);
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                cUser.nId = pResultSet->GetInt("id");
                cUser.nSex = pResultSet->GetInt("sex");
                cUser.strNick = pResultSet->GetString("nick");
                cUser.strDomain = pResultSet->GetString("domain");
                cUser.strName = pResultSet->GetString("name");
                cUser.strTel = pResultSet->GetString("phone");
                cUser.strEmail = pResultSet->GetString("email");
                cUser.strAvatar = pResultSet->GetString("avatar");
                cUser.nDeptId = pResultSet->GetInt("departId");
                cUser.nStatus = pResultSet->GetInt("status");
                bRet = true;
            }
            pResultSet->Clear();
        }
        else
        {
            log("no result set for sql:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
    return bRet;
}

bool CUserModel::updateUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMUser set `sex`=" + int2string(cUser.nSex)+ ", `nick`='" + cUser.strNick +"', `domain`='"+ cUser.strDomain + "', `name`='" + cUser.strName + "', `phone`='" + cUser.strTel + "', `email`='" + cUser.strEmail+ "', `avatar`='" + cUser.strAvatar + "', `departId`='" + int2string(cUser.nDeptId) + "', `status`=" + int2string(cUser.nStatus) + ", `updated`="+int2string(nNow) + " where id="+int2string(cUser.nId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("update failed:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
    return bRet;
}

bool CUserModel::insertUser(DBUserInfo_t &cUser)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        string strSql = "insert into IMUser(`id`,`sex`,`nick`,`domain`,`name`,`phone`,`email`,`avatar`,`departId`,`status`,`created`,`updated`) values(?,?,?,?,?,?,?,?,?,?,?,?)";
        CPrepareStatement* stmt = new CPrepareStatement();
        if (stmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t nNow = (uint32_t) time(NULL);
            uint32_t index = 0;
            uint32_t nGender = cUser.nSex;
            uint32_t nStatus = cUser.nStatus;
            stmt->SetParam(index++, cUser.nId);
            stmt->SetParam(index++, nGender);
            stmt->SetParam(index++, cUser.strNick);
            stmt->SetParam(index++, cUser.strDomain);
            stmt->SetParam(index++, cUser.strName);
            stmt->SetParam(index++, cUser.strTel);
            stmt->SetParam(index++, cUser.strEmail);
            stmt->SetParam(index++, cUser.strAvatar);
            stmt->SetParam(index++, cUser.nDeptId);
            stmt->SetParam(index++, nStatus);
            stmt->SetParam(index++, nNow);
            stmt->SetParam(index++, nNow);
            bRet = stmt->ExecuteUpdate();
            
            if (!bRet)
            {
                log("insert user failed: %s", strSql.c_str());
            }
        }
        delete stmt;
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
    return bRet;
}

void CUserModel::clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType)
{
    if(IM::BaseDefine::SessionType_IsValid(nSessionType))
    {
        CacheManager* pCacheManager = CacheManager::getInstance();
        CacheConn* pCacheConn = pCacheManager->GetCacheConn("unread");
        if (pCacheConn)
        {
            // Clear P2P msg Counter
            if(nSessionType == IM::BaseDefine::SESSION_TYPE_SINGLE)
            {
                int nRet = pCacheConn->hdel("unread:toId_" + int2string(nUserId), int2string(nPeerId));
                if(!nRet)
                {
                    log("hdel failed %d->%d", nPeerId, nUserId);
                }
            }
            // Clear Group msg Counter
            else if(nSessionType == IM::BaseDefine::SESSION_TYPE_GROUP)
            {
                string strGroupKey = int2string(nPeerId) + GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX;
                map<string, string> mapGroupCount;
                bool bRet = pCacheConn->hgetAll(strGroupKey, mapGroupCount);
                if(bRet)
                {
                    string strUserKey = int2string(nUserId) + "_" + int2string(nPeerId) + GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX;
                    string strReply = pCacheConn->hmset(strUserKey, mapGroupCount);
                    if(strReply.empty()) {
                        log("hmset %s failed !", strUserKey.c_str());
                    }
                }
                else
                {
                    log("hgetall %s failed!", strGroupKey.c_str());
                }
                
            }
            pCacheManager->RelCacheConn(pCacheConn);
        }
        else
        {
            log("no cache connection for unread");
        }
    }
    else{
        log("invalid sessionType. userId=%u, fromId=%u, sessionType=%u", nUserId, nPeerId, nSessionType);
    }
}

bool CUserModel::updateUserAvatar(uint32_t nUserId, string strAvatarUrl)
{
	bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
    	string strEscAvatarUrl = pDBConn->EscapeString( strAvatarUrl.c_str(), strAvatarUrl.length());
		
        uint32_t nNow = (uint32_t)time(NULL);
        string strSql = "update IMFxUser set `user_headlink`='" + strEscAvatarUrl + "', `user_updatetime`="+int2string(nNow) + " where user_uid="+int2string(nUserId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("update failed:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
    return bRet;
}


bool CUserModel::addFriend(IM::Buddy::IMAddFriendReq& addFriend)
{
	uint32_t user_id, friend_id, friend_groupid;
	string 	 friend_remark;

	user_id = addFriend.from_user_id();
	friend_id =	addFriend.to_user_id();
	friend_groupid = addFriend.friend_groupid();
	
	friend_remark = addFriend.friend_remark();
	
	string extra_info = addFriend.extra_info();

	if(user_id==friend_id)
	{
		log("addFriend: can't add self user_id=%d,friend_id=%d", user_id, friend_id);
		return false;
	}

	if(friend_groupid==0)
	{
		log("addFriend: can't add to black group user_id=%d,friend_id=%d", user_id, friend_id);
		return false;
	}

	//count limit
	{
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{
			string strFromFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(user_id);
			if(pCacheConn->scard(strFromFriendKey)>=100)
			{
				log("addFriend: strFromFriendKey >=100 user_id=%d,friend_id=%d", user_id, friend_id);
				return false;
			}

			string strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(friend_id);
			if(pCacheConn->scard(strToFriendKey)>=100)
			{
				log("addFriend: strToFriendKey >=100 user_id=%d,friend_id=%d", user_id, friend_id);
				return false;
			}
		}
	}

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
	
    bool bRet = false;
    if (pDBConn)
    {
    	do
    	{
    	friend_remark = pDBConn->EscapeString( friend_remark.c_str(), friend_remark.length());
		extra_info = pDBConn->EscapeString( extra_info.c_str(), extra_info.length());
		
		string strSql;

		//
		strSql = "SELECT * FROM IMFxUser WHERE user_uid=" + int2string(friend_id);
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(!pResultSet)
        {
            log("friend_id is not exist:%s", strSql.c_str());
			break;
        }
		else if(!pResultSet->Next())
		{
			pResultSet->Clear();
			log("friend_id is not exist:%s", strSql.c_str());
			break;
		}
		
		pResultSet->Clear();

		//
		strSql = "select * from IMFxUser t1,IMFxUserRelationGroup t2 where "\
			" t2.user_uid = t1.user_uid and t1.user_uid= " + int2string(user_id) + " and t2.`group_id` = " + int2string(friend_groupid) ;
		
		pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(!pResultSet)
        {
            log("user_id or friend_groupid is not exist:%s", strSql.c_str());
			break;
        }
		else if(!pResultSet->Next())
		{
			pResultSet->Clear();
			log("user_id or friend_groupid is not exist:%s", strSql.c_str());
			break;
		}

		//
		addFriend.set_user_nickname( pResultSet->GetString("user_nickname"));
		addFriend.set_user_headlink( pResultSet->GetString("user_headlink"));
		addFriend.set_user_gender( pResultSet->GetInt("user_sex"));
		addFriend.set_user_uid( pResultSet->GetInt("user_uid"));
		
		pResultSet->Clear();

		//
		strSql = "SELECT COUNT(*) AS cnt FROM IMFxUserRelationPrivate t1 WHERE t1.status>=3 AND (t1.user_uid,t1.friend_friendid) IN (("\
 			+ int2string(user_id) + "," + int2string(friend_id) +"),("+ int2string(friend_id) +","+ int2string(user_id) +"))";
		
		pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
        	if(pResultSet->Next() && pResultSet->GetInt("cnt") >=2)
			{
				
	        	pResultSet->Clear();  
	        	log("they are already friends:%s", strSql.c_str());
				break;
        	}
			
        	pResultSet->Clear();            
        }
		

		//
		string strNowtime = int2string(uint32_t(time(NULL)));
		//A-->B  1:用户已经发出好友请求
        strSql = "INSERT into IMFxUserRelationPrivate(user_uid, friend_friendid, friend_groupid, friend_remark, status"\
        	", update_time) VALUES(" + int2string(user_id) + "," +  int2string(friend_id) + "," + int2string(friend_groupid) 
        	+ ", '" + friend_remark +"',1," + strNowtime + ");";
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
        	log("addFriend %s", strSql.c_str());
			strSql = "UPDATE IMFxUserRelationPrivate SET status=1,friend_groupid=" + int2string(friend_groupid) + ",friend_remark='" + friend_remark \
				+ "',update_time="+strNowtime+" WHERE user_uid=" \
				+ int2string(user_id) + " AND friend_friendid=" + int2string(friend_id);
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			if(!bRet)
            	log("addFriend failed:%s", strSql.c_str());
        }

		//B-->A 2:收到好友请求,但没有回复
		strSql = "INSERT into IMFxUserRelationPrivate(user_uid, friend_friendid, friend_groupid, friend_remark, status, extra_info, update_time)"\
			" VALUES(" + int2string(friend_id) + "," +  int2string(user_id) + ",1,'',2,'" + extra_info + "'," + strNowtime + ");";
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());		
        if(!bRet)
        {
        	log("addFriend %s", strSql.c_str());
			strSql = "UPDATE IMFxUserRelationPrivate SET status=2 ,update_time=" + strNowtime \
				+ ",extra_info='" + extra_info + "' WHERE user_uid=" + int2string(friend_id) + " AND friend_friendid=" + int2string(user_id);
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			if(!bRet)
            	log("addFriend failed:%s", strSql.c_str());
        }
    	}while(false);
        pDBManager->RelDBConn(pDBConn);
		
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }

	return bRet;
}

bool CUserModel::reverseAddFriend(IM::Buddy::IMReverseAddFriendReq& reverseAddFriend, IM::Buddy::IMFriendNotifyReq& msgNotify)
{
	uint32_t user_id, friend_id;
	string 	 friend_remark="";
	string refuse_reason;
	uint32_t friend_groupid=1;

	user_id = reverseAddFriend.from_user_id();
	friend_id =	reverseAddFriend.to_user_id();
	IM::BaseDefine::FriendResStatusType friendres_status_type = reverseAddFriend.friendres_status_type();
	if(friendres_status_type==IM::BaseDefine::FRIENDRES_STATUS_AGREE)
	{
		friend_groupid = reverseAddFriend.friend_groupid();
		friend_remark = reverseAddFriend.friend_remark();

		if(friend_groupid==0)
		{
			log("reverseAddFriend: can't add to black group user_id=%d,friend_id=%d", user_id, friend_id);
			return false;
		}

		//count limit	
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{
			string strFromFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(user_id);
			if(pCacheConn->scard(strFromFriendKey)>=100)
			{
				log("addFriend: strFromFriendKey >=100 user_id=%d,friend_id=%d", user_id, friend_id);
				return false;
			}

			string strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(friend_id);
			if(pCacheConn->scard(strToFriendKey)>=100)
			{
				log("addFriend: strToFriendKey >=100 user_id=%d,friend_id=%d", user_id, friend_id);
				return false;
			}
			
		}
	
	}
	else
	{
		refuse_reason = reverseAddFriend.refuse_reason();
	}

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
	
    bool bRet =false;
    if (pDBConn)
    {
    	do
    	{
    	friend_remark = pDBConn->EscapeString( friend_remark.c_str(), friend_remark.length());
		refuse_reason = pDBConn->EscapeString( refuse_reason.c_str(), refuse_reason.length());
		
		string strSql;
		string strNowtime = int2string(uint32_t(time(NULL)));

		
		strSql = "SELECT * FROM IMFxUser WHERE user_uid=" + int2string(friend_id);
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(!pResultSet)
        {
            log("friend_id is not exist:%s", strSql.c_str());
			break;
        }
		else if(!pResultSet->Next())
		{
			pResultSet->Clear();
			log("friend_id is not exist:%s", strSql.c_str());
			break;
		}
				
		pResultSet->Clear();

		strSql = "select * from IMFxUser t1,IMFxUserRelationGroup t2 where "\
			" t2.user_uid = t1.user_uid and t1.user_uid= " + int2string(user_id) + " and t2.`group_id` = " + int2string(friend_groupid) ;
		
		pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(!pResultSet)
        {
            log("user_id or friend_groupid is not exist:%s", strSql.c_str());
			break;
        }
		else if(!pResultSet->Next())
		{
			pResultSet->Clear();
			log("user_id or friend_groupid is not exist:%s", strSql.c_str());
			break;
		}

		//
		msgNotify.set_user_nickname( pResultSet->GetString("user_nickname"));
		msgNotify.set_user_headlink( pResultSet->GetString("user_headlink"));
		msgNotify.set_user_gender( pResultSet->GetInt("user_sex"));
		msgNotify.set_user_uid( pResultSet->GetInt("user_uid"));
		
		pResultSet->Clear();
		
		//
      	strSql = "SELECT * FROM IMFxUserRelationPrivate WHERE status=1 AND user_uid=" + int2string(friend_id) + " AND friend_friendid=" + int2string(user_id);
		pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(!pResultSet)
        {//if A did not send addFriend Req
        	log("reverseAddFriend Failed %s", strSql.c_str());
			break;
        }
		else if(!pResultSet->Next())
		{
			pResultSet->Clear();
			log("reverseAddFriend Failed %s", strSql.c_str());
			break;
		}
		
		pResultSet->Clear();
		
		if(friendres_status_type==IM::BaseDefine::FRIENDRES_STATUS_REFUSE)
		{//refuse
			//A:被拒绝但未通知
			strSql = "UPDATE IMFxUserRelationPrivate SET status=-2,extra_info='" + refuse_reason + "',update_time=" + strNowtime \
				+ " WHERE user_uid=" + int2string(friend_id) + " AND friend_friendid=" + int2string(user_id);
			
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			if(!bRet)
	        	log("reverseAddFriend failed:%s", strSql.c_str());

			//B:  应该删除这个好友
			strSql = "UPDATE IMFxUserRelationPrivate SET status=-1,update_time=" + strNowtime \
				+ " WHERE user_uid=" + int2string(user_id) + " AND friend_friendid=" + int2string(friend_id);
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
				
	        if(!bRet)
	        	log("reverseAddFriend failed:%s", strSql.c_str());
			
			break;
		}
		else if(friendres_status_type==IM::BaseDefine::FRIENDRES_STATUS_IGNORE)
		{//ignore
			//A:
			strSql = "UPDATE IMFxUserRelationPrivate SET status=0 ,update_time=" + strNowtime \
				+ " WHERE user_uid=" + int2string(friend_id) + " AND friend_friendid=" + int2string(user_id);
			
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			if(!bRet)
	        	log("reverseAddFriend failed:%s", strSql.c_str());

			//B:
			strSql = "UPDATE IMFxUserRelationPrivate SET status=0 ,update_time=" + strNowtime \
				+ " WHERE user_uid=" + int2string(user_id) + " AND friend_friendid=" + int2string(friend_id);
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
				
	        if(!bRet)
	        	log("reverseAddFriend failed:%s", strSql.c_str());
			
			break;
		}
		
		//agree		
		//A + B:被同意但未通知
		strSql = "UPDATE IMFxUserRelationPrivate SET status=4 ,update_time=" + strNowtime \
				+ " WHERE user_uid=" + int2string(friend_id) + " AND friend_friendid=" + int2string(user_id);
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)
        	log("reverseAddFriend failed:%s", strSql.c_str());

		//B + A:好友已经相互添加
		strSql = "UPDATE IMFxUserRelationPrivate SET status=3, friend_groupid="+ int2string(friend_groupid) +", friend_remark='"+ friend_remark  + \
			"',update_time=" + strNowtime + "  WHERE user_uid=" + int2string(user_id) + " AND friend_friendid=" + int2string(friend_id);
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			
        if(!bRet)
        {
        	log("reverseAddFriend %s", strSql.c_str());
			strSql = "INSERT into IMFxUserRelationPrivate(user_uid, friend_friendid, friend_groupid, friend_remark, status, update_time)"\
				" VALUES(" + int2string(user_id) + "," +  int2string(friend_id) + "," + int2string(friend_groupid) + ", '" + friend_remark +"',3,"\
				+ strNowtime +");";
        	bRet = pDBConn->ExecuteUpdate(strSql.c_str());	
			if(!bRet)
            	log("reverseAddFriend failed:%s", strSql.c_str());
        }

		//redis insert
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{
			string strFromFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(user_id);
			pCacheConn->sadd( strFromFriendKey, int2string(friend_id));

			string strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(friend_id);
			pCacheConn->sadd( strToFriendKey, int2string(user_id));
		}
		
    	}while(false);
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
		
    }
	
	return bRet;
}


bool CUserModel::delFriend(IM::Buddy::IMDelFriendReq& delFriend)
{
	uint32_t user_id, friend_id;
	user_id = delFriend.user_id();
	friend_id =	delFriend.del_user_id();

	//redis delete
	{
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{	
			string strFromFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(user_id);						
			pCacheConn->srem( strFromFriendKey, int2string(friend_id));

			string strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(friend_id);
			pCacheConn->srem( strToFriendKey, int2string(user_id));
		
			string strFromBlackKey = "IMFxUserRelationPrivate:BlackSet:userId_"+int2string(user_id);
			pCacheConn->srem( strFromBlackKey, int2string(friend_id));			
		}
	}

	//delete IMMessage
	CMessageModel::getInstance()->cleanMsgList(user_id, friend_id);

	//delete IMFxUserRelationPrivate
	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
	
    bool bRet =false;
    if (pDBConn)
    {
    	do
    	{
    	string strNowtime = int2string(uint32_t(time(NULL)));
		string strSql;

        strSql = "update IMFxUserRelationPrivate set status=-1,update_time="+strNowtime+" where user_uid = " + \
			int2string(user_id) + " and friend_friendid = " + int2string(friend_id);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("delFriend failed:%s", strSql.c_str());
			//break;
        }

		strSql = "update IMFxUserRelationPrivate set status=-1,update_time="+strNowtime+" where user_uid = " + \
			int2string(friend_id) + " and friend_friendid = " + int2string(user_id);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("delFriend failed:%s", strSql.c_str());
			//break;
        }
    	}while(false);
        pDBManager->RelDBConn(pDBConn);
		
		bRet = true;
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
	
	return bRet;
}

bool CUserModel::chgFriendRemark(IM::Buddy::IMChgFriendRemarkReq& chgFriendRemark)
{
	uint32_t user_id, friend_id;
	string   friend_nick;
	user_id = chgFriendRemark.user_id();
	friend_id =	chgFriendRemark.friend_id();
	friend_nick = chgFriendRemark.friend_nick();

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	bool bRet =false;
    if (pDBConn)
    {
    	friend_nick = pDBConn->EscapeString( friend_nick.c_str(), friend_nick.length());
		
		string strSql;
        strSql = "update IMFxUserRelationPrivate set friend_remark = '" + friend_nick + 	"' where user_uid = " + int2string(user_id) + " and friend_friendid = " + int2string(friend_id);
		
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("chgFriendRemark failed:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
	
	return bRet;
}


bool CUserModel::friendNotifyResp(IM::Buddy::IMFriendNotifyRes& friendNotifyRes)
{
	uint32_t user_id, friend_id;
	user_id = friendNotifyRes.from_user_id();
	friend_id =	friendNotifyRes.to_user_id();

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	bool bRet =false;
    if (pDBConn)
    {
		string strSql;
        strSql = "UPDATE IMFxUserRelationPrivate SET status= (CASE status WHEN -2 THEN -1 WHEN 4 THEN 3 ELSE status END )"\
			" where user_uid = " + int2string(user_id) + " and friend_friendid = " + int2string(friend_id);
		
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("friendNotifyResp failed:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
	
	return bRet;
}


bool CUserModel::createFriendGroup(IM::Buddy::IMCreateFriendGroupReq& createGroup, uint32_t& group_id)
{
	uint32_t user_id;
	string group_name;

	user_id = createGroup.user_id();
	group_name = createGroup.group_name();

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	group_id = -1;	
	bool bRet =false;
    if (pDBConn)
    {
    	group_name = pDBConn->EscapeString( group_name.c_str(), group_name.length());
		
		string strSql;
    //    strSql = "INSERT into IMFxUserRelationGroup(group_name, user_uid) \
	//		VALUES('" + group_name + "' ," +  int2string(user_id)+ ")";

	
		strSql = "INSERT INTO IMFxUserRelationGroup(group_id, group_name, user_uid, flag, sort) "\
				" SELECT MAX(group_id)+1,'" + group_name + "',user_uid,0,COUNT(sort)"\
				" FROM IMFxUserRelationGroup WHERE user_uid=" + int2string(user_id);
	
        bool bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("createFriendGroup failed:%s", strSql.c_str());					
        }
		else
		{
			strSql = "SELECT MAX(group_id) AS group_id FROM IMFxUserRelationGroup WHERE user_uid=" + int2string(user_id);
			CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
	        if(pResultSet)
			{		
				if (pResultSet->Next())
				{
					group_id = pResultSet->GetInt("group_id");
				}
				pResultSet->Clear();
	        }
			else
				log("createFriendGroup failed:%s", strSql.c_str());	
		}
		
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }

	return bRet;
}

bool CUserModel::delFriendGroup(IM::Buddy::IMDelFriendGroupReq& delGroup)
{
	uint32_t user_id, group_id;

	user_id = delGroup.user_id();
	group_id = delGroup.group_id();

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	bool bRet = false;
    if (pDBConn)
    {
    	do
		{
		string strSql;
		// AND flag=0 //can only delete user's group
        strSql = "delete from IMFxUserRelationGroup where group_id=" + int2string(group_id) + " AND flag=0 and user_uid = " + int2string(user_id);

        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("delFriendGroup failed:%s", strSql.c_str());
			break;
        }

		// default group 
		//SELECT group_id FROM IMFxUserRelationGroup WHERE flag=1 AND user_uid=7
		
		strSql = "update IMFxUserRelationPrivate set friend_groupid = 1 where friend_groupid= " \
			+ int2string(group_id) + " and user_uid = " + int2string(user_id);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
        	bRet = true;
            log("not find data:%s", strSql.c_str());
			break;
        }
		
		}while(false);
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");		
    }
	
	return bRet;
}

bool CUserModel::moveFriendToGroup(IM::Buddy::IMMoveFriendToGroupReq& moveFriend)
{

	uint32_t user_id, group_id;
	uint32_t friend_id_count;	
	string friend_id_string;
	
	user_id = moveFriend.user_id();
	group_id = moveFriend.group_id();	
	friend_id_count = moveFriend.friend_id_list_size();

	//redis
	CacheConn* pCacheConn = NULL;
	CAutoCache autoCache( "table_redis", &pCacheConn);

	CDBManager* pDBManager = CDBManager::getInstance();
   	CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	bool bRet = false;
   	if (pDBConn)
    {
			
		do
		{
		
		if(friend_id_count<=0)
			break;
		
		string strFriendId = int2string(moveFriend.friend_id_list(0));
		friend_id_string = "(" + strFriendId ;

		//redis smove srem
		string strUserId = int2string(user_id);
		string strFromFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+strUserId;
		string strFromBlackKey = "IMFxUserRelationPrivate:BlackSet:userId_"+strUserId;	
		string strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+strFriendId;
		if(pCacheConn && group_id==0)
		{			
			pCacheConn->smove(strFromFriendKey, strFromBlackKey, strFriendId);
			pCacheConn->srem(strToFriendKey, strUserId); 
		}
		
		for(uint32_t i = 1; i < friend_id_count;++i) {
			
			strFriendId = int2string(moveFriend.friend_id_list(i));
			
			friend_id_string += "," + strFriendId;
			
			if(pCacheConn && group_id==0)
			{
				pCacheConn->smove(strFromFriendKey, strFromBlackKey, strFriendId);
				
				strToFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+strFriendId;
				pCacheConn->srem(strToFriendKey, strUserId); 
			}
		}
		friend_id_string += ")";

		string strSql;
		//can't move friends from black_group to friend_group 
		strSql = "update  IMFxUserRelationPrivate set friend_groupid =(CASE friend_groupid WHEN 0 THEN 0 ELSE "+  int2string(group_id) + " END)"  \
			" WHERE user_uid = " + int2string(user_id) + " and friend_friendid in " +  friend_id_string;
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)	
		{       	 	
			log("moveFriendToGroup failed:%s", strSql.c_str());
			break;
		}
		 
		//or black_group if(group_id==0) 
		//黑名单则删除好友关系表
		
		if(group_id == 0)
		{
			/*	strSql = "delete from IMFxUserRelationPrivate  WHERE  friend_friendid  =" + int2string(user_id) \
				+  " and user_uid in " +  friend_id_string ;	*/
			strSql = "update IMFxUserRelationPrivate set status = -1 WHERE  friend_friendid  =" + int2string(user_id) \
				+  " and user_uid in " +  friend_id_string ;
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
	     	if(!bRet)	
      	 	{
	        	log("move friend to the blacklist failed:%s", strSql.c_str());
				break;
	 	 	}
		}
		
		}while(false);
       	pDBManager->RelDBConn(pDBConn);
   	}
   	else
  	{
  	
       log("no db connection for dffxIMDB_master");
    }    

	return bRet;
}

bool CUserModel::chgFriendGroupName(IM::Buddy::IMChgFriendGroupNameReq& chgGroupName)
{
	uint32_t user_id, group_id;
	string group_name;
	
	user_id = chgGroupName.user_id();
	group_id = chgGroupName.group_id();
	group_name = chgGroupName.group_name();

	CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");

	bool bRet = false;
    if (pDBConn)
    {
    	group_name = pDBConn->EscapeString( group_name.c_str(), group_name.length());

		string strSql;
      //  strSql = "UPDATE IMFxUserRelationGroup set group_name = '" + group_name + "' where group_id = " + int2string(user_id);
      	strSql = "UPDATE IMFxUserRelationGroup set group_name = '" + group_name + "' where group_id = " + int2string(group_id)\
      		+ " and user_uid = " + int2string(user_id);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        if(!bRet)
        {
            log("chgFriendGroupName failed:%s", strSql.c_str());			
        }
		
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }	

	return bRet;

}

bool CUserModel::addFiendInfo(uint32_t nUserId,list<IM::Buddy::IMCommonOperFrienInfo>& laddInfo,
	list<IM::Buddy::IMCommonOperFrienInfo>& lRefuseInfo, list<IM::Buddy::IMCommonOperFrienInfo>& lAgreeInfo)
{
	CDBManager* pDBManager = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");

	bool bRet=false;
	if (pDBConn)
	{		
		do{
		string strSql = "SELECT t1.`user_uid`,t1.`user_nickname`, t1.`user_sex`, t1.`user_headlink`,t2.* FROM IMFxUser t1 JOIN IMFxUserRelationPrivate t2"\
			" ON t1.user_uid=t2.friend_friendid WHERE t2.user_uid="+int2string(nUserId)+" and (t2.status= 2 or t2.status=4 or t2.status=-2);";
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		log("addfriend or refuse or agree information %s", strSql.c_str());
		if(pResultSet)
		{
			while (pResultSet->Next()) {
				if(pResultSet->GetInt("status") == 2)
				{
					IM::Buddy::IMCommonOperFrienInfo cInfo;
					uint32_t oper_user_id = pResultSet->GetInt("friend_friendid");
					string extra_info = pResultSet->GetString("extra_info");;
					uint32_t oper_time = pResultSet->GetInt("update_time");
				
					cInfo.set_oper_user_id(oper_user_id);
					cInfo.set_extra_info(extra_info);
					cInfo.set_oper_time(oper_time);

					//
					cInfo.set_user_nickname( pResultSet->GetString("user_nickname"));
					cInfo.set_user_headlink( pResultSet->GetString("user_headlink"));
					cInfo.set_user_gender( pResultSet->GetInt("user_sex"));
					cInfo.set_user_uid( pResultSet->GetInt("user_uid"));
				
					laddInfo.push_back(cInfo);
				}
				else if(pResultSet->GetInt("status") == -2)
				{
					IM::Buddy::IMCommonOperFrienInfo cInfo;
					uint32_t oper_user_id = pResultSet->GetInt("friend_friendid");
					string refuse_reason = pResultSet->GetString("extra_info");
					uint32_t oper_time = pResultSet->GetInt("update_time");
						
					cInfo.set_oper_user_id(oper_user_id);
					cInfo.set_extra_info(refuse_reason);
					cInfo.set_oper_time(oper_time);

					//
					cInfo.set_user_nickname( pResultSet->GetString("user_nickname"));
					cInfo.set_user_headlink( pResultSet->GetString("user_headlink"));
					cInfo.set_user_gender( pResultSet->GetInt("user_sex"));
					cInfo.set_user_uid( pResultSet->GetInt("user_uid"));
				
					lRefuseInfo.push_back(cInfo);
				}
				else if(pResultSet->GetInt("status") == 4)
				{
					IM::Buddy::IMCommonOperFrienInfo cInfo;
					uint32_t oper_user_id = pResultSet->GetInt("friend_friendid");
					uint32_t oper_time = pResultSet->GetInt("update_time");

					cInfo.set_oper_user_id(oper_user_id);
					cInfo.set_oper_time(oper_time);

					//
					cInfo.set_user_nickname( pResultSet->GetString("user_nickname"));
					cInfo.set_user_headlink( pResultSet->GetString("user_headlink"));
					cInfo.set_user_gender( pResultSet->GetInt("user_sex"));
					cInfo.set_user_uid( pResultSet->GetInt("user_uid"));
				
					lAgreeInfo.push_back(cInfo);
				} 		
				
			}
			pResultSet->Clear();
					
		}
		else
		{		
			log("get addfriend or refuse or agree information failed:%s", strSql.c_str());
			break;
		}

		//change adduserinfo status
		strSql = "UPDATE IMFxUserRelationPrivate SET status= (CASE status WHEN -2 THEN -1 WHEN 4 THEN 3 ELSE status END )"\
   			" where user_uid = " + int2string(nUserId);
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		log("chg addfriendinfo status %s", strSql.c_str());
		
		}while(false);
		pDBManager->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffxIMDB_slave.");
	}

	return bRet;
}


bool CUserModel::findUserInfo(string find_string, list<IM::BaseDefine::UserInfo>& lUser)
{
	int nId;
    bool bRet = false;
    
    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
    if (pDBConn) {
		find_string = pDBConn->EscapeString( find_string.c_str(), find_string.length());
		
        string strSql = "select * from IMFxUser where user_uid='" + find_string + "' or user_phone='" + find_string +"' LIMIT 10" ;  //+ " and status=0";
		log("strSql=%s", strSql.c_str());
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet){			
			int nId, nuser_uid, nLevel, nGender,nStatus;
			string strNickName, strHeadLink , strBirthday,strUser_phone;
			
			while (pResultSet->Next()) {
				nId = pResultSet->GetInt("user_uid");
				nLevel = pResultSet->GetInt("user_level");
				nStatus = pResultSet->GetInt("user_status");
				nGender = pResultSet->GetInt("user_sex");
				strNickName = pResultSet->GetString("user_nickname");
				strHeadLink = pResultSet->GetString("user_headlink");
				strBirthday = pResultSet->GetString("user_birthday");
				nuser_uid = pResultSet->GetInt("user_uid");
				strUser_phone = pResultSet->GetString("user_phone");

				IM::BaseDefine::UserInfo cUser;
				cUser.set_user_id(nId);
				cUser.set_user_nickname(strNickName);
				cUser.set_user_gender(nGender);
				cUser.set_user_birthday(strBirthday);
				cUser.set_user_headlink(strHeadLink);
				cUser.set_user_level(nLevel);
			    cUser.set_user_status(nStatus);
				cUser.set_user_uid(nuser_uid);
				cUser.set_user_phone(strUser_phone);	
				
				cUser.set_user_type(pResultSet->GetInt("user_type"));
				cUser.set_user_ischeck(pResultSet->GetInt("user_ischeck"));
				cUser.set_user_desc(pResultSet->GetString("user_desc"));

				lUser.push_back(cUser);
			}

			pResultSet->Clear();
			bRet = true;
        }else{
				log("findUserInfo failed:%s", strSql.c_str());
		}
				
		pDBManger->RelDBConn(pDBConn);
	
	}else{
		log("no db connection for dffxIMDB_slave");
	}
	return bRet;	
}

bool CUserModel::notifyUserStatus(uint32_t nUserId, IM::BaseDefine::UserStatType userStatus)
{
	CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_master");

	bool bRet = false;
    if (pDBConn) 
   	{
   		//set online/offline
		string strSql = "UPDATE IMFxUser SET user_status="+int2string((uint32_t)userStatus)+" WHERE user_uid=" + int2string(nUserId) ; 
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		
		log("strSql=%s", strSql.c_str());
		
		pDBManger->RelDBConn(pDBConn);

		bRet = true;
   	}
    else
   	{
		log("no db connection for dffxIMDB_master");
	}
	
	return bRet;
}

bool CUserModel::getFriendsidReq(uint32_t nUserId, list<string>& lUser)
{
	//redis find
	bool isRedisFind = false;
	{
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{
			string strFriendKey = "IMFxUserRelationPrivate:FriendSet:userId_"+int2string(nUserId);
			
			isRedisFind = pCacheConn->smembers(strFriendKey, lUser);
		}
	}
	if(isRedisFind)
		return isRedisFind;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");

	bool bRet = false;
	if (pDBConn) 
	{
		//set online/offline
		string strSql = "SELECT friend_friendid FROM IMFxUserRelationPrivate WHERE status>=3 AND user_uid="+int2string(nUserId) ; 
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());

		if(pResultSet)
		{
			log("strSql=%s", strSql.c_str());

			while(pResultSet->Next())
			{
				lUser.push_back(pResultSet->GetString("friend_friendid"));
			}

			pResultSet->Clear();

			bRet = true;
		}
		
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffxIMDB_slave");
	}

	return bRet;
}

bool CUserModel::isInBlackGroup(uint32_t nUserId, uint32_t nFriendId)
{
	//redis find
	bool isRedisFind = false;	
	bool bRet = false;
	
	{
		CacheConn* pCacheConn = NULL;
		CAutoCache autoCache( "table_redis", &pCacheConn);

		if(pCacheConn)
		{
			string strBlackKey = "IMFxUserRelationPrivate:BlackSet:userId_"+int2string(nUserId);
			long lRet = pCacheConn->sismember(strBlackKey, int2string(nFriendId));
			if( lRet>=0)
			{
				isRedisFind=true;
				bRet = lRet==1?true:false;
				log("isInBlackGroup user-->peer %d-->%d isBlack?%d", nUserId, nFriendId, lRet);
			}
		}
	}
	if(isRedisFind)
		return bRet;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");

	if (pDBConn) 
	{
		//friend_groupid=0
		string strSql = "SELECT * FROM IMFxUserRelationPrivate WHERE friend_groupid=0 AND user_uid="+int2string(nUserId)\
			+" AND friend_friendid ="+int2string(nFriendId); 
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());

		if(pResultSet)
		{
			log("strSql=%s", strSql.c_str());

			if(pResultSet->Next())
			{//isBlack
				bRet = true;
			}

			pResultSet->Clear();
		}
		
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffxIMDB_slave");
	}

	return bRet;
}

bool CUserModel::msgServerRestart(uint32_t msg_server_id, uint32_t cur_time)
{
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_master");

	if (pDBConn) 
	{
		//friend_groupid=0
		string strSql = "UPDATE IMFxUser SET pc_status=2 WHERE pc_msgserver_id="+int2string(msg_server_id)\
			+"  AND pc_logintime<"+int2string(cur_time)+" ;";			

		log("strSql=%s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());

		strSql = "UPDATE IMFxUser SET mobile_status=2 WHERE mobile_msgserver_id="\
			+int2string(msg_server_id)+"  AND mobile_logintime<"+int2string(cur_time)+" ;";

		log("strSql=%s", strSql.c_str());

		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffxIMDB_master");
	}

	return bRet;

}
bool CUserModel::updateUserStatus(uint32_t user_uid,uint32_t status,uint32_t msg_server_id,uint32_t client_type,uint32_t cur_time)
{
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_master");

	if (pDBConn) 
	{
		//friend_groupid=0
		string strSql = "";

		if(CHECK_CLIENT_TYPE_PC(client_type))
		{
			strSql = "UPDATE IMFxUser SET pc_status="+int2string(status)+", pc_msgserver_id="+int2string(msg_server_id)\
				+"	, pc_logintime="+int2string(cur_time)+" WHERE user_uid="+int2string(user_uid)+";";
		}
		else
		{
			strSql = "UPDATE IMFxUser SET mobile_status="+int2string(status)+" , mobile_msgserver_id="\
				+int2string(msg_server_id)+"  , mobile_logintime="+int2string(cur_time)+" WHERE user_uid="+int2string(user_uid)+";";
		}

		log("strSql=%s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffxIMDB_master");
	}

	return bRet;

}


bool CUserModel::orderstatusread(uint32_t user_id, uint32_t order_id, bool& is_null)
{
		bool bRet = false;
		CacheConn* pCacheConn = NULL;	
		CAutoCache autoCache( "unread", &pCacheConn);		
		if(pCacheConn)	
		{	
			string strTableKey = "order_prompt_message_" + int2string(user_id);			
			bRet = pCacheConn->hdel( strTableKey, int2string(order_id));		
			if(bRet==-1)			
			{			
				log("redis set failed %s", strTableKey.c_str());	
				return bRet;
			}
			bRet = true;
			
			int nRet = pCacheConn->isExists(strTableKey);
			if(nRet == false)
			{
				is_null = 1;
			}
			else
			{
				is_null = 0;
			}
		}
		return bRet;
}


