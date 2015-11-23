/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：UserModel.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月05日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __USERMODEL_H__
#define __USERMODEL_H__

#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "ImPduBase.h"
#include "public_define.h"
class CUserModel
{
public:
    static CUserModel* getInstance();
    ~CUserModel();
    void getChangedId(uint32_t& nLastTime, list<uint32_t>& lsIds, uint32_t user_id);
    void getUsers(list<uint32_t> lsIds, list<IM::BaseDefine::UserInfo>& lsUsers, uint32_t nUserId);
	void getFriendsList(uint32_t& nLastTime, uint32_t user_id, list<IM::BaseDefine::UserInfo> &lsUsers);
    bool getUser(uint32_t nUserId, DBUserInfo_t& cUser);
    bool updateUser(DBUserInfo_t& cUser);
    bool insertUser(DBUserInfo_t& cUser);
//    void getUserByNick(const list<string>& lsNicks, list<IM::BaseDefine::UserInfo>& lsUsers);
    void clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType);
	bool updateUserAvatar(uint32_t nUserId, string strAvatarUrl);

//operation for friend
	bool addFriend(IM::Buddy::IMAddFriendReq& addFriend);
	bool reverseAddFriend(IM::Buddy::IMReverseAddFriendReq& reverseAddFriend, IM::Buddy::IMFriendNotifyReq& msgNotify);
	bool delFriend(IM::Buddy::IMDelFriendReq& delFriend);
	bool chgFriendRemark(IM::Buddy::IMChgFriendRemarkReq& chgFriendRemark);
	bool friendNotifyResp(IM::Buddy::IMFriendNotifyRes& friendNotifyRes);

//operation for friend_group
	bool createFriendGroup(IM::Buddy::IMCreateFriendGroupReq& createGroup, uint32_t& group_id);
	bool delFriendGroup(IM::Buddy::IMDelFriendGroupReq& delGroup);
	bool moveFriendToGroup(IM::Buddy::IMMoveFriendToGroupReq& moveFriend);
	bool chgFriendGroupName(IM::Buddy::IMChgFriendGroupNameReq& chgGroupName);
	
//handle friend message notify
	bool addFiendInfo(uint32_t nUserId, list<IM::Buddy::IMCommonOperFrienInfo>& laddInfo, 
		list<IM::Buddy::IMCommonOperFrienInfo>& lRefuseInfo, list<IM::Buddy::IMCommonOperFrienInfo>& lAgreeInfo);
	bool findUserInfo(string find_string,  list<IM::BaseDefine::UserInfo>& lUser);

	//online offline
	bool notifyUserStatus(uint32_t nUserId, IM::BaseDefine::UserStatType userStatus);	
	bool getFriendsidReq(uint32_t nUserId, list<string>& lUser);

	//black
	bool isInBlackGroup(uint32_t nUserId, uint32_t nFriendId);

	bool msgServerRestart(uint32_t msg_server_id, uint32_t cur_time);
	bool updateUserStatus(uint32_t user_uid,uint32_t status,uint32_t msg_server_id,uint32_t client_type,uint32_t cur_time);

	bool orderstatusread(uint32_t user_id, uint32_t order_id, bool& is_null);
	
private:
    CUserModel();
private:
    static CUserModel* m_pInstance;
};

#endif /*defined(__USERMODEL_H__) */
