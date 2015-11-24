/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：AudioModel.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月15日
 *   描    述：
 *
 ================================================================*/

#ifndef PUSH_MODEL_H_
#define PUSH_MODEL_H_


#include <list>
#include <map>
#include <vector>
#include "public_define.h"
#include "util.h"
#include "IM.BaseDefine.pb.h"

using namespace std;

typedef struct OrderMsg{
	uint32_t id;
	uint32_t msg_orderid;
	uint32_t msg_skillid;
	uint64_t msg_lasttime;
	uint32_t msg_province;
	uint32_t msg_city;
	uint32_t msg_area;
	uint32_t msg_workyear;
	uint16_t msg_case;
	uint16_t msg_status;
	uint16_t msg_sign;
	uint16_t msg_sex; 
	uint16_t msg_isPushProvince;
	uint16_t msg_isPushCity;
	uint16_t msg_isPushArea; 
	string   msg_context;

}OrderMsg;


typedef struct OrderEntrust{
	uint32_t id;
	uint32_t msg_orderid;
	uint32_t msg_lawyer;
	uint32_t msg_user;
	uint32_t msg_status;
	uint16_t msg_sign;
	string   msg_context;
}OrderEntrust;

typedef struct OrderStatusChg{
	uint32_t id;
	uint32_t msg_orderid;
	uint32_t msg_lawyer;
	uint32_t msg_user;
	uint32_t msg_status;
}OrderStatusChg;


/*grab_status判断抢单的状态
	1.发送给客户端的抢单信息   2.抢单信息发送成功
	3.不发送给客户端但处于等待的抢单信息
	4.抢单成功				   5.抢单失败
	6.抢单成功已经通知律师端   7.抢单失败已经通知律师端
	10.咨询与文书代写有一人抢单成功未通知客户端
	11.咨询与文书代写有一人抢单成功已通知客户端

	状态1、3由web service写入
	状态2--->4、2---->5、3---->5、3---->5由web service修改
	状态1--->2、4---->6、5---->7由此消息服务器修改
*/
typedef struct GrabRecord{
	uint64_t id;
	uint64_t grab_time; 
	uint64_t grab_userTime;
    uint64_t grab_lawyerTime;
	uint32_t grab_orderid;
	uint32_t grab_status;
	uint32_t grab_useruid;
	uint32_t grab_lawyeruid;
	uint64_t   grab_orderendtime;
	string	 grab_ordersn;
	string   grab_context;
}GrabRecord; 

typedef struct CheckUser{
	uint32_t check_id;
	uint32_t check_userid;
	uint16_t check_status;
}CheckUser;


//top-up withdrawal
typedef struct top_up_or_withdrawal{
	uint32_t id;
	uint32_t result_userid;
	uint16_t result_type;
	uint16_t result_sign;
	uint16_t result_result;
	string   result_linkid;
}TopUP_withDrawal;

enum CONFIGSTATUS{
	CONFIGTRUE  = 1,
	CONFIGFALSE = 0,
};

enum CaseType{
	CONSULT    = 1,
	LAWSUIT    = 2,
	ATTENDANCE = 3,
	DOCUMENTS  = 4,
};

//dffx_order_msg dffx_order
//#define SPECIAL_SKILLID 10001

enum Dffx_Order_Msg_Status{
	ORDINARYORDER =  1, //普通订单
	ENTRUSTORDER  =  2, //委托订单
	ORDERACCEPT   =  5, //订单受理
	ORDERCANCEL   = -1, //订单取消
	BYTHETIMEUNDO = -3, //订单截止时间撤销
	HAVEGRAB	  =  3, //有抢单
};

enum Dffx_Order_Msg_Sign{
	PUSHTOINIT			 	 =  0, //sign初始默认为0
	PUSHEDTOAREA			 =  1, //已推送至区，下一级推送市
	PUSHEDTOCITY 			 =  2, //已推送至市，下一级推送省
	PUSHEDTOPROVINCE 		 =  3,
	PUSHEDTOCOUNTRY 		 =  4,
	PUSHEDTOSPECIALLAWYER 	 =  5,
	PUSHEDTOENTRUST 		 = 11, //委托已推送
	PUSHEDTOACCEPT			 = 12,
	PUSHEDTOCANCEL			 = 13,
};

//dffx_order_grab
enum Dffx_Order_Grab_Sign{
	ENTERLISTPUSHING 		= 1,   //进入抢单列表待推送
	ENTERLISTPUSHED  		= 2,   //进入抢单列表已推送
	NOTIFYSUCCESSPUSHING	= 4,
	NOTIFYFAILPUSHING 		= 5,
	NOTIFYSUCCESSPUSHED 	= 6,
	NOTIFYFAILPUSHED 		= 7,
	DIRECTGRABPUSHING 		= 10,
	DIRECTGRABPUSHED 		= 11,
};

//dffx_common_pushresult
enum Dffx_Common_Pushresult_Type{
	TOPUPTYPE  = 1,			 //充值
	BALANCE_WITHDRAWAL = 2,  //余额提现  
	INTEGRAL_WITHDRAWAL = 3, //积分提现
};

enum Dffx_Common_Pushresult_Result{
	RESULTSUCCESS = 1,
	RESULTFAILED  = 2,
};


enum LawyerSSex{
	SEXULIMITE = 0,
	MALE	   = 1, 
	FEMALE	   = 2,
};

enum UserCheckResult{
	CHECKSUCCESS = 0,
	CHECKFAILED  = 1,
};

enum AllCancelOrComplete{
	ValidCancel = -3,
	UserCancel 	= -1,
	Complete 	=  0,
	Revoke 		= 15,
};

class CPushModel {
public:
	virtual ~CPushModel();

	static CPushModel* getInstance();
    void setUrl(string& strFileUrl);

	//订单消息推送给律师端
	bool getOrderMsgList(list<OrderMsg >& lsOrderMsg);
	bool getLawyerList(OrderMsg cOrderMsg, list<uint32_t >& lsLawyerUid);
	bool updateOrderMsgTable(list<OrderMsg > lsOrderMsg);
	bool updateOrderPushtime(list<OrderMsg > lsOrderMsg);
	bool updateOrderUpdatetime(list<OrderMsg > lsOrderMsg);
	bool updateOrderSendsign(list<OrderMsg > lsOrderMsg);
	bool pushRecord(uint32_t nPushOrderid , list<uint32_t > lsLawyerUid);

	//抢单消息通知客户端
	bool getGrabLawyer(list<GrabRecord >& lsGrabRecord);	
	bool updateGrabLawyerToClient(list<GrabRecord > lsGrabRecord);

	//抢单结果通知律师端
	bool getLawyerGrabResult(list<GrabRecord >& lsGrabRecord);
	bool updateGrabResultToLawyer(list<GrabRecord > lsGrabRecord);

	//委托订单处理
	bool handleEntrustOrder(list<OrderEntrust >& lsOrderEntrust);
	bool updateEntrustOrder(list<OrderEntrust > lsOrderEntrust);

	//订单等待付款(律师申请付款)通知客户端
	bool getWaitPayment(list<OrderStatusChg >& lsOrderStatusChg);
	bool updateWaitPayment(list<OrderStatusChg > lsOrderStatusChg);

	//订单撤销与完成通知律师端
	bool getCancelOrCompleteTolawyer(list<OrderStatusChg >& lsOrderStatusChg);
	bool updateCancelOrCompleteTolawyer(list<OrderStatusChg > lsOrderStatusChg);


	//充值或提现状态消息推送
	bool getTopUP_withDrawal(list<TopUP_withDrawal>& lsTopUP_withDrawal);
	bool updateTopUP_withDrawal(list<TopUP_withDrawal> lsTopUP_withDrawal);

	//会员过期
	void updateVipExp();

	//订单过期
	void updateOrderExp();

	//用户认证结果
	bool getCheckUser(list<CheckUser>& lsCheckUser);
	bool delCheckUser(list<CheckUser>& lsCheckUser);

	//获取推送配置 dffx_common_skillpushtime;
	void getPushConfig();

	map<string ,vector<int >> m_PushConfigMap;
	
private:
	CPushModel();

private:
	static CPushModel*	m_pInstance;
    string m_strFileSite;
	
};




#endif /* PUSH_MODEL_H_ */
