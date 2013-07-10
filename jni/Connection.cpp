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
#include <string.h>

#include "Connection.h"

#include "log4z.h"
#include "ScopeLock.h"

using namespace std;



const int Connection::KEEP_ALIVE = 1;
const int Connection::KEEP_IDLE = 30;
const int Connection::KEEP_INTERVAL = 30;
const int Connection::KEEP_COUNT = 3;
const int Connection::RETRY = 1000;

const int Connection::RETRY_INTERVAL_SECONDS = 1;

const int Connection::BUF_SIZE = 1024;

Connection::Connection(const std::string &name, struct ev_loop *loop)
	: name_(name), ip_(), port_(0), addressInited_(false), status_(CLOSED), sockfd_(-1), loop_(loop), dataHandler_(NULL), closedHandler_(NULL)
{
	bzero(&this->evwatcher_, sizeof(this->evwatcher_));
	this->evwatcher_.data = this;
	pthread_mutex_init(&this->mutex_, NULL);
}

Connection::~Connection()
{
	this->disconnect();
}

/**
 * initialize destination ip and port
 */
int Connection::initServerAddr(const std::string &ip, int port)
{
	ScopeLock lock(&this->mutex_);

	if (this->status_ != CLOSED) {
		return -1;
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
		return -1;
	} else {
		this->addressInited_ = true;
		return 0;
	}
}

/**
 *	connect to server and start event loop
 *	first check if already connected, if so, return 0 directly
 */
int Connection::connect()
{
	ScopeLock lock(&this->mutex_);

	// already connected
	if (this->status_ != CLOSED) {
		return 0;
	} else if (this->addressInited_ == false) {
		return -1;
	} else {
		return this->do_connect(RETRY);
	}
}

/**
 * when disconnected, reconnect
 * retry count before give up if nonzero, otherwise retry forever
 * return 0 on success
 * return -1 otherwise
 */
int Connection::do_connect(int retry)
{
	this->do_disconnect();

	bool forever = (retry == 0) ? true : false;
	int retryTime = 1, error = 0, sockfd = -1;
	bool successful = false;
	while (forever || retryTime <= retry) {
		LOGD("try connecting to " << this->ip_ << ':' << this->port_ << " for the " << retryTime << "th time");
		++retryTime;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			error = errno;
			LOGE("cannot create socket " << strerror(error));
			break;
		}
		// set keep alive on
		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &KEEP_ALIVE, sizeof(KEEP_ALIVE)) == -1) {
			error = errno;
			LOGE("cannot setsockopt SO_KEEPALIVE=" << KEEP_ALIVE << ' ' << strerror(error));
			close(sockfd);
			break;
		}
		// send first keepalive 30 seconds after connected
		if (setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &KEEP_IDLE, sizeof(KEEP_IDLE)) == -1) {
			error = errno;
			LOGE("cannot setsockopt TCP_KEEPIDLE=" << KEEP_IDLE << ' ' << strerror(error));
			close(sockfd);
			break;
		}
		// send keepalive package every 30 seconds
		if (setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &KEEP_INTERVAL, sizeof(KEEP_INTERVAL)) == -1) {
			error = errno;
			LOGE("cannot setsockopt TCP_KEEPINTVL=" << KEEP_INTERVAL << ' ' << strerror(error));
			close(sockfd);
			break;
		}
		// close socket if 3 keepalive packet unanswered
		if (setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &KEEP_COUNT, sizeof(KEEP_COUNT)) == -1) {
			error = errno;
			LOGE("cannot setsockopt TCP_KEEPCNT=" << KEEP_COUNT << ' ' << strerror(error));
			close(sockfd);
			break;
		}

		if (::connect(sockfd, (sockaddr*)&this->servaddr_, sizeof(this->servaddr_)) == -1) {
			error = errno;
			LOGE("cannot connect to " << this->ip_ << ':' << this->port_ << ": " << strerror(error));
			close(sockfd);

			sleep(RETRY_INTERVAL_SECONDS);
			continue;
		}
		successful = true;
		break;
	}

	if (successful == true) {
		this->sockfd_ = sockfd;
		this->status_ = CONNECTED;
		ev_io_init(&this->evwatcher_, sock_cb, this->sockfd_, EV_READ);
		ev_io_start(loop_, &this->evwatcher_);
		return 0;
	} else {
		this->sockfd_ = -1;
		this->status_ = CLOSED;
		return -1;
	}
}

/**
 * disconnect from server
 */
int Connection::disconnect()
{
	ScopeLock lock(&this->mutex_);
	return do_disconnect();
}

/**
 * do the disconnect stuff, without locking
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

void Connection::sock_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
	Connection *connection = static_cast<Connection*>(w->data);
	ScopeLock lock(&connection->mutex_);
	if (connection->status_ == CLOSED) {
		return;
	}
	assert(connection->status_ == CONNECTED);

	do {
		char buf[BUF_SIZE];
		int ret = recv(connection->sockfd_, buf, sizeof(buf), MSG_DONTWAIT);
		if (ret > 0) {
			LOGD("socket " << connection->sockfd_ << " recieved " << ret << " bytes");
			if (connection->dataHandler_ != NULL) {
				connection->dataHandler_(connection, buf, ret);
			}
		} else if (ret == 0) {
			LOGD("socket " << connection->sockfd_ << " closed by peer");
			if (connection->do_connect(RETRY) != 0 && connection->closedHandler_ != NULL) {
				connection->closedHandler_(connection);
			}
			break;
		} else if (errno == EAGAIN) {
			break;
		} else {
			LOGE("recv on socket " << connection->sockfd_  <<" failed: " << strerror(errno));
			// fixme
			// todo need reconnect or not
			break;
		}
	} while (1);
}






































