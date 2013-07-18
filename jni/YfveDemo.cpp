//============================================================================
// Name        : YfveDemo.cpp
// Author      : haow1990
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <iostream>

#include <ev.h>

#include "log4z.h"
#include "message/message.h"
#include "YzHelper.h"

using namespace zsummer::log4z;

#include "Connection.h"

void testHandler(const Connection *conn, const char *buf, int len)
{
	LOGI("recieved " << len << " bytes");
}

void closedHandler(const Connection &conn)
{
	LOGI("connection " << conn.name() << " closed");
}

void *worker(void *param)
{
	struct ev_loop *loop = static_cast<struct ev_loop*>(param);
	std::cerr << "work id:" << pthread_self() << '\n';
	ev_loop(loop, 0);
	return NULL;
}

void idle_cb(struct ev_loop *loop, struct ev_idle *w, int revents)
{
	LOGI("idle");
	sleep(1);
}

int main(int argc, char **argv)
{
//	if (argc != 3) {
//		fprintf(stderr, "Usage: %s <server address> <port>\n", argv[0]);
//		return -1;
//	}
	ILog4zManager::GetInstance()->Start();
	std::cerr << "main id:" << pthread_self() << '\n';

	set_global_phone_num((MSG_BYTE*)"12345678901", 11);

	struct ev_loop *loop = ev_default_loop(0);
	struct ev_idle *idle_watcher = (ev_idle*)(malloc(sizeof(struct ev_idle)));
	ev_idle_init(idle_watcher, idle_cb);
	ev_idle_start(loop, idle_watcher);

	MSG_WORD platformResponseMsgIds[] = {YZMSGID_GENERAL_PLATFORM_RESPONSE, YZMSGID_TERMINAL_REGISTER_RESPONSE};
	Connection::responseMsgIdSet.insert(platformResponseMsgIds, platformResponseMsgIds + sizeof(platformResponseMsgIds) / sizeof(platformResponseMsgIds[0]));

	Connection conn("test connection", loop);
	conn.initServerAddr("121.101.223.68", 6973);
	conn.setAuthorizationCode("12345");
	conn.setClosedHandler(closedHandler);

	pthread_t tid;
	pthread_create(&tid, NULL, worker, loop);
	sleep(1);

	TerminalRegisterMessage msg;
	conn.connect();
	conn.registerTerminal(msg);
	conn.connectAndAuthorize();
	conn.registerTerminal(msg);

	while (true) {
		sleep(10);
	}

	exit(0);
}
