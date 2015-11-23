/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：UserAction.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef __USER_ACTION_H__
#define __USER_ACTION_H__

#include "ImPduBase.h"

namespace DB_PROXY {
	//operation for user
    void getUserInfo(CImPdu* pPdu, uint32_t conn_uuid);
    void getChangedUser(CImPdu* pPdu, uint32_t conn_uuid);
	void changeAvatar(CImPdu* pPdu, uint32_t conn_uuid);

	//operation for friend
	void addFriend(CImPdu* pPdu, uint32_t conn_uuid);
	void reverseAddFriend(CImPdu* pPdu, uint32_t conn_uuid);
	void delFriend(CImPdu* pPdu, uint32_t conn_uuid);
	void chgFriendRemark(CImPdu* pPdu, uint32_t conn_uuid);
	void friendNofityResp(CImPdu* pPdu, uint32_t conn_uuid);

	//operation for friend_group
	void createFriendGroup(CImPdu* pPdu, uint32_t conn_uuid);
	void delFriendGroup(CImPdu* pPdu, uint32_t conn_uuid);
	void moveFriendToGroup(CImPdu* pPdu, uint32_t conn_uuid);
	void chgFriendGroupName(CImPdu* pPdu, uint32_t conn_uuid);

	
	//handle friend message notify
	void addGetFiendInfo(CImPdu* pPdu, uint32_t conn_uuid);
	void findUserInfo(CImPdu* pPdu, uint32_t conn_uuid);

	//online offline
	void notifyUserStatus(CImPdu* pPdu, uint32_t conn_uuid);
	void getFriendsidReq(CImPdu* pPdu, uint32_t conn_uuid);
	void userStatusUpdate(CImPdu* pPdu, uint32_t conn_uuid);

	void msgServerRestart(CImPdu* pPdu, uint32_t conn_uuid);

	void orderstatusread(CImPdu* pPdu, uint32_t conn_uuid);

};


#endif /* __USER_ACTION_H__ */
