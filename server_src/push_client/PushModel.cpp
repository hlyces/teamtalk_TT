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


bool CPushModel::getOrderMsgList(list<OrderMsg >& lsOrderMsg)
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
				cOrderMsg.msg_workyear = pResultSet->GetInt("msg_workyear");
				
				lsOrderMsg.push_back(cOrderMsg);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("getOrderMsgList failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;	

}

bool CPushModel::getLawyerList(OrderMsg cOrderMsg, list<uint32_t >& lsLawyerUid )
{
	bool bRet = false;

	string strCase = "";
	if(cOrderMsg.msg_case == CONSULT|| cOrderMsg.msg_case == DOCUMENTS)
	{
		strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2 where t1.user_ischeck = 2 and  t1.user_uid = t2.lawyer_uid and  \
					t1.user_type = 2 and t2.lawyer_skillid  = " + int2string(cOrderMsg.msg_skillid) ; 
	}
	else if(cOrderMsg.msg_case == LAWSUIT)
	{
		long int current_time = time(NULL);
		long int start_work_time;
		long int day_time = 365 * 24 * 60 * 60;
		start_work_time = 1000 * current_time - 1000 * day_time * cOrderMsg.msg_workyear;

		if(SEXULIMITE == cOrderMsg.msg_sex)
		{
			strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2 , dffx_user_lawyer t3 where  \
						t1.user_ischeck = 2 and t1.user_uid = t2.lawyer_uid and t2.lawyer_uid = t3.lawyer_uid and t1.user_type = 2  and t2.lawyer_skillid  =" + int2string(cOrderMsg.msg_skillid) \
		                + " and ("+ "  t3.lawyer_worktime <= " +  long2string(start_work_time) +  "  )";
		}
		else
		{
			strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2 , dffx_user_lawyer t3 where  \
						t1.user_ischeck = 2 and t1.user_uid = t2.lawyer_uid and t2.lawyer_uid = t3.lawyer_uid and t1.user_type = 2 and t1.user_sex = "+ int2string(cOrderMsg.msg_sex) \
						+ " and t2.lawyer_skillid  =" + int2string(cOrderMsg.msg_skillid) \
		                + " and ("+ "  t3.lawyer_worktime <= " +  long2string(start_work_time) +  "  )";
		}    

	}
	else if(cOrderMsg.msg_case == ATTENDANCE)
	{
		if(SEXULIMITE == cOrderMsg.msg_sex)
		{
			strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2  where  \
						t1.user_ischeck = 2 and t1.user_uid = t2.lawyer_uid and t1.user_type = 2 and t2.lawyer_skillid  =" + int2string(cOrderMsg.msg_skillid) ; 
		}
		else
		{
			strCase = "select t1.user_uid from dffx_user t1, dffx_user_lawyerskill  t2  where  \
						t1.user_ischeck = 2 and t1.user_uid = t2.lawyer_uid and t1.user_type = 2 and t1.user_sex = "	\
						+ int2string(cOrderMsg.msg_sex) + " and t2.lawyer_skillid  =" + int2string(cOrderMsg.msg_skillid) ; 
		}
		
	}

	string strRegion = "";
	if(cOrderMsg.msg_sign == PUSHTOINIT) //区
	{ 
		strRegion += " t1.user_area = " + int2string(cOrderMsg.msg_area) ;
	}
	else if(cOrderMsg.msg_sign == PUSHEDTOAREA)//市
	{
		if(cOrderMsg.msg_isPushArea == CONFIGFALSE)
		{
			strRegion += " t1.user_city = " + int2string(cOrderMsg.msg_city) ;
		}
		else
		{
			strRegion += " t1.user_area != " + int2string(cOrderMsg.msg_area) +  " and t1.user_city = " + int2string(cOrderMsg.msg_city) ;
		}
	
	}
	else if(cOrderMsg.msg_sign == PUSHEDTOCITY) //省
	{
		//strRegion += " t1.user_city != " + int2string(t_OrderMsg.msg_city) + " and t1.user_province = " + int2string(t_OrderMsg.msg_province) ;
		if(cOrderMsg.msg_isPushCity== CONFIGFALSE)
		{
			strRegion += "  t1.user_province = " + int2string(cOrderMsg.msg_province) ;
		}
		else
		{
			strRegion += " t1.user_city != " + int2string(cOrderMsg.msg_city) + " and t1.user_province = " + int2string(cOrderMsg.msg_province) ;
		}		
	}
	else if(cOrderMsg.msg_sign == PUSHEDTOPROVINCE) //国
	{
		if(cOrderMsg.msg_isPushProvince == CONFIGFALSE)
		{
			strRegion += " 1=1 ";
		}
		else
		{
			strRegion += " t1.user_province != " + int2string(cOrderMsg.msg_province) ;
		}
		
		
	}
	else if(cOrderMsg.msg_sign == PUSHEDTOCOUNTRY) //特殊顾问
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
				lsLawyerUid.push_back(user_uid);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("strSql = %s", strSql.c_str());
			log("get lawyer_id failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	
	return bRet;
}


bool CPushModel::updateOrderPushtime(list<OrderMsg > lsOrderMsg )
{
	list<OrderMsg >::iterator lsIter = lsOrderMsg.begin();
	
	uint32_t nMsgOrderid = lsIter->msg_orderid;
	string strMsgOrderid = "(" + int2string(nMsgOrderid);
	lsIter++;
	
	for(; lsIter != lsOrderMsg.end(); lsIter++)
	{
		nMsgOrderid = lsIter->msg_orderid;
		strMsgOrderid += "," + int2string(nMsgOrderid);
	}
	strMsgOrderid += ")";
	
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
					msg_orderid in  " + strMsgOrderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("updateOrderPushtime failed:%s", strSql.c_str());
				bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;
}

bool CPushModel::updateOrderUpdatetime(list<OrderMsg > lsOrderMsg )
{
	list<OrderMsg >::iterator lsIter = lsOrderMsg.begin();
	
	uint32_t nMsgOrderid = lsIter->msg_orderid;
	string strMsgOrderid = "(" + int2string(nMsgOrderid);
	lsIter++;
	
	for(; lsIter != lsOrderMsg.end(); lsIter++)
	{
		nMsgOrderid = lsIter->msg_orderid;
		strMsgOrderid += "," + int2string(nMsgOrderid);
	}
	strMsgOrderid += ")";
	
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
			strSql = "update dffx_order_msg t1 set t1.msg_lasttime = " +  long2string(m_currentTime) + " where  t1.msg_orderid in " + strMsgOrderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("updateOrderUpdatetime failed:%s", strSql.c_str());
				bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;

}

bool CPushModel::updateOrderSendsign(list<OrderMsg > lsOrderMsg )
{
	list<OrderMsg >::iterator lsIter = lsOrderMsg.begin();
	
	uint32_t nMsgOrderid = lsIter->msg_orderid;
	string strMsgOrderid = "(" + int2string(nMsgOrderid);
	lsIter++;
	
	for(; lsIter != lsOrderMsg.end(); lsIter++)
	{
		nMsgOrderid = lsIter->msg_orderid;
		strMsgOrderid += "," + int2string(nMsgOrderid);
	}
	strMsgOrderid += ")";
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{
			//update 标识(发送次数)
			strSql = "update dffx_order_msg t1 set t1.msg_sign = t1.msg_sign + 1  where  t1.msg_orderid in " + strMsgOrderid;
			log("strSql = %s", strSql.c_str());
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		}
		else
		{
				log("updateOrderSendsign failed:%s", strSql.c_str());
				bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;

}

bool CPushModel::pushRecord(uint32_t cPushOrderid, list<uint32_t > lsLawyerUid )
{
	uint64_t lCurrentTime = 1000 * time(NULL);

	string strData = "";

	list<uint32_t >::iterator lsIter = lsLawyerUid.begin();
	strData += "("+ int2string(cPushOrderid) + "," + int2string(*lsIter) + "," + long2string(lCurrentTime) + ")";
	lsIter++;	
	for(; lsIter != lsLawyerUid.end(); lsIter++)
	{
		strData += ",("+ int2string(cPushOrderid) + "," + int2string(*lsIter) + "," + long2string(lCurrentTime) + ")";
	}
	strData += ";";
		
	
	bool bRet = false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if(pDBConn)
	{
		string strSql ;
		if (pDBConn)
		{		
			strSql = "insert into  dffx_order_pushrecord(push_orderid, push_uid, push_time) VALUES " + strData;
			bRet = pDBConn->ExecuteUpdate(strSql.c_str());
			bRet = false;
		}
		else
		{
			log("pushRecord failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);		
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
	return bRet;
}



bool CPushModel::getGrabLawyer(list<GrabRecord >& lsGrabRecord )
{

	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn)
	{		 
		string strSql = "select * from dffx_order_grabrecord where grab_status = 1 or grab_status = 10 limit 300";
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
				cGrabRecord.grab_status  = pResultSet->GetInt("grab_status");
				cGrabRecord.grab_ordersn   = pResultSet->GetString("grab_ordersn");
				cGrabRecord.grab_context   = pResultSet->GetString("grab_context");

				lsGrabRecord.push_back(cGrabRecord);		
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("getGrabLawyer failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;			
}

bool CPushModel::updateGrabLawyerToClient(list<GrabRecord > lsGrabRecord )
{
	string strId = "(";
	list<GrabRecord >::iterator lsIter = lsGrabRecord.begin();
	strId += long2string(lsIter->id);
	
	for(lsIter++; lsIter != lsGrabRecord.end(); lsIter++)
	{
		strId += "," + long2string(lsIter->id);
	}
	strId += ")";

	bool bRet = false;
	uint64_t lCurrentTime = 1000 * time(NULL);

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_order_grabrecord set grab_status = (CASE grab_status WHEN 1 THEN 2 WHEN 10 THEN 11 \
						ELSE grab_status END ), grab_userTime = " + long2string(lCurrentTime) + " where id in " + strId;
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());

		if(!bRet)
		{
			log("updateGrabLawyerToClient failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	
	return bRet;	
}



//==================================================================================

bool CPushModel::getLawyerGrabResult(list<GrabRecord >& lsGrabRecord )
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "select *,UNIX_TIMESTAMP(grab_orderendtime) as endtime from dffx_order_grabrecord where grab_status = 4 or grab_status = 5 limit 300";
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
				cGrabRecord.grab_orderendtime   = 1000 * pResultSet->GetLong("endtime");
				cGrabRecord.grab_context  = pResultSet->GetString("grab_context");
				
				lsGrabRecord.push_back(cGrabRecord);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("getLawyerGrabResult failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);	
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	
	return bRet;				
}


bool CPushModel::updateGrabResultToLawyer(list<GrabRecord > lsGrabRecord )
{
	string strId = "(";
	list<GrabRecord >::iterator lsIter = lsGrabRecord.begin();
	strId += long2string(lsIter->id);
	
	for(lsIter++; lsIter != lsGrabRecord.end(); lsIter++)
	{
		strId += "," + long2string(lsIter->id);
	}
	strId += ")";

	bool bRet = false;
	uint64_t grab_lawyerTime_current = 1000 * time(NULL);

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_order_grabrecord SET grab_status = (CASE grab_status WHEN 4 THEN 6 WHEN 5 THEN 7  \
						ELSE grab_status END ), grab_lawyerTime = " + long2string(grab_lawyerTime_current) + " where id in " + strId;
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)
		{
			log("updateGrabResultToLawyer failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}

	return bRet;		
}


//委托订单处理
bool CPushModel::handleEntrustOrder(list<OrderEntrust >& lsOrderEntrust )
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
  		string strSql = "select * from dffx_order_msg where ((msg_status = 2 and msg_sign != 11) or (msg_status = -1 and msg_sign != 12) \
							or (msg_status = 5 and msg_sign != 13)) and msg_skillid = 10001";
		log("strSql = %s", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			while (pResultSet->Next()) 
			{
				OrderEntrust cOrderEntrust;
				cOrderEntrust.msg_orderid = pResultSet->GetInt("msg_orderid");
				cOrderEntrust.msg_lawyer  = pResultSet->GetInt("msg_lawyer");
				cOrderEntrust.msg_user    = pResultSet->GetInt("msg_user");
				cOrderEntrust.msg_status  = pResultSet->GetInt("msg_status");
				cOrderEntrust.msg_sign    = pResultSet->GetInt("msg_sign");
				cOrderEntrust.msg_context = pResultSet->GetString("msg_context");			
				lsOrderEntrust.push_back(cOrderEntrust);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("handleEntrustOrder failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;
}

bool CPushModel::updateEntrustOrder(list<OrderEntrust > lsOrderEntrust )
{
	string strOrderid = "(";
	list<OrderEntrust >::iterator lsIter = lsOrderEntrust.begin();
	strOrderid += long2string(lsIter->msg_orderid);
	
	for(lsIter++; lsIter != lsOrderEntrust.end(); lsIter++)
	{
		strOrderid += "," + long2string(lsIter->msg_orderid);
	}
	strOrderid += ")";

	bool bRet = false;
	uint64_t entrust_time_current = 1000 * time(NULL);
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_order_msg SET msg_sign = (CASE msg_status WHEN 2 THEN 11 WHEN -1 THEN 12 WHEN 5 THEN 13  \
						ELSE msg_status END ), msg_lasttime = " + long2string(entrust_time_current) + " where msg_orderid in " + strOrderid;
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)
		{
			log("updateEntrustOrder failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}

	return bRet;	

}


bool CPushModel::getTopUP_withDrawal(list<TopUP_withDrawal>& lsTopUP_withDrawal )
{
	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
  		string strSql = "select * from dffx_common_pushresult where result_sign=0 limit 300";
		log("strSql = %s", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			while (pResultSet->Next()) 
			{
				TopUP_withDrawal cTopUP_withDrawal;
				cTopUP_withDrawal.id = pResultSet->GetInt("id");
				cTopUP_withDrawal.result_userid = pResultSet->GetInt("result_userid");
				cTopUP_withDrawal.result_type   = pResultSet->GetInt("result_type");
				cTopUP_withDrawal.result_result = pResultSet->GetInt("result_result");
				cTopUP_withDrawal.result_linkid = pResultSet->GetString("result_linkid");
				
				lsTopUP_withDrawal.push_back(cTopUP_withDrawal);
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("getTopUP_withDrawal failed:%s", strSql.c_str());
			bRet = false;
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
	return bRet;

}

bool CPushModel::updateTopUP_withDrawal(list<TopUP_withDrawal> lsTopUP_withDrawal )
{
	string strId = "(";
	list<TopUP_withDrawal >::iterator lsIter = lsTopUP_withDrawal.begin();
	strId += long2string(lsIter->id);
	
	for(lsIter++; lsIter != lsTopUP_withDrawal.end(); lsIter++)
	{
		strId += "," + long2string(lsIter->id);
	}
	strId += ")";

	bool bRet = false;

	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strSql = "update dffx_common_pushresult set result_sign = 1 , result_pushtime = NOW() where id in " + strId + " ;";
		log("strSql = %s", strSql.c_str());
		
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		if(!bRet)
		{
			log("updateEntrustOrder failed:%s", strSql.c_str());
		}
					
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}

	return bRet;
}






//更新过期用户状态
#define SQL_SELECT_VIP_EXPRI_STATUS  "SELECT vipvalid_uid  FROM dffx_user_vipvalid WHERE 1000 * unix_timestamp() > vipvalid_endtime and vipvalid_status > 0 limit 100"
#define SQL_UPDATE_USERTABLE_VIP_EXPRI_STATUS  "UPDATE dffx_user SET user_isvip = 0 WHERE user_isvip > 0 AND  user_uid IN "
#define SQL_UPDATE_VIP_EXPRI_STATUS  "UPDATE dffx_user_vipvalid SET vipvalid_status = 0 WHERE vipvalid_uid IN "
//更新过期订单状态
#define SQL_SELECT_ORDER_EXPRT        "SELECT valid_orderid FROM dffx_order_valid  WHERE  NOW() > valid_time limit 100"
#define SQL_UPDATE_ORDER_EXPRI_STATUS  "UPDATE dffx_order SET order_status = -3 ,order_updatetime = 1000 * unix_timestamp() WHERE (order_status= 1  OR order_status= 2 OR order_status= 3) AND  id IN "
#define SQL_UPDATE_ORDER_MSG_EXPRI_STATUS  "UPDATE dffx_order_msg SET msg_status = -3 WHERE (msg_status= 1  OR msg_status= 2 OR msg_status= 3) AND  msg_orderid IN "
#define SQL_DELETE_ORDER_EXPRT        "DELETE FROM dffx_order_valid  WHERE  valid_orderid in "


//考虑原子操作一步更新
void CPushModel::updateVipExp()
{
	bool bRet =false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string expIdStr = "(";
		
		string strSql = SQL_SELECT_VIP_EXPRI_STATUS;
		log("strSql = %s ", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			while (pResultSet->Next()) 
			{
				string expId = pResultSet->GetString("vipvalid_uid");
				expIdStr += expId + ",";
			}
			expIdStr +=  "0) ";
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			pDBManger->RelDBConn(pDBConn);
			return ;
		}

		strSql = SQL_UPDATE_USERTABLE_VIP_EXPRI_STATUS + expIdStr;
		log("strSql = %s ", strSql.c_str());
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
	/*	if(!bRet)
		{
			log("SQL failed:%s", strSql.c_str());
			pDBManger->RelDBConn(pDBConn);
			return ;
		}
*/
		strSql = SQL_UPDATE_VIP_EXPRI_STATUS + expIdStr;
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
		log("strSql = %s ", strSql.c_str());
	/*	if(!bRet)
		{
			log("SQL failed:%s", strSql.c_str());
			pDBManger->RelDBConn(pDBConn);
			return ;
		}
	*/
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}
 	

 
}
void  CPushModel::updateOrderExp()
{
	bool bRet =false;
	
	CDBManager* pDBManger = CDBManager::getInstance();
	CDBConn* pDBConn = pDBManger->GetDBConn("dffxIMDB_slave");
	if (pDBConn) 
	{
		string strValidOrderid = "(";
		
		string strSql = SQL_SELECT_ORDER_EXPRT;
		log("strSql = %s ", strSql.c_str());
		CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
		if(pResultSet)
		{ 		
			while (pResultSet->Next()) 
			{
				string validOrderid = pResultSet->GetString("valid_orderid");
				strValidOrderid += validOrderid + ",";
			}
			strValidOrderid +=  "0) ";
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			pDBManger->RelDBConn(pDBConn);
			return ;
		}


		strSql = SQL_UPDATE_ORDER_EXPRI_STATUS + strValidOrderid ;
		log("strSql = %s ", strSql.c_str());
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
	/*	if(!bRet)
		{
			log("SQL failed:%s", strSql.c_str());
			pDBManger->RelDBConn(pDBConn);
			return ;
		}
	*/	
		strSql = SQL_UPDATE_ORDER_MSG_EXPRI_STATUS + strValidOrderid;
		log("strSql = %s ", strSql.c_str());
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
	/*	if(!bRet)
		{
			log("SQL failed:%s", strSql.c_str());
			pDBManger->RelDBConn(pDBConn);
			return ;
		}
*/
		
		strSql = SQL_DELETE_ORDER_EXPRT + strValidOrderid;
		log("strSql = %s ", strSql.c_str());
		bRet = pDBConn->ExecuteUpdate(strSql.c_str());
	/*	if(!bRet)
		{
			log("SQL failed:%s", strSql.c_str());
			pDBManger->RelDBConn(pDBConn);
			return ;
		}

	*/
		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
	}
 	
	return ;
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
				string sVecKey;
				vector<int> sVecValue(5);

				sVecKey  =  pResultSet->GetString("skill_caseid");
				sVecKey +=  ",";
				sVecKey +=  pResultSet->GetString("skill_skillid");
				sVecValue[0] =  pResultSet->GetInt("skill_ispusharea");
				sVecValue[1] =  pResultSet->GetInt("skill_ispushcity");
				sVecValue[2] =  pResultSet->GetInt("skill_ispushprovince");
				sVecValue[3] =  CONFIGTRUE;
				sVecValue[4] =  CONFIGTRUE;

				m_PushConfigMap.insert(map<string,vector<int>>::value_type(sVecKey, sVecValue));
				
			}
			pResultSet->Clear();
			bRet = true;
		}
		else
		{
			log("getPushConfig failed:%s", strSql.c_str());
			bRet = false;
		}

		pDBManger->RelDBConn(pDBConn);
	}
	else
	{
		log("no db connection for dffx_db_bs");
		bRet = false;
	}

}





