/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：PushThread.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
================================================================*/
#include "PushThread.h"  
#include "Lock.h"


 CLock g_csLock;



CPushConfig::CPushConfig()
{
	StartThread();
}

CPushConfig::~CPushConfig()
{

}

/*
	获取配置表,该表每天更新一次
*/
void CPushConfig::OnThreadTick()
{	
	log("*************CPushConfig******************");
	
	CPushModel* pPush = CPushModel::getInstance();

	pPush->getPushConfig();

	sleep(24*60*60);
}




CPushThread::CPushThread()
{
	g_pClient = NULL;
	StartThread();
}

CPushThread::~CPushThread()
{

}

/*
	1.获取订单消息表
	2.分别获取每个订单消息对应的律师
	3.发送订单消息给对应律师
	4.将推送消息写入推送记录表
	5.更新订单消息表的推送间隔时间，最后推送时间，推送次数
*/
void CPushThread::OnThreadTick()
{	
	log("*************CPushThread******************");
	if(g_pClient == NULL || !g_pClient->g_bLogined || 
		!g_pClient->g_pConn)
	{
		sleep(3);
		return;
	}
		
	CPushModel* pPush = CPushModel::getInstance();

	list<OrderMsg > lsOrderMsg;

	lsOrderMsg.clear();

	pPush->getOrderMsgList(lsOrderMsg);
	if(lsOrderMsg.size() == 0)
	{
		sleep(5);
		return ;
	}
		
	//获取配置表里面当前区域可以发送的订单
	list<OrderMsg > lsTrueOrderMsg;
	list<OrderMsg >::iterator lsOrderIter;

	for( lsOrderIter = lsOrderMsg.begin();  lsOrderIter != lsOrderMsg.end(); lsOrderIter++)
	{
		string strMapKey;
		strMapKey  = int2string(lsOrderIter->msg_case);
		strMapKey +=  ",";
		strMapKey += int2string(lsOrderIter->msg_skillid);

		map<string, vector<int>>::iterator mIterMap;
		mIterMap = pPush->m_PushConfigMap.find(strMapKey);
		if(mIterMap == pPush->m_PushConfigMap.end())
		{
			continue;
		}
	
		vector<int> vMapValue(5);
		vMapValue = mIterMap->second;

		if(vMapValue[lsOrderIter->msg_sign] == CONFIGTRUE)
		{
			lsOrderIter->msg_isPushArea = vMapValue[0];
			lsOrderIter->msg_isPushCity = vMapValue[1];
			lsOrderIter->msg_isPushProvince = vMapValue[2];
			OrderMsg t_order = *lsOrderIter;
			lsTrueOrderMsg.push_back(t_order);
		}
	}

	
	//查找适合订单的律师，并发送订单
	list<OrderMsg>::iterator lsIter1 = lsTrueOrderMsg.begin();

	for(; lsIter1 != lsTrueOrderMsg.end(); lsIter1++)
	{		
		list<uint32_t > lsLawyerUid;
		pPush->getLawyerList(*lsIter1, lsLawyerUid);
		if(lsLawyerUid.size() == 0)
			continue;

		list<uint32_t >::iterator lsIter2 = lsLawyerUid.begin();

	
		for(; lsIter2 != lsLawyerUid.end(); lsIter2++)
		{
				CAutoLock cAutoLock(&g_csLock);
				g_pClient->sendMsg(*lsIter2, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_PUSH, lsIter1->msg_context);
			 	usleep(100);
		}


		//sql语句有长度限制，每次插入400条数据
		list<uint32_t>::iterator lsIter3 = lsLawyerUid.begin();
		list<uint32_t > lsTmpMsg ;
		for(; lsIter3 != lsLawyerUid.end(); lsIter3++)
		{
			lsTmpMsg.push_back(*lsIter3);

			if(lsTmpMsg.size() >= 400)
			{
				pPush->pushRecord(lsIter1->msg_orderid , lsTmpMsg);
				lsTmpMsg.clear();
				continue;
			}
		}
		if(lsIter3 == lsLawyerUid.end() && lsTmpMsg.size() != 0)
		{
			pPush->pushRecord(lsIter1->msg_orderid , lsTmpMsg);
			lsTmpMsg.clear();
		}

	}

	//更新订单表，如果订单不符合发送条件，则不更新订单时间及发送次数
	if(lsOrderMsg.size() != 0)
	{
		pPush->updateOrderPushtime(lsOrderMsg);
		pPush->updateOrderSendsign(lsOrderMsg);
		
	}
	if(lsTrueOrderMsg.size() != 0)
	{
		pPush->updateOrderUpdatetime(lsTrueOrderMsg);
	}	
	
	usleep(5000);
}

CNotifyClientGrabThread::CNotifyClientGrabThread()
{
	g_pClient = NULL;
	StartThread();
}

CNotifyClientGrabThread::~CNotifyClientGrabThread()
{

}

/*
	1.获取抢单律师
	2.将抢单律师通知给客户端
	3.更新抢单流水表
*/
void CNotifyClientGrabThread::OnThreadTick()
{	
	log("*************CNotifyClientGrabThread******************");
	if(g_pClient == NULL || !g_pClient->g_bLogined || 
		!g_pClient->g_pConn)
	{
		sleep(3);
		return;
	}

	CPushModel* pPush = CPushModel::getInstance();

	list<GrabRecord > lsGrabRecord;
	lsGrabRecord.clear();
	
	pPush->getGrabLawyer(lsGrabRecord);
	if(lsGrabRecord.size() == 0)
	{
		sleep(5);
		return ;
	}
	
	list<GrabRecord >::iterator lsIter;
	for(lsIter = lsGrabRecord.begin(); lsIter != lsGrabRecord.end(); lsIter++)
	{
		int    intResult;
		
		CAutoLock cAutoLock(&g_csLock);
		if(lsIter->grab_status == ENTERLISTPUSHING) //打官司 出勤需进入五人列表
		{
			intResult = IM::BaseDefine::LAWSUIT_ATTENDANCE_GRAB;
			
		}
		else if(lsIter->grab_status == DIRECTGRABPUSHING) //文书 咨询一人抢单则直接抢单成功
		{
			intResult = IM::BaseDefine::CONSULT_DOCUMENTS_GRAB;
		}
		
		string strMsg = "{\"grabStatus\":" + int2string(intResult) + ", \"grabLawyerInfo\":[" + lsIter->grab_context + "]}";
		g_pClient->sendMsg(lsIter->grab_useruid, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_GRAB, strMsg);
	
		usleep(100);
	}

	pPush->updateGrabLawyerToClient(lsGrabRecord);

	usleep(500000);
}



CNotifyLawyerGrabThread::CNotifyLawyerGrabThread()
{
	g_pClient = NULL;
	StartThread();
}

CNotifyLawyerGrabThread::~CNotifyLawyerGrabThread()
{

}

/*
	1.获取抢单结果
	2.将抢单结果通知律师端
	3.更新抢单流水表
*/
void CNotifyLawyerGrabThread::OnThreadTick()
{	
	log("*************CNotifyLawyerGrabThread******************");
	if(g_pClient == NULL || !g_pClient->g_bLogined || 
		!g_pClient->g_pConn)
	{
		sleep(3);
		return;
	}

	CPushModel* pPush = CPushModel::getInstance();

	list<GrabRecord > lsGrabRecord;
	lsGrabRecord.clear();
	
	pPush->getLawyerGrabResult(lsGrabRecord);
	if(lsGrabRecord.size() == 0)
	{
		sleep(5);
		return ;
	}


	list<GrabRecord >::iterator lsIter;
	for(lsIter = lsGrabRecord.begin(); lsIter != lsGrabRecord.end(); lsIter++)
	{
		string strResult = "";
		int    intResult;
		if(lsIter->grab_status == NOTIFYSUCCESSPUSHING)
		{
			intResult = IM::BaseDefine::GRAB_SUCCESS;		
			strResult = "抢单成功";	
		}
		else if(lsIter->grab_status == NOTIFYFAILPUSHING)
		{
			intResult = IM::BaseDefine::GRAB_FAID;
			strResult = "抢单失败";
		}
		
		string strMsg = "{\"orderSn\":\"" + lsIter->grab_ordersn + "\", \"orderId\":" + int2string(lsIter->grab_orderid) + ", \"orderUserId\":" + \
						int2string(lsIter->grab_useruid) + ", \"caseEtime\":\"" + lsIter->grab_orderendtime \
						+ "\", \"grabStatus\":" + int2string(intResult) + ", \"grabMessage\":\"" + strResult + "\", \"grabContext\":[" + lsIter->grab_context+ "]}";	

		CAutoLock cAutoLock(&g_csLock);
		g_pClient->sendMsg(lsIter->grab_lawyeruid, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_RESULT, strMsg);
		usleep(100);
		
	}

	
	pPush->updateGrabResultToLawyer(lsGrabRecord);

	usleep(500000);
}




CEntrushThread::CEntrushThread()
{
	g_pClient = NULL;
	StartThread();
}

CEntrushThread::~CEntrushThread()
{

}

/*
	1.获取委托，委托受理，委托取消订单
	2.发送给委托对象
	3.更新订单消息表
*/
void CEntrushThread::OnThreadTick()
{	
	log("*************CEntrushThread******************");
	if(g_pClient == NULL || !g_pClient->g_bLogined || 
		!g_pClient->g_pConn)
	{
		sleep(3);
		return;
	}

	CPushModel* pPush = CPushModel::getInstance();

	list<OrderEntrust > lsOrderEntrust;
	lsOrderEntrust.clear();
	
	pPush->handleEntrustOrder(lsOrderEntrust);
	if(lsOrderEntrust.size() == 0)
	{
		sleep(5);
		return ;
	}

	list<OrderEntrust >::iterator lsIter;
	for(lsIter = lsOrderEntrust.begin(); lsIter != lsOrderEntrust.end(); lsIter++)
	{
		string strResult = "";
		int    intResult;
		if(lsIter->msg_status == ENTRUSTORDER)
		{
			CAutoLock cAutoLock(&g_csLock);
			 //委托通知客户端
			g_pClient->sendMsg(lsIter->msg_lawyer, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_ENTRUST, lsIter->msg_context);
		}
		else if(lsIter->msg_status == ORDERCANCEL)
		{
			intResult = IM::BaseDefine::ENTRUST_CANCEL;
			
			strResult = "委托取消"; //通知律师端
			string strMsg = "{\"orderId\":" + int2string(lsIter->msg_orderid) + ", \"orderUserId\":" + \
							int2string(lsIter->msg_user) + ", \"grabStatus\":" + int2string(intResult) +  \
							", \"grabMessage\":\"" + strResult + "\", \"grabContext\":[" + lsIter->msg_context+ "]}";	

			CAutoLock cAutoLock(&g_csLock);
			g_pClient->sendMsg(lsIter->msg_lawyer, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_CANCEL, strMsg);
		}
		else if(lsIter->msg_status == ORDERACCEPT)
		{
			intResult = IM::BaseDefine::ENTRUST_ACCEPT;
			strResult = "委托受理"; //通知客户端
			string strMsg = "{\"orderId\":" + int2string(lsIter->msg_orderid) + ", \"orderLawyerId\":" + \
							int2string(lsIter->msg_lawyer) + ", \"grabStatus\":" + int2string(intResult) +  \
							", \"grabMessage\":\"" + strResult + "\", \"grabContext\":[" + lsIter->msg_context+ "]}";	

			CAutoLock cAutoLock(&g_csLock);
			g_pClient->sendMsg(lsIter->msg_user, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_ACCEPT	, strMsg);

		}
	
		usleep(100);
		
	}

	
	pPush->updateEntrustOrder(lsOrderEntrust);

	usleep(500000);
}


CTopUP_withDrawalThread::CTopUP_withDrawalThread()
{
	g_pClient = NULL;
	StartThread();
}

CTopUP_withDrawalThread::~CTopUP_withDrawalThread()
{

}

/*
	1.获取提现或者充值的消息id
	2.发送给提现或者充值用户
	3.更新充值或提现推送表(dffx_common_pushresult)
*/
void CTopUP_withDrawalThread::OnThreadTick()
{	
	log("*************TopUP_withDrawalThread******************");
	if(g_pClient == NULL || !g_pClient->g_bLogined || 
		!g_pClient->g_pConn)
	{
		sleep(3);
		return;
	}

	CPushModel* pPush = CPushModel::getInstance();

	list<TopUP_withDrawal > lsTopUP_withDrawal;
	lsTopUP_withDrawal.clear();
	
	pPush->getTopUP_withDrawal(lsTopUP_withDrawal);
	if(lsTopUP_withDrawal.size() == 0)
	{
		sleep(5);
		return ;
	}

	list<TopUP_withDrawal >::iterator lsIter;
	for(lsIter = lsTopUP_withDrawal.begin(); lsIter != lsTopUP_withDrawal.end(); lsIter++)
	{
		string strResult = "";
		int    intResult;

		if(lsIter->result_type == TOPUPTYPE) //充值 
		{
			if(lsIter->result_result == RESULTFAILED) //失败
			{
				intResult = IM::BaseDefine::TOPUP_FAILED;
				strResult += "充值失败";

			}
			else if(lsIter->result_result == RESULTSUCCESS) //成功
			{
				intResult = IM::BaseDefine::TOPUP_SUCCESS;
				strResult += "充值成功";
				
			}
		}
		else if(lsIter->result_type == WITHDRAWAL) //提现
		{
			if(lsIter->result_result == RESULTFAILED) //失败
			{
				intResult = IM::BaseDefine::WITHDRAWAL_FAILED;
				strResult += "提现失败";
	
			}
			else if(lsIter->result_result == RESULTSUCCESS) //成功
			{
				intResult = IM::BaseDefine::WITHDRAWAL_SUCCESS;
				strResult += "提现成功";
			}
		}

		string strMsg = "{\"UserId\":" + int2string(lsIter->result_userid) +  ", \"resultStatus\":" + int2string(intResult) +  \
						 ", \"linkId\":\"" + lsIter->result_linkid +  "\", \"resultMessage\":\"" + strResult + "\"}";
				
		CAutoLock cAutoLock(&g_csLock);
		g_pClient->sendMsg(lsIter->result_userid, IM::BaseDefine::MsgType::MSG_TYPE_TOPUP_WITHDRAWAL, strMsg);
		
		usleep(100);
	}

	
	pPush->updateTopUP_withDrawal(lsTopUP_withDrawal);

	usleep(500000);

}



CMyTimerThread::CMyTimerThread()
{
	m_last_update_tick = 0;
	StartThread();
}

CMyTimerThread::~CMyTimerThread()
{

}

//一天更新一次会员状态
#define  THREAD_TIMER_INTERVAL 3600*24*1000;

void CMyTimerThread::OnThreadTick()
{	
	log("*************CMyTimerThread******************");
	
	CPushModel* pPush = CPushModel::getInstance();
	
	uint64_t lCurTick = get_tick_count();
	if(lCurTick >= m_last_update_tick )
	{
		m_last_update_tick = lCurTick + THREAD_TIMER_INTERVAL;
		pPush->updateVipExp();
	}
	
	pPush->updateOrderExp();
	sleep(60);
}







