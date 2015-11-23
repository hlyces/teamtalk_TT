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
#include "playsound.h"
#include "Common.h"
#include "security/security.h"

extern string g_temp_path;

static ConnMap_t g_client_conn_map;

ClientConn::ClientConn(IPacketCallback* pCallback):
m_bOpen(false)
{
	m_pCallback = pCallback;
    m_pSeqAlloctor = CSeqAlloctor::getInstance();
}

ClientConn::~ClientConn()
{

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
    msg.set_client_type(IM::BaseDefine::CLIENT_TYPE_IOS);
    msg.set_client_version("100.0.0");
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_LOGIN);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_LOGIN_REQ_USERLOGIN);

	printf("login   %d\n",cPdu.GetCommandId());
	
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUser(uint32_t nUserId, uint32_t nTime)
{
    CImPdu cPdu;
    IM::Buddy::IMAllUserReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ALL_USER_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::getUserInfo(uint32_t nUserId, list<uint32_t>& lsUserId)
{
    CImPdu cPdu;
    IM::Buddy::IMUsersInfoReq msg;
    msg.set_user_id(nUserId);
    for (auto it=lsUserId.begin(); it!=lsUserId.end(); ++it) {
        msg.add_user_id_list(*it);
    }
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_USER_INFO_REQUEST);
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
	log("nfromId = %d ntoId = %d ", nFromId, nToId);
	
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

uint32_t ClientConn::addFriendReq(uint32_t nFromId,uint32_t nToId,uint32_t nGroupId,string strRemark)
{
	CImPdu cPdu;
	IM::Buddy::IMAddFriendReq msg;
	msg.set_from_user_id(nFromId);
	msg.set_to_user_id(nToId);
	msg.set_friend_groupid(nGroupId);
	msg.set_friend_remark(strRemark);

	string strExtra = "i am XXXXX";
	msg.set_extra_info(strExtra);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::delFriendReq(uint32_t nFromId,uint32_t nToId)
{
	CImPdu cPdu;
	IM::Buddy::IMDelFriendReq msg;
	msg.set_user_id(nFromId);
	msg.set_del_user_id(nToId);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIEND_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::chgFriendRemarkReq(uint32_t nFromId,uint32_t nToId,string strRemark)
{
	CImPdu cPdu;
	IM::Buddy::IMChgFriendRemarkReq msg;
	msg.set_user_id(nFromId);
	msg.set_friend_id(nToId);
	msg.set_friend_nick(strRemark);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDREMARK_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::getAddFriendReq(uint32_t nFromId)
{
	CImPdu cPdu;
	IM::Buddy::IMGetAddFriendReq msg;
	msg.set_user_id(nFromId);

	log("user_id = %d", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_GETADDFRIEND_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;
}

uint32_t ClientConn::findFriendReq(uint32_t nFromId, string find_string)
{
	CImPdu cPdu;
	IM::Buddy::IMFindUserInfoReq msg;
	msg.set_user_id(nFromId);
	msg.set_find_string(find_string);

	log("user_id = %d", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_FINDUSERINFO_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::createFriendGroup(uint32_t nFromId, string group_name)
{
	CImPdu cPdu;
	IM::Buddy::IMCreateFriendGroupReq msg;
	msg.set_user_id(nFromId);
	msg.set_group_name(group_name);

	log("user_id = %d ", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CREATEFRIENDGROUP_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;
}

uint32_t ClientConn::delFriendGroup(uint32_t nFromId, uint32_t group_id)
{
	CImPdu cPdu;
	IM::Buddy::IMDelFriendGroupReq msg;
	msg.set_user_id(nFromId);
	msg.set_group_id(group_id);

	log("user_id = %d ", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIENDGROUP_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::moveFriendToGroup(uint32_t nFromId, uint32_t  group_id, list<uint32_t > friend_id_list)
{
	CImPdu cPdu;
	IM::Buddy::IMMoveFriendToGroupReq msg;
	msg.set_user_id(nFromId);
	msg.set_group_id(group_id);
//	msg.set_friend_id_list(friend_id_list);

/*	for(list<uint32_t>::iterator it=friend_id_list.begin(); it!=friend_id_list.end(); ++it)
	{
		uint32_t* pUser = msg.add_friend_id_list();
		*pUser = *it;			
	}*/

	
	for(auto it=friend_id_list.begin(); it!=friend_id_list.end(); ++it)
	{
		msg.add_friend_id_list(*it);
			
	}
	


	log("user_id = %d ", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_MOVEFRIENDTOGROUP_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::chgFriendGroupName(uint32_t nFromId, uint32_t	group_id, string group_name)
{
	CImPdu cPdu;
	IM::Buddy::IMChgFriendGroupNameReq msg;
	msg.set_user_id(nFromId);
	msg.set_group_id(group_id);
	msg.set_group_name(group_name);

	log("user_id = %d ", nFromId);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDGROUPNAME_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::sendFileRequest(uint32_t from_user_id, uint32_t to_user_id, string file_name, uint32_t file_size)
{
	CImPdu cPdu;
	IM::File::IMFileReq msg;

	msg.set_from_user_id(from_user_id);
	msg.set_to_user_id(to_user_id);
	msg.set_file_name(file_name);
	msg.set_file_size(file_size);
	msg.set_trans_mode(IM::BaseDefine::FILE_TYPE_OFFLINE);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_REQUEST);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::getUnreadMsgCnt(uint32_t nUserId)
{
    CImPdu cPdu;
    IM::Message::IMUnreadMsgCntReq msg;
    msg.set_user_id(nUserId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_UNREAD_CNT_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getRecentSession(uint32_t nUserId, uint32_t nLastTime)
{
    CImPdu cPdu;
    IM::Buddy::IMRecentContactSessionReq msg;
    msg.set_user_id(nUserId);
    msg.set_latest_update_time(nLastTime);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}

uint32_t ClientConn::delRecentSession(uint32_t nUserId, uint32_t nSessionId, IM::BaseDefine::SessionType session_type)
{
	CImPdu cPdu;
    IM::Buddy::IMRemoveSessionReq msg;
    msg.set_user_id(nUserId);
    msg.set_session_type(session_type);
	msg.set_session_id(nSessionId);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_REMOVE_SESSION_REQ);
    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
    cPdu.SetSeqNum(nSeqNo);
    SendPdu(&cPdu);
    return nSeqNo;
}


uint32_t ClientConn::getMsgList(uint32_t nUserId, IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt)
{
    CImPdu cPdu;
    IM::Message::IMGetMsgListReq msg;
    msg.set_user_id(nUserId);
    msg.set_session_type(nType);
    msg.set_session_id(nPeerId);
    msg.set_msg_id_begin(nMsgId);
    msg.set_msg_cnt(nMsgCnt);
    cPdu.SetPBMsg(&msg);
    cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
    cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_LIST_REQUEST);
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
        case IM::BaseDefine::DFFX_CID_BUDDY_LIST_ALL_USER_RESPONSE:
            _HandleUser(pPdu);
        break;
        case IM::BaseDefine::DFFX_CID_BUDDY_LIST_USER_INFO_RESPONSE:
            _HandleUserInfo(pPdu);
        break;
        case IM::BaseDefine::DFFX_CID_MSG_DATA_ACK:
            _HandleSendMsg(pPdu);
        break;
        case IM::BaseDefine::DFFX_CID_MSG_UNREAD_CNT_RESPONSE:
            _HandleUnreadCnt(pPdu);
            break;
        case IM::BaseDefine::DFFX_CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE:
            _HandleRecentSession(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_REMOVE_SESSION_RES:
            _HandleDelRecentSession(pPdu);
            break;
        case IM::BaseDefine::DFFX_CID_MSG_LIST_RESPONSE:
            _HandleMsgList(pPdu);
            break;
        case IM::BaseDefine::DFFX_CID_MSG_DATA:
            _HandleMsgData(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_CREATEFRIENDGROUP_RES:
			printf("createGroup msg_type=%d\n", pPdu->GetCommandId());
            log("createGroup msg_type=%d\n", pPdu->GetCommandId());
			_HandleCreateFriendGroupRes(pPdu);
            break;	
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIENDGROUP_RES:
			printf("delFriendGroup msg_type=%d\n", pPdu->GetCommandId());
            log("delFriendGroup msg_type=%d\n", pPdu->GetCommandId());
			_HandleDelFriendGroupRes(pPdu);
            break;	
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_MOVEFRIENDTOGROUP_RES:
			printf("moveFriendToGroup msg_type=%d\n", pPdu->GetCommandId());
            log("moveFriendToGroup msg_type=%d\n", pPdu->GetCommandId());
			_HandleMoveFriendToGroupRes(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDGROUPNAME_RES:
			printf("chgFriendGroupName msg_type=%d\n", pPdu->GetCommandId());
            log("chgFriendGroupName msg_type=%d\n", pPdu->GetCommandId());
			_HandleChgFriendGroupNameRes(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_REQ:
			printf("addFriendReq msg_type=%d\n", pPdu->GetCommandId());
            log("addFriendReq msg_type=%d\n", pPdu->GetCommandId());
			_HandleAddFriendReq(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_RES:
			printf("addFriendRes msg_type=%d\n", pPdu->GetCommandId());
            log("addFriendRes msg_type=%d\n", pPdu->GetCommandId());
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_REVERSEADDFRIEND_RES:
			printf("reverseAddFriendRes msg_type=%d\n", pPdu->GetCommandId());
            log("reverseAddFriendRes msg_type=%d\n", pPdu->GetCommandId());
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIEND_RES:
			printf("delFriendRes msg_type=%d\n", pPdu->GetCommandId());
            log("delFriendRes msg_type=%d\n", pPdu->GetCommandId());
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDREMARK_RES:
			printf("chgFriendRemarkRes msg_type=%d\n", pPdu->GetCommandId());
            log("chgFriendRemarkRes msg_type=%d\n", pPdu->GetCommandId());
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_FRIENDNOTIFY_REQ:
			printf("FriendNotifyReq msg_type=%d\n", pPdu->GetCommandId());
            log("FriendNotifyReq msg_type=%d\n", pPdu->GetCommandId());
			_HandleFriendNotifyReq(pPdu);
            break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_GETADDFRIEND_REQ:
			printf("getAddFriendReq msg_type=%d\n", pPdu->GetCommandId());
            log("getAddFriendReq msg_type=%d\n", pPdu->GetCommandId());
			 _HandleGetAddFriendReq(pPdu);
			break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_GETADDFRIEND_RES:
			printf("getAddFriendRes msg_type=%d\n", pPdu->GetCommandId());
            log("getAddFriendRes msg_type=%d\n", pPdu->GetCommandId());
			_HandleGetAddFriendRes(pPdu);
			break;
		case IM::BaseDefine::DFFX_CID_BUDDY_LIST_FINDUSERINFO_RES:
			printf("findFriendRes msg_type=%d\n", pPdu->GetCommandId());
			log("findFriendRes msg_type=%d\n", pPdu->GetCommandId());
			_HandleFindFriendRes(pPdu);
			break;

		case IM::BaseDefine::DFFX_CID_FILE_RESPONSE:
			printf("DFFX_CID_FILE_RESPONSE msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_RESPONSE msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFileRsp(pPdu);
			break;
		case IM::BaseDefine::DFFX_CID_FILE_NOTIFY:
			printf("DFFX_CID_FILE_NOTIFY msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_NOTIFY msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFileNotify(pPdu);
			break;

		case IM::BaseDefine::DFFX_CID_FILE_HAS_OFFLINE_RES:
			printf("DFFX_CID_FILE_HAS_OFFLINE_RES msg_type=%d\n", pPdu->GetCommandId());
			log("DFFX_CID_FILE_HAS_OFFLINE_RES msg_type=%d\n", pPdu->GetCommandId());
			_HandleIMFileHasOfflineRsp(pPdu);
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

void ClientConn::_HandleUser(CImPdu* pPdu)
{
    IM::Buddy::IMAllUserRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_list_size();
        printf("get %d users\n", userCnt);
        list<IM::BaseDefine::UserInfo> lsUsers;
        for(uint32_t i=0; i<userCnt; ++i)
        {
            IM::BaseDefine::UserInfo cUserInfo = msgResp.user_list(i);
            lsUsers.push_back(cUserInfo);
        }
        m_pCallback->onGetChangedUser(nSeqNo, lsUsers);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleUserInfo(CImPdu* pPdu)
{
    IM::Buddy::IMUsersInfoRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t userCnt = msgResp.user_info_list_size();
        list<IM::BaseDefine::UserInfo> lsUser;
        for (uint32_t i=0; i<userCnt; ++i) {
            IM::BaseDefine::UserInfo userInfo = msgResp.user_info_list(i);
            lsUser.push_back(userInfo);
        }
        m_pCallback->onGetUserInfo(nSeqNo, lsUser);
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


void ClientConn::_HandleUnreadCnt(CImPdu* pPdu)
{
    IM::Message::IMUnreadMsgCntRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::UnreadInfo> lsUnreadInfo;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nTotalCnt = msgResp.total_cnt();
        uint32_t nCnt = msgResp.unreadinfo_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::UnreadInfo unreadInfo = msgResp.unreadinfo_list(i);
            lsUnreadInfo.push_back(unreadInfo);
        }
        m_pCallback->onGetUnreadMsgCnt(nSeqNo, nUserId, nTotalCnt, lsUnreadInfo);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb fail");
    }
}

void ClientConn::_HandleRecentSession(CImPdu *pPdu)
{
    IM::Buddy::IMRecentContactSessionRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        list<IM::BaseDefine::ContactSessionInfo> lsSession;
        uint32_t nUserId = msgResp.user_id();
        uint32_t nCnt = msgResp.contact_session_list_size();
        for (uint32_t i=0; i<nCnt; ++i) {
            IM::BaseDefine::ContactSessionInfo session = msgResp.contact_session_list(i);
            lsSession.push_back(session);
			printf("user_id = %d  session_id = %d\n", msgResp.user_id(), session.session_id());
			
        }
        m_pCallback->onGetRecentSession(nSeqNo, nUserId, lsSession);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleDelRecentSession(CImPdu* pPdu)
{
	IM::Buddy::IMRemoveSessionRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nUserId = msgResp.user_id();
        uint32_t result_code = msgResp.result_code();
		IM::BaseDefine::SessionType session_type = msgResp.session_type();
		uint32_t session_id = msgResp.session_id();
			
		printf("user_id = %d  session_id = %d session_type = %d\n", nUserId, session_id, session_type);
    }
    else
    {
       log("parse pb error");
    }
}


void ClientConn::_HandleMsgList(CImPdu *pPdu)
{
    IM::Message::IMGetMsgListRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        uint32_t nUserId= msgResp.user_id();
        IM::BaseDefine::SessionType nSessionType = msgResp.session_type();
        uint32_t nPeerId = msgResp.session_id();
        uint32_t nMsgId = msgResp.msg_id_begin();
        uint32_t nMsgCnt = msgResp.msg_list_size();
        list<IM::BaseDefine::MsgInfo> lsMsg;
        for(uint32_t i=0; i<nMsgCnt; ++i)
        {
            IM::BaseDefine::MsgInfo msgInfo = msgResp.msg_list(i);
            lsMsg.push_back(msgInfo);
        }
        m_pCallback->onGetMsgList(nSeqNo, nUserId, nPeerId, nSessionType, nMsgId, nMsgCnt, lsMsg);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb falied");
    }
}
void ClientConn::_HandleMsgData(CImPdu* pPdu)
{
    IM::Message::IMMsgData msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        play("message.wav");
        
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
			|| nMsgType == IM::BaseDefine::MSG_TYPE_FILE_TRANSFER
			|| nMsgType== IM::BaseDefine::MSG_TYPE_ORDER_ENTRUST
			|| nMsgType== IM::BaseDefine::MSG_TYPE_ORDER_ACCEPT
			|| nMsgType== IM::BaseDefine::MSG_TYPE_ORDER_CANCEL
			|| nMsgType == IM::BaseDefine::MSG_TYPE_USER_CHECK 
			|| nMsgType == IM::BaseDefine::MSG_TYPE_ORDER_WAITPAYMENT
			|| nMsgType == IM::BaseDefine::MSG_TYPE_ORDER_ALLCANCEL
			|| nMsgType== IM::BaseDefine::MSG_TYPE_TOPUP_WITHDRAWAL)
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

void ClientConn::_HandleAddFriendReq(CImPdu* pPdu)
{
    IM::Buddy::IMAddFriendReq msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {        
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_user_id();
		uint32_t nGroupId = 1;
		string friend_remark = "myFriendRemark";

		string extra_info = msg.extra_info();
		log("extra_info:%s", extra_info.c_str());

		IM::Buddy::IMCommonOperFriendRes msgRes;
		CImPdu pduRes;
		pduRes.SetPBMsg(&msgRes);
	    pduRes.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	    pduRes.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_RES);
	    uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	    pduRes.SetSeqNum(nSeqNo);
	    SendPdu(&pduRes);

		usleep(10*1000);
		IM::Buddy::IMReverseAddFriendReq msg2;
		CImPdu pdu2;
		msg2.set_from_user_id(nToId);
		msg2.set_to_user_id(nFromId);
		if(nToId==1)
		{//agree
			msg2.set_friend_groupid(nGroupId);
			msg2.set_friend_remark(friend_remark);
			msg2.set_friendres_status_type(IM::BaseDefine::FRIENDRES_STATUS_AGREE);
		}
		else if(nToId==2)
		{//ignore
			msg2.set_friendres_status_type(IM::BaseDefine::FRIENDRES_STATUS_IGNORE);			
		}
		else if(nToId==3)
		{
			return ;
		}
		else
		{//refuse
			msg2.set_friendres_status_type(IM::BaseDefine::FRIENDRES_STATUS_REFUSE);
			string strReason = "refuse-reason-XYN";
			msg2.set_refuse_reason(strReason);
		}
		
		pdu2.SetPBMsg(&msg2);
	    pdu2.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	    pdu2.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_REVERSEADDFRIEND_REQ);
	    nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	    pdu2.SetSeqNum(nSeqNo);
	    SendPdu(&pdu2);
    }
}

void ClientConn::_HandleFriendNotifyReq(CImPdu* pPdu)
{
    IM::Buddy::IMFriendNotifyReq msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {        
        uint32_t nFromId = msg.from_user_id();
        uint32_t nToId = msg.to_user_id();

		log("%d,%d,%d,%s",nFromId,nToId,(uint32_t)msg.friendres_status_type(),msg.refuse_reason().c_str());

		IM::Buddy::IMFriendNotifyRes msgRes;
		msgRes.set_from_user_id(nToId);
		msgRes.set_to_user_id(nFromId);
		CImPdu pduRes;
		pduRes.SetPBMsg(&msgRes);
	    pduRes.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	    pduRes.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_FRIENDNOTIFY_RES);
	    pduRes.SetSeqNum(nSeqNo);
		
		usleep(10*1000);
	    SendPdu(&pduRes);

    }
}

void ClientConn::_HandleGetAddFriendReq(CImPdu* pPdu)
{
	IM::Buddy::IMGetAddFriendReq msg;
    uint32_t nSeqNo = pPdu->GetSeqNum();
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {        
        uint32_t nFromId = msg.user_id();

		log("user_id:%d",nFromId);

		IM::Buddy::IMGetAddFriendRes msgRes;
		msgRes.set_user_id(nFromId);
	
		CImPdu pduRes;
		pduRes.SetPBMsg(&msgRes);
	    pduRes.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	    pduRes.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_GETADDFRIEND_REQ);
	    pduRes.SetSeqNum(nSeqNo);
		
		usleep(10*1000);
	    SendPdu(&pduRes);

	}
}

void ClientConn::_HandleGetAddFriendRes(CImPdu* pPdu)
{
	IM::Buddy::IMGetAddFriendRes msgResp;
	CHECK_PB_PARSE_MSG(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));


	uint32_t from_user_id = msgResp.user_id();
	//list<IM::Buddy::AddFrienInfo> addfriend_info_list = msgResp.add_addfriend_info_list();

	printf("_HandleGetAddFriendRes: user_id = %d  addfriend_info_list_size=%d\n", from_user_id,msgResp.addfriend_info_list_size());

	for(int i = 0; i < msgResp.addfriend_info_list_size(); i++)
	{
		IM::Buddy::IMCommonOperFrienInfo friendInfo = msgResp.addfriend_info_list(i);

		printf("AddFrienInfo: from_user_id=%d  extra_info=%s create_time=%d\n", friendInfo.oper_user_id(),\
			friendInfo.extra_info().c_str(), friendInfo.oper_time());
	}

	for(int i = 0; i < msgResp.agree_info_list_size(); i++)
	{
		IM::Buddy::IMCommonOperFrienInfo friendInfo = msgResp.agree_info_list(i);

		printf("AgreeAddFrienInfo: agree_user_id=%d resp_time=%d\n", friendInfo.oper_user_id(), friendInfo.oper_time());
	}

	for(int i = 0; i < msgResp.refuse_info_list_size(); i++)
	{
		IM::Buddy::IMCommonOperFrienInfo friendInfo = msgResp.refuse_info_list(i);

		printf("RefuseAddFrienInfo: refuse_user_id=%d  refuse_reason=%s resp_time=%d\n", friendInfo.oper_user_id(),\
			friendInfo.extra_info().c_str(), friendInfo.oper_time());
	}
	

	printf("over!\n");
}


void ClientConn::_HandleFindFriendRes(CImPdu* pPdu)
{
	IM::Buddy::IMUsersInfoRsp msgResp;
	CHECK_PB_PARSE_MSG(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t nSeqNo = pPdu->GetSeqNum();

	uint32_t user_id = msgResp.user_id();
	//IM::BaseDefine::UserInfo findUserInfo = msgResp.userInfo_list();
	printf("_HandleFindFriendRes: user_id = %d  userinfo_list_size=%d\n", user_id,msgResp.user_info_list_size());
	for(int i = 0; i < msgResp.user_info_list_size(); i++)
	{
		IM::BaseDefine::UserInfo friendInfo = msgResp.user_info_list(i);

		printf("_HandleFindFriendRes: from_user_id=%d  extra_info=%s \n", friendInfo.user_id(),\
			friendInfo.user_nickname().c_str());
	}
	printf("over!\n");
	
}

void ClientConn::_HandleCreateFriendGroupRes(CImPdu* pPdu)
{
	IM::Buddy::IMCommonOperFriendGroupRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.user_id();
        uint32_t nGroupId = msgResp.result_code();
       
       	printf("_HandleCreateFriendGroupRes:  user_id = %d   group_id = %d\n", nUserId, nGroupId);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}


void ClientConn::_HandleDelFriendGroupRes(CImPdu* pPdu)
{
	IM::Buddy::IMCommonOperFriendGroupRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.user_id();
        uint32_t nResult = msgResp.result_code();
       
       	printf("_HandleDelFriendGroupRes:  user_id = %d   nResult = %d\n", nUserId, nResult);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleMoveFriendToGroupRes(CImPdu* pPdu)
{
	IM::Buddy::IMCommonOperFriendGroupRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.user_id();
        uint32_t nResult = msgResp.result_code();
       
       	printf("_HandleMoveFriendToGroupRes:  user_id = %d   nResult = %d\n", nUserId, nResult);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }

}

void ClientConn::_HandleChgFriendGroupNameRes(CImPdu* pPdu)
{
	IM::Buddy::IMCommonOperFriendGroupRes msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.user_id();
        uint32_t nResult = msgResp.result_code();
       
       	printf("_HandleChgFriendGroupNameRes:  user_id = %d   nResult = %d\n", nUserId, nResult);
    }
    else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }

}

//////////////////////////////////////////////////////////////////////////////////////////
uint32_t ClientConn::fileReq(uint32_t from_user_id, uint32_t to_user_id, string file_name, uint32_t type)
{
	struct stat stat_buf;
	string sPath = g_temp_path+file_name;
	if(stat(sPath.c_str(), &stat_buf))
	{
		log("fileReq error!!!");
		return -1;
	}
	
	CImPdu cPdu;
	IM::File::IMFileReq msg;
	msg.set_from_user_id(from_user_id);
	msg.set_to_user_id(to_user_id);
	msg.set_file_name(file_name);
	msg.set_file_size(stat_buf.st_size);
	msg.set_trans_mode((IM::BaseDefine::FileType)type);

	log("user_id = %d ", from_user_id);

	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_REQUEST);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::addOffFile(uint32_t from_user_id, uint32_t to_user_id, string task_id, string file_name)
{
	struct stat stat_buf;
	string sPath = g_temp_path+file_name;
	if(stat(sPath.c_str(), &stat_buf))
	{
		log("fileReq error!!!");
		return -1;
	}
	
	CImPdu cPdu;
	IM::File::IMFileAddOfflineReq msg;
	msg.set_from_user_id(from_user_id);
	msg.set_to_user_id(to_user_id);
	msg.set_task_id(task_id);
	msg.set_file_name(file_name);
	msg.set_file_size(stat_buf.st_size);

	//while(file_size--)
	{
		IM::BaseDefine::IpAddr* pIpAddr = msg.add_ip_addr_list();
		pIpAddr->set_ip("192.168.1.234");
		pIpAddr->set_port(8600);
	}
	
	log("user_id = %d ", from_user_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_ADD_OFFLINE_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::delOffFile(uint32_t from_user_id, uint32_t to_user_id, string task_id)
{
	CImPdu cPdu;
	IM::File::IMFileDelOfflineReq msg;
	msg.set_from_user_id(from_user_id);
	msg.set_to_user_id(to_user_id);
	msg.set_task_id(task_id);
	
	log("user_id = %d ", from_user_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_DEL_OFFLINE_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::hasOffFile(uint32_t from_user_id)
{
	CImPdu cPdu;
	IM::File::IMFileHasOfflineReq msg;
	msg.set_user_id(from_user_id);
	
	log("user_id = %d ", from_user_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_FILE_HAS_OFFLINE_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::cleanMsg(uint32_t from_user_id, uint32_t session_id)
{
	CImPdu cPdu;
	IM::Message::IMCleanMsgListReq msg;
	msg.set_user_id(from_user_id);
	msg.set_session_id(session_id);
	msg.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);
	
	log("user_id = %d session_id=%d ", from_user_id, session_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_CLEAN_MSGLIST_REQ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}

uint32_t ClientConn::orderStatusRead(uint32_t from_user_id, uint32_t order_id)
{
	CImPdu cPdu;
	IM::Message::IMOrderStatusRead msg;
	msg.set_user_id(from_user_id);
	msg.set_order_id(order_id);
	msg.set_orderlist_is_null(2);
	log("user_id = %d order_id=%d ", from_user_id, order_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_MSG_ORDERSTATUS_READ);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;	
}

uint32_t ClientConn::getFriendGroup(uint32_t from_user_id)
{
	CImPdu cPdu;
	IM::Buddy::IMDepartmentReq msg;
	msg.set_user_id(from_user_id);
    msg.set_latest_update_time(0);
	
	log("user_id = %d ", from_user_id);
	
	cPdu.SetPBMsg(&msg);
	cPdu.SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
	cPdu.SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_DEPARTMENT_REQUEST);
	uint32_t nSeqNo = m_pSeqAlloctor->getSeq(ALLOCTOR_PACKET);
	cPdu.SetSeqNum(nSeqNo);
	SendPdu(&cPdu);
	return nSeqNo;

}


void ClientConn::_HandleIMFileRsp(CImPdu* pPdu)
{
	IM::File::IMFileRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.from_user_id();
        uint32_t nResult = msgResp.result_code();

	    IM::BaseDefine::IpAddr ipAddr = msgResp.ip_addr_list(0);
       	printf("_HandleIMFileRsp:  user_id = %d   nResult = %d taskid=%s ip=%s port=%d \n",
			nUserId, nResult, msgResp.task_id().c_str(), ipAddr.ip().c_str(), ipAddr.port());

		m_pFileCliConn = new FileCliConn();
		
		m_pFileCliConn->m_from_user_id = msgResp.from_user_id();
		m_pFileCliConn->m_to_user_id = msgResp.to_user_id();
		m_pFileCliConn->m_file_name = msgResp.file_name();
		m_pFileCliConn->m_task_id = msgResp.task_id();
		m_pFileCliConn->m_ip_addr_list = msgResp.ip_addr_list(0);
		m_pFileCliConn->m_trans_mode = msgResp.trans_mode();
		m_pFileCliConn->m_file_role = msgResp.trans_mode()==IM::BaseDefine::FILE_TYPE_ONLINE?IM::BaseDefine::CLIENT_REALTIME_SENDER:IM::BaseDefine::CLIENT_OFFLINE_UPLOAD;

		m_pFileCliConn->connect( ipAddr.ip(), ipAddr.port());
    }
	else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleIMFileNotify(CImPdu* pPdu)
{
	IM::File::IMFileNotify msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.from_user_id();
		printf("_HandleIMFileNotify:  user_id = %d   taskid=%s file_name=%s  file_size=%d\n",
				nUserId, msgResp.task_id().c_str(), msgResp.file_name().c_str(), msgResp.file_size());
		for(int j=0;j<msgResp.ip_addr_list_size();++j)
        {
        	IM::BaseDefine::IpAddr ipAddr = msgResp.ip_addr_list(j);
       		printf(" ip=%s port=%d \n", ipAddr.ip().c_str(), ipAddr.port());

			m_pFileCliConn = new FileCliConn();
		
			m_pFileCliConn->m_from_user_id = msgResp.from_user_id();
			m_pFileCliConn->m_to_user_id = msgResp.to_user_id();
			m_pFileCliConn->m_file_name = msgResp.file_name();
			m_pFileCliConn->m_file_size = msgResp.file_size();
			m_pFileCliConn->m_task_id = msgResp.task_id();
			m_pFileCliConn->m_ip_addr_list = msgResp.ip_addr_list(0);
			m_pFileCliConn->m_trans_mode = msgResp.trans_mode();
			m_pFileCliConn->m_file_role = msgResp.trans_mode()==IM::BaseDefine::FILE_TYPE_ONLINE?IM::BaseDefine::CLIENT_REALTIME_RECVER:IM::BaseDefine::CLIENT_OFFLINE_DOWNLOAD;

			m_pFileCliConn->connect( ipAddr.ip(), ipAddr.port());
		}
		
    }
	else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

void ClientConn::_HandleIMFileHasOfflineRsp(CImPdu* pPdu)
{
	IM::File::IMFileHasOfflineRsp msgResp;
    uint32_t nSeqNo = pPdu->GetSeqNum();
	
    if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
       
        uint32_t nUserId = msgResp.user_id();
        uint32_t nResult = msgResp.offline_file_list_size();

		for(int i=0;i<msgResp.offline_file_list_size();++i)
		{
			IM::BaseDefine::OfflineFileInfo offInfo = msgResp.offline_file_list(i);

			printf("_HandleIMFileRsp:  user_id = %d   nResult = %d taskid=%s \n",
				nUserId, nResult, offInfo.task_id().c_str());
			for(int j=0;j<offInfo.ip_addr_list_size();++j)
	    	{
	    		IM::BaseDefine::IpAddr ipAddr = offInfo.ip_addr_list(j);
				printf(" ip=%s port=%d \n", ipAddr.ip().c_str(), ipAddr.port());

				m_pFileCliConn = new FileCliConn();
		
				m_pFileCliConn->m_from_user_id = offInfo.from_user_id();
				m_pFileCliConn->m_to_user_id = nUserId;
				m_pFileCliConn->m_file_name = offInfo.file_name();
				m_pFileCliConn->m_file_size = offInfo.file_size();
				m_pFileCliConn->m_task_id = offInfo.task_id();
				m_pFileCliConn->m_ip_addr_list = ipAddr;
				m_pFileCliConn->m_trans_mode = IM::BaseDefine::FILE_TYPE_OFFLINE;
				m_pFileCliConn->m_file_role = IM::BaseDefine::CLIENT_OFFLINE_DOWNLOAD;

				m_pFileCliConn->connect( ipAddr.ip(), ipAddr.port());
			}
			
	     
		}
		
    }
	else
    {
        m_pCallback->onError(nSeqNo, pPdu->GetCommandId(), "parse pb error");
    }
}

