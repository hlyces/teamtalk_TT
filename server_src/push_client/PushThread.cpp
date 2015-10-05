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

	list<OrderMsg> m_OrderMsgList;

	m_OrderMsgList.clear();

	pPush->getOrderMsgList(m_OrderMsgList);
	if(m_OrderMsgList.size() == 0)
	{
		sleep(5);
		return ;
	}
		
	//获取配置表里面当前区域可以发送的订单
	list<OrderMsg > t_trueOrderMsgList;
	list<OrderMsg >::iterator t_orderIter;

	for( t_orderIter = m_OrderMsgList.begin();  t_orderIter != m_OrderMsgList.end(); t_orderIter++)
	{
		string t_mapKey;
		t_mapKey  = int2string(t_orderIter->msg_case);
		t_mapKey +=  ",";
		t_mapKey += int2string(t_orderIter->msg_skillid);

		map<string, vector<int>>::iterator t_iterMap;
		t_iterMap = pPush->m_PushConfigMap.find(t_mapKey);
		if(t_iterMap == pPush->m_PushConfigMap.end())
		{
			continue;
		}
	
		vector<int> t_mapValue(5);
		t_mapValue = t_iterMap->second;

		if(t_mapValue[t_orderIter->msg_sign]  == 1)
		{
			t_orderIter->msg_isPushArea = t_mapValue[0];
			t_orderIter->msg_isPushCity = t_mapValue[1];
			t_orderIter->msg_isPushProvince = t_mapValue[2];
			OrderMsg t_order = *t_orderIter;
			t_trueOrderMsgList.push_back(t_order);
		}
	}

	
	//查找适合订单的律师，并发送订单
	list<OrderMsg>::iterator t_iter1 = t_trueOrderMsgList.begin();

	for(; t_iter1 != t_trueOrderMsgList.end(); t_iter1++)
	{		
		list<uint32_t > m_LawyerUidList;
		pPush->getLawyerList(*t_iter1, m_LawyerUidList);
		if(m_LawyerUidList.size() == 0)
			continue;

		list<uint32_t >::iterator t_iter2 = m_LawyerUidList.begin();

	
		for(; t_iter2 != m_LawyerUidList.end(); t_iter2++)
		{
				g_pClient->sendMsg(*t_iter2, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_PUSH, t_iter1->msg_context);
			 	usleep(100);
		}


		//sql语句有长度限制，每次插入400条数据
		list<uint32_t>::iterator t_iter3 = m_LawyerUidList.begin();
		list<uint32_t > t_tmpMsg ;
		for(; t_iter3 != m_LawyerUidList.end(); t_iter3++)
		{
			t_tmpMsg.push_back(*t_iter3);

			if(t_tmpMsg.size() >= 400)
			{
				pPush->pushRecord(t_iter1->msg_orderid , t_tmpMsg);
				t_tmpMsg.clear();
				continue;
			}
		}
		if(t_iter3 == m_LawyerUidList.end() && t_tmpMsg.size() != 0)
		{
			pPush->pushRecord(t_iter1->msg_orderid , t_tmpMsg);
			t_tmpMsg.clear();
		}

	}

	//更新订单表，如果订单不符合发送条件，则不更新订单时间及发送次数
	if(m_OrderMsgList.size() != 0)
	{
		pPush->updateOrderPushtime(m_OrderMsgList);
		pPush->updateOrderSendsign(m_OrderMsgList);
		
	}
	if(t_trueOrderMsgList.size() != 0)
	{
		pPush->updateOrderUpdatetime(t_trueOrderMsgList);
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

	list<GrabRecord > m_GrabRecordList;
	m_GrabRecordList.clear();
	
	pPush->getGrabLawyer(m_GrabRecordList);
	if(m_GrabRecordList.size() == 0)
	{
		sleep(5);
		return ;
	}
	
	list<GrabRecord >::iterator t_iter;
	for(t_iter = m_GrabRecordList.begin(); t_iter != m_GrabRecordList.end(); t_iter++)
	{
		g_pClient->sendMsg(t_iter->grab_useruid, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_GRAB, t_iter->grab_context);	
		usleep(100);
	}

	pPush->updateGrabLawyerToClient(m_GrabRecordList);

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

	list<GrabRecord > m_GrabRecordList;
	m_GrabRecordList.clear();
	
	pPush->getLawyerGrabResult(m_GrabRecordList);
	if(m_GrabRecordList.size() == 0)
	{
		sleep(5);
		return ;
	}


	list<GrabRecord >::iterator t_iter;
	for(t_iter = m_GrabRecordList.begin(); t_iter != m_GrabRecordList.end(); t_iter++)
	{
		string strResult = "";
		if(t_iter->grab_status == 4)
		{
			strResult = "抢单成功";
		}
		else if(t_iter->grab_status == 5)
		{
			strResult = "抢单失败";
		}
		
		string strMsg = "{\"orderSn\":" + t_iter->grab_ordersn + "，\"orderId\":" + int2string(t_iter->grab_orderid) + ", \"orderUserId\":" + \
						int2string(t_iter->grab_useruid) + ", \"caseEtime\":" + t_iter->grab_orderendtime \
						+ ", \"grabStatus\":" + strResult + "};";	

		g_pClient->sendMsg(t_iter->grab_lawyeruid, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_RESULT, strMsg);
		usleep(100);
		
	}

	
	pPush->updateGrabResultToLawyer(m_GrabRecordList);

	usleep(500000);
}




CMyTimerThread::CMyTimerThread()
{
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
	
	uint64_t cur_tick = get_tick_count();
	if(cur_tick >= m_last_update_tick )
	{
		m_last_update_tick = cur_tick + THREAD_TIMER_INTERVAL;
		pPush->updateVipExp();
	}
	
	pPush->updateOrderExp();
	sleep(60);
}







