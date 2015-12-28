//
//  push_session_handler.cpp
//  my_push_server
//

#include "push_session_handler.h"
#include "push_define.h"
#include "session_manager.h"
#include "apns_msg.h"
#include "timer/Timer.hpp"
#include "IM.Server.pb.h"


void CPushSessionHandler::OnClose(uint32_t nsockid)
{
    PUSH_SERVER_WARN("push session closed, sockid: %u.", nsockid);
    m_Msg.Clear();
    CSessionManager::GetInstance()->RemovePushSessionBySockID(nsockid);
}


void CPushSessionHandler::OnException(uint32_t nsockid, int32_t nErrorCode)
{
    PUSH_SERVER_WARN("push session has exception, sockid: %u, error code: %u.", nsockid, nErrorCode);
    push_session_ptr pSession = CSessionManager::GetInstance()->GetPushSessionBySockID(nsockid);
    if (pSession)
    {
        pSession->Stop();
    }
}

void CPushSessionHandler::OnRecvData(const char *szBuf, int32_t nBufSize)
{
    m_Msg.Append(szBuf, nBufSize);
    while (m_Msg.CheckMsgAvailable())
    {
        uint32_t cmd_id = m_Msg.GetCommandID();
        switch (cmd_id) {
            case IM::BaseDefine::DFFX_CID_OTHER_HEARTBEAT:
                _HandleHeartBeat(m_Msg.Data(), m_Msg.GetPduLength());
                break;
            case IM::BaseDefine::DFFX_CID_OTHER_PUSH_TO_USER_REQ:
                _HandlePushMsg(m_Msg.Data(), m_Msg.GetPduLength());
                break;
            default:
                PUSH_SERVER_WARN("push session recv undefind msg, cmd id: %u.", cmd_id);
                push_session_ptr pSession = CSessionManager::GetInstance()->GetPushSessionBySockID(_GetSockID());
                if (pSession)
                {
                    pSession->Stop();
                }
                break;
        }
        m_Msg.Remove(m_Msg.GetPduLength());
    }
}

void CPushSessionHandler::_HandleHeartBeat(const char *szBuf, int32_t nBufSize)
{
    PUSH_SERVER_TRACE("HeartBeat");
    push_session_ptr pSession = CSessionManager::GetInstance()->GetPushSessionBySockID(_GetSockID());
    if (pSession == nullptr)
    {
        return;
    }
    pSession->SetHeartBeat(S_GetTickCount());
    pSession->SendMsg(szBuf, nBufSize);
}

void CPushSessionHandler::_HandlePushMsg(const char* szBuf, int32_t nBufSize)
{
    CPduMsg pdu_msg;
    if (!pdu_msg.ParseFromArray(szBuf, nBufSize))
    {
        PUSH_SERVER_ERROR("HandlePushMsg, msg parse failed.");
        return;
    }
    
    push_session_ptr pSession = CSessionManager::GetInstance()->GetPushSessionBySockID(_GetSockID());
    if (pSession == nullptr)
    {
        return;
    }
    pSession->SetHeartBeat(S_GetTickCount());
    IM::Server::IMPushToUserReq msg;
    if (!msg.ParseFromArray(pdu_msg.Body(), pdu_msg.GetBodyLength()))
    {
        PUSH_SERVER_ERROR("HandlePushMsg, msg parse failed.");
        pSession->Stop();
        return;
    }
    string strFlash = msg.flash();
    string strUserData = msg.push_data();
    apns_client_ptr pApnsClient = CSessionManager::GetInstance()->GetAPNSClient();	
    
    IM::Server::IMPushToUserRsp msg2;
    for (uint32_t i = 0; i < msg.user_token_list_size(); i++)
    {
        IM::BaseDefine::PushResult* push_result = msg2.add_push_result_list();
        IM::BaseDefine::UserTokenInfo user_token = msg.user_token_list(i);
        if (user_token.user_type() == IM::BaseDefine::CLIENT_TYPE_ANDROID) {
			
			//todo android push...
			m_NotificationID++;
			
			PUSH_SERVER_INFO("HandlePushMsg, token: %s, push count: %d, push_type:%d, notification id: %u.", 
				user_token.token().c_str(), user_token.push_count(),
                user_token.push_type(), m_NotificationID);

			Json::Value root;
			root["appkey"] = CPushApp::GetInstance()->m_strAppKey;
			root["timestamp"] = (int)time(NULL);
			root["type"] = "unicast";
			root["device_tokens"] = user_token.token().c_str();
			root["production_mode"] = "true";
			root["description"] = "";

			Json::Value payload;
			payload["display_type"]="notification";
			payload["extra"]=strUserData.c_str();
			
			Json::Value body;
			body["ticker"]="法仔";
			body["title"]="法仔";
			body["text"]=strFlash.c_str();

			body["play_vibrate"]="true";
			body["play_lights"]="true";
			body["play_sound"]="true";

			body["after_open"]="go_app";

			payload["body"] = body;

			root["payload"] = payload;

			string strPost = root.toStyledString();

			CHttpClient httpClient;
			string strUrl = CPushApp::GetInstance()->m_strAppSendUrl;
			
			string strResponse;
			string app_master_secret= CPushApp::GetInstance()->m_strAppMasterSecret;
			
			string strSign = _SignUmeng( "POST", strUrl, strPost, app_master_secret);

			strUrl += "?sign=" + strSign;

			int nTryCnt = 1;
		lResend:
			CURLcode res = httpClient.Post( strUrl, strPost, strResponse);

			PUSH_SERVER_INFO("HandlePushMsg, android push url=%s,post=%s,resp=%s,res=%d.",
				strUrl.c_str(), strPost.c_str(), strResponse.c_str(), res);

			if (CURLE_OK == res)
			{
            	push_result->set_result_code(0);
			}
			else if(CURLE_OPERATION_TIMEDOUT == res
				&& nTryCnt-- > 0)
			{
				goto lResend;
			}
			else
			{
				push_result->set_result_code(1);
                PUSH_SERVER_WARN("HandlePushMsg, android push failed.");
			}
			
            push_result->set_user_token(user_token.token());

			
			
        }
		else if (user_token.user_type() == IM::BaseDefine::CLIENT_TYPE_IOS) {
			
			//todo ios push...
			
			if (!pApnsClient)
				continue;
			
            m_NotificationID++;
        
            PUSH_SERVER_INFO("HandlePushMsg, token: %s, push count: %d, push_type:%d, notification id: %u.", user_token.token().c_str(), user_token.push_count(),
                              user_token.push_type(), m_NotificationID);
            CAPNSGateWayMsg msg;
            msg.SetAlterBody(strFlash);
            msg.SetCustomData(strUserData);
            msg.SetDeviceToken(user_token.token());
            time_t time_now = 0;
            time(&time_now);
            msg.SetExpirationDate(time_now + 3600);
            if (user_token.push_type() == PUSH_TYPE_NORMAL)
            {
                msg.SetSound(TRUE);
            }
            else
            {
                msg.SetSound(FALSE);
            }
            msg.SetBadge(user_token.push_count());
            msg.SetNotificationID(m_NotificationID);
            if (msg.SerializeToArray())
            {
                pApnsClient->SendPushMsgToGateway(msg.Data(), msg.GetDataLength());
                push_result->set_result_code(0);
            }
            else
            {
                push_result->set_result_code(1);
                PUSH_SERVER_WARN("HandlePushMsg, serialize CAPNSGateWayMsg failed.");
            }
            push_result->set_user_token(user_token.token());
			
        }
		else {
			
            push_result->set_user_token(user_token.token());
            push_result->set_result_code(1);
			
        }
		
       
    }
    
    CPduMsg pdu_msg2;
    pdu_msg2.SetServiceID(IM::BaseDefine::DFFX_SID_OTHER);
    pdu_msg2.SetCommandID(IM::BaseDefine::DFFX_CID_OTHER_PUSH_TO_USER_RSP);
    
    if (pdu_msg2.SerializeToArray(&msg2))
    {
        pSession->SendMsg(pdu_msg2.Data(), pdu_msg2.GetDataLength());
    }
    else
    {
        PUSH_SERVER_WARN("HandlePushMsg, serialize IMPushToUserRsp failed.");
    }
}

string CPushSessionHandler::_SignUmeng(const string& method,const string& url,const string& post_body,const string& app_master_secret)
{
	char szMd5[33] = {0};
	string strSign=method+url+post_body+app_master_secret;
	CMd5::MD5_Calculate( strSign.c_str(), strSign.length(), szMd5);
	return string(szMd5);
}

