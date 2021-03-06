/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：Login.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#include <list>
#include "../ProxyConn.h"
#include "../HttpClient.h"
#include "../SyncCenter.h"
#include "Login.h"
#include "TokenValidator.h"
#include "json/json.h"
#include "Common.h"
#include "IM.Server.pb.h"
#include "Base64.h"
#include "InterLogin.h"
#include "UserModel.h"


CInterLoginStrategy g_loginStrategy;

hash_map<string, list<uint32_t> > g_hmLimits;
CLock g_cLimitLock;
namespace DB_PROXY {
  
void doLogin(CImPdu* pPdu, uint32_t conn_uuid)
{
    
    CImPdu* pPduResp = new CImPdu;
    
    IM::Server::IMValidateReq msg;
    IM::Server::IMValidateRsp msgResp;
    
    if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
    {
        
        string strDomain = msg.user_name();
        string strPass = msg.password();
        
        msgResp.set_user_name(strDomain);
        msgResp.set_attach_data(msg.attach_data());
		
        log("%s request login.", strDomain.c_str());

		if( !g_loginStrategy.veryfyVersion( msg.client_type(), msg.client_version(), msgResp)) 
        {
        	msgResp.set_result_code(7);
            msgResp.set_result_string("客户端版本太老");
            pPduResp->SetPBMsg(&msgResp);
            pPduResp->SetSeqNum(pPdu->GetSeqNum());
            pPduResp->SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
            pPduResp->SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_VALIDATE_RSP);
            CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
            return ;
		}
		
        do
        {
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            uint32_t tmNow = time(NULL);
            
            //清理超过30分钟的错误时间点记录
            /*
             清理放在这里还是放在密码错误后添加的时候呢？
             放在这里，每次都要遍历，会有一点点性能的损失。
             放在后面，可能会造成30分钟之前有10次错的，但是本次是对的就没办法再访问了。
             */
            auto itTime=lsErrorTime.begin();
            for(; itTime!=lsErrorTime.end();++itTime)
            {
                if(tmNow - *itTime > 30*60)
                {
                    break;
                }
            }
            if(itTime != lsErrorTime.end())
            {
                lsErrorTime.erase(itTime, lsErrorTime.end());
            }
            
            // 判断30分钟内密码错误次数是否大于10
			if(lsErrorTime.size() > 10)
  		    //if(lsErrorTime.size() > 100)
            {
                itTime = lsErrorTime.begin();
                if(tmNow - *itTime <= 30*60)
                {
                    msgResp.set_result_code(6);
                    msgResp.set_result_string("用户名/密码错误次数太多");
                    pPduResp->SetPBMsg(&msgResp);
                    pPduResp->SetSeqNum(pPdu->GetSeqNum());
                    pPduResp->SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
                    pPduResp->SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_VALIDATE_RSP);
                    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
                    return ;
                }
            }
        } while(false);
        
        IM::BaseDefine::UserInfo cUser;

		_AcctInfo acctInfo;
        if(g_loginStrategy.doLogin(strDomain, strPass, cUser, acctInfo)) 
        {
            IM::BaseDefine::UserInfo* pUser = msgResp.mutable_user_info();
			*pUser = cUser;
			/*
            pUser->set_user_id(cUser.user_id());
            pUser->set_user_gender(cUser.user_gender());
            pUser->set_user_nickname(cUser.user_nickname());
            pUser->set_user_birthday(cUser.user_birthday());
            pUser->set_user_headlink(cUser.user_headlink());
            pUser->set_user_level(cUser.user_level());
            pUser->set_user_status(cUser.user_status());
            */
            msgResp.set_result_code(0);
            msgResp.set_result_string("成功");
            
            //如果登陆成功，则清除错误尝试限制
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.clear();	

			//token
			if(g_loginStrategy.setLoginToken( acctInfo, msg.client_type()))
			{
				msgResp.set_user_token( acctInfo.token);

                g_loginStrategy.insertLogLogin( acctInfo, msg.client_type(), msg.client_ip());

				g_loginStrategy.updateInviteRecord( acctInfo);

				// .. update
				//pc_status,mobile_status,pc_msgserver_id,mobile_msgserver_id,pc_logintime,mobile_logintime
				if( !CUserModel::getInstance()->updateUserStatus( pUser->user_uid(), (int)msg.online_status()
					, msg.msg_server_id(), (int)msg.client_type(), msg.cur_time()))
				{
					log("!!!error:user_uid=%d status=%d msg_server_id:%d cur_time:%d", pUser->user_uid()
						, (int)msg.online_status(), msg.msg_server_id(), msg.cur_time());			
				}
			}
			else
			{
				msgResp.set_result_code(6);
           		msgResp.set_result_string("setLoginToken error!");
			}
        }
        else
        {
            //密码错误，记录一次登陆失败
            uint32_t tmCurrent = time(NULL);
            CAutoLock cAutoLock(&g_cLimitLock);
            list<uint32_t>& lsErrorTime = g_hmLimits[strDomain];
            lsErrorTime.push_front(tmCurrent);
            
            log("get result false");
            msgResp.set_result_code(6);
            msgResp.set_result_string("用户名/密码错误");
        }
    }
    else
    {
        msgResp.set_result_code(6);
        msgResp.set_result_string("服务端内部错误");
    }
    
    
    pPduResp->SetPBMsg(&msgResp);
    pPduResp->SetSeqNum(pPdu->GetSeqNum());
    pPduResp->SetServiceId(IM::BaseDefine::DFFX_SID_OTHER);
    pPduResp->SetCommandId(IM::BaseDefine::DFFX_CID_OTHER_VALIDATE_RSP);
    CProxyConn::AddResponsePdu(conn_uuid, pPduResp);
}

};

