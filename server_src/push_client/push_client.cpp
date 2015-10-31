/*
 * push_client.cpp
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "ThreadPool.h"
#include "DBPool.h"
#include "CachePool.h"
#include "PushModel.h"
#include "ServInfo.h"
#include "EncDec.h"
#include "Client.h"
#include "PushThread.h"

#define CONFIGFILE_NAME "pushclient.conf"

int main(int argc, char* argv[])
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0))
	{
		printf("Client Version: PushClient/%s\n", VERSION);
		printf("Client Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	CacheManager* pCacheManager = CacheManager::getInstance(CONFIGFILE_NAME);
	if (!pCacheManager)
	{
		log("CacheManager init failed");
		return -1;
	}

	CDBManager* pDBManager = CDBManager::getInstance(CONFIGFILE_NAME);
	if (!pDBManager)
	{
		log("DBManager init failed");
		return -1;
	}

	// 主线程初始化单例，不然在工作线程可能会出现多次初始化
	if (!CPushModel::getInstance()) 
	{
		return -1;
	}
   

	CConfigFileReader config_file(CONFIGFILE_NAME);

	string strUrl = config_file.GetConfigName( "LoginServerUrl");
	string sName = config_file.GetConfigName( "UserName");
	string sPass = config_file.GetConfigName( "Password");
	char* push_thread = config_file.GetConfigName( "PushThread");
	char szMd5[33] = {0};
	CMd5::MD5_Calculate( sPass.c_str(), sPass.length(), szMd5);
	sPass = szMd5;
	
	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	CClient* pClient = new CClient(sName, sPass, strUrl);
	pClient->connect();
	if(pClient->g_configReturn == false)
	{
		log("Push_client connect failed, please check config or network!");
		return -1;
	}

	CPushConfig* pPushConfigThread;
	CPushThread* pPushThread;
	CNotifyClientGrabThread* pClientPushThread;
	CNotifyLawyerGrabThread* pLawyerPushThread ;
	CEntrushThread* pEntrushThread;
	CTopUP_withDrawalThread* pTopUP_withDrawalThread;
	CMyTimerThread* pMyTimerThread;

	CStrExplode push_thread_list(push_thread, ';');	
	for (uint32_t i = 0; i < push_thread_list.GetItemCnt(); i++)	
	{		
		if(strcmp(push_thread_list.GetItem(i), "CPushConfig") == 0)		
		{			
			//读取配置表线程			
			pPushConfigThread = new CPushConfig();	
			log("new pPushConfigThread");
		}		
		if(strcmp(push_thread_list.GetItem(i), "CPushThread") == 0)		
		{			
			//读取订单消息表并发送订单给律师端			
			pPushThread = new CPushThread();				
			pPushThread->setGPClient(*pClient);	
			log("new pPushThread");
		}		
		if(strcmp(push_thread_list.GetItem(i), "CNotifyClientGrabThread") == 0)		
		{			
			//将抢单消息通知客户端			
			pClientPushThread = new CNotifyClientGrabThread();	
			pClientPushThread->setGPClient(*pClient);	
			log("new pClientPushThread");
		}		
		if(strcmp(push_thread_list.GetItem(i), "CNotifyLawyerGrabThread") == 0)		
		{			
			//将抢单单结果通知给律师端			
			pLawyerPushThread = new CNotifyLawyerGrabThread();		
			pLawyerPushThread->setGPClient(*pClient);	
			log("new pLawyerPushThread");
		}		
		if(strcmp(push_thread_list.GetItem(i), "CEntrushThread") == 0)		
		{			
				//委托订单处理			
			pEntrushThread = new CEntrushThread();		
			pEntrushThread->setGPClient(*pClient);	
			log("new pEntrushThread");
		}	
		if(strcmp(push_thread_list.GetItem(i), "CTopUP_withDrawalThread") == 0)	
		{			
			//充值提现处理		
			pTopUP_withDrawalThread = new CTopUP_withDrawalThread();	
			pTopUP_withDrawalThread->setGPClient(*pClient);	
			log("new pTopUP_withDrawalThread");
		}		
		if(strcmp(push_thread_list.GetItem(i), "CMyTimerThread") == 0)	
		{			
			//清空过期vip用户及过期清单		
			pMyTimerThread = new CMyTimerThread();	
			log("new pMyTimerThread");
		}	
	}


	printf("now enter the event loop...\n");
    writePid();
	netlib_eventloop(10);

	if(pClient)
	{
		delete pClient;
		pClient = NULL;
	}

	for (uint32_t i = 0; i < push_thread_list.GetItemCnt(); i++)	
	{		
		if(strcmp(push_thread_list.GetItem(i), "CPushConfig") == 0)		
		{		
			if(pPushConfigThread)
			{
				delete pPushConfigThread;
				pPushConfigThread = NULL;
				log("free pPushConfigThread");
			}
		}		
		if(strcmp(push_thread_list.GetItem(i), "CPushThread") == 0)		
		{		
			if(pPushThread)
			{
				delete pPushThread;
				pPushThread = NULL;
				log("free pPushThread");
			}
		}		
		if(strcmp(push_thread_list.GetItem(i), "CNotifyClientGrabThread") == 0)		
		{				
			if(pClientPushThread)
			{
				delete pClientPushThread;
				pClientPushThread = NULL;
				log("free pClientPushThread");
			}
		}		
		if(strcmp(push_thread_list.GetItem(i), "CNotifyLawyerGrabThread") == 0)		
		{
			if(pLawyerPushThread)
			{
				delete pLawyerPushThread;
				pLawyerPushThread = NULL;
				log("free pLawyerPushThread");
			}
		}		
		if(strcmp(push_thread_list.GetItem(i), "CEntrushThread") == 0)		
		{				
			if(pEntrushThread)
			{
				delete pEntrushThread;
				pEntrushThread = NULL;
				log("free pEntrushThread");
			}
		}	
		if(strcmp(push_thread_list.GetItem(i), "CTopUP_withDrawalThread") == 0)	
		{			
			if(pTopUP_withDrawalThread)
			{
				delete pTopUP_withDrawalThread;
				pTopUP_withDrawalThread = NULL;
				log("free pTopUP_withDrawalThread");
			}
		}		
		if(strcmp(push_thread_list.GetItem(i), "CMyTimerThread") == 0)	
		{			
			if(pMyTimerThread)
			{
				delete pMyTimerThread;
				pMyTimerThread = NULL;
				log("free pMyTimerThread");
			}
		}	
	}

	return 0;
}


