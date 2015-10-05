/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：PushModel.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#include "AutoPool.h"
#include "DBPool.h"
#include "PushModel.h"

using namespace std;

//PushModel
CPushModel* CPushModel::m_pInstance = NULL;

/**
 *  构造函数
 */
CPushModel::CPushModel()
{

}

/**
 *  析构函数
 */
CPushModel::~CPushModel()
{

}

/**
 *  单例
 *
 *  @return 单例的指针
 */
CPushModel* CPushModel::getInstance()
{
	if (!m_pInstance) 
	{
		m_pInstance = new CPushModel();
	}

	return m_pInstance;
}


bool CPushModel::getOrderMsgList(list<OrderMsg >& OrderMsgList)
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		uint64_t m_currentTime = 1000 * time(NULL);

		string strSql = "select * from dffx_order_msg where msg_status = 1 and msg_sign >= 0 and msg_sign < 5 and 1000 * msg_pushtime <= "\
						+ long2string(m_currentTime) + " - msg_lasttime order by msg_lasttime limit 10";
		log("strSql = %s", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			
			while (pResultSet->Next()) 
			{
				OrderMsg cOrderMsg;
				cOrderMsg.msg_orderid  = pResultSet->GetInt("msg_orderid");
				cOrderMsg.msg_skillid  = pResultSet->GetInt("msg_skillid");
				cOrderMsg.msg_lasttime = pResultSet->GetLong("msg_lasttime");
				cOrderMsg.msg_case     = pResultSet->GetInt("msg_case");
				cOrderMsg.msg_status   = pResultSet->GetInt("msg_status");
				cOrderMsg.msg_sign     = pResultSet->GetInt("msg_sign");
				cOrderMsg.msg_context  = pResultSet->GetString("msg_context");
				cOrderMsg.msg_province = pResultSet->GetInt("msg_province");
				cOrderMsg.msg_city     = pResultSet->GetInt("msg_city");
				cOrderMsg.msg_area     = pResultSet->GetInt("msg_area");
				cOrderMsg.msg_sex      = pResultSet->GetInt("msg_sex");
				cOrderMsg.msg_workyear      = pResultSet->GetInt("msg_workyear");
				
				OrderMsgList.push_back(cOrderMsg);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("getOrderMsgList failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;	

}

bool CPushModel::getLawyerList(OrderMsg t_OrderMsg, list<uint32_t >& LawyerUidList)
{
	bool bRet = false;

	string strCase = "";
	if(t_OrderMsg.msg_case == CONSULT|| t_OrderMsg.msg_case == DOCUMENTS)
	{
		strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2 where  t1.user_uid = t2.lawyer_uid and  \
					t1.user_type = 2 and t2.lawyer_skillid  = " + int2string(t_OrderMsg.msg_skillid) ; 
	}
	else if(t_OrderMsg.msg_case == LAWSUIT)
	{
		long int current_time = time(NULL);
		long int start_work_time;
		long int day_time = 365 * 24 * 60 * 60;
		start_work_time = 1000 * current_time - 1000 * day_time * t_OrderMsg.msg_workyear;
	
	    strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2 , dffx_user_lawyer t3 where  \
						t1.user_uid = t2.lawyer_uid and t2.lawyer_uid = t3.lawyer_uid and t1.user_type = 2 and t1.user_sex = "+ int2string(t_OrderMsg.msg_sex) \
						+ " and t2.lawyer_skillid  =" + int2string(t_OrderMsg.msg_skillid) \
		                + " and ("+ "  t3.lawyer_worktime <= " +  long2string(start_work_time) +  "  )";

	}
	else if(t_OrderMsg.msg_case == ATTENDANCE)
	{
		strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2  where  \
						t1.user_uid = t2.lawyer_uid and t1.user_type = 2 and t1.user_sex = "	\
						+ int2string(t_OrderMsg.msg_sex) + " and t2.lawyer_skillid  =" + int2string(t_OrderMsg.msg_skillid) ; 
	}

	string strRegion = "";
	if(t_OrderMsg.msg_sign == 0) //区
	{ 
		strRegion += " t1.user_area = " + int2string(t_OrderMsg.msg_area) ;
	}
	else if(t_OrderMsg.msg_sign == 1)//市
	{
	//	strRegion += " t1.user_area != " + int2string(t_OrderMsg.msg_area) +  " and t1.user_city = " + int2string(t_OrderMsg.msg_city) ;
		if(t_OrderMsg.msg_isPushArea = 0)
		{
			strRegion += " t1.user_city = " + int2string(t_OrderMsg.msg_city) ;
		}
		else
		{
			strRegion += " t1.user_area != " + int2string(t_OrderMsg.msg_area) +  " and t1.user_city = " + int2string(t_OrderMsg.msg_city) ;
		}
	
	}
	else if(t_OrderMsg.msg_sign == 2) //省
	{
		//strRegion += " t1.user_city != " + int2string(t_OrderMsg.msg_city) + " and t1.user_province = " + int2string(t_OrderMsg.msg_province) ;
		if(t_OrderMsg.msg_isPushCity= 0)
		{
			strRegion += "  t1.user_province = " + int2string(t_OrderMsg.msg_province) ;
		}
		else
		{
			strRegion += " t1.user_city != " + int2string(t_OrderMsg.msg_city) + " and t1.user_province = " + int2string(t_OrderMsg.msg_province) ;
		}		
	}
	else if(t_OrderMsg.msg_sign == 3) //国
	{
		if(t_OrderMsg.msg_isPushProvince = 0)
		{
			strRegion += " 1=1 ";
		}
		else
		{
			strRegion += " t1.user_province != " + int2string(t_OrderMsg.msg_province) ;
		}
		
		
	}
	else if(t_OrderMsg.msg_sign == 4) //特殊顾问
	{
		 strCase = "";
		 strCase += "select t1.user_uid from dffx_user t1  where 1=1  ";
		 strRegion += " t1.user_type = 3 " ;
	}
		

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn)
	{
		string strSql =  strCase + " and " + strRegion;
		log("strSql = %s", strSql.c_str());

		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			uint32_t user_uid;

			while (pResultSet->Next())
			{
				user_uid = pResultSet->GetInt("user_uid");
				LawyerUidList.push_back(user_uid);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("get lawyer_id failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}

}


bool CPushModel::updateOrderPushtime(list<OrderMsg >& OrderMsgList)
{
	list<OrderMsg >::iterator t_iter = OrderMsgList.begin();
	
	uint32_t msg_orderid = t_iter->msg_orderid;
	string str_msg_orderid = "(" + int2string(msg_orderid);
	t_iter++;
	
	for(; t_iter != OrderMsgList.end(); t_iter++)
	{
		msg_orderid = t_iter->msg_orderid;
		str_msg_orderid += "," + int2string(msg_orderid);
	}
	str_msg_orderid += ")";
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{
			//update 推送间隔时间
			strSql = "update dffx_order_msg t1, dffx_common_skillpushtime t2 set t1.msg_pushtime = t2.skill_pushtime \
					where  t1.msg_sign = 0 and t1.msg_case = t2.skill_caseid and t1.msg_skillid = t2.skill_skillid and \
					msg_orderid in  " + str_msg_orderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("strSql = %s", strSql.c_str());
				log("updateOrderPushtime failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;
}

bool CPushModel::updateOrderUpdatetime(list<OrderMsg >& OrderMsgList)
{
	list<OrderMsg >::iterator t_iter = OrderMsgList.begin();
	
	uint32_t msg_orderid = t_iter->msg_orderid;
	string str_msg_orderid = "(" + int2string(msg_orderid);
	t_iter++;
	
	for(; t_iter != OrderMsgList.end(); t_iter++)
	{
		msg_orderid = t_iter->msg_orderid;
		str_msg_orderid += "," + int2string(msg_orderid);
	}
	str_msg_orderid += ")";
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{
			//update 当前时间 
			uint64_t m_currentTime = 1000 * time(NULL);
			strSql = "update dffx_order_msg t1 set t1.msg_lasttime = " +  long2string(m_currentTime) + " where  t1.msg_orderid in " + str_msg_orderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("strSql = %s", strSql.c_str());
				log("updateOrderUpdatetime failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;

}

bool CPushModel::updateOrderSendsign(list<OrderMsg >& OrderMsgList)
{
	list<OrderMsg >::iterator t_iter = OrderMsgList.begin();
	
	uint32_t msg_orderid = t_iter->msg_orderid;
	string str_msg_orderid = "(" + int2string(msg_orderid);
	t_iter++;
	
	for(; t_iter != OrderMsgList.end(); t_iter++)
	{
		msg_orderid = t_iter->msg_orderid;
		str_msg_orderid += "," + int2string(msg_orderid);
	}
	str_msg_orderid += ")";
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{
			//update 标识(发送次数)
			strSql = "update dffx_order_msg t1 set t1.msg_sign = t1.msg_sign + 1  where  t1.msg_orderid in " + str_msg_orderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("strSql = %s", strSql.c_str());
				log("updateOrderSendsign failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;

}




bool CPushModel::deleteOrderList()
{
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{		
			strSql = "delete from dffx_order_msg where msg_sign > 2 or msg_status >= 3";
			
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("strSql = %s", strSql.c_str());
				log("deleteOrderList failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;	
}

bool CPushModel::pushRecord(uint32_t push_orderid, list<uint32_t >& LawyerUidList)
{
	uint64_t m_currentTime = 1000 * time(NULL);

	string str = "";

	list<uint32_t >::iterator t_iter = LawyerUidList.begin();
	str += "("+ int2string(push_orderid) + "," + int2string(*t_iter) + "," + long2string(m_currentTime) + ")";
	t_iter++;	
	for(; t_iter != LawyerUidList.end(); t_iter++)
	{
		str += ",("+ int2string(push_orderid) + "," + int2string(*t_iter) + "," + long2string(m_currentTime) + ")";
	}
	str += ";";
		
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{		
			strSql = "insert into  dffx_order_pushrecord(push_orderid, push_uid, push_time) VALUES " + str;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}else
		{
			log("strSql = %s", strSql.c_str());
			log("pushRecord failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);		
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;
}



bool CPushModel::getGrabLawyer(list<GrabRecord >& GrabRecordList)
{

	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn)
	{		 
		string strSql = "select * from dffx_order_grabrecord where grab_status = 1 limit 300";
		log("strSql = %s", strSql.c_str());
		
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 			
			while (pResultSet->Next()) 
			{
				GrabRecord cGrabRecord;

				cGrabRecord.id = pResultSet->GetLong("id");
				cGrabRecord.grab_useruid= pResultSet->GetInt("grab_useruid");
				cGrabRecord.grab_lawyeruid= pResultSet->GetInt("grab_lawyeruid");
				cGrabRecord.grab_time = pResultSet->GetLong("grab_time");
				cGrabRecord.grab_orderid   = pResultSet->GetInt("grab_orderid");
				cGrabRecord.grab_ordersn   = pResultSet->GetString("grab_ordersn");
				cGrabRecord.grab_context   = pResultSet->GetString("grab_context");

				GrabRecordList.push_back(cGrabRecord);		
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("getGrabLawyer failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;			
}

bool CPushModel::updateGrabLawyerToClient(list<GrabRecord >& GrabRecordList)
{
	string str = "(";
	list<GrabRecord >::iterator t_iter = GrabRecordList.begin();
	str += long2string(t_iter->id);
	
	for(t_iter++; t_iter != GrabRecordList.end(); t_iter++)
	{
		str += "," + long2string(t_iter->id);
	}
	str += ")";

	bool bRet = false;
	uint64_t grab_userTime_current = 1000 * time(NULL);

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_order_grabrecord set grab_status = 2, grab_userTime = " \
							+ long2string(grab_userTime_current) + " where id in " + str;
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());

		if(!bRet)
		{
			log("strSql = %s", strSql.c_str());
			log("updateGrabLawyerToClient failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	
	return bRet;	
}



//==================================================================================

bool CPushModel::getLawyerGrabResult(list<GrabRecord >& GrabRecordList)
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "select * from dffx_order_grabrecord where grab_status = 4 or grab_status = 5 limit 300";
		log("strSql = %s", strSql.c_str());
		
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 				
			while (pResultSet->Next())
			{
				GrabRecord cGrabRecord;

				cGrabRecord.id = pResultSet->GetLong("id");
				cGrabRecord.grab_useruid= pResultSet->GetInt("grab_useruid");
				cGrabRecord.grab_lawyeruid  = pResultSet->GetInt("grab_lawyeruid");
				cGrabRecord.grab_time = pResultSet->GetLong("grab_time");
				cGrabRecord.grab_orderid   = pResultSet->GetInt("grab_orderid");
				cGrabRecord.grab_status= pResultSet->GetInt("grab_status");
				cGrabRecord.grab_ordersn   = pResultSet->GetString("grab_ordersn");
				cGrabRecord.grab_orderendtime   = pResultSet->GetString("grab_orderendtime");
				
				GrabRecordList.push_back(cGrabRecord);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("getLawyerGrabResult failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	
	return bRet;				
}


bool CPushModel::updateGrabResultToLawyer(list<GrabRecord >& GrabRecordList)
{
	string str = "(";
	list<GrabRecord >::iterator t_iter = GrabRecordList.begin();
	str += long2string(t_iter->id);
	
	for(t_iter++; t_iter != GrabRecordList.end(); t_iter++)
	{
		str += "," + long2string(t_iter->id);
	}
	str += ")";

	bool bRet = false;
	uint64_t grab_lawyerTime_current = 1000 * time(NULL);

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_order_grabrecord SET grab_status = (CASE grab_status WHEN 4 THEN 6 WHEN 5 THEN 7  \
						ELSE grab_status END ), grab_lawyerTime = " + long2string(grab_lawyerTime_current) + " where id in " + str;
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)
		{
			log("strSql = %s", strSql.c_str());
			log("updateGrabResultToLawyer failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}

	return bRet;		
}
//更新过期用户状态
#define SQL_UPDATE_VIP_EXPRI_STATUS  "UPDATE dffx_user SET user_isvip = 0 WHERE user_isvip = 2 AND  id IN (SELECT vipvalid_uid  FROM dffx_user_vipvalid WHERE NOW() > vipvalid_endtime);"
//更新过期订单状态
#define SQL_UPDATE_ORDER_EXPRI_STATUS  "UPDATE dffx_order SET order_status = -3 WHERE order_status= 1 AND  id IN (SELECT case_orderid FROM dffx_order_case WHERE  case_type != " + int2string(LAWSUIT) + " and   NOW() > case_etime);"

//考虑原子操作一步更新
void CPushModel::updateVipExp()
{
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = SQL_UPDATE_VIP_EXPRI_STATUS;

		pDBConn->ExecuteUpdate(strSql.c_str());
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
 
}
void  CPushModel::updateOrderExp()
{
 
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = SQL_UPDATE_ORDER_EXPRI_STATUS;

		pDBConn->ExecuteUpdate(strSql.c_str());
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
 	

}

void  CPushModel::getPushConfig()
{
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "select * from dffx_common_skillpushtime";
		
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 				
			while (pResultSet->Next())
			{
				string m_vecKey;
				vector<int> m_vecValue(5);

				m_vecKey  =  pResultSet->GetString("skill_caseid");
				m_vecKey +=  ",";
				m_vecKey +=  pResultSet->GetString("skill_skillid");
				m_vecValue[0] =  pResultSet->GetInt("skill_ispusharea");
				m_vecValue[1] =  pResultSet->GetInt("skill_ispushcity");
				m_vecValue[2] =  pResultSet->GetInt("skill_ispushprovince");
				m_vecValue[3] =  1;
				m_vecValue[4] =  1;

				m_PushConfigMap.insert(map<string,vector<int>>::value_type(m_vecKey, m_vecValue));
				
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("getPushConfig failed:%s", strSql.c_str());
		}

		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}

}





