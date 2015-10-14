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


CClient::CClient(const string& strName, const string& strPass, const string strDomain):
m_strName(strName),
m_strPass(strPass),
m_strLoginDomain(strDomain),
m_nLastGetUser(0)
{
	g_pConn = NULL;
	g_bLogined = false;
	g_configReturn = false;
}

CClient::~CClient()
{
}

void CClient::TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
 	CClient* pCClient = (CClient*)callback_data;
    if ( pCClient->g_pConn) {
        uint64_t cur_time = get_tick_count();
        pCClient->g_pConn->OnTimer(cur_time);
    }

	if(pCClient->g_pConn == NULL)
		pCClient->connect();
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
		g_configReturn = false;
        PROMPTION;
        return;
    }
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(strResp, value))
    {
        printf("login falied. parse response error:%s\n", strResp.c_str());
		g_configReturn = false;
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
			g_configReturn = false;
            PROMPTION;
            return;
        }
        strPriorIp = value["priorIP"].asString();
        strBackupIp = value["backupIp"].asString();
        nPort = string2int(value["port"].asString());
        
    } catch (std::runtime_error msg) {
        printf("login falied. get json error:%s\n", strResp.c_str());
		g_configReturn = false;
        PROMPTION;
        return;
    }

    g_pConn = new ClientConn(this);
    m_nHandle = g_pConn->connect(strPriorIp.c_str(), nPort, m_strName, m_strPass);
    if(m_nHandle != INVALID_SOCKET)
    {
        netlib_register_timer(CClient::TimerCallback, (void*)this, 1000);	
		g_configReturn = true;
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
	g_pConn = NULL;
}

void CClient::onClose()
{
    g_pConn = NULL;
	g_bLogined = false;
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

uint32_t CClient::sendMsg(uint32_t nToId, IM::BaseDefine::MsgType nType, const string &strMsg)
{
    uint32_t nFromId = m_cSelfInfo.user_id();

	log("nFromId = %d  nToId = %d", nFromId, nToId);
    return g_pConn->sendMessage(nFromId, nToId, nType, strMsg);
}

void CClient::onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    printf("send msg succes. seqNo:%u, msgId:%u  nSendId = %d   nRecvId = %d\n", nSeqNo, nMsgId, nSendId, nRecvId);
}

void CClient::onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string &strMsgData)
{
    printf("CClient::onRecvMsg");
//	log("CClient::onRecvMsg from=%u to=%u msg=%s", nFromId, nToId, strMsgData.c_str());
}
