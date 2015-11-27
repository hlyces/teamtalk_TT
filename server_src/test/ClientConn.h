/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "imconn.h"
#include "IM.BaseDefine.pb.h"
#include "IM.File.pb.h"
#include "IM.Login.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IM.Buddy.pb.h"
#include "IM.Message.pb.h"
#include "IM.Group.pb.h"
#include "IPacketCallback.h"
#include "SeqAlloctor.h"
#include "FileCliConn.h"

class ClientConn : public CImConn
{
public:
	ClientConn(IPacketCallback* pCallback);
	virtual ~ClientConn();

	bool IsOpen() { return m_bOpen; }

    net_handle_t connect(const string& strIp, uint16_t nPort, const string& strName, const string& strPass);
    
    virtual void Close();
public:
    uint32_t login(const string& strName, const string& strPass);
    uint32_t getUser(uint32_t nUserId, uint32_t nTime =0);
    uint32_t getUserInfo(uint32_t nUserId, list<uint32_t>& lsUserId);
    uint32_t sendMessage(uint32_t nFromId, uint32_t nToId, IM::BaseDefine::MsgType nType, const string& strMsgData);
    uint32_t getUnreadMsgCnt(uint32_t nUserId);
    uint32_t getRecentSession(uint32_t nUserId, uint32_t nLastTime);
	uint32_t delRecentSession(uint32_t nUserId, uint32_t nSessionId, IM::BaseDefine::SessionType session_type);
    uint32_t getMsgList(uint32_t nUserId, IM::BaseDefine::SessionType nType, uint32_t nPeerId, uint32_t nMsgId, uint32_t nMsgCnt);
    uint32_t sendMsgAck(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nType, uint32_t nMsgId);

	
	uint32_t addFriendReq(uint32_t nFromId, uint32_t nToId, uint32_t nGroupId, string strRemark);
    uint32_t delFriendReq(uint32_t nFromId, uint32_t nToId);
	uint32_t chgFriendRemarkReq(uint32_t nFromId, uint32_t nToId, string strRemark);
	uint32_t getAddFriendReq(uint32_t nFromId);
	uint32_t findFriendReq(uint32_t nFromId, string find_string);

	uint32_t createFriendGroup(uint32_t nFromId, string group_name);
	uint32_t delFriendGroup(uint32_t nFromId, uint32_t group_id);
	uint32_t moveFriendToGroup(uint32_t nFromId, uint32_t  group_id, list<uint32_t > friend_id_list);
	uint32_t chgFriendGroupName(uint32_t nFromId, uint32_t  group_id, string group_name);

	uint32_t sendFileRequest(uint32_t from_user_id, uint32_t to_user_id, string file_name, uint32_t file_size);

	uint32_t fileReq(uint32_t from_user_id, uint32_t to_user_id, string file_name, uint32_t type);
    uint32_t addOffFile(uint32_t from_user_id, uint32_t to_user_id, string task_id, string file_name);
	uint32_t delOffFile(uint32_t from_user_id, uint32_t to_user_id, string task_id);
	uint32_t hasOffFile(uint32_t from_user_id);

	uint32_t cleanMsg(uint32_t from_user_id, uint32_t session_id);

	
	uint32_t orderStatusRead(uint32_t from_user_id, uint32_t order_id);

	uint32_t getFriendGroup(uint32_t from_user_id);
    
public:
	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	void _HandleLoginResponse(CImPdu* pPdu);
    void _HandleUser(CImPdu* pPdu);
    void _HandleUserInfo(CImPdu* pPdu);
    void _HandleSendMsg(CImPdu* pPdu);
    void _HandleUnreadCnt(CImPdu* pPdu);
    void _HandleRecentSession(CImPdu* pPdu);
	void _HandleDelRecentSession(CImPdu* pPdu);
    void _HandleMsgList(CImPdu* pPdu);
    void _HandleMsgData(CImPdu* pPdu);
	
	void _HandleAddFriendReq(CImPdu* pPdu);
	void _HandleFriendNotifyReq(CImPdu* pPdu);
	void _HandleGetAddFriendReq(CImPdu* pPdu);
	void _HandleGetAddFriendRes(CImPdu* pPdu);
	void _HandleFindFriendRes(CImPdu* pPdu);

	void _HandleCreateFriendGroupRes(CImPdu* pPdu);
	void _HandleDelFriendGroupRes(CImPdu* pPdu);
	void _HandleMoveFriendToGroupRes(CImPdu* pPdu);
	void _HandleChgFriendGroupNameRes(CImPdu* pPdu);
	
	void _HandleIMFileRsp(CImPdu* pPdu);
	void _HandleIMFileNotify(CImPdu* pPdu);
	void _HandleIMFileHasOfflineRsp(CImPdu* pPdu);

	void _HandleOrderStatusReadBroadcastRsp(CImPdu* pPdu);
	
private:
	bool 		m_bOpen;
    IPacketCallback* m_pCallback;
    CSeqAlloctor*   m_pSeqAlloctor;
	    
	FileCliConn *m_pFileCliConn;
};


#endif /* CLIENTCONN_H_ */
