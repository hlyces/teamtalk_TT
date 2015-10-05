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




/*grab_status判断抢单的状态
	1.发送给客户端的抢单信息   2.抢单信息发送成功
	3.不发送给客户端但处于等待的抢单信息
	4.抢单成功				   5.抢单失败
	6.抢单成功已经通知律师端   7.抢单失败已经通知律师端

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
	string   grab_orderendtime;
	string	 grab_ordersn;
	string   grab_context;
}GrabRecord; 

enum CaseType{
	CONSULT = 1,
	LAWSUIT = 2,
	ATTENDANCE = 3,
	DOCUMENTS = 4,
};



class CPushModel {
public:
	virtual ~CPushModel();

	static CPushModel* getInstance();
    void setUrl(string& strFileUrl);

	//订单消息推送给律师端
	bool getOrderMsgList(list<OrderMsg >& OrderMsgList);
	bool getLawyerList(OrderMsg t_OrderMsg, list<uint32_t >& LawyerUidList);
	bool updateOrderMsgTable(list<OrderMsg >& OrderMsgList);
	bool updateOrderPushtime(list<OrderMsg >& OrderMsgList);
	bool updateOrderUpdatetime(list<OrderMsg >& OrderMsgList);
	bool updateOrderSendsign(list<OrderMsg >& OrderMsgList);
	bool deleteOrderList();
	bool pushRecord(uint32_t push_orderid , list<uint32_t >& LawyerUidList);

	//抢单消息通知客户端
	bool getGrabLawyer(list<GrabRecord >& GrabRecordList);	
	bool updateGrabLawyerToClient(list<GrabRecord >& GrabRecordList);

	//抢单结果通知律师端
	bool getLawyerGrabResult(list<GrabRecord >& GrabRecordList);
	bool updateGrabResultToLawyer(list<GrabRecord >& GrabRecordList);


	//会员过期
	void updateVipExp();

	//订单过期
	void updateOrderExp();

	//获取推送配置 dffx_common_skillpushtime;
	void getPushConfig();

//	map<vector<int >,vector<int >> m_PushConfigMap;
	map<string ,vector<int >> m_PushConfigMap;
	
private:
	CPushModel();

private:
	static CPushModel*	m_pInstance;
    string m_strFileSite;
	
};




#endif /* PUSH_MODEL_H_ */
