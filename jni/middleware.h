/*
 * middleware.h
 *
 *  Created on: Jul 20, 2013
 *      Author: lookflying
 */

#include <string>
#include "YzHelper.h"
#include "Connection.h"
#include "ConnectionManager.h"
#ifndef MIDDLEWARE_H_
#define MIDDLEWARE_H_

extern ConnectionManager g_conn_manager;

/**
 * 初始化中间件
 */
void initMiddleware(const std::string server_ip, const int server_port);



#endif /* MIDDLEWARE_H_ */
