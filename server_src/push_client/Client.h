/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Client.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月20日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <iostream>
#include "ostype.h"
#include "IM.BaseDefine.pb.h"
#include "IPacketCallback.h"
#include "ClientConn.h"

using namespace std;

class CClient:public IPacketCallback
{
public:
    CClient(const string& strName, const string& strPass, const string strDomain="121.201.36.80:8080");
    ~CClient();
public:
    string getName(){ return m_strName; }
    string getDomain() { return m_strLoginDomain; }
    static void TimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);
    
public:
    void connect();
    void close();
    uint32_t login(const string& strName, const string& strPass);
	
    uint32_t sendMsg(uint32_t nToId,IM::BaseDefine::MsgType nType, const string& strMsg);
    	
	 
public:
    virtual void onError(uint32_t nSeqNo, uint32_t nCmd, const string& strMsg);
    virtual void onConnect();
    virtual void onClose();
    virtual void onLogin(uint32_t nSeqNo, uint32_t nResultCode, string& strMsg, IM::BaseDefine::UserInfo* pUser = NULL);   
    virtual void onSendMsg(uint32_t nSeqNo, uint32_t nSendId, uint32_t nRecvId, IM::BaseDefine::SessionType nType, uint32_t nMsgId);
    virtual void onRecvMsg(uint32_t nSeqNo, uint32_t nFromId, uint32_t nToId, uint32_t nMsgId, uint32_t nCreateTime, IM::BaseDefine::MsgType nMsgType, const string& strMsgData);

public:


private:
    string          m_strName;
    string          m_strPass;
    string          m_strLoginDomain;
    uint32_t        m_nLastGetUser;
    uint32_t        m_nLastGetSession;
    net_handle_t    m_nHandle;
    IM::BaseDefine::UserInfo    m_cSelfInfo;

public:
	
	ClientConn*  g_pConn;

	bool         g_bLogined;

	bool 		 g_configReturn;

};


#endif /*defined(__CLIENT_H__) */
