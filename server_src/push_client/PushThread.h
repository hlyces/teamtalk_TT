/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：Thread.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __PUSH_THREAD_H__
#define __PUSH_THREAD_H__

#include "Thread.h"
#include "PushModel.h"
#include <list>
#include "Client.h"
#include <map>


class CPushConfig : public CEventThread
{
public:
	CPushConfig();
	virtual ~CPushConfig();
    
 	virtual void OnThreadTick(void);

private:
};



class CPushThread : public CEventThread
{
public:
	CPushThread();
	virtual ~CPushThread();
    
 	virtual void OnThreadTick(void);

	void setGPClient(CClient& pClient){ g_pClient = &pClient;}
	
private:
	CClient* g_pClient;

};

class CNotifyClientGrabThread : public CEventThread
{
public:
	CNotifyClientGrabThread();
	virtual ~CNotifyClientGrabThread();
    
 	virtual void OnThreadTick(void);

	void setGPClient(CClient& pClient){ g_pClient = &pClient;}
	
private:
	CClient* g_pClient;
};

class CNotifyLawyerGrabThread : public CEventThread
{
public:
	CNotifyLawyerGrabThread();
	virtual ~CNotifyLawyerGrabThread();
    
 	virtual void OnThreadTick(void);

	void setGPClient(CClient& pClient){ g_pClient = &pClient;}
	
private:
	CClient* g_pClient;
};


class CEntrushThread : public CEventThread
{
public:
	CEntrushThread();
	virtual ~CEntrushThread();
    
 	virtual void OnThreadTick(void);

	void setGPClient(CClient& pClient){ g_pClient = &pClient;}
	
private:
	CClient* g_pClient;

};


class CTopUP_withDrawalThread : public CEventThread
{
public:
	CTopUP_withDrawalThread();
	virtual ~CTopUP_withDrawalThread();
    
 	virtual void OnThreadTick(void);

	void setGPClient(CClient& pClient){ g_pClient = &pClient;}
	
private:
	CClient* g_pClient;

};



class CMyTimerThread : public CEventThread
{
public:
	CMyTimerThread();
	virtual ~CMyTimerThread();
    
 	virtual void OnThreadTick(void);
private:
	uint64_t m_last_update_tick;
};




#endif
