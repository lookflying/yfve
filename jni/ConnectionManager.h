/*
 * ConnectionManager.h
 *
 *  Created on: Jul 7, 2013
 *      Author: ushrimp
 */

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include <string>
#include <pthread.h>
#define EV_STANDALONE 1
#include "ev.h"

#include "Connection.h"

class ConnectionManager {
public:

	enum {
		MESSAGE_CONNECTION_IDX,
		DISPATCH_CONNECTION_IDX,
		WEATHER_CONNECTION_IDX,
		TOTAL_CONNECTIONS
	};

	ConnectionManager();
	~ConnectionManager();

	/**
	 * init connection manager, will start a thread for loop
	 * return 0 if succeeded
	 * return -1 otherwise
	 */
	int start();

	/**
	 * destroy all connections and stop myself
	 */
	int stop();

	Connection *getConnection(size_t idx) { return idx < TOTAL_CONNECTIONS ? &this->connections_[idx] : NULL; }
private:
	static void *connection_thread_worker(void *param);

	static void idle_cb(struct ev_loop *loop, struct ev_idle *w, int revents);

	bool inited_;
	pthread_t workertid_;
	Connection connections_[TOTAL_CONNECTIONS];
	struct ev_loop *loop_;
	struct ev_idle idle_;
};

#endif /* CONNECTIONMANAGER_H_ */
