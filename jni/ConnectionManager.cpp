/*
 * ConnectionManager.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: ushrimp
 */

#include <string.h>

#include "ConnectionManager.h"
#include "log4z.h"

ConnectionManager::ConnectionManager()
	: inited_(false), workertid_(0)
{
	this->loop_ = ev_default_loop();
	bzero(&this->idle_, sizeof(this->idle_));
	this->connections_[MESSAGE_CONNECTION_IDX].set_name("message connection");
	this->connections_[MESSAGE_CONNECTION_IDX].set_loop(this->loop_);
	this->connections_[DISPATCH_CONNECTION_IDX].set_name("message connection");
	this->connections_[DISPATCH_CONNECTION_IDX].set_loop(this->loop_);
	this->connections_[WEATHER_CONNECTION_IDX].set_name("message connection");
	this->connections_[WEATHER_CONNECTION_IDX].set_loop(this->loop_);
}

ConnectionManager::~ConnectionManager()
{
	this->stop();
}

/**
 * init connection manager, will start a thread for loop
 * return 0 if succeeded
 * return -1 otherwise
 */
int ConnectionManager::start()
{
	if (this->inited_ == true) {
		return 0;
	}

	ev_idle_init(&this->idle_, idle_cb);
	ev_idle_start(this->loop_, &this->idle_);
	if (pthread_create(&this->workertid_, NULL, connection_thread_worker, this) == 0) {
		this->inited_ = true;
		return 0;
	} else {
		this->workertid_ = 0;
		return -1;
	}
}

/**
 * destroy all connections and stop myself
 */
int ConnectionManager::stop()
{
	if (this->inited_ == true) {
		for (int i = 0; i < TOTAL_CONNECTIONS; ++i) {
			this->connections_[i].disconnect();
		}
		ev_unloop(this->loop_, EVUNLOOP_ALL);
		this->inited_ = false;
		this->workertid_ = 0;
	}
	return 0;
}


void ConnectionManager::idle_cb(struct ev_loop *loop, struct ev_idle *w, int revents)
{
	LOGD("idle_cb" << Connection::DEFAULT_CONNECT_RETRY_INTERVAL_SECONDS ) ;
	sleep(1);
}

void *ConnectionManager::connection_thread_worker(void *param)
{
	ConnectionManager *manager = static_cast<ConnectionManager*>(param);
	ev_loop(manager->loop_, 0);
	return NULL;
}
















