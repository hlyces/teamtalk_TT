/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：UserAction.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#include <list>
#include <map>

#include "../ProxyConn.h"
#include "../SyncCenter.h"
#include "public_define.h"
#include "UserModel.h"
#include "IM.Buddy.pb.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Message.pb.h"

#include "../AutoPool.h"
#include "IM.Server.pb.h"



namespace DB_PROXY
{

	void getUserInfo(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMUsersInfoReq msg;
		IM::Buddy::IMUsersInfoRsp msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.user_id();
			uint32_t userCount = msg.user_id_list_size();

			std::list<uint32_t> idList;
			for(uint32_t i = 0; i < userCount; ++i)
			{
				idList.push_back(msg.user_id_list(i));
			}
			std::list<IM::BaseDefine::UserInfo> lsUser;
			CUserModel::getInstance()->getUsers(idList, lsUser, from_user_id);
			msgResp.set_user_id(from_user_id);
			for(list<IM::BaseDefine::UserInfo>::iterator it=lsUser.begin();
			    it!=lsUser.end(); ++it)
			{
				IM::BaseDefine::UserInfo* pUser = msgResp.add_user_info_list();
				*pUser = *it;
				/*pUser->set_user_id(it->user_id());
				pUser->set_user_nickname(it->user_nickname());
				pUser->set_user_gender(it->user_gender());
				pUser->set_user_birthday(it->user_birthday());
				pUser->set_user_headlink(it->user_headlink());
				pUser->set_user_level(it->user_level());
				pUser->set_user_status(it->user_status());
				*/
			}
			log("userId=%u, userCnt=%u", from_user_id, userCount);

			msgResp.set_attach_data(msg.attach_data());
			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_USER_INFO_RESPONSE);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void getChangedUser(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMAllUserReq msg;
		IM::Buddy::IMAllUserRsp msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t nReqId = msg.user_id();
			uint32_t nLastTime = msg.latest_update_time();

			list<IM::BaseDefine::UserInfo> lsUsers;

			//list<uint32_t> lsIds;

			//CUserModel::getInstance()->getChangedId(nLastTime, lsIds, nReqId);
			//CUserModel::getInstance()->getUsers(lsIds, lsUsers);
			CUserModel::getInstance()->getFriendsList(nLastTime, nReqId, lsUsers);

			msgResp.set_user_id(nReqId);
			msgResp.set_latest_update_time(nLastTime);
			for (list<IM::BaseDefine::UserInfo>::iterator it=lsUsers.begin();
			     it!=lsUsers.end(); ++it)
			{
				IM::BaseDefine::UserInfo* pUser = msgResp.add_user_list();
				*pUser = *it;
				/*pUser->set_user_id(it->user_id());
				pUser->set_user_nickname(it->user_nickname());
				pUser->set_user_gender(it->user_gender());
				pUser->set_user_birthday(it->user_birthday());
				pUser->set_user_headlink(it->user_headlink());
				pUser->set_user_level(it->user_level());
				pUser->set_user_status(it->user_status());

				pUser->set_friend_groupid(it->friend_groupid());
				pUser->set_friend_remark(it->friend_remark());
				*/
			}
			log("userId=%u, last_time=%u, userCnt=%u", nReqId, nLastTime, msgResp.user_list_size());
			msgResp.set_attach_data(msg.attach_data());
			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ALL_USER_RESPONSE);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void changeAvatar(CImPdu* pPdu, uint32_t conn_uuid)
	{
/*		IM::Buddy::IMChangeAvatarReq msg;
		IM::Buddy::IMChangeAvatarRsp msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.user_id();
			string strAvatarUrl = msg.avatar_url();

			int result = 0;
			if( !CUserModel::getInstance()->updateUserAvatar(from_user_id, strAvatarUrl))
				result = 1;

			msgResp.set_user_id(from_user_id);
			msgResp.set_avatar_url(strAvatarUrl);
			msgResp.set_result_code(result);

			log("userId=%u, strAvatarUrl=%s", from_user_id, strAvatarUrl.c_str());

			msgResp.set_attach_data(msg.attach_data());
			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHANGE_AVATAR_RESPONSE);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}*/
	}

	void addFriend(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMAddFriendReq msg;
		IM::Buddy::IMCommonOperFriendRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.from_user_id();
			uint32_t to_user_id = msg.to_user_id();
			uint32_t friend_groupid = msg.friend_groupid();
			string   friend_remark = msg.friend_remark();

			int result = 0;
			if( !CUserModel::getInstance()->addFriend(msg))
				result = 1;

			log("addfriend :::::fromUserId=%u, toUserId=%d", from_user_id, to_user_id);

			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_friend_id(to_user_id);
			msgResp.set_attach_data(msg.attach_data());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);

			if(!result)
			{
				// 2.send req to myfriend
				msg.clear_friend_groupid();
				msg.clear_friend_remark();
				CImPdu* pPduReq = new CImPdu;
				pPduReq->SetPBMsg(&msg);
				pPduReq->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
				pPduReq->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_ADDFRIEND_REQ);
				CProxyConn::AddResponsePdu(conn_uuid, pPduReq);
			}

		}
		else
		{
			log("parse pb failed");
		}


	}

	void reverseAddFriend(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMReverseAddFriendReq msg;
		IM::Buddy::IMCommonOperFriendRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.from_user_id();
			uint32_t to_user_id = msg.to_user_id();
			IM::BaseDefine::FriendResStatusType friendres_status_type = msg.friendres_status_type();

			IM::Buddy::IMFriendNotifyReq msgNotify;

			int result = 0;
			if( !CUserModel::getInstance()->reverseAddFriend(msg, msgNotify))
				result = 1;

			log("reverseAddFriend :::::fromUserId=%u, toUserId=%d", from_user_id, to_user_id);

			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_friend_id(to_user_id);
			msgResp.set_attach_data(msg.attach_data());
			msgResp.set_friendres_status_type(msg.friendres_status_type());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_REVERSEADDFRIEND_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);

			if(!result && friendres_status_type!=IM::BaseDefine::FRIENDRES_STATUS_IGNORE)
			{
				// 2.send notify to myfriend
				CImPdu* pPduNotify = new CImPdu;

				msgNotify.set_from_user_id(msg.from_user_id());
				msgNotify.set_to_user_id(msg.to_user_id());
				msgNotify.set_friendres_status_type(msg.friendres_status_type());
				msgNotify.set_refuse_reason(msg.refuse_reason());

				pPduNotify->SetPBMsg(&msgNotify);
				pPduNotify->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
				pPduNotify->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_FRIENDNOTIFY_REQ);
				CProxyConn::AddResponsePdu(conn_uuid, pPduNotify);
			}
		}
		else
		{
			log("parse pb failed");
		}


	}

	void delFriend(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMDelFriendReq msg;
		IM::Buddy::IMCommonOperFriendRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.user_id();
			uint32_t to_user_id = msg.del_user_id();


			int result = 0;
			if(!CUserModel::getInstance()->delFriend(msg))
				result = 1;

			log("fromUserId=%u, toDelUserId=%d", from_user_id, to_user_id);

			// 1.send ack back
			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_friend_id(to_user_id);
			msgResp.set_attach_data(msg.attach_data());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIEND_RES);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
	}

	void chgFriendRemark(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMChgFriendRemarkReq msg;
		IM::Buddy::IMCommonOperFriendRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{

			uint32_t from_user_id = msg.user_id();
			uint32_t to_user_id = msg.friend_id();
			string   friend_nick = msg.friend_nick();

			int result = 0;
			if( !CUserModel::getInstance()->chgFriendRemark(msg))
				result = 1;
			log("from_user_id=%u, to_user_id=%d  friend_nick = %s", from_user_id, to_user_id, friend_nick.c_str());

			CImPdu* pPduRes = new CImPdu;
			// 1.send ack back
			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_friend_id(to_user_id);
			msgResp.set_attach_data(msg.attach_data());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDREMARK_RES);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
	}

	void friendNofityResp(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMFriendNotifyRes msgResp;

		if(msgResp.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{

			uint32_t from_user_id = msgResp.from_user_id();
			uint32_t to_user_id = msgResp.to_user_id();

			int result = 0;
			if( !CUserModel::getInstance()->friendNotifyResp(msgResp))
				result = 1;
			log("from_user_id=%u, to_user_id=%d  ", from_user_id, to_user_id);

		}
	}

	void createFriendGroup(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMCreateFriendGroupReq msg;
		IM::Buddy::IMCommonOperFriendGroupRes msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t from_user_id = msg.user_id();
			string group_name = msg.group_name();
			uint32_t group_id=-1;

			int result = 0;
			if( !CUserModel::getInstance()->createFriendGroup(msg, group_id))
				result = 1;

			msgResp.set_result_code(group_id);

			log("create_group :::userId=%u, group_id=%d", from_user_id, group_id);

			msgResp.set_user_id(from_user_id);
			msgResp.set_attach_data(msg.attach_data());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CREATEFRIENDGROUP_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void delFriendGroup(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMDelFriendGroupReq msg;
		IM::Buddy::IMCommonOperFriendGroupRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{

			uint32_t from_user_id = msg.user_id();
			uint32_t group_id = msg.group_id();

			int result = 0;
			if( !CUserModel::getInstance()->delFriendGroup(msg))
				result = 1;
			log("delete friend froup:::UserId=%u, groupId=%d ", from_user_id, group_id);

			// resp
			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_attach_data(msg.attach_data());

			CImPdu* pPduRes = new CImPdu;

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_DELFRIENDGROUP_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void  moveFriendToGroup(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMMoveFriendToGroupReq msg;
		IM::Buddy::IMCommonOperFriendGroupRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{

			uint32_t from_user_id = msg.user_id();
			uint32_t group_id = msg.group_id();

			int result = 0;
			if( !CUserModel::getInstance()->moveFriendToGroup(msg))
				result = 1;
			log("move friend froup:::UserId=%u, groupId=%d ", from_user_id, group_id);

			// resp
			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_attach_data(msg.attach_data());

			CImPdu* pPduRes = new CImPdu;

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_MOVEFRIENDTOGROUP_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
	}

	void chgFriendGroupName(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMChgFriendGroupNameReq msg;
		IM::Buddy::IMCommonOperFriendGroupRes msgResp;

		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{

			uint32_t from_user_id = msg.user_id();
			uint32_t group_id = msg.group_id();
			string group_name = msg.group_name();

			int result = 0;
			if( !CUserModel::getInstance()->chgFriendGroupName(msg))
				result = 1;
			log("chgFriendGroupName:::UserId=%u, groupId=%d ", from_user_id, group_id);

			// resp
			msgResp.set_user_id(from_user_id);
			msgResp.set_result_code(result);
			msgResp.set_attach_data(msg.attach_data());

			CImPdu* pPduRes = new CImPdu;

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_CHGFRIENDGROUPNAME_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
	}


	void addGetFiendInfo(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMGetAddFriendReq msg;
		IM::Buddy::IMGetAddFriendRes msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t user_id = msg.user_id();

			int result = 0;
			list<IM::Buddy::IMCommonOperFrienInfo> laddFriendInfo;
			list<IM::Buddy::IMCommonOperFrienInfo> lrefuseFriendInfo;
			list<IM::Buddy::IMCommonOperFrienInfo> lagreeFriendInfo;

			if( !CUserModel::getInstance()->addFiendInfo(user_id, laddFriendInfo, lrefuseFriendInfo, lagreeFriendInfo))
				result = 1;

			msgResp.set_user_id(user_id);
			//响应加我好友的id
			for(auto it=laddFriendInfo.begin(); it!=laddFriendInfo.end(); ++it)
			{
				IM::Buddy::IMCommonOperFrienInfo* pContact = msgResp.add_addfriend_info_list();
				*pContact = *it;
				/*pContact->set_from_user_id(it->from_user_id());
				pContact->set_extra_info(it->extra_info());*/
			}
			//响应拒绝我的好友申请的id
			for(auto it=lrefuseFriendInfo.begin(); it!=lrefuseFriendInfo.end(); ++it)
			{
				IM::Buddy::IMCommonOperFrienInfo* pContact = msgResp.add_refuse_info_list();
				*pContact = *it;
				/*
				pContact->set_refuse_user_id(it->refuse_user_id());
				pContact->set_refuse_reason(it->refuse_reason());*/
			}
			//响应同意我的好友申请的id
			for(auto it=lagreeFriendInfo.begin(); it!=lagreeFriendInfo.end(); ++it)
			{
				IM::Buddy::IMCommonOperFrienInfo* pContact = msgResp.add_agree_info_list();
				*pContact = *it;
			}

			log("userId=%d addCnt=%d refuseCnt=%d agreeCnt=%d", user_id, laddFriendInfo.size(), lrefuseFriendInfo.size(), lagreeFriendInfo.size());

			msgResp.set_attach_data(msg.attach_data());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_GETADDFRIEND_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void findUserInfo(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMFindUserInfoReq msg;
		IM::Buddy::IMUsersInfoRsp msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			CImPdu* pPduRes = new CImPdu;

			uint32_t user_id = msg.user_id();
			string find_string= msg.find_string();

			int result = 0;
			list<IM::BaseDefine::UserInfo> lUser;
			if( !CUserModel::getInstance()->findUserInfo(find_string, lUser))
				result = 1;

			msgResp.set_user_id(user_id);

			msgResp.set_attach_data(msg.attach_data());

			for(auto it=lUser.begin(); it!=lUser.end(); ++it)
			{
				IM::BaseDefine::UserInfo* pContact = msgResp.add_user_info_list();
				*pContact = *it;
				/*
				pContact->set_user_id(it->user_id());
				pContact->set_user_gender(it->user_gender());
				pContact->set_user_nickname(it->user_nickname());
				pContact->set_user_birthday(it->user_birthday());
				pContact->set_user_headlink(it->user_headlink());
				pContact->set_user_level(it->user_level());
				pContact->set_user_status(it->user_status());
				pContact->set_user_uid(it->user_uid());
				pContact->set_user_phone(it->user_phone());*/
			}

			log("userId=%d find_string=%s findCnt=%d", user_id, find_string.c_str(), lUser.size());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_BUDDY_LIST);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_BUDDY_LIST_FINDUSERINFO_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void notifyUserStatus(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Buddy::IMUserStatNotify msg;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			uint32_t user_id = msg.user_stat().user_id();
			IM::BaseDefine::UserStatType status = msg.user_stat().status();

			int result = 0;
			if( !CUserModel::getInstance()->notifyUserStatus(user_id, status))
				result = 1;

			log("userId=%d status=%d", user_id, (int)status);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void getFriendsidReq(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Server::IMGetFriendsidReq msg;
		IM::Server::IMGetFriendsidRes msgResp;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			IM::Buddy::IMUserStatNotify msgStat;
			if(msgStat.ParseFromArray((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length()))
			{
				uint32_t user_id = msgStat.user_stat().user_id();
				IM::BaseDefine::UserStatType status = msgStat.user_stat().status();

				int result = 0;
				if( !CUserModel::getInstance()->notifyUserStatus(user_id, status))
					result = 1;
			}
			else
			{
				log("parse pb failed");
				return ;
			}

			CImPdu* pPduRes = new CImPdu;

			uint32_t user_id = msg.user_id();

			int result = 0;
			list<string> lUser;
			if( !CUserModel::getInstance()->getFriendsidReq(user_id, lUser))
				result = 1;

			msgResp.set_attach_data(msg.attach_data());

			for(auto it=lUser.begin(); it!=lUser.end(); ++it)
			{
				msgResp.add_friend_id_list(string2int(*it));

			}

			log("userId=%d friendCnt=%d", user_id, lUser.size());

			pPduRes->SetPBMsg(&msgResp);
			pPduRes->SetSeqNum(pPdu->GetSeqNum());
			pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
			pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_GETFRIENDSID_RES);
			CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
		}
		else
		{
			log("parse pb failed");
		}
	}

	void userStatusUpdate(CImPdu* pPdu, uint32_t conn_uuid)
	{		
		IM::Server::IMUserStatusUpdate msg;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			uint32_t user_id = msg.user_id();
			uint32_t user_uid = msg.user_uid();
			IM::BaseDefine::UserStatType status = (IM::BaseDefine::UserStatType)msg.user_status();
			IM::BaseDefine::ClientType client_type = msg.client_type();

			log("userId=%d token staus:%d client:%d", user_id, (int)status, (int)client_type);

			if(status!=IM::BaseDefine::USER_STATUS_OFFLINE
			   || user_uid < 10000)
				return;
			
			// .. update
			//pc_status,mobile_status,pc_msgserver_id,mobile_msgserver_id,pc_logintime,mobile_logintime
			uint32_t msg_server_id = msg.msg_server_id();
			uint32_t cur_time = msg.cur_time();
			if( !CUserModel::getInstance()->updateUserStatus( user_uid, (int)status, msg_server_id, (int)client_type, cur_time))
			{
				log("!!!error:status=%d msg_server_id:%d cur_time:%d", (int)status, msg_server_id, cur_time);			
			}

			//
			CacheConn* pCacheConn = NULL;
			CAutoCache autoCache( "login_token", &pCacheConn);

			if (pCacheConn)
			{
				string account_uid_Key = "account_uid_"+int2string(user_uid);

				std::map<std::string, std::string> map_key_value;
				string strToken_key = "token";
				if(CHECK_CLIENT_TYPE_PC(client_type))
					strToken_key += "_pc";
				else
					strToken_key += "_mobile";
				map_key_value[strToken_key] = "";

				pCacheConn->hmset( account_uid_Key, map_key_value);
			}
			else
			{
				log("userStatusUpdate token error:%d", user_id);
			}
		}
		else
		{
			log("parse pb failed");
		}
	}

	void msgServerRestart(CImPdu* pPdu, uint32_t conn_uuid)
	{		
		IM::Server::IMMsgServerRestartNotify msg;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			uint32_t oprt_status = msg.oprt_status();
			uint32_t msg_server_id = msg.msg_server_id();
			uint32_t cur_time = msg.cur_time();			

			log("oprt_status=%d msg_server_id:%d cur_time:%d", oprt_status, msg_server_id, cur_time);
			
			if( !CUserModel::getInstance()->msgServerRestart(msg_server_id, cur_time))
			{
				log("!!!error:oprt_status=%d msg_server_id:%d cur_time:%d", oprt_status, msg_server_id, cur_time);			
			}
		}
		else
		{
			log("parse pb failed");
		}
	}

	void orderstatusread(CImPdu* pPdu, uint32_t conn_uuid)
	{
		IM::Message::IMOrderStatusRead msg;
		if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
		{
			uint32_t user_id = msg.user_id();
			uint32_t order_id = msg.order_id();		
			bool	 is_null;
		//	log("user_id=%d order_id:%d ", user_id, order_id);
			
			if( !CUserModel::getInstance()->orderstatusread(user_id, order_id, is_null))
			{
				log("!!!error:user_id=%d order_id:%d ", user_id, order_id, is_null);	
				return ;
			}

			msg.set_orderlist_is_null(is_null);
		}
		else
		{
			log("parse pb failed");
		}
		 log("   user_id = %d order_id = %d orderlist_is_null = %d", msg.user_id(), msg.order_id(), msg.orderlist_is_null());
		
		CImPdu* pPduRes = new CImPdu;
		pPduRes->SetPBMsg(&msg);
		pPduRes->SetSeqNum(pPdu->GetSeqNum());
		pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_MSG);
		pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_MSG_ORDERSTATUS_READ_BROADCAST);
		CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
	}
};



