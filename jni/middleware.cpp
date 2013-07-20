/*
 * ConnectionHelper.cpp
 *
 *  Created on: Jul 20, 2013
 *      Author: lookflying
 */

#include "middleware.h"

using namespace std;

ConnectionManager g_conn_manager;

bool messageHandler(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial,
		const msg_body_t &msg, MSG_WORD *responseMsgid, string *response) {
	logcatf("got message id = %u size = %u", msgid, msg.length);
	return false;
}

void connClosedHandler(const Connection &conn) {

}

void initMiddleware(const std::string server_ip, const int server_port) {
	MSG_WORD platformResponseMsgIds[] = { YZMSGID_GENERAL_PLATFORM_RESPONSE,
			YZMSGID_TERMINAL_REGISTER_RESPONSE,

			};
	Connection::responseMsgIdSet.insert(platformResponseMsgIds,
			platformResponseMsgIds
					+ sizeof(platformResponseMsgIds)
							/ sizeof(platformResponseMsgIds[0]));

	(void) g_conn_manager.start();

	Connection &g_connection = *g_conn_manager.getConnection(0);
	g_connection.initServerAddr(server_ip, server_port);
	g_connection.setMessageHandler(messageHandler);
	g_connection.setClosedHandler(connClosedHandler);

}
