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
	char szMd5[33] = {0};
	CMd5::MD5_Calculate( sPass.c_str(), sPass.length(), szMd5);
	sPass = szMd5;
	
	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	CClient* pClient = new CClient(sName, sPass, strUrl);
	pClient->connect();

	//读取配置表线程
	CPushConfig* pPushConfigThread = new CPushConfig();	
	
	//读取订单消息表并发送订单给律师端
	CPushThread* pPushThread = new CPushThread();	
	pPushThread->setGPClient(*pClient);

	//将抢单消息通知客户端
	CNotifyClientGrabThread* pClientPushThread = new CNotifyClientGrabThread();
	pClientPushThread->setGPClient(*pClient);

	//将抢单单结果通知给律师端
	CNotifyLawyerGrabThread* pLawyerPushThread = new CNotifyLawyerGrabThread();
	pLawyerPushThread->setGPClient(*pClient);

	//清空过期vip用户及过期清单
	CMyTimerThread* pMyTimerThread = new CMyTimerThread();

	
	printf("now enter the event loop...\n");
    writePid();
	netlib_eventloop(10);

	if(pClient)
	{
		delete pClient;
		pClient = NULL;
	}
	
	if(pPushThread)
	{
		delete pPushThread;
		pPushThread = NULL;
	}

	return 0;
}


