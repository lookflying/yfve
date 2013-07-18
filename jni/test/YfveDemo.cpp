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
#include "ConnectionManager.h"

bool messageHandler(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial, const msg_body_t &msg, MSG_WORD *responseMsgid, std::string *response)
{
	LOGD("handling message");
	return false;
}

int main(int argc, char **argv)
{
	ILog4zManager::GetInstance()->Start();

	MSG_WORD platformResponseMsgIds[] = {YZMSGID_GENERAL_PLATFORM_RESPONSE, YZMSGID_TERMINAL_REGISTER_RESPONSE};
	Connection::responseMsgIdSet.insert(platformResponseMsgIds, platformResponseMsgIds + sizeof(platformResponseMsgIds) / sizeof(platformResponseMsgIds[0]));

	ConnectionManager manager;
	manager.start();

	Connection &conn = *manager.getConnection(0);
	conn.initServerAddr("121.101.223.68", 6973);
	conn.setMessageHandler(messageHandler);

	conn.connectAndAuthorize();
	TerminalRegisterMessage msg;
	conn.registerTerminal(msg);

	while (true) {
		sleep(10);
	}

	exit(0);

}
