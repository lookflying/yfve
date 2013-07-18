/*
 * Connection.cpp
 *
 *  Created on: Jul 7, 2013
 *  Author: ushrimp
 */

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

#include "Connection.h"

#include "message/message.h"
#include "MessageTemplates.h"

#include "log4z.h"
#include "ScopeLock.h"
#include "YzHelper.h"

using namespace std;

const int Connection::DEFAULT_MESSAGE_RETRY_INTERVAL_SECONDS = 30;
const int Connection::DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS = 30;
const int Connection::DEFAULT_HEARTBEAT_ITNERVAL_SECONDS = 10;

const int ENCRYPTION = 0;

const int Connection::BUF_SIZE = 1024;
std::set<MSG_WORD> Connection::responseMsgIdSet;

class PackedMessage {
public:
	vector<msg_serialized_message_t> packets;

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

Connection::Connection(const std::string &name, struct ev_loop *loop,
		               int messageRetryIntervalSeconds/* = DEFAULT_MESSAGE_RETRY_INTERVAL_SECONDS*/,
		               int connectRetryIntervalSeconds/* = DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS*/,
		               int maxConnectRetry/* = 0*/,
		               int heartbeatInterval/* = DEFAULT_HEARTBEAT_ITNERVAL_SECONDS*/,
		               int heartbeatDeadCount/* = 3*/)
	: authorizationCode_(), name_(name), ip_(), port_(0), addressInited_(false), status_(sCLOSED), sockfd_(-1),
	  loop_(loop), //dataHandler_(NULL), closedHandler_(NULL),
	  closedHandler_(NULL), messageHandler_(NULL),
	  messageRetryIntervalSeconds_(messageRetryIntervalSeconds),
	  connectRetryIntervalSeconds_(connectRetryIntervalSeconds),
	  maxConnectRetry_(maxConnectRetry), packetStarted_(false), latestPacketRecievedTime_(0),
	  heartbeatIntervalSeconds_(heartbeatInterval), heartbeatDeadCount_(heartbeatDeadCount)
{
	bzero(&this->evwatcher_, sizeof(this->evwatcher_));
	bzero(&this->evtimer_, sizeof(this->evtimer_));
	this->evwatcher_.data = this;
	this->evtimer_.data = this;
	pthread_mutex_init(&this->mutex_, NULL);
	pthread_cond_init(&this->cond_, NULL);
	buffer_.reserve(BUF_SIZE);
}

Connection::~Connection()
{
	this->disconnect();
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

	if (this->status_ != sCLOSED) {
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
	if (this->status_ != sCLOSED) {
		return YZ_OK;
	}
	if (this->addressInited_ == false) {
		return YZ_SOCK_ERROR;
	}
	return this->do_connect(this->maxConnectRetry_);
}

/**
 * disconnect first and then reconnect
 * @param retry count before give up if nonzero, otherwise retry forever
 * return YZ_OK on success
 * return nonzero values if error occurred, cases are
 * YZ_CON_TIMEOUT
 * YZ_SOCK_ERROR rarely happens, maybe memory is insufficient
 */
int Connection::do_connect(int retry)
{
	this->do_disconnect();
	this->status_ = sCONNECTING;

	bool forever = (retry == 0) ? true : false;
	int retryTime = 1, error = 0, sockfd = -1;
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
		this->status_ = sCONNECTED;
		this->startEv();
	} else {
		this->sockfd_ = -1;
		this->status_ = sCLOSED;
	}
	return error;
}

/**
 * send authorize message to server and wait response
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
	return this->do_connectAndAuthorize();
}

int Connection::do_connectAndAuthorize()
{
	if (this->status_ == sCLOSED) {
		if (this->addressInited_ == false) {
			return YZ_SOCK_ERROR;
		}
		int ret = this->do_connect(this->maxConnectRetry_);
		if (ret != 0) {
			return ret;
		}
	}

	if (this->status_ == sCONNECTED) {
		this->status_ = sCONNECTED_AUTHORIZING;
		PackedMessage packedmsg;
		MSG_WORD msgSerial = 0;
		if (pack_msg(YZMSGID_TERMINAL_AUTHORIZE, authorizationCode_.c_str(), ENCRYPTION, authorizationCode_.length(), packedmsg.packets, msgSerial) == false) {
			this->disconnect();
			return YZ_OUT_OF_MEM;
		}

		bool retryForever = this->maxConnectRetry_ <= 0 ? true : false;
		int retryCount = 1;
		int interval = this->connectRetryIntervalSeconds_;
		while (retryForever || retryCount <= this->maxConnectRetry_) {
			msg_body_t *pbody;
			if (this->sendMessageOnceAndWait(interval, msgSerial, packedmsg.packets, &pbody) == true) {
				GeneralResponseMessage response(*pbody);
				delete[] pbody->content;
				delete pbody;
				if (response.result == GENERAL_RESPONSE_SUCCEED) {
					this->status_ = sCONNECTED_AUTHORIZED;
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
					this->status_ = sCLOSED;
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

	if (this->status_ == sCLOSED) {
		return YZ_CON_CLOSED;
	}

	char *buf = msg.toBytes();
	size_t len = msg.len();
	PackedMessage packedmsg;
	MSG_WORD msgSerial = 0;
	if (pack_msg(YZMSGID_TERMINAL_REGISTER, buf, ENCRYPTION, len, packedmsg.packets, msgSerial) == false) {
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
		if (this->sendMessageOnceAndWait(interval, msgSerial, packedmsg.packets, &pbody) == true) {
			TerminalRegisterResponseMessage response;
			response.parse(*pbody);
			delete pbody->content;
			delete pbody;
			// fixme
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
				this->status_ = sCLOSED;
				return ret;
			}
		}
	}
	return YZ_CON_TIMEOUT;

	// we never get here
	return 0;
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
	if (this->status_ != sCLOSED) {
		LOGI("disconnect from " << this->ip_ << ':' << this->port_);

		ev_io_stop(loop_, &this->evwatcher_);
		::close(this->sockfd_);
		this->sockfd_ = -1;
		this->status_ = sCLOSED;
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
	if (connection->status_ == sCLOSED) {
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
			connection->status_ = sCONNECTING;
			close(connection->sockfd_);
			break;
		} else if (errno == EAGAIN) {
			break;
		} else {
			LOGE("recv on socket " << connection->sockfd_  <<" failed: " << strerror(errno));
			connection->status_ = sCONNECTING;
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
			// fixme cast is not good
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
	LOGD("recieved message " << msgid);
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
int Connection::sendMessageAndWait(MSG_WORD msgid, const char *content, size_t len, msg_body_t **pbody)
{
	ScopeLock lock(&this->mutex_);
	if (this->status_ != sCONNECTED_AUTHORIZED) {
		return YZ_CON_CLOSED;
	}

	PackedMessage packedmsg;
	MSG_WORD msgSerial = 0;
	if (pack_msg(msgid, content, ENCRYPTION, len, packedmsg.packets, msgSerial) == false) {
		return YZ_OUT_OF_MEM;
	}

	bool retryForever = this->maxConnectRetry_ <= 0 ? true : false;
	int retryCount = 1;
	int interval = this->messageRetryIntervalSeconds_;
	while (retryForever || retryCount <= this->maxConnectRetry_) {
		if (this->sendMessageOnceAndWait(interval, msgSerial, packedmsg.packets, pbody) == true) {
			return YZ_OK;
		} else {
			++retryCount;
			interval = interval * retryCount;
			int ret = this->do_connectAndAuthorize();
			if (ret != 0) {
				return ret;
			}
		}

	}

	return YZ_CON_TIMEOUT;
}

void Connection::sendMessage(MSG_WORD msgid, const char *content, size_t len)
{
	PackedMessage packedmsg;
	MSG_WORD msgSerial = 0;
	if (pack_msg(msgid, content, ENCRYPTION, len, packedmsg.packets, msgSerial) == false) {
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

	// the lock have already been acquired
	//pthread_mutex_lock(&this->mutex_);

	map<MSG_WORD, msg_body_t*>::iterator iter = this->msgBuffer_.insert(pair<MSG_WORD, msg_body_t*>(msgSerial, NULL)).first;
	int error = 0;
	while (iter->second == NULL && this->status_ != sCLOSED && this->status_ != sCONNECTING && error != ETIMEDOUT) {
		error = pthread_cond_timedwait(&this->cond_, &this->mutex_, &outtime);
		iter = this->msgBuffer_.find(msgSerial);
	}
	msg_body_t *msg = iter->second;
	this->msgBuffer_.erase(iter);

	//pthread_mutex_unlock(&this->mutex_);
	return msg;
}



/**
 * callback when timer comes
 */
void Connection::timer_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
	Connection *connection = static_cast<Connection*>(w->data);
	ScopeLock lock(&connection->mutex_);
	if (connection->status_ != sCLOSED) {
		time_t now = time(NULL);
		if (now - connection->latestPacketRecievedTime_ > connection->heartbeatDeadCount_ * connection->heartbeatIntervalSeconds_) {
			connection->status_ = sCONNECTING;
			pthread_cond_broadcast(&connection->cond_);
		} else {
			// send heatbeat msg
			LOGI("sending heart beat");
			connection->sendMessage(YZMSGID_TERMINAL_HEARTBEAT, NULL, 0);
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
































