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

#include <ev.h>

#include "log4z.h"
using namespace zsummer::log4z;

#include "Connection.h"

void testHandler(const Connection *conn, const char *buf, int len)
{
	LOGI("recieved " << len << " bytes");
}

void closedHandler(const Connection *conn)
{
	LOGI("closed");
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <server address> <port>\n", argv[0]);
		return -1;
	}
	ILog4zManager::GetInstance()->Start();

	struct ev_loop *loop = ev_default_loop();
	Connection conn("test connection", loop);
	conn.initServerAddr(argv[1], atoi(argv[2]));
//	conn.setClosedHandler(closedHandler);
//	conn.setDataHandler(testHandler);
//	conn.connect();

	ev_loop(loop, 0);
	return 0;
}
