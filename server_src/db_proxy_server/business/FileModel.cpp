/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileModel.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
*
================================================================*/
#include "FileModel.h"
#include "../DBPool.h"

CFileModel* CFileModel::m_pInstance = NULL;

CFileModel::CFileModel()
{
    
}

CFileModel::~CFileModel()
{
    
}

CFileModel* CFileModel::getInstance()
{
    if (m_pInstance == NULL) {
        m_pInstance = new CFileModel();
    }
    return m_pInstance;
}

void CFileModel::getOfflineFile(uint32_t userId, list<IM::BaseDefine::OfflineFileInfo>& lsOffline)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_slave");
    if (pDBConn)
    {
    	string strExpiredTime = int2string(time(NULL)-7*24*3600);
        string strSql = "select * from IMTransmitFile where toId="+int2string(userId) + " and status=0 and created>="+strExpiredTime+" order by created limit 100";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if(pResultSet)
        {
            while (pResultSet->Next())
            {
                IM::BaseDefine::OfflineFileInfo offlineFile;
                offlineFile.set_from_user_id(pResultSet->GetInt("fromId"));
                offlineFile.set_task_id(pResultSet->GetString("taskId"));
                offlineFile.set_file_name(pResultSet->GetString("fileName"));
                offlineFile.set_file_size(pResultSet->GetInt("fileSize"));
				offlineFile.set_send_time(pResultSet->GetInt("created"));

				string strIpAddr = pResultSet->GetString("ipAddr");
				
				CStrExplode ip_list((char*)strIpAddr.c_str(), ';');			    
			    for (uint32_t i = 0; i < ip_list.GetItemCnt(); i++) {
					
			        CStrExplode ipAddr(ip_list.GetItem(i), ':');
					if(ipAddr.GetItemCnt()==2)
			        {
			        	IM::BaseDefine::IpAddr* pIp_addr=offlineFile.add_ip_addr_list();					
			        	pIp_addr->set_ip(ipAddr.GetItem(0));
			        	pIp_addr->set_port(string2int(ipAddr.GetItem(1)));
					}			    	
			    }
				
                lsOffline.push_back(offlineFile);
            }
            pResultSet->Clear();
        }
        else
        {
            log("no result for:%s", strSql.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_slave");
    }
}

void CFileModel::addOfflineFile(uint32_t fromId, uint32_t toId, string& taskId, string& fileName, uint32_t fileSize, string& strIpAddr)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        string strSql = "insert into IMTransmitFile (`fromId`,`toId`,`fileName`,`fileSize`,`taskId`,`status`,`created`,`updated`, `ipAddr`)"\
			" values(?,?,?,?,?,?,?,?,?)";
        
        // 必须在释放连接前delete CPrepareStatement对象，否则有可能多个线程操作mysql对象，会crash
        CPrepareStatement* pStmt = new CPrepareStatement();
        if (pStmt->Init(pDBConn->GetMysql(), strSql))
        {
            uint32_t status = 0;
            uint32_t nCreated = (uint32_t)time(NULL);
            
            uint32_t index = 0;
            pStmt->SetParam(index++, fromId);
            pStmt->SetParam(index++, toId);
            pStmt->SetParam(index++, fileName);
            pStmt->SetParam(index++, fileSize);
            pStmt->SetParam(index++, taskId);
            pStmt->SetParam(index++, status);
            pStmt->SetParam(index++, nCreated);
            pStmt->SetParam(index++, nCreated);
			pStmt->SetParam(index++, strIpAddr);
            
            bool bRet = pStmt->ExecuteUpdate();
            
            if (!bRet)
            {
                log("insert message failed: %s", strSql.c_str());
            }
        }
        delete pStmt;
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
}

void CFileModel::delOfflineFile(uint32_t fromId, uint32_t toId, string& taskId)
{
    CDBManager* pDBManager = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManager->GetDBConn("dffxIMDB_master");
    if (pDBConn)
    {
        string strSql = "update IMTransmitFile set status=1 where fromId=" + int2string(fromId) + " and toId="+int2string(toId) + " and taskId='" + taskId + "'";
        if(pDBConn->ExecuteUpdate(strSql.c_str()))
        {
            log("update offline file success.%d->%d:%s", fromId, toId, taskId.c_str());
        }
        else
        {
            log("update offline file failed.%d->%d:%s", fromId, toId, taskId.c_str());
        }
        pDBManager->RelDBConn(pDBConn);
    }
    else
    {
        log("no db connection for dffxIMDB_master");
    }
}