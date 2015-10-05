/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/
#include "ClientConn.h"
#include "Common.h"
#include "security/security.h"


static ConnMap_t g_client_conn_map;

ClientConn::ClientConn(IPacketCallback* pCallback):
m_bOpen(false)
{
	m_pCallback = pCallback;
    m_pSeqAlloctor = CSeqAlloctor::getInstance();
}

ClientConn::~ClientConn()
{
	if(m_pCallback)
    {
        m_pCallback->onClose();
    }
}

net_handle_t ClientConn::connect(const string& strIp, uint16_t nPort, const string& strName, const string& strPass)
{
	m_handle = netlib_connect(strIp.c_str(), nPort, imconn_callback, &g_client_conn_map);
	if(m_handle != NETLIB_INVALID_HANDLE)
		g_client_conn_map.insert(make_pair(m_handle, this));
    return  m_handle;
}



void ClientConn::OnConfirm()
{
    if(m_pCallback)
    {
        m_pCallback->onConnect();
    }
}

void ClientConn::OnClose()
{
	if(m_pCallback)
    {
        m_pCallback->onClose();
    }
    log("onclose from handle=%d\n", m_handle);
    Close();
}

void ClientConn::OnTimer(uint64_t curr_tick)
{
    if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
        CImPdu cPdu;
        IM::Other::IMHeartBeat msg;
        cPdu.SetPBMsg(&msg);
        cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
        cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_HEARTBEAT);
        uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
        cPdu.SetSeqNum(nSeqNo);
        SendPdu(&cPdu);
    }
    
    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
        log("conn to msg_server timeout\n");
        Close();
    }
}


uint32_t ClientConn::login(const string &strName, const string &strPass)
{
    CImPdu cPdu;
    IM::Login::IMLoginReq msg;
    msg.set_user_name(strName);
    msg.set_password(strPass);
    msg.set_online_status(IM::BaseDefine::USER_STATUS_ONLINE);
    msg.set_client_type(IM::BaseDefine::CLIENT_TYPE_WINDOWS);
    msg.set_client_version("1.0");
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_LOGIN);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_LOGIN_REQ_USERLOGIN);

	printf("login   %d\n",cPdu.GetCommandId());
	
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::sendMessage(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, const string& strMsgData)
{
    CImPdu cPdu;
    IM::Message::IMMsgData msg;
    msg.set_from_user_id(nFromId);
    msg.set_to_session_id(nToId);
    msg.set_msg_id(0);
    msg.set_create_time(time(NULL));
    msg.set_msg_type(nType);
	
	//msg.set_msg_data(strMsgData);
	//EncryptMsg
	char * pOutData=NULL;
	uint32_t nOutLen = 0;
	int retCode = EncryptMsg( strMsgData.c_str(), strMsgData.length(), &pOutData, nOutLen);
	if (retCode == 0 && nOutLen > 0 && pOutData != 0)
	{
		msg.set_msg_data( pOutData, nOutLen);
		delete pOutData;
		pOutData = NULL;
	}
	else 
	{
		log("EncryptMsg error:%s\n", strMsgData.c_str());
		msg.set_msg_data( strMsgData);
	}
	
    
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_DATA);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::sendMsgAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId)
{
    CImPdu cPdu;
    IM::Message::IMMsgDataAck msg;
    msg.set_user_id(nUserId);
    msg.set_session_id(nPeerId);
    msg.set_session_type(nType);
    msg.set_msg_id(nMsgId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_READ_ACK);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

void ClientConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_client_conn_map.erase(m_handle);
	}
	ReleaseRef();
}

void ClientConn::HandlePdu(CImPdu* pPdu)
{
    //printf("pdu type = %u\n", pPdu->GetPduType());
	//printf("msg_type=%d\n", pPdu->GetCommandId());
	switch (pPdu->GetCommandId()) {
        case IM::BaseDefine::DFFX_CID_OTHER_HEARTBEAT:
//		printf("Heartbeat\n");
		break;
        case IM::BaseDefine::DFFX_CID_LOGIN_RES_USERLOGIN:
            _HandleLoginResponse(pPdu);
		break;
		
        case IM::BaseDefine::DFFX_CID_MSG_DATA_ACK:
            _HandleSendMsg(pPdu);
        break;		
       
        case IM::BaseDefine::DFFX_CID_MSG_DATA:
            _HandleMsgData(pPdu);
            break;
				
		default:
			printf("wrong msg_type=%d\n", pPdu->GetCommandId());
			log("wrong msg_type=%d\n", pPdu->GetCommandId());
			break;
	}
}
void ClientConn::_HandleLoginResponse(CImPdu* pPdu)
{
    IM::Login::IMLoginRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nRet = msgResp.result_code();
        string strMsg = msgResp.result_string();
		log("msgResp.result_code() = %d", msgResp.result_code());
		
        if(nRet == 0)
        {
            m_bOpen = true;
            IM::BaseDefine::UserInfo cUser = msgResp.user_info();
            m_pCallback->onLogin(nSeqNo, nRet, strMsg, &cUser);
        }
        else
        {
            m_pCallback->onLogin(nSeqNo, nRet, strMsg);
        }
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}


void ClientConn::_HandleSendMsg(CImPdu* pPdu)
{
    IM::Message::IMMsgDataAck msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nSendId = msgResp.user_id();
        uint32_t nRecvId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id();
        IM::BaseDefine::SessionType nType = msgResp.session_type();
        m_pCallback->onSendMsg(nSeqNo, nSendId, nRecvId, nType, nMsgId);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_session_id();
        uint32_t nMsgId = msg.msg_id();
        IM::BaseDefine::MsgType nMsgType = msg.msg_type();
        uint32_t nCreateTime = msg.create_time();
		
        //string strMsg(msg.msg_data().c_str(), msg.msg_data().length());
        string strMsg = "";
		//DecryptMsg
		char * pOutData=NULL;
		uint32_t nOutLen = 0;
		int retCode = DecryptMsg( msg.msg_data().c_str(), msg.msg_data().length(), &pOutData, nOutLen);
		if (retCode == 0 && nOutLen > 0 && pOutData != 0)
		{
			strMsg = std::string(pOutData, nOutLen);
			delete pOutData;
			pOutData = NULL;			
		}
		else 
		{
			strMsg = std::string( msg.msg_data().c_str(), msg.msg_data().length());
			log("DecryptMsg error:%s\n", strMsg.c_str());			
		}
			
		//MSG_TYPE_ORDER_PUSH MSG_TYPE_ORDER_GRAB MSG_TYPE_ORDER_RESULT
        
        IM::BaseDefine::SessionType nSessionType;
        if(nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_TEXT 
			|| nMsgType == IM::BaseDefine::MSG_TYPE_SINGLE_AUDIO
			|| nMsgType == IM::BaseDefine::MSG_TYPE_ORDER_PUSH
			|| nMsgType == IM::BaseDefine::MSG_TYPE_ORDER_GRAB
			|| nMsgType == IM::BaseDefine::MSG_TYPE_ORDER_RESULT
			|| nMsgType == IM::BaseDefine::MSG_TYPE_LOCATION_SHARING
			|| nMsgType == IM::BaseDefine::MSG_TYPE_FILE_TRANSFER)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_SINGLE;
        }
        else if(nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_TEXT
			|| nMsgType == IM::BaseDefine::MSG_TYPE_GROUP_AUDIO)
        {
            nSessionType = IM::BaseDefine::SESSION_TYPE_GROUP;
        }
		
        sendMsgAck(nFromId, nToId, nSessionType, nMsgId);
        m_pCallback->onRecvMsg(nSeqNo, nFromId, nToId, nMsgId, nCreateTime, nMsgType, strMsg);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}

