/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileAction.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
*
================================================================*/
#include "FileAction.h"
#include "FileModel.h"
#include "IM.File.pb.h"
#include "../ProxyConn.h"


namespace DB_PROXY {

    
    void hasOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::File::IMFileHasOfflineReq msg;
        IM::File::IMFileHasOfflineRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t nUserId = msg.user_id();
            CFileModel* pModel = CFileModel::getInstance();
            list<IM::BaseDefine::OfflineFileInfo> lsOffline;
            pModel->getOfflineFile(nUserId, lsOffline);
            msgResp.set_user_id(nUserId);
            for (list<IM::BaseDefine::OfflineFileInfo>::iterator it=lsOffline.begin();
                 it != lsOffline.end(); ++it) {
                IM::BaseDefine::OfflineFileInfo* pInfo = msgResp.add_offline_file_list();
                *pInfo = *it;
                /*
                pInfo->set_from_user_id(it->from_user_id());
                pInfo->set_task_id(it->task_id());
                pInfo->set_file_name(it->file_name());
                pInfo->set_file_size(it->file_size());
                */
            }
            
            log("userId=%u, count=%u", nUserId, msgResp.offline_file_list_size());
            
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::DFFX_SID_FILE);
            pPduRes->SetCommandId(IM::BaseDefine::DFFX_CID_FILE_HAS_OFFLINE_RES);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
        }
        else
        {
            log("parse pb failed");
        }
    }
    
    void addOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::File::IMFileAddOfflineReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.from_user_id();
            uint32_t nToId = msg.to_user_id();
            string strTaskId = msg.task_id();
            string strFileName = msg.file_name();
            uint32_t nFileSize = msg.file_size();
			string strIpAddr = "";
			for(int i=0;i<msg.ip_addr_list_size();++i)
			{
				IM::BaseDefine::IpAddr ipAddr = msg.ip_addr_list(i);
				if(i>0)
					strIpAddr += ";";
				strIpAddr += ipAddr.ip()+":"+int2string(ipAddr.port());				
			}
            CFileModel* pModel = CFileModel::getInstance();
            pModel->addOfflineFile(nUserId, nToId, strTaskId, strFileName, nFileSize, strIpAddr);
            log("fromId=%u, toId=%u, taskId=%s, fileName=%s, fileSize=%u, strIpAddr=%s", 
				nUserId, nToId, strTaskId.c_str(), strFileName.c_str(), nFileSize, strIpAddr.c_str());
        }
		else
        {
            log("parse pb failed");
        }
    }
    
    void delOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::File::IMFileDelOfflineReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.from_user_id();
            uint32_t nToId = msg.to_user_id();
            string strTaskId = msg.task_id();
            CFileModel* pModel = CFileModel::getInstance();
            pModel->delOfflineFile(nUserId, nToId, strTaskId);
            log("fromId=%u, toId=%u, taskId=%s", nUserId, nToId, strTaskId.c_str());
        }
		else
        {
            log("parse pb failed");
        }
    }
};