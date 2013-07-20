/*
 * Connection.h
 *
 *  Created on: Jul 7, 2013
 *      Author: ushrimp
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <string>
#include <netinet/in.h>
#include <map>
#include <set>

#define EV_STANDALONE 1
#include "ev.h"
#include "message/message.h"
#include "MessageTemplates.h"

class PackedMessage;

class Connection {
public:
	/**
	 * connection status
	 */
	enum Status {
		CLOSED,
		CONNECTING,
		CONNECTED,
		CONNECTED_AUTHORIZING,
		CONNECTED_AUTHORIZED
	};

	typedef void (*closedHandler_t)(const Connection &conn);
	typedef bool (*messageHandler_t)(const Connection &conn, MSG_WORD msgid, MSG_WORD msgSerial, const msg_body_t &msg, MSG_WORD *responseMsgid, std::string *response);

	Connection();
	~Connection();

	/**
	 * initialize destination ip and port
	 * return YZ_OK(0) if succeeded
	 * otherwise return nonzero, cases are
	 * 1 status of the connection is CONNECTED, do nothing
	 * 2 ip address is not corrected, original ip:port information is destroyed
	 */
	int initServerAddr(const std::string &ip, int port);

	/**
	 * connect to server
	 * return YZ_OK on success
	 * return YZ_CON_TIMEOUT if fail to connect
	 * return YZ_SOCK_ERROR, unlikely, such as memory insufficient
	 */
	int connect();

	/**
	 * send authorize message to server and wait response
	 * return YZ_OK on success
	 * return YZ_DUP_LOGIN if already logged in
	 * return YZ_LOGIN_FAIL if failed to login
	 * return YZ_CON_TIMEOUT if fail to connect
	 * return YZ_SOCK_ERROR, unlikely, such as memory insufficient
	 */
	int connectAndAuthorize();

	/**
	 * register terminal
	 */
	int registerTerminal(const TerminalRegisterMessage &msg);

	/**
	 * disconnect from server anyway
	 * return 0 if successfully disconnected
	 * return YZ_CON_CLOSED if not connected
	 */
	int disconnect();

	/**
	 * set authorization code of this connection
	 */
	void setAuthorizationCode(const std::string &value) { this->authorizationCode_ = value; }

	const std::string authorizationCode() { return this->authorizationCode_; }

	/**
	 * send a message and wait its response
	 * return YZ_OK if succeeded
	 * return nonzero values if error occurred, cases are
	 * YZ_CON_TIMEOUT
	 * YZ_SOCK_ERROR rarely happens, maybe memory is insufficient
	 *
	 */
	int sendMessageAndWait(MSG_WORD msgid, const char *content, size_t len, msg_body_t **pbody, bool critical = false);

	/**
	 * name of the connection
	 */
	const std::string &name()const { return this->name_; }

	void set_name(const std::string &value) { this->name_ = value; }

	void set_loop(struct ev_loop *value) { this->loop_ = value; }

	void set_messageRetryIntervalSeconds(int value) { this->messageRetryIntervalSeconds_ = value; }

	void set_connectRetryIntervalSeconds(int value) { this->connectRetryIntervalSeconds_ = value; }

	void set_maxConnectRetry(int value) { this->maxConnectRetry_ = value; }

	void set_heartbeatIntervalSeconds(int value) { this->heartbeatIntervalSeconds_ = value; }

	void set_heartbeanDeadCount(int value) { this->heartbeatDeadCount_ = value; }

	/**
	 * destination ip of the connection
	 */
	const std::string &ip()const { return this->ip_; }

	/**
	 * destination port of the connection
	 */
	int port()const { return port_; }

	/**
	 * status of current connection
	 */
	Status status()const { return this->status_; }

	/**
	 * server address inited or not
	 */
	bool addressInited() const { return addressInited_; }

	/**
	 * call back when connection is closed and not reconnectable within RETRY times
	 */
	closedHandler_t closedHandler()const { return closedHandler_; }
	void setClosedHandler(closedHandler_t handler) { closedHandler_ = handler; }

	/**
	 * call back new message arrived, return true if message handled, otherwise return false
	 */
	messageHandler_t messageHandler()const { return messageHandler_; }
	void setMessageHandler(messageHandler_t handler) { messageHandler_ = handler; }

	static std::set<MSG_WORD> responseMsgIdSet;
	static const int DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS;
private:
	/**
	 * callback when socket reabab#	modified:   Connection.cpp
	 * le, disconnected
	 */
	static void sock_cb(struct ev_loop *loop, struct ev_io *w, int revents);

	/**
	 * callback when timer comes
	 */
	static void timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents);

	int do_sendMessageAndWait(const PackedMessage &msg, msg_body_t **pbody);

	/**
	 * when disconnected, reconnect
	 * retry count before give up if nonzero, otherwise retry forever
	 * return 0 on success
	 * return -1 otherwise
	 */
	int do_connect(int retry);

	/**
	 * do the disconnect stuff, without locking
	 */
	int do_disconnect();

	/**
	 * send authorize message to server and wait response
	 * return YZ_OK on success
	 * return YZ_DUP_LOGIN if already logged in
	 * return YZ_LOGIN_FAIL if failed to login
	 * return YZ_CON_TIMEOUT if fail to connect
	 * return YZ_SOCK_ERROR, unlikely, such as memory insufficient
	 */
	int do_connectAndAuthorize();

	/**
	 * wait message, return pointer on success
	 * or NULL if connection closed
	 */
	msg_body *waitMessage(MSG_WORD msgSerial, int timeoutseconds);

	/**
	 * simply sent message
	 */
	void sendMessage(MSG_WORD msgid, const char *content, size_t len);

	/**
	 * send message and wait its response
	 */
	bool sendMessageOnceAndWait(int timeoutseconds, MSG_WORD msgSerial, const std::vector<msg_serialized_message_t> &packets, msg_body_t **pmsg);

	/**
	 * call when data arrived
	 */
	void dataHandler(const char *data, size_t len);

	/**
	 * call when a message arrived, called in datahandler
	 */
	void messageHandler(MSG_WORD msgid, MSG_WORD msgSerail, msg_body *body);

	static void *reconnectWorker(void *);

	/**
	 * start ev watcher
	 */
	void startEv();

	/**
	 * stop ev watcher
	 */
	void stopEv();

	std::string authorizationCode_;
	std::string name_;
	std::string ip_;
	int port_;
	struct sockaddr_in servaddr_;
	bool addressInited_;
	Status status_;
	bool needReauthorization_;
	int sockfd_;
	struct ev_io evwatcher_;
	struct ev_timer evtimer_;
	struct ev_loop * loop_;
	closedHandler_t closedHandler_;
	messageHandler_t messageHandler_;
	int messageRetryIntervalSeconds_;
	int connectRetryIntervalSeconds_;
	int maxConnectRetry_;
	pthread_mutex_t mutex_;
	pthread_cond_t cond_;
	std::map<MSG_WORD, msg_body*> msgBuffer_;
	std::string buffer_;
	bool packetStarted_;
	int latestPacketRecievedTime_;
	int heartbeatIntervalSeconds_;
	int heartbeatDeadCount_;
	std::vector<PackedMessage*> pendingMsgs_;
	pthread_t reconnectTid_;

	static const int DEFAULT_MESSAGE_RETRY_INTERVAL_SECONDS;
	static const int DEFAULT_HEARTBEAT_ITNERVAL_SECONDS;

	static const int BUF_SIZE;
};

#endif /* CONNECTION_H_ */

