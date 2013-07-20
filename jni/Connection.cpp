/*
 * Connection.cpp
 *
 *  Created on: Jul 7, 2013
 *  Author: ushrimp
 */

#define EV_STANDALONE 1
#include "ev.c"
#include "Connection.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <vector>
#include <utility>

#include "message/message.h"
#include "MessageTemplates.h"

#include "log4z.h"
#include "ScopeLock.h"
#include "YzHelper.h"


using namespace std;

const int Connection::DEFAULT_MESSAGE_RETRY_INTERVAL_SECONDS = 30;
const int Connection::DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS = 30;
const int Connection::DEFAULT_HEARTBEAT_ITNERVAL_SECONDS = 30;

const int ENCRYPTION = 0;

const int Connection::BUF_SIZE = 1024;
std::set<MSG_WORD> Connection::responseMsgIdSet;

class PackedMessage {
public:
	vector<msg_serialized_message_t> packets;
	MSG_WORD msgid;
	MSG_WORD serial;

	PackedMessage(MSG_WORD id, MSG_WORD ser)
		: msgid(id), serial(ser)
	{ }

	~PackedMessage()
	{
		for (vector<msg_serialized_message_t>::iterator iter = packets.begin();
				iter != packets.end(); ++iter) {
			clear_serialized_msg(*iter);
		}
	}
};

int sendAll(int sockfd, const void *buf, size_t len, int opts)
{
	size_t idx = 0;
	while (idx < len) {
		int ret = ::send(sockfd, buf, len - idx, opts);
		if (ret == -1) {
			return -1;
		}
		idx += ret;
	}
	return 0;
}

Connection::Connection()
	: authorizationCode_(), name_("unknown"), ip_(), port_(0), addressInited_(false), status_(CLOSED),needReauthorization_(false), sockfd_(-1),
	  loop_(NULL), //dataHandler_(NULL), closedHandler_(NULL),
	  closedHandler_(NULL), messageHandler_(NULL),
	  messageRetryIntervalSeconds_(DEFAULT_MESSAGE_RETRY_INTERVAL_SECONDS),
	  connectRetryIntervalSeconds_(DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS),
	  maxConnectRetry_(3), packetStarted_(false), latestPacketRecievedTime_(0),
	  heartbeatIntervalSeconds_(DEFAULT_HEARTBEAT_ITNERVAL_SECONDS), heartbeatDeadCount_(3), reconnectTid_(0)
{
	bzero(&this->evwatcher_, sizeof(this->evwatcher_));
	bzero(&this->evtimer_, sizeof(this->evtimer_));
	this->evwatcher_.data = this;
	this->evtimer_.data = this;
	pthread_mutex_init(&this->mutex_, NULL);
	pthread_cond_init(&this->cond_, NULL);
	buffer_.reserve(BUF_SIZE);

	pthread_create(&this->reconnectTid_, NULL, reconnectWorker, this);
}

Connection::~Connection()
{
	this->disconnect();
	// ndk doesn't support pthread_cancel
//	pthread_cancel(this->reconnectTid_);
	pthread_kill(this->reconnectTid_, SIGUSR1);
	pthread_mutex_destroy(&this->mutex_);
	pthread_cond_destroy(&this->cond_);
}

/**
 * initialize destination ip and port
 * return YZ_OK(0) if succeeded
 * otherwise return nonzero, cases are
 * 1 status of the connection is CONNECTED, do nothing
 * 2 ip address is not corrected, original ip:port information is destroyed
 */
int Connection::initServerAddr(const std::string &ip, int port)
{
	ScopeLock lock(&this->mutex_);

	if (this->status_ != CLOSED) {
		return 1;
	}
	LOGD("initialize connection " << this->name_ << " with " << ip << ':' << port);
	this->ip_ = ip;
	this->port_ = port;

	bzero(&this->servaddr_, sizeof(this->servaddr_));
	this->servaddr_.sin_family = AF_INET;
	this->servaddr_.sin_port = htons(this->port_);
	if (inet_pton(AF_INET, this->ip_.c_str(), &this->servaddr_.sin_addr) != 1) {
		LOGE("IP address " << this->ip_ << " does not contain a character string representing a valid network address or address family not supported");
		this->addressInited_ = false;
		return 2;
	} else {
		this->addressInited_ = true;
		return 0;
	}
}

/**
 * connect to server, no authorization
 * return YZ_OK on success
 * return YZ_CON_TIMEOUT if fail to connect
 * return YZ_SOCK_ERROR, unlikely, such as memory insufficient
 */
int Connection::connect()
{
	ScopeLock lock(&this->mutex_);
	if (this->addressInited_ == false) {
		return YZ_SOCK_ERROR;
	}
	if (this->status_ > CONNECTING) {
		return YZ_OK;
	}
	return this->do_connect(this->maxConnectRetry_);
}

/**
 * disconnect first and then connect, mutex_ should be hold
 * @param retry count before give up if nonzero, otherwise retry forever
 * return YZ_OK on success
 * return nonzero values if error occurred, cases are
 * YZ_CON_TIMEOUT
 * YZ_SOCK_ERROR rarely happens, maybe memory is insufficient
 */
int Connection::do_connect(int retry)
{
	this->do_disconnect();
	this->status_ = CONNECTING;

	bool forever = (retry == 0) ? true : false;
	int retryTime = 1, error = YZ_OK, sockfd = -1;
	while (forever || retryTime <= retry) {
		LOGD("try connecting to " << this->ip_ << ':' << this->port_ << " for the " << retryTime << "th time");
		++retryTime;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			error = errno;
			LOGE("cannot create socket " << strerror(error));
			error = YZ_SOCK_ERROR;
			break;
		}

		if (::connect(sockfd, (sockaddr*)&this->servaddr_, sizeof(this->servaddr_)) == -1) {
			error = errno;
			LOGE("cannot connect to " << this->ip_ << ':' << this->port_ << ": " << strerror(error));
			close(sockfd);
			if (error != ETIMEDOUT) {
				// should give up, because it makes no sense
				error = YZ_SOCK_ERROR;
				break;
			} else {
				error = YZ_CON_TIMEOUT;
			}

			sleep(this->connectRetryIntervalSeconds_);
			continue;
		}
		error = YZ_OK;
		break;
	}

	if (error == YZ_OK) {
		this->sockfd_ = sockfd;
		this->status_ = CONNECTED;
		this->startEv();
	} else {
		this->sockfd_ = -1;
		this->status_ = CLOSED;
	}
	return error;
}

/**
 * send authorize message to server and wait response, send pending messages
 * return YZ_OK on success
 * return YZ_DUP_LOGIN if already logged in
 * return YZ_LOGIN_FAIL if failed to login
 * return YZ_CON_TIMEOUT if fail to connect
 * YZ_OUT_OF_MEM
 * return YZ_SOCK_ERROR, unlikely, such as memory insufficient
 */
int Connection::connectAndAuthorize()
{
	ScopeLock lock(&this->mutex_);
	int ret = this->do_connectAndAuthorize();
	this->needReauthorization_ = ret == 0 || ret == YZ_DUP_LOGIN;
	if (ret != 0) {
		return ret;
	}

	//
	while (this->pendingMsgs_.empty() == false) {
		msg_body_t *body;
		ret = this->do_sendMessageAndWait(**this->pendingMsgs_.rbegin(), &body);
		if (ret != 0) {
			return ret;
		} else {
			delete body->content;
			delete body;
			delete *(this->pendingMsgs_.rbegin());
			this->pendingMsgs_.erase(this->pendingMsgs_.end() - 1);
		}
	}

	return 0;
}



/**
 * deregisterTerminal
 */
int Connection::deregisterTerminal()
{
	ScopeLock lock(&this->mutex_);
	if (this->status_!= CONNECTED_AUTHORIZED) {
		return YZ_NOT_LOGIN;
	}
	msg_body_t *body;
	int ret = this->sendMessageAndWait(YZMSGID_TERMINAL_LOGOUT, NULL, 0, &body, false);
	if (ret == YZ_OK) {
		delete body->content;
		delete body;
	}
	return ret;
}

int Connection::do_connectAndAuthorize()
{
	if (this->status_ < CONNECTED) {
		if (this->addressInited_ == false) {
			return YZ_SOCK_ERROR;
		}
		int ret = this->do_connect(this->maxConnectRetry_);
		if (ret != 0) {
			return ret;
		}
	}

	if (this->status_ == CONNECTED) {
		LOGD("start authorizing");
		this->status_ = CONNECTED_AUTHORIZING;
		PackedMessage packedmsg(YZMSGID_TERMINAL_AUTHORIZE, 0);
		if (pack_msg(packedmsg.msgid, authorizationCode_.c_str(), ENCRYPTION, authorizationCode_.length(), packedmsg.packets, packedmsg.serial) == false) {
			this->disconnect();
			return YZ_OUT_OF_MEM;
		}

		bool retryForever = this->maxConnectRetry_ <= 0 ? true : false;
		int retryCount = 1;
		int interval = this->connectRetryIntervalSeconds_;
		while (retryForever || retryCount <= this->maxConnectRetry_) {
			msg_body_t *pbody;
			if (this->sendMessageOnceAndWait(interval, packedmsg.serial, packedmsg.packets, &pbody) == true) {
				GeneralResponseMessage response(*pbody);
				delete[] pbody->content;
				delete pbody;
				if (response.result == GENERAL_RESPONSE_SUCCEED) {
					this->status_ = CONNECTED_AUTHORIZED;
					return YZ_OK;
				} else {
					this->disconnect();
					return YZ_LOGIN_FAIL;
				}
			} else {
				++retryCount;
				interval = interval * retryCount;
				int ret = this->do_connect(1);
				if (ret != 0) {
					this->status_ = CLOSED;
					return ret;
				}
			}
		}
		return YZ_CON_TIMEOUT;
	} else {
		return YZ_DUP_LOGIN;
	}

	// we never get here
	return 0;
}

/**
 * register terminal
 */
int Connection::registerTerminal(const TerminalRegisterMessage &msg)
{
	ScopeLock lock(&this->mutex_);

	if (this->status_ == CLOSED) {
		return YZ_CON_CLOSED;
	}

	char *buf = msg.toBytes();
	size_t len = msg.len();
	PackedMessage packedmsg(YZMSGID_TERMINAL_REGISTER, 0);
	if (pack_msg(YZMSGID_TERMINAL_REGISTER, buf, ENCRYPTION, len, packedmsg.packets, packedmsg.serial) == false) {
		this->disconnect();
		delete buf;
		return YZ_OUT_OF_MEM;
	}
	delete buf;

	bool retryForever = this->maxConnectRetry_ <= 0 ? true : false;
	int retryCount = 1;
	int interval = this->connectRetryIntervalSeconds_;
	while (retryForever || retryCount <= this->maxConnectRetry_) {
		msg_body_t *pbody;
		if (this->sendMessageOnceAndWait(interval, packedmsg.serial, packedmsg.packets, &pbody) == true) {
			TerminalRegisterResponseMessage response;
			response.parse(*pbody);
			delete pbody->content;
			delete pbody;
			// fixme result definition is not clear enough
			switch (response.result)
			{
			case 0:
				this->authorizationCode_ = response.authorizationCode;
				return YZ_OK;
			case 1:
				return YZ_DEVICE_NOT_REGIST;
			case 2:
				return YZ_DEVICE_NOT_REGIST;
			case 3:
				return YZ_DEVICE_NOT_REGIST;
			default:
				return YZ_UNEXPECTED;
			}
		} else {
			++retryCount;
			interval = interval * retryCount;
			int ret = this->do_connect(1);
			if (ret != 0) {
				this->status_ = CLOSED;
				return ret;
			}
		}
	}

	return YZ_CON_TIMEOUT;
}

/**
 * disconnect from server anyway
 * return 0 if successfully disconnected
 * return YZ_CON_CLOSED if not connected
 */
int Connection::disconnect()
{
	ScopeLock lock(&this->mutex_);
	return do_disconnect();
}

/**
 * return 0 if successfully disconnected
 * return YZ_CON_CLOSED if not connected
 */
int Connection::do_disconnect()
{
	// disconnect if connected
	if (this->status_ != CLOSED) {
		LOGI("disconnect from " << this->ip_ << ':' << this->port_);

		ev_io_stop(loop_, &this->evwatcher_);
		::close(this->sockfd_);
		this->sockfd_ = -1;
		this->status_ = CLOSED;
		return 0;
	} else {
		return -1;
	}
}

/**
 * callback when socket reabable, disconnected
 */
void Connection::sock_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	Connection *connection = static_cast<Connection*>(w->data);
	ScopeLock lock(&connection->mutex_);
	if (connection->status_ == CLOSED) {
		return;
	}

	do {
		char buf[BUF_SIZE];
		ssize_t ret = recv(connection->sockfd_, buf, sizeof(buf), MSG_DONTWAIT);
		if (ret > 0) {
			LOGD("socket " << connection->sockfd_ << " recieved " << ret << " bytes");
			connection->dataHandler(buf, ret);
		} else if (ret == 0) {
			LOGD("socket " << connection->sockfd_ << " closed by peer");
			connection->status_ = CONNECTING;
			close(connection->sockfd_);
			break;
		} else if (errno == EAGAIN) {
			break;
		} else {
			LOGE("recv on socket " << connection->sockfd_  <<" failed: " << strerror(errno));
			connection->status_ = CONNECTING;
			close(connection->sockfd_);
			break;
		}
	} while (1);
	pthread_cond_broadcast(&connection->cond_);
}

/**
 * call when data arrived
 */
void Connection::dataHandler(const char *data, size_t len)
{
	size_t startIdx = 0, endIdx;
	// this piece of data may contain several packets
	while (startIdx < len) {
		if (this->packetStarted_ == false) {
			// the packet haven't started yet, so try to find the start
			while (startIdx < len && data[startIdx] != MSG_FLAG) {
				++startIdx;
			}
			// this piece of data doesn't contain a start byte, so ignore it
			if (startIdx >= len) {
				break;
			}
			this->packetStarted_ = true;
		}
		// trying to find the end
		endIdx = startIdx + 1;
		while (endIdx < len && data[endIdx] != MSG_FLAG) {
			++endIdx;
		}
		if (endIdx == len) {
			// this piece of data doesn't contain end byte
			this->buffer_.append(data + startIdx, data + len);
			break;
		} else {
			// found the end byte
			this->buffer_.append(data + startIdx, data + endIdx + 1);
			startIdx = endIdx + 1;
			this->packetStarted_ = false;

			// trying to parse the message
			msg_serialized_message_t serialized;
			serialized.data = (MSG_BYTE*)(this->buffer_.c_str());
			serialized.length = this->buffer_.size();
			msg_message_t msg;
			if (deserialize(serialized, msg) == true) {
				MSG_WORD msgSerial;
				MSG_WORD msgid;
				char *data = NULL;
				unsigned int len;
				if (unpack_msg(msg, msgid, msgSerial, &data, len) == true) {
					msg_body_t *body = new msg_body_t();
					body->content = (MSG_BYTE*)data;
					body->length = len;
					messageHandler(msgid, msgSerial, body);
				}
			} else {
				LOGE("fail when seserialize message");
			}
			this->buffer_.clear();
		}
	}
}

/**
 * call when a message arrived, called in datahandler
 */
void Connection::messageHandler(MSG_WORD msgid, MSG_WORD msgSerial, msg_body *body)
{
	LOGI("recieved message " << msgid);
	this->latestPacketRecievedTime_ = time(NULL);
	ev_timer_again(this->loop_, &this->evtimer_);

	// if this is a response message
	if (responseMsgIdSet.count(msgid) != 0) {
		MSG_WORD targetSerial;
		MSG_SET_WORD(targetSerial, body->content[0], body->content[1]);
		map<MSG_WORD, msg_body_t*>::iterator iter = this->msgBuffer_.find(msgSerial);
		if (iter != this->msgBuffer_.end()) {
			iter->second = body;
			pthread_cond_broadcast(&this->cond_);
			return;
		} else {
			// may recieved a heartbeat response packet
		}
	} else {
		string response;
		MSG_WORD responseMsgid;
		if (this->messageHandler_ != NULL &&
				this->messageHandler_(*this, msgid, msgSerial, *body, &responseMsgid, &response) == true) {
			this->sendMessage(responseMsgid, response.c_str(), response.length());
		}
	}
	delete[] body->content;
	delete body;
}

bool Connection::sendMessageOnceAndWait(int timeoutseconds, MSG_WORD msgSerial, const std::vector<msg_serialized_message_t> &packets, msg_body_t **pmsg)
{
	for (vector<msg_serialized_message_t>::const_iterator iter = packets.begin();
			iter != packets.end(); ++iter) {
		if (::sendAll(this->sockfd_, iter->data, iter->length, 0) != 0) {
			int error = errno;
			LOGE("send failed " << strerror(errno));
			return false;
		}
	}
	*pmsg = this->waitMessage(msgSerial, timeoutseconds);
	if (*pmsg == NULL) {
		return false;
	}
	return true;
}

/**
 * send a message and wait its response
 * return YZ_OK if succeeded
 * return nonzero values if error occurred, cases are
 * YZ_CON_TIMEOUT
 * YZ_SOCK_ERROR rarely happens, maybe memory is insufficient
 *
 */
int Connection::sendMessageAndWait(MSG_WORD msgid, const char *content, size_t len, msg_body_t **pbody, bool critical/* = false*/)
{

	PackedMessage *packedmsg = new PackedMessage(msgid, 0);
	if (pack_msg(msgid, content, ENCRYPTION, len, packedmsg->packets, packedmsg->serial) == false) {
		delete packedmsg;
		return YZ_OUT_OF_MEM;
	}
	ScopeLock lock(&this->mutex_);
	if (this->status_ != CONNECTED_AUTHORIZED) {
		if (critical == true) {
			this->pendingMsgs_.push_back(packedmsg);
		}
		delete packedmsg;
		return YZ_CON_CLOSED;
	}

	int ret = this->do_sendMessageAndWait(*packedmsg, pbody);
	if (ret != 0 && critical == true) {
		this->pendingMsgs_.push_back(packedmsg);
	} else {
		delete packedmsg;
	}
	return ret;
}

int Connection::do_sendMessageAndWait(const PackedMessage &msg, msg_body_t **pbody)
{
	bool retryForever = this->maxConnectRetry_ <= 0 ? true : false;
	int retryCount = 1;
	int interval = this->messageRetryIntervalSeconds_;
	while (retryForever || retryCount <= this->maxConnectRetry_) {
		if (this->sendMessageOnceAndWait(interval, msg.serial, msg.packets, pbody) == true) {
			return YZ_OK;
		} else {
			++retryCount;
			interval = interval * retryCount;
			int ret = this->do_connectAndAuthorize();
			this->needReauthorization_ = ret == 0 || ret == YZ_DUP_LOGIN;
			if (ret != 0) {
				return ret;
			}
		}

	}

	return YZ_CON_TIMEOUT;
}

void Connection::sendMessage(MSG_WORD msgid, const char *content, size_t len)
{
	PackedMessage packedmsg(msgid, 0);
	if (pack_msg(msgid, content, ENCRYPTION, len, packedmsg.packets, packedmsg.serial) == false) {
		return;
	}
	for (vector<msg_serialized_message_t>::const_iterator iter = packedmsg.packets.begin();
			iter != packedmsg.packets.end(); ++iter) {
		if (::sendAll(this->sockfd_, iter->data, iter->length, 0) != 0) {
			break;
		}
	}

}

/**
 * wait message, return pointer on success, or NULL
 */
msg_body_t *Connection::waitMessage(MSG_WORD msgSerial, int timeoutseconds)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timespec outtime;
	outtime.tv_sec = now.tv_sec;
	outtime.tv_nsec = (now.tv_usec + timeoutseconds * 1000 * 1000) * 1000;
	outtime.tv_sec += outtime.tv_nsec / (1000 * 1000 * 1000);
	outtime.tv_nsec = outtime.tv_nsec % 1000 * 1000 * 1000;

	map<MSG_WORD, msg_body_t*>::iterator iter = this->msgBuffer_.insert(pair<MSG_WORD, msg_body_t*>(msgSerial, NULL)).first;
	int error = 0;
	while (iter->second == NULL && this->status_ != CLOSED && this->status_ != CONNECTING && error != ETIMEDOUT) {
		error = pthread_cond_timedwait(&this->cond_, &this->mutex_, &outtime);
		iter = this->msgBuffer_.find(msgSerial);
	}
	msg_body_t *msg = iter->second;
	this->msgBuffer_.erase(iter);

	return msg;
}



/**
 * callback when timer comes
 */
void Connection::timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Connection *connection = static_cast<Connection*>(w->data);
	ScopeLock lock(&connection->mutex_);
	if (connection->status_ != CLOSED) {
		time_t now = time(NULL);
		if (now - connection->latestPacketRecievedTime_ > connection->heartbeatDeadCount_ * connection->heartbeatIntervalSeconds_) {
			connection->status_ = CONNECTING;
			pthread_cond_broadcast(&connection->cond_);
		} else {
			if (connection->status_ > CONNECTING) {
				// send heatbeat msg
				LOGI("sending heart beat");
				connection->sendMessage(YZMSGID_TERMINAL_HEARTBEAT, NULL, 0);
			}
		}
	}
}

/**
 * start ev watcher
 */
void Connection::startEv()
{
	stopEv();
	ev_io_init(&this->evwatcher_, sock_cb, this->sockfd_, EV_READ);
	ev_timer_init(&this->evtimer_, timer_cb, this->heartbeatIntervalSeconds_, this->heartbeatIntervalSeconds_);
	ev_io_start(loop_, &this->evwatcher_);
	ev_timer_start(loop_, &this->evtimer_);
}

/**
 * stop ev watcher
 */
void Connection::stopEv()
{
	ev_io_stop(this->loop_, &this->evwatcher_);
	ev_timer_stop(this->loop_, &this->evtimer_);
}

void thread_exit_handler(int sig)
{
	pthread_exit(0);
}

void *Connection::reconnectWorker(void *param)
{
	struct sigaction actions;
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;
	sigaction(SIGUSR1,&actions,NULL);


	Connection *conn = static_cast<Connection*>(param);
	ScopeLock lock(&conn->mutex_);

	while (true) {
		LOGD("reconnect worker, while " << conn->status_ << ' ' << conn->needReauthorization_);
		if (conn->status_ == CONNECTING) {
			if (conn->needReauthorization_) {
				LOGD("reconnecting...");

				int ret = conn->do_connectAndAuthorize();
				if (conn->status_ < CONNECTED && conn->closedHandler_ != NULL) {
					conn->closedHandler_(*conn);
				}
				LOGD("reconnect " << ret << " " << conn->status_);
				conn->needReauthorization_ = ret == 0 || ret == YZ_DUP_LOGIN;
			} else {
				// fixme need reconnect or not
				// did not reconnect here, because most of the messages will
				// be sent when authoeized; in another way, only registerTerminal
				// method call makes sense when connected and unauthorized
			}
		}

		pthread_cond_wait(&conn->cond_, &conn->mutex_);
	}
	return NULL;
}





























