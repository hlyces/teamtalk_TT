/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：test_client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/
//--//
#include <vector>
#include <iostream>
#include "ClientConn.h"
#include "netlib.h"
#include "TokenValidator.h"
#include "Thread.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Buddy.pb.h"
#include "playsound.h"
#include "Common.h"
#include "Client.h"
using namespace std;

#define MAX_LINE_LEN	1024
string g_login_domain = "http://127.0.0.1:8080";
string g_cmd_string[10];
int g_cmd_num;
CClient* g_pClient = NULL;
void split_cmd(char* buf)
{
	int len = strlen(buf);
	string element;

	g_cmd_num = 0;
	for (int i = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			if (!element.empty()) {
				g_cmd_string[g_cmd_num++] = element;
				element.clear();
			}
		} else {
			element += buf[i];
		}
	}

	// put the last one
	if (!element.empty()) {
		g_cmd_string[g_cmd_num++] = element;
	}
}

void print_help()
{
	printf("Usage:\n");
    printf("login user_name user_pass\n");
    /*
	printf("connect serv_ip serv_port user_name user_pass\n");
    printf("getuserinfo\n");
    printf("send toId msg\n");
    printf("unreadcnt\n");
     */
	printf("close\n");
	printf("quit\n");
}

void doLogin(const string& strName, const string& strPass)
{    
	int nTotal = string2int(strName);
	string sPass = "202cb962ac59075b964b07152d234b70";
	//string sPass = "e10adc3949ba59abbe56e057f20f883e";
	
    try
    {
//    	for(int i=0;i<nTotal;i++)
//	    {
//	     	string sName = int2string((uint32_t)(i+1));
	     	string sName = strName;
	        CClient* pClient = new CClient(sName, sPass, g_login_domain);
			pClient->connect();
			g_pClient = pClient;
  //  	}
    }
    catch(...)
    {
        printf("get error while alloc memory\n");
        PROMPTION;
        return;
    }
    
}
void exec_cmd()
{
	if (g_cmd_num == 0) {
		return;
	}
    
    if(g_cmd_string[0] == "login")
    {
        if(g_cmd_num == 3)
        {
            doLogin(g_cmd_string[1], g_cmd_string[2]);
        }
        else
        {
            print_help();
        }
    }
    else if (strcmp(g_cmd_string[0].c_str(), "close") == 0) {
        g_pClient->close();
    }
    else if (strcmp(g_cmd_string[0].c_str(), "quit") == 0) {
		exit(0);

    }
    else if(strcmp(g_cmd_string[0].c_str(), "list") == 0)
    {
        printf("+---------------------+\n");
        printf("|        用户名        |\n");
        printf("+---------------------+\n");
        CMapNick2User_t mapUser = g_pClient->getNick2UserMap();
        auto it = mapUser.begin();
        for(;it!=mapUser.end();++it)
        {
            uint32_t nLen = 21 - it->first.length();
            printf("|");
            for(uint32_t i=0; i<nLen/2; ++i)
            {
                printf(" ");
            }
            printf("%s", it->first.c_str());
            for(uint32_t i=0; i<nLen/2; ++i)
            {
                printf(" ");
            }
            printf("|\n");
            printf("+---------------------+\n");
        }
    }
    else if(strcmp(g_cmd_string[0].c_str(), "getUser") == 0)
    {
		g_pClient->getChangedUser();
	}
    else if(strcmp(g_cmd_string[0].c_str(), "getFriendGroup") == 0)
    {
		g_pClient->getFriendGroup();
	}
    else if(strcmp(g_cmd_string[0].c_str(), "sendMsg") == 0)
    {
    	time_t tStart = time(NULL);
		uint32_t nToId = string2int(g_cmd_string[1]);
		//string sMessage = g_cmd_string[2].c_str();

		string sMsgCnt = g_cmd_string[2].c_str();
		int nTotal = string2int( sMsgCnt);
		for(int i=0;i<nTotal;++i)
		{
			string sMessage = int2string((uint32_t)(i+1)) + "_+TestMessage_end.";
			g_pClient->sendMsg(nToId, IM::BaseDefine::MsgType::MSG_TYPE_ORDER_PUSH, sMessage);
		//	g_pClient->sendMsg(nToId, IM::BaseDefine::MsgType::MSG_TYPE_SINGLE_TEXT, sMessage);
		}
		time_t tEnd = time(NULL);
		printf("end - start = %d", tEnd-tStart);
		log("cnt:%d end - start = %d\n", nTotal, tEnd-tStart);
	}
	else if(strcmp(g_cmd_string[0].c_str(), "addFriend") == 0)
    {
    	uint32_t nToId = string2int(g_cmd_string[1]);
		uint32_t nGroupId = string2int(g_cmd_string[2]);
		string strRemark = g_cmd_string[3].c_str();

		g_pClient->addFriendReq( nToId, nGroupId, strRemark);

		printf("addFriendReq had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "delFriend") == 0)
    {
    	uint32_t nToId = string2int(g_cmd_string[1]);

		g_pClient->delFriendReq( nToId);

		printf("delFriendReq had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "chgFriend") == 0)
    {
    	uint32_t nToId = string2int(g_cmd_string[1]);
		string strRemark = g_cmd_string[2].c_str();

		g_pClient->chgFriendRemarkReq( nToId, strRemark);

		printf("chgFriendRemarkReq had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "getAddFriend") == 0)
	{

		g_pClient->getAddFriendReq();

		printf("getAddFriend had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "findFriend") == 0)
	{
		g_pClient->findFriendReq(g_cmd_string[1].c_str());

		printf("findFriend had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "getRecentSession") == 0)
	{
		g_pClient->getRecentSession(string2int(g_cmd_string[1].c_str()));

		printf("getRecentSession had sent\n");
	}	
	else if(strcmp(g_cmd_string[0].c_str(), "delRecentSession") == 0)
	{
		g_pClient->delRecentSession(string2int(g_cmd_string[1]), IM::BaseDefine::SessionType::SESSION_TYPE_SINGLE);

		printf("delRecentSession had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "createFriendGroup") == 0)
	{
		g_pClient->createFriendGroup(g_cmd_string[1]);

		printf("createGroup had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "delFriendGroup") == 0)
	{
		g_pClient->delFriendGroup(string2int(g_cmd_string[1]));

		printf("delFriendGroup had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "moveFriendToGroup") == 0)
	{
		list<uint32_t> friend_id_list;
	
		friend_id_list.push_back(string2int(g_cmd_string[2]));
	
		g_pClient->moveFriendToGroup(string2int(g_cmd_string[1]), friend_id_list);

		printf("moveFriendToGroup had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "chgFriendGroupName") == 0)
	{
		g_pClient->chgFriendGroupName(string2int(g_cmd_string[1]), g_cmd_string[2]);

		printf("chgFriendGroupName had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "FileRequest") == 0)
	{

		uint32_t to_user_id = 7; 
		string file_name = "hello world";	
		uint32_t file_size = 200;

		
		g_pClient->sendFileRequest(to_user_id, file_name, file_size);

		printf("FileRequest had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "fileReq") == 0)
	{
		g_pClient->fileReq( string2int(g_cmd_string[1]), g_cmd_string[2], string2int(g_cmd_string[3]));

		printf("fileReq had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "addOffFile") == 0)
	{
		g_pClient->addOffFile( string2int(g_cmd_string[1]), g_cmd_string[2], g_cmd_string[3]);

		printf("addOffFile had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "delOffFile") == 0)
	{
		g_pClient->delOffFile( string2int(g_cmd_string[1]), g_cmd_string[2]);

		printf("delOffFile had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "hasOffFile") == 0)
	{
		g_pClient->hasOffFile();

		printf("hasOffFile had sent\n");
	}
	else if(strcmp(g_cmd_string[0].c_str(), "cleanMsg") == 0)
	{
		g_pClient->cleanMsg(string2int(g_cmd_string[1].c_str()));

		printf("cleanMsg had sent\n");
	}
    else {
		print_help();
	}
}


class CmdThread : public CThread
{
public:
	void OnThreadRun()
	{
		while (true)
		{
			fprintf(stderr, "%s", PROMPT);	// print to error will not buffer the printed message

			if (fgets(m_buf, MAX_LINE_LEN - 1, stdin) == NULL)
			{
				fprintf(stderr, "fgets failed: %d\n", errno);
				continue;
			}

			m_buf[strlen(m_buf) - 1] = '\0';	// remove newline character

			split_cmd(m_buf);

			exec_cmd();
		}
	}
private:
	char	m_buf[MAX_LINE_LEN];
};

CmdThread g_cmd_thread;

int main(int argc, char* argv[])
{
//    play("message.wav");
	g_cmd_thread.StartThread();

	signal(SIGPIPE, SIG_IGN);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;
    
	netlib_eventloop();

	return 0;
}
