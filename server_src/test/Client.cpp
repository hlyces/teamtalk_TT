/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Client.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
================================================================*/
#include "Client.h"
#include "HttpClient.h"
#include "Common.h"
#include "json/json.h"
//#include "ClientConn.h"


//static ClientConn*  g_pConn = NULL;

//static bool         g_bLogined = false;

CClient::CClient(const string& strName, const string& strPass, const string strDomain):
m_strName(strName),
m_strPass(strPass),
m_strLoginDomain(strDomain),
m_nLastGetUser(0)
{
	g_pConn = NULL;
	g_bLogined = false;
}

CClient::~CClient()
{
}

void CClient::TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
 	CClient* pCClient = (CClient*)callback_data;
    if (pCClient->g_pConn && 
		pCClient->g_bLogined) {
        uint64_t cur_time = get_tick_count();
        pCClient->g_pConn->OnTimer(cur_time);
    }
}



void CClient::onError(uint32_t nSeqNo, uint32_t nCmd,const string& strMsg)
{
    g_imlog.Error("get error:%d, msg:%s", nCmd, strMsg.c_str());
}

void CClient::connect()
{
    CHttpClient httpClient;
    string strUrl = m_strLoginDomain + "/msg_server";
    string strResp;
    CURLcode nRet = httpClient.Get(strUrl, strResp);
    if(nRet != CURLE_OK)
    {
        printf("login falied. access url:%s error\n", strUrl.c_str());
        PROMPTION;
        return;
    }
    Json::Reader reader(Json::Features::strictMode());
    Json::Value value;
    if(!reader.parse(strResp, value))
    {
        printf("login falied. parse response error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }
    string strPriorIp, strBackupIp;
    uint16_t nPort;
    try {
        uint32_t nRet = value["code"].asUInt();
        if(nRet != 0)
        {
            string strMsg = value["msg"].asString();
            printf("login falied. errorMsg:%s\n", strMsg.c_str());
            PROMPTION;
            return;
        }
        strPriorIp = value["priorIP"].asString();
        strBackupIp = value["backupIp"].asString();
        nPort = string2int(value["port"].asString());
        
    } catch (std::runtime_error msg) {
        printf("login falied. get json error:%s\n", strResp.c_str());
        PROMPTION;
        return;
    }
   
  	//string strPriorIp = "192.168.1.49";
  	//string strPriorIp = "127.0.0.1";
 	//uint16_t nPort=8000;
    g_pConn = new ClientConn(this);
    m_nHandle = g_pConn->connect(strPriorIp.c_str(), nPort, m_strName, m_strPass);
    if(m_nHandle != INVALID_SOCKET)
    {
        netlib_register_timer(CClient::TimerCallback, (void*)this, 1000);		
    }
    else
    {
        printf("invalid socket handle\n");
    }
}

void CClient::onConnect()
{
    login(m_strName, m_strPass);
}


void CClient::close()
{
    g_pConn->Close();
}

void CClient::onClose()
{
    
}

uint32_t CClient::login(const string& strName, const string& strPass)
{
    return g_pConn->login(strName, strPass);
}

void CClient::onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser)
{
    if(nResultCode != 0)
    {
        printf("login failed.errorCode=%u, msg=%s\n",nResultCode, strMsg.c_str());
		log("login failed.errorCode=%u, msg=%s\n",nResultCode, strMsg.c_str());
        return;
    }
    if(pUser)
    {
        m_cSelfInfo = *pUser;
		printf("onLogin::user_id = %d", m_cSelfInfo.user_id());
		log("onLogin::user_id = %d", m_cSelfInfo.user_id());
        g_bLogined = true;
    }
    else
    {
        printf("pUser is null\n");
    }
}

uint32_t CClient::getChangedUser()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUser(nUserId, m_nLastGetUser);
}

void CClient::onGetChangedUser(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
    for(auto it=lsUser.begin(); it!=lsUser.end(); ++it)
    {
        IM::BaseDefine::UserInfo* pUserInfo = new IM::BaseDefine::UserInfo();
        *pUserInfo = *it;
        uint32_t nUserId = pUserInfo->user_id();
        string strNick = pUserInfo->friend_remark();
        if(it->user_status() != 3)
        {
            auto it1 = m_mapId2UserInfo.find(nUserId);
            if(it1 == m_mapId2UserInfo.end())
            {
                m_mapId2UserInfo.insert(pair<uint32_t, IM::BaseDefine::UserInfo*>(nUserId, pUserInfo));
            }
            else
            {
            	m_mapNick2UserInfo.erase(pUserInfo->friend_remark());
            	delete it1->second;
                m_mapId2UserInfo[nUserId] = pUserInfo;
            }
            
            auto it2 = m_mapNick2UserInfo.find(strNick);
            if(it2 == m_mapNick2UserInfo.end())
            {
                m_mapNick2UserInfo.insert(pair<string, IM::BaseDefine::UserInfo*>(strNick, pUserInfo));
            }
            else
            {
            	m_mapId2UserInfo.erase(pUserInfo->user_id());
                delete it2->second;
                m_mapNick2UserInfo[strNick] = pUserInfo;
            }
        }
    }
}

uint32_t CClient::getUserInfo(list<uint32_t>& lsUserId)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUserInfo(nUserId, lsUserId);
}

void CClient::onGetUserInfo(uint32_t nSeqNo, const list<IM::BaseDefine::UserInfo> &lsUser)
{
    
}

uint32_t CClient::sendMsg(uint32_t nToId, IM::BaseDefine::MsgType nType, const string &strMsg)
{
	

    uint32_t nFromId = m_cSelfInfo.user_id();
	log("nfromId = %d ntoId = %d ", nFromId, nToId);
    return g_pConn->sendMessage(nFromId, nToId, nType, strMsg);
}

void CClient::onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    printf("send msg succes. seqNo:%u, msgId:%u\n", nSeqNo, nMsgId);
}

uint32_t CClient::addFriendReq(uint32_t nToId, uint32_t nGroupId, const string &strRemark)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->addFriendReq(nFromId, nToId, nGroupId, strRemark);
}

uint32_t CClient::delFriendReq(uint32_t nToId)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->delFriendReq(nFromId, nToId);
}

uint32_t CClient::chgFriendRemarkReq(uint32_t nToId, const string &strRemark)
{
    uint32_t nFromId = m_cSelfInfo.user_id();
    return g_pConn->chgFriendRemarkReq(nFromId, nToId, strRemark);
}

uint32_t CClient::getAddFriendReq()
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->getAddFriendReq(nFromId);

}

uint32_t CClient::findFriendReq(string find_string)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->findFriendReq(nFromId, find_string);
}

uint32_t CClient::createFriendGroup(string group_name)
{	
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->createFriendGroup(nFromId, group_name);
}

uint32_t CClient::delFriendGroup(uint32_t  group_id)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->delFriendGroup(nFromId, group_id);
}

uint32_t CClient::moveFriendToGroup(uint32_t  group_id, list<uint32_t > friend_id_list)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->moveFriendToGroup(nFromId, group_id, friend_id_list);
}

uint32_t CClient::chgFriendGroupName(uint32_t  group_id, string group_name)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->chgFriendGroupName(nFromId, group_id, group_name);
}

uint32_t CClient::fileReq( uint32_t to_user_id, string file_name, uint32_t type)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->fileReq(nFromId, to_user_id, file_name, type);
}

uint32_t CClient::addOffFile( uint32_t to_user_id, string task_id, string file_name)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->addOffFile(nFromId, to_user_id, task_id, file_name);
}

uint32_t CClient::delOffFile( uint32_t to_user_id, string task_id)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->delOffFile(to_user_id, nFromId, task_id);
}
uint32_t CClient::hasOffFile()
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->hasOffFile(nFromId);
}

uint32_t CClient::cleanMsg( uint32_t session_id)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->cleanMsg(nFromId, session_id);
}

uint32_t CClient::orderStatusRead(uint32_t order_id)
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->orderStatusRead(nFromId, order_id);
}

uint32_t CClient::getFriendGroup()
{
	uint32_t nFromId = m_cSelfInfo.user_id();
	log("user_id = %d", nFromId);
	return g_pConn->getFriendGroup(nFromId);
}


uint32_t CClient::sendFileRequest(uint32_t to_user_id, string file_name, uint32_t file_size)
{
	uint32_t from_user_id = m_cSelfInfo.user_id();
	log("user_id = %d", from_user_id);
	return g_pConn->sendFileRequest(from_user_id, to_user_id, file_name, file_size);
}

uint32_t CClient::getUnreadMsgCnt()
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getUnreadMsgCnt(nUserId);
}

void CClient::onGetUnreadMsgCnt(uint32_t nSeqNo, uint32_t nUserId, uint32_t nTotalCnt, const list<IM::BaseDefine::UnreadInfo>& lsUnreadCnt)
{
    
}

uint32_t CClient::getRecentSession(int time)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getRecentSession(nUserId, time);
}

uint32_t CClient::delRecentSession(uint32_t nSessionId, IM::BaseDefine::SessionType session_type)
{
	uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->delRecentSession(nUserId, nSessionId, session_type);
}


void CClient::onGetRecentSession(uint32_t nSeqNo, uint32_t nUserId, const list<IM::BaseDefine::ContactSessionInfo> &lsSession)
{
    
}

uint32_t CClient::getMsgList(IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    uint32_t nUserId = m_cSelfInfo.user_id();
    return g_pConn->getMsgList(nUserId, nType, nPeerId, nMsgId, nMsgCnt);
}

void CClient::onGetMsgList(uint32_t nSeqNo, uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId, uint32_t nMsgCnt, const list<IM::BaseDefine::MsgInfo> &lsMsg)
{
    
}

void CClient::onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string &strMsgData)
{
    printf("CClient::onRecvMsg\n");
	printf("CClient::onRecvMsg from=%u to=%u msg=%s", nFromId, nToId, strMsgData.c_str());
	log("CClient::onRecvMsg from=%u to=%u msg=%s", nFromId, nToId, strMsgData.c_str());
}
