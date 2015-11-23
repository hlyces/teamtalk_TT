/*================================================================
 *     Copyright (c) 2014年 lanhu. All rights reserved.
 *
 *   文件名称：HandlerMap.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月02日
 *   描    述：
 *
 ================================================================*/

#include "HandlerMap.h"

#include "business/Login.h"
#include "business/MessageContent.h"
#include "business/RecentSession.h"
#include "business/UserAction.h"
#include "business/MessageCounter.h"
#include "business/GroupAction.h"
#include "business/DepartAction.h"
#include "business/FileAction.h"
#include "IM.BaseDefine.pb.h"

using namespace IM::BaseDefine;


CHandlerMap* CHandlerMap::s_handler_instance = NULL;

/**
 *  构造函数
 */
CHandlerMap::CHandlerMap()
{

}

/**
 *  析构函数
 */
CHandlerMap::~CHandlerMap()
{

}

/**
 *  单例
 *
 *  @return 返回指向CHandlerMap的单例指针
 */
CHandlerMap* CHandlerMap::getInstance()
{
	if (!s_handler_instance) {
		s_handler_instance = new CHandlerMap();
		s_handler_instance->Init();
	}

	return s_handler_instance;
}

/**
 *  初始化函数,加载了各种commandId 对应的处理函数
 */
void CHandlerMap::Init()
{
	// Login validate
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_VALIDATE_REQ), DB_PROXY::doLogin));
    
    
    // recent session
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST), DB_PROXY::getRecentSession));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_REMOVE_SESSION_REQ), DB_PROXY::deleteRecentSession));
    
    // users
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_USER_INFO_REQUEST), DB_PROXY::getUserInfo));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_ALL_USER_REQUEST), DB_PROXY::getChangedUser));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_DEPARTMENT_REQUEST), DB_PROXY::getChgedDepart));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_CHANGE_AVATAR_REQUEST), DB_PROXY::changeAvatar));

	//friend
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_ADDFRIEND_REQ), DB_PROXY::addFriend));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_REVERSEADDFRIEND_REQ), DB_PROXY::reverseAddFriend));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_FRIENDNOTIFY_RES), DB_PROXY::friendNofityResp));	
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_DELFRIEND_REQ), DB_PROXY::delFriend));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_CHGFRIENDREMARK_REQ), DB_PROXY::chgFriendRemark));	
	//friend's group
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_CREATEFRIENDGROUP_REQ), DB_PROXY::createFriendGroup));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_DELFRIENDGROUP_REQ), DB_PROXY::delFriendGroup));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_MOVEFRIENDTOGROUP_REQ), DB_PROXY::moveFriendToGroup));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_CHGFRIENDGROUPNAME_REQ), DB_PROXY::chgFriendGroupName));	
	
    // message content
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_DATA), DB_PROXY::sendMessage));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_LIST_REQUEST), DB_PROXY::getMessage));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_UNREAD_CNT_REQUEST), DB_PROXY::getUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_READ_ACK), DB_PROXY::clearUnreadMsgCounter));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_GET_BY_MSG_ID_REQ), DB_PROXY::getMessageById));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_GET_LATEST_MSG_ID_REQ), DB_PROXY::getLatestMsgId));
	
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_CLEAN_MSGLIST_REQ), DB_PROXY::cleanMsgList));
    
    // device token
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_LOGIN_REQ_DEVICETOKEN), DB_PROXY::setDevicesToken));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_GET_DEVICE_TOKEN_REQ), DB_PROXY::getDevicesToken));
    
    //push 推送设置
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_GROUP_SHIELD_GROUP_REQUEST), DB_PROXY::setGroupPush));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_GET_SHIELD_REQ), DB_PROXY::getGroupPush));
    
    
    // group
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_GROUP_NORMAL_LIST_REQUEST), DB_PROXY::getNormalGroupList));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_GROUP_INFO_REQUEST), DB_PROXY::getGroupInfo));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_GROUP_CREATE_REQUEST), DB_PROXY::createGroup));
    m_handler_map.insert(make_pair(uint32_t(DFFX_CID_GROUP_CHANGE_MEMBER_REQUEST), DB_PROXY::modifyMember));

	//addfriend info
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_GETADDFRIEND_REQ), DB_PROXY::addGetFiendInfo));
	//finduserinfo
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_FINDUSERINFO_REQ), DB_PROXY::findUserInfo));
	//user statusnotify
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_BUDDY_LIST_STATUS_NOTIFY), DB_PROXY::notifyUserStatus));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_GETFRIENDSID_REQ), DB_PROXY::getFriendsidReq));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_USER_STATUS_UPDATE), DB_PROXY::userStatusUpdate));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_OTHER_MSGSRVRESTART_NOTIFY), DB_PROXY::msgServerRestart));
	

	//handle file
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_FILE_HAS_OFFLINE_REQ), DB_PROXY::hasOfflineFile));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_FILE_ADD_OFFLINE_REQ), DB_PROXY::addOfflineFile));
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_FILE_DEL_OFFLINE_REQ), DB_PROXY::delOfflineFile)); 

	//handle order status read
	m_handler_map.insert(make_pair(uint32_t(DFFX_CID_MSG_ORDERSTATUS_READ), DB_PROXY::orderstatusread));
}
   

/**
 *  通过commandId获取处理函数
 *
 *  @param pdu_type commandId
 *
 *  @return 处理函数的函数指针
 */
pdu_handler_t CHandlerMap::GetHandler(uint32_t pdu_type)
{
	HandlerMap_t::iterator it = m_handler_map.find(pdu_type);
	if (it != m_handler_map.end()) {
		return it->second;
	} else {
		return NULL;
	}
}


