     //
//  main.cpp
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include <iostream>
#include "push_app.h"
#include "timer/Timer.hpp"
#include <sys/signal.h>
#include "version.h"
#include "util.h"

/*
void writePid()
{
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t) GetCurrentProcess();
#else
    curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}
*/

int main(int argc, const char * argv[]) {
    // insert code here...
    if ((argc == 2) && (strcmp(argv[1], "-v") == 0))
	{
		printf("Client Version: PushServer/%s\n", VERSION);
		printf("Client Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}
	
    printf("start push server...\n");
    signal(SIGPIPE, SIG_IGN);
    CPushApp::GetInstance()->Init();
    CPushApp::GetInstance()->Start();
    writePid();
    while (true) {
        S_Sleep(1000);
    }
    return 0;
}
