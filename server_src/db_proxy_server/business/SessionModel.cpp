/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：SessionModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年03月16日
*   描    述：
*
================================================================*/
#include "SessionModel.h"
#include "MessageModel.h"
#include "GroupMessageModel.h"


CSessionModel* CSessionModel::m_pInstance = NULL;

CSessionModel* CSessionModel::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CSessionModel();
    }
    
    return m_pInstance;
}

void CSessionModel::getRecentSession(uint32_t nUserId, uint32_t lastTime, list<IM::BaseDefine::ContactSessionInfo>& lsContact)
{
	//redis find
	CacheConn* pCacheConn = NULL;
	CAutoCache autoCache( "table_redis", &pCacheConn);
	
	if(pCacheConn)
	{
		string strZsetKey = "IMRecentSession:Zset:userId_"+int2string(nUserId);
		list<string> lsRetValue;
		bool bRet = false;
		if(lastTime>0)
			bRet = pCacheConn->zrevrangebyscore( strZsetKey, "+inf", int2string(lastTime), 100, lsRetValue);
		else	
			bRet = pCacheConn->zrevrange( strZsetKey, 0, 99, lsRetValue);
		if(bRet)	
		{
			
			for(auto _it=lsRetValue.begin();_it!=lsRetValue.end(); ++_it)
			{
				map<string, string> mapRetValue;
				string strDataKey = "IMRecentSession:Data:id_" + *_it ;
				if(!pCacheConn->hgetAll( strDataKey, mapRetValue) || mapRetValue.size()==0)
				{
					pCacheConn->zrem( strZsetKey, *_it);
					
					continue;
				}
				
				IM::BaseDefine::ContactSessionInfo cRelate;
                cRelate.set_session_id(string2int(mapRetValue["peerId"]));
                cRelate.set_session_status(::IM::BaseDefine::SessionStatusType(string2int(mapRetValue["status"])));
                
                IM::BaseDefine::SessionType nSessionType = IM::BaseDefine::SessionType(string2int(mapRetValue["type"]));
                if(IM::BaseDefine::SessionType_IsValid(nSessionType))
                {
                    cRelate.set_session_type(IM::BaseDefine::SessionType(nSessionType));
                    cRelate.set_updated_time(string2int(mapRetValue["updated"]));
                    lsContact.push_back(cRelate);
                }
                else
                {
                	pCacheConn->zrem( strZsetKey, *_it);
					pCacheConn->del( strDataKey);
					string strSelectKey = "IMRecentSession:Select:u2p"+mapRetValue["userId"]+"_"+mapRetValue["peerId"]+"_"+mapRetValue["type"];
					pCacheConn->del( strSelectKey);
                    log("invalid sessionType. userId=%u, peerId=%s, sessionType=%u", nUserId, mapRetValue["peerId"].c_str(), nSessionType);
                }
			}
		}
	}
	else
    {
        log("no redis connection for table_redis");
    }
	
	if(!lsContact.empty())
    {
        fillSessionMsg(nUserId, lsContact);
		return;
    }

	//db find
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if (pDBConn)
    {
      //  string strSql = "select * from IMRecentSession where userId = " + int2string(nUserId) + " and status = 0 and updated >" + int2string(lastTime) + " order by updated desc limit 100";

	  string strSql = "select t1.* from IMRecentSession t1,IMFxUser t2 where userId=" + int2string(nUserId)+ " and t1.status = 0 and t1.peerId=t2.user_uid and t1.updated >" + int2string(lastTime) + " order by t1.updated desc limit 100";
			  
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::ContactSessionInfo cRelate;
                uint32_t nPeerId = pResultSet->GetInt("peerId");
                cRelate.set_session_id(nPeerId);
                cRelate.set_session_status(::IM::BaseDefine::SessionStatusType(pResultSet->GetInt("status")));
                
                IM::BaseDefine::SessionType nSessionType = IM::BaseDefine::SessionType(pResultSet->GetInt("type"));
                if(IM::BaseDefine::SessionType_IsValid(nSessionType))
                {
                    cRelate.set_session_type(IM::BaseDefine::SessionType(nSessionType));
                    cRelate.set_updated_time(pResultSet->GetInt("updated"));
                    lsContact.push_back(cRelate);

					//redis insert
					if(pCacheConn)
					{
						map<string, string> mapRetValue;
						mapRetValue["id"] = pResultSet->GetString("id");
						mapRetValue["userId"] = pResultSet->GetString("userId");
						mapRetValue["peerId"] = pResultSet->GetString("peerId");
						mapRetValue["type"] = pResultSet->GetString("type");
						mapRetValue["status"] = pResultSet->GetString("status");
						mapRetValue["updated"] = pResultSet->GetString("updated");
						
						string strZsetKey = "IMRecentSession:Zset:userId_" + mapRetValue["userId"];
						pCacheConn->zadd(strZsetKey, cRelate.updated_time(), mapRetValue["id"]);
												
						string strDataKey = "IMRecentSession:Data:id_" + mapRetValue["id"] ;
						pCacheConn->hmset(strDataKey, mapRetValue);

						string strSelectKey = "IMRecentSession:Select:u2p_"+mapRetValue["userId"]+"_"+mapRetValue["peerId"]+"_"+mapRetValue["type"];		
						int nRet = pCacheConn->hset( strSelectKey, "id", mapRetValue["id"]);
					}
                }
                else
                {
                    log("invalid sessionType. userId=%u, peerId=%u, sessionType=%u", nUserId, nPeerId, nSessionType);
                }
            }
            pResultSet->Clear();
        }
        else
        {
            log("no result set for sql: %s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
        if(!lsContact.empty())
        {
            fillSessionMsg(nUserId, lsContact);

			//redis remove > 100
			if(pCacheConn)
			{
				redisRemoveOutCnt(nUserId, 100, pCacheConn);
			}
        }
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
	
}

uint32_t CSessionModel::getSessionId(uint32_t nUserId, uint32_t nPeerId, uint32_t nType, bool isAll)
{
	
    uint32_t nSessionId = INVALID_VALUE;
	
	//redis find
	CacheConn* pCacheConn = NULL;
	CAutoCache autoCache( "table_redis", &pCacheConn);
	bool isRedisFind = false;
	if(pCacheConn)
	{
		string strSelectKey = "IMRecentSession:Select:u2p_"+int2string(nUserId)+"_"+int2string(nPeerId)+"_"+int2string(nType);
		
		string strValue = pCacheConn->hget( strSelectKey, "id");
		if(!strValue.empty())	
		{
			isRedisFind = true;
			nSessionId = string2int( strValue);
		}
	}
	else
    {
        log("no redis connection for table_redis");
    }
	if(isRedisFind)
		return nSessionId;

	//db find
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if(pDBConn)
    {
        string strSql;
        if (isAll) {
            strSql= "select * from IMRecentSession where userId=" + int2string(nUserId) + " and peerId=" + int2string(nPeerId) + " and type=" + int2string(nType);
        }
        else
        {
            strSql= "select * from IMRecentSession where userId=" + int2string(nUserId) + " and peerId=" + int2string(nPeerId) + " and type=" + int2string(nType) + " and status=0";
        }
        
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            if (pResultSet->Next()) {
                nSessionId = pResultSet->GetInt("id");

				//redis insert
				if(pCacheConn && nSessionId != INVALID_VALUE)
				{					
					uint32_t nTimeNow = time(NULL);
					
					map<string, string> mapRetValue;
					mapRetValue["id"] = pResultSet->GetString("id");
					mapRetValue["userId"] = pResultSet->GetString("userId");
					mapRetValue["peerId"] = pResultSet->GetString("peerId");
					mapRetValue["type"] = pResultSet->GetString("type");
					mapRetValue["status"] = "0";
					mapRetValue["updated"] = int2string( nTimeNow);
					
					string strZsetKey = "IMRecentSession:Zset:userId_" + mapRetValue["userId"];
					pCacheConn->zadd(strZsetKey, nTimeNow, mapRetValue["id"]);
					
					string strDataKey = "IMRecentSession:Data:id_" + mapRetValue["id"] ;
					pCacheConn->hmset(strDataKey, mapRetValue);

					string strSelectKey = "IMRecentSession:Select:u2p_"+mapRetValue["userId"]+"_"+mapRetValue["peerId"]+"_"+mapRetValue["type"];		
					int nRet = pCacheConn->hset( strSelectKey, "id", mapRetValue["id"]);

					//redis remove > 100
					redisRemoveOutCnt(nUserId, 100, pCacheConn);
				}
            }
            pResultSet->Clear();
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }

    return nSessionId;
}

bool CSessionModel::updateSession(uint32_t nSessionId, uint32_t nUpdateTime)
{
	//redis update
	CacheConn* pCacheConn = NULL;
	CAutoCache autoCache( "table_redis", &pCacheConn);
	bool isRedisFind = false;
	if(pCacheConn)
	{
		string strDataKey = "IMRecentSession:Data:id_" + int2string(nSessionId) ;
		map<string, string> mapRetValue;
		if(pCacheConn->hgetAll(strDataKey, mapRetValue) && mapRetValue.size()!=0)
		{
			if(mapRetValue["userId"]=="")
			{
				log("!!! userId == null");
			}
			else
			{
				pCacheConn->hset( strDataKey, "updated", int2string(nUpdateTime));
			
				string strZsetKey = "IMRecentSession:Zset:userId_" + mapRetValue["userId"];
				pCacheConn->zadd(strZsetKey, nUpdateTime, int2string(nSessionId));

				isRedisFind = true;
			}
		}
		
	}
	else
    {
        log("no redis connection for table_redis");
    }
	
	if(isRedisFind)
		return true;
	
	//db update
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        string strSql = "update IMRecentSession set `updated`="+int2string(nUpdateTime) + " where id="+int2string(nSessionId);
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
    return bRet;
}

bool CSessionModel::removeSession( uint32_t nUserId, uint32_t nPeerId, uint32_t nType)
{
    bool bRet = false;
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        uint32_t nNow = (uint32_t) time(NULL);
        string strSql = "update IMRecentSession set status = 1, updated="+int2string(nNow)+" where userId="\
			+ int2string(nUserId) + " and peerId=" + int2string(nPeerId) + " and type=" + int2string(nType);
		log(" strSql = %s", strSql.c_str());
        bRet = pDBConn->ExecuteUpdate(strSql.c_str());
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }

	//redis delete
	CacheConn* pCacheConn = NULL;
	CAutoCache autoCache( "table_redis", &pCacheConn);
	if(pCacheConn )
	{
		string strSelectKey = "IMRecentSession:Select:u2p_"+int2string(nUserId)+"_"+int2string(nPeerId)+"_"+int2string(nType);
		string strValue = pCacheConn->hget( strSelectKey, "id");
		if(!strValue.empty())	
		{
			string strZsetKey = "IMRecentSession:Zset:userId_"+int2string(nUserId);
			pCacheConn->zrem(strZsetKey, strValue);

			string strDataKey = "IMRecentSession:Data:id_" + strValue ;
			pCacheConn->del(strDataKey);
		}
		int nRet = pCacheConn->del( strSelectKey);
		if(!nRet)	
		{
			log("redis hdel failed %s", strSelectKey.c_str());
		}
	}
	
    return bRet;
}

uint32_t CSessionModel::addSession(uint32_t nUserId, uint32_t nPeerId, uint32_t nType)
{
	CAutoLock autoLock(&m_csLock);
	
    uint32_t nSessionId = INVALID_VALUE;
    
    nSessionId = getSessionId(nUserId, nPeerId, nType, true);
    uint32_t nTimeNow = time(NULL);
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        if(INVALID_VALUE != nSessionId)
        {
            string strSql = "update IMRecentSession set status=0, updated=" + int2string(nTimeNow) + " where id=" + int2string(nSessionId);
            bool bRet = pDBConn->ExecuteUpdate(strSql.c_str());
            if(!bRet)
            {
                nSessionId = INVALID_VALUE;
            }
            log("has relation ship set status");
        }
        else
        {
            string strSql = "insert into IMRecentSession (`userId`,`peerId`,`type`,`status`,`created`,`updated`) values(?,?,?,?,?,?)";
            // 必须在释放连接前delete CPrepareStatement对象，否则有可能多个线程操作mysql对象，会crash
            CPrepareStatement* stmt = new CPrepareStatement();
            if (stmt->Init(pDBConn->GetMysql(), strSql))
            {
                uint32_t nStatus = 0;
                uint32_t index = 0;
                stmt->SetParam(index++, nUserId);
                stmt->SetParam(index++, nPeerId);
                stmt->SetParam(index++, nType);
                stmt->SetParam(index++, nStatus);
                stmt->SetParam(index++, nTimeNow);
                stmt->SetParam(index++, nTimeNow);
                bool bRet = stmt->ExecuteUpdate();
                if (bRet)
                {
                    nSessionId = pDBConn->GetInsertId();

					//redis insert
					CacheConn* pCacheConn = NULL;
					CAutoCache autoCache( "table_redis", &pCacheConn);
					if(pCacheConn )
					{
						map<string, string> mapRetValue;
						mapRetValue["id"] = int2string(nSessionId);
						mapRetValue["userId"] = int2string(nUserId);
						mapRetValue["peerId"] = int2string(nPeerId);
						mapRetValue["type"] = int2string(nType);
						mapRetValue["status"] = int2string(nStatus);
						mapRetValue["updated"] = int2string(nTimeNow);
						
						string strZsetKey = "IMRecentSession:Zset:userId_"+int2string(nUserId);
						pCacheConn->zadd(strZsetKey, nTimeNow, mapRetValue["id"]);						
						
						string strDataKey = "IMRecentSession:Data:id_" + mapRetValue["id"] ;
						pCacheConn->hmset(strDataKey, mapRetValue);

						string strSelectKey = "IMRecentSession:Select:u2p_"+mapRetValue["userId"]+"_"+mapRetValue["peerId"]+"_"+mapRetValue["type"];		
						int nRet = pCacheConn->hset( strSelectKey, "id", mapRetValue["id"]);
						if(nRet==-1)	
						{
							log("redis hset failed %s", strSelectKey.c_str());
						}

						//redis remove > 100
						redisRemoveOutCnt(nUserId, 100, pCacheConn);
					}
					else
				    {
				        log("no redis connection for table_redis");
				    }
                }
                else
                {
                    log("insert IMRecentSession failed. %s", strSql.c_str());
                }
            }
            delete stmt;
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
					
    return nSessionId;
}



void CSessionModel::fillSessionMsg(uint32_t nUserId, list<IM::BaseDefine::ContactSessionInfo>& lsContact)
{
    for (auto it=lsContact.begin(); it!=lsContact.end();)
    {
        uint32_t nMsgId = 0;
        string strMsgData;
        IM::BaseDefine::MsgType nMsgType;
        uint32_t nFromId = 0;
        if( it->session_type() == IM::BaseDefine::SESSION_TYPE_SINGLE)
        {
            nFromId = it->session_id();
            CMessageModel::getInstance()->getLastMsg(it->session_id(), nUserId, nMsgId, strMsgData, nMsgType);
        }
        else
        {
            CGroupMessageModel::getInstance()->getLastMsg(it->session_id(), nMsgId, strMsgData, nMsgType, nFromId);
        }
        if(!IM::BaseDefine::MsgType_IsValid(nMsgType))
        {
            it = lsContact.erase(it);
        }
        else
        {
            it->set_latest_msg_from_user_id(nFromId);
            it->set_latest_msg_id(nMsgId);
            it->set_latest_msg_data(strMsgData);
            it->set_latest_msg_type(nMsgType);
            ++it;
        }
    }
}

//redis remove > 100
bool CSessionModel::redisRemoveOutCnt(uint32_t nUserId, int nLimit, CacheConn* pCacheConn)
{
	//CacheConn* pCacheConn = NULL;
	//CAutoCache autoCache( "table_redis", &pCacheConn);
	bool bRet = false;
	
	if(pCacheConn)
	{	
		string strZsetKey = "IMRecentSession:Zset:userId_"+int2string(nUserId);
		int nCnt = pCacheConn->zcard( strZsetKey);
		if(nCnt>nLimit)
		{
			list<string> lsRetValue;
			bool bRet = pCacheConn->zrange(strZsetKey, 0, nCnt-nLimit-1, lsRetValue);
			for(auto _it=lsRetValue.begin();_it!=lsRetValue.end(); ++_it)
			{
				map<string, string> mapRetValue;
				string strDataKey = "IMRecentSession:Data:id_" + *_it ;
				if(pCacheConn->hgetAll( strDataKey, mapRetValue) && mapRetValue.size()!=0)
				{
					string strSelectKey = "IMRecentSession:Select:u2p_"+mapRetValue["userId"]+"_"+mapRetValue["peerId"]+"_"+mapRetValue["type"];
					pCacheConn->del(strSelectKey);
				}
				pCacheConn->del(strDataKey);
				pCacheConn->zrem( strZsetKey,*_it);
			}
		}
		bRet = true;
	}
	else
    {
        log("no redis connection for table_redis");
    }

	return bRet;
}











