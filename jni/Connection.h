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
#include <ev.h>

class Connection {
public:
	/**
	 * connection status
	 */
	enum Status {
		CLOSED,
		CONNECTING,
		CONNECTED
	};

	typedef void (*dataHandler_t)(const Connection *connection, const char *data, int len);
	typedef void (*closedHandler_t)(const Connection *connection);

	Connection(const std::string &name, struct ev_loop *loop);
	~Connection();

	/**
	 * initialize destination ip and port
	 * return 0 if succeeded
	 * otherwise return -1, cases are
	 * 1 status of the connection is CONNECTED, do nothing
	 * 2 ip address is not corrected, original ip:port information is destroyed
	 */
	int initServerAddr(const std::string &ip, int port);

	/**
	 *	connect to server and start event loop
	 *	return 0 if already connected, or successfully connected within retry times
	 *	return -1 if failed, either server address not inited or exceeds max retry times
	 */
	int connect();

	/**
	 * disconnect from server anyway
	 * return 0 if successfully disconnected
	 * return -1 if not connected
	 */
	int disconnect();

	/**
	 * name of the connection
	 */
	const std::string &name()const { return this->name_; }

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
	 * send data to the other side
	 * return 0 on success
	 * return -1 on fail, cases are:
	 * 1 connection not open
	 * 2 ?
	 */
	int send(const char *data, int len);

	/**
	 * call back when data arrived
	 */
	dataHandler_t dataHandler()const { return dataHandler_; }
	void setDataHandler(dataHandler_t handler) { dataHandler_ = handler; }

	/**
	 * call back when connection is closed and not reconnectable within RETRY times
	 */
	closedHandler_t closedHandler()const { return closedHandler_; }
	void setClosedHandler(closedHandler_t handler) { closedHandler_ = handler; }
private:
	/**
	 * callback when socket reabable, disconnected
	 */
	static void sock_cb(struct ev_loop *loop, struct ev_io *w, int revents);

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

	std::string name_;
	std::string ip_;
	int port_;
	struct sockaddr_in servaddr_;
	bool addressInited_;
	Status status_;
	int sockfd_;
	struct ev_io evwatcher_;
	struct ev_loop *const loop_;
	dataHandler_t dataHandler_;
	closedHandler_t closedHandler_;
	pthread_mutex_t mutex_;

	static const int KEEP_ALIVE;
	static const int KEEP_IDLE;
	static const int KEEP_INTERVAL;
	static const int KEEP_COUNT;
	static const int RETRY;

	static const int RETRY_INTERVAL_SECONDS;

	static const int BUF_SIZE;
};

#endif /* CONNECTION_H_ */
