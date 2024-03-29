/*
 * Log4z License
 * -----------
 * 
 * Log4z is licensed under the terms of the MIT license reproduced below.
 * This means that Log4z is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 * 
 * 
 * ===============================================================================
 * 
 * Copyright (C) 2012 YaweiZhang <yawei_zhang@foxmail.com>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * ===============================================================================
 * 
 * (end of COPYRIGHT)
 */


/*
 * AUTHORS:  YaweiZhang <yawei_zhang@foxmail.com>
 * VERSION:  2.1
 * PURPOSE:  A lightweight library for error reporting and logging to file and screen .
 * CREATION: 2010.10.4
 * LCHANGE:  2013.05.22
 * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
 */


/*
 *
 * QQ Group: 19811947
 * Web Site: www.zsummer.net
 * mail: yawei_zhang@foxmail.com
 */


/* 
 * UPDATES LOG
 * 
 * VERSION 0.1.0 <DATE: 2010.10.4>
 * 	create the first project.  
 * 	It support put log to screen and files, 
 * 	support log level, support one day one log file.
 * 	support multiple thread, multiple operating system.
 * 
 * VERSION .... <DATE: ...>
 * 	...
 * 
 * VERSION 0.9.0 <DATE: 2012.12.24>
 * 	support config files.
 * 	support color text in screen.
 * 	support multiple logger.
 * 
 * VERSION 1.0.0 <DATE: 2012.12.29>
 * 	support comments in the config file.
 * 	add a advanced demo in the ./project
 * 	fix some details.
 * 
 * VERSION 1.0.1 <DATE: 2013.01.01>
 * 	the source code haven't any change.
 * 	fix some Comments in the log4z
 * 	add some comments in the test projects.
 * 	delete some needless code in the 'fast_test' demo projects, it's so simple.
 * 
 * VERSION 1.1.0 <DATE: 2013.01.24>
 * 	the method Start will wait for the logger thread started.
 * 	config and add method change. 
 * 	namespace change.
 * 	append some macro.
 * 
 * VERSION 1.1.1 <DATE: 2013.02.23>
 * 	add GetStatus**** mothed.
 * 	optimize. 
 * VERSION 1.2.0 <DATE: 2013.04.05>
 * 	optimize log macro.
 * 
 * VERSION 1.2.1 <DATE: 2013.04.13>
 * 	1.20 optimize detail fixed.
 * 
 * VERSION 2.0.0 <DATE: 2013.04.25>
 * 	new interface.
 * 	new config design.
 * 	file name append process id.
 *
 * VERSION 2.1 <DATE: 2013.05.22>
 * 	support binary text output
 * 	fix vs2005 can't open Chinese characters path file.
 * 
 */

#pragma once
#ifndef _ZSUMMER_LOG4Z_H_
#define _ZSUMMER_LOG4Z_H_

#include <string>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

//! logger ID type.
typedef int LoggerId;

//! the max logger count.
#define LOG4Z_LOGGER_MAX 10

//! the max log content length.
#define LOG4Z_LOG_BUF_SIZE 2048


//! the invalid logger id. DO NOT TOUCH
#define LOG4Z_INVALID_LOGGER_ID -1

//! the main logger id. DO NOT TOUCH
#define LOG4Z_MAIN_LOGGER_ID 0



//! LOG Level
enum ENUM_LOG_LEVEL
{
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_ALARM,
	LOG_LEVEL_FATAL,
};


#ifndef _ZSUMMER_BEGIN
#define _ZSUMMER_BEGIN namespace zsummer {
#endif  
#ifndef _ZSUMMER_LOG4Z_BEGIN
#define _ZSUMMER_LOG4Z_BEGIN namespace log4z {
#endif
_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN







//! log4z class
class ILog4zManager
{
public:
	ILog4zManager(){};
	virtual ~ILog4zManager(){};
	virtual std::string GetExampleConfig() = 0;

	//! log4z Singleton
	static ILog4zManager * GetInstance();

	//! config
	virtual bool Config(std::string cfgPath) = 0;
	//! create | write over 
	virtual LoggerId CreateLogger(std::string name, std::string path="./log/",int nLevel = LOG_LEVEL_DEBUG,bool display = true) = 0;

	//! start & stop.
	virtual bool Start() = 0;
	virtual bool Stop() = 0;

	//! find logger. thread safe.
	virtual LoggerId FindLogger(std::string name) =0;

	//! push log, thread safe.
	virtual bool PushLog(LoggerId id, int level, const char * log) = 0;

	//! set logger's attribute, thread safe.
	virtual bool SetLoggerLevel(LoggerId nLoggerID, int nLevel) = 0;
	virtual bool SetLoggerDisplay(LoggerId nLoggerID, bool enable) = 0;

	//! log4z status statistics, thread safe.
	virtual unsigned long long GetStatusTotalWriteCount() = 0;
	virtual unsigned long long GetStatusTotalWriteBytes() = 0;
	virtual unsigned long long GetStatusWaitingCount() = 0;
	virtual unsigned int GetStatusActiveLoggers() = 0;

};

#ifndef _ZSUMMER_END
#define _ZSUMMER_END }
#endif  
#ifndef _ZSUMMER_LOG4Z_END
#define _ZSUMMER_LOG4Z_END }
#endif

_ZSUMMER_LOG4Z_END
_ZSUMMER_END

class CStringStream;

//! optimize by TLS
#ifdef WIN32
extern __declspec(thread) char g_log4zstreambuf[LOG4Z_LOG_BUF_SIZE];
#else
extern __thread char g_log4zstreambuf[LOG4Z_LOG_BUF_SIZE];
#endif

//! base micro.
#define LOG_STREAM(id, level, log)\
{\
	zsummer::log4z::CStringStream ss(g_log4zstreambuf, LOG4Z_LOG_BUF_SIZE);\
	ss << log;\
	ss << " ( " << __FILE__ << " ) : "  << __LINE__;\
	zsummer::log4z::ILog4zManager::GetInstance()->PushLog(id, level, g_log4zstreambuf);\
}

//! fast micro
#define LOG_DEBUG(id, log) LOG_STREAM(id, LOG_LEVEL_DEBUG, log)
#define LOG_INFO(id, log)  LOG_STREAM(id, LOG_LEVEL_INFO, log)
#define LOG_WARN(id, log)  LOG_STREAM(id, LOG_LEVEL_WARN, log)
#define LOG_ERROR(id, log) LOG_STREAM(id, LOG_LEVEL_ERROR, log)
#define LOG_ALARM(id, log) LOG_STREAM(id, LOG_LEVEL_ALARM, log)
#define LOG_FATAL(id, log) LOG_STREAM(id, LOG_LEVEL_FATAL, log)

//! super micro.
#ifdef NDEBUG
	#define LOGD( log )
	#define LOGI( log )
	#define LOGW( log )
	#define LOGE( log )
	#define LOGA( log )
	#define LOGF( log )
#else
	#define LOGD( log ) LOG_DEBUG(LOG4Z_MAIN_LOGGER_ID, log )
	#define LOGI( log ) LOG_INFO(LOG4Z_MAIN_LOGGER_ID, log )
	#define LOGW( log ) LOG_WARN(LOG4Z_MAIN_LOGGER_ID, log )
	#define LOGE( log ) LOG_ERROR(LOG4Z_MAIN_LOGGER_ID, log )
	#define LOGA( log ) LOG_ALARM(LOG4Z_MAIN_LOGGER_ID, log )
	#define LOGF( log ) LOG_FATAL(LOG4Z_MAIN_LOGGER_ID, log )


#endif // NDEBUG







_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN

//! optimze from std::stringstream to CStringStream
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif
struct BinaryBlock
{
	BinaryBlock(const char * buf, int len)
	{
		_buf = buf;
		_len = len;
	}
	const char * _buf;
	int  _len;
};
class CStringStream
{
public:
	CStringStream(char * buf, int len)
	{
		m_pBegin = buf;
		m_pEnd = buf + len;
		m_pCur = m_pBegin;
	}

	template<class T>
	void WriteData(const char * ft, T t)
	{
		if (m_pCur < m_pEnd)
		{
			int len = 0;
			int count = (int)(m_pEnd - m_pCur);
#ifdef WIN32
			len = _snprintf(m_pCur, count, ft, t);
			if (len == count || (len == -1 && errno == ERANGE))
			{
				len = count;
				*(m_pEnd-1) = '\0';
			}
			else if (len < 0)
			{
				*m_pCur = '\0';
				len = 0;
			}
#else
			len = snprintf(m_pCur, count, ft, t);
			if (len < 0)
			{
				*m_pCur = '\0';
				len = 0;
			}
			else if (len >= count)
			{
				len = count;
				*(m_pEnd-1) = '\0';
			}
#endif
			m_pCur += len;
		}
	}

	template<class T>
	CStringStream & operator <<(const T * t)
	{	
#ifdef WIN32
		if (sizeof(t) == 8)
		{
			WriteData("%016I64x", (unsigned long long)t);
		}
		else
		{
			WriteData("%08I64x", (unsigned long long)t);
		}
#else
		if (sizeof(t) == 8)
		{
			WriteData("%016llx", (unsigned long long)t);
		}
		else
		{
			WriteData("%08llx", (unsigned long long)t);
		}
#endif
		return *this;
	}
	template<class T>
	CStringStream & operator <<(T * t)
	{
		return (*this << (const T*) t);
	}

	CStringStream & operator <<(char * t)
	{
		WriteData("%s", t);
		return *this;
	}
	CStringStream & operator <<(const char * t)
	{
		WriteData("%s", t);
		return *this;
	}
	CStringStream & operator <<(bool t)
	{
		if(t)WriteData("%s", "true");
		else WriteData("%s", "false");
		return *this;
	}
	CStringStream & operator <<(char t)
	{
		WriteData("%c", t);
		return *this;
	}

	CStringStream & operator <<(unsigned char t)
	{
		WriteData("%d",(int)t);
		return *this;
	}
	CStringStream & operator <<(short t)
	{
		WriteData("%d", (int)t);
		return *this;
	}
	CStringStream & operator <<(unsigned short t)
	{
		WriteData("%d", (int)t);
		return *this;
	}
	CStringStream & operator <<(int t)
	{
		WriteData("%d", t);
		return *this;
	}
	CStringStream & operator <<(unsigned int t)
	{
		WriteData("%u", t);
		return *this;
	}
	CStringStream & operator <<(long t)
	{
		if (sizeof(long) == sizeof(int))
		{
			WriteData("%d", t);
		}
		else
		{
			WriteData("%lld", t);
		}
		return *this;
	}
	CStringStream & operator <<(unsigned long t)
	{
		if (sizeof(unsigned long) == sizeof(unsigned int))
		{
			WriteData("%u", t);
		}
		else
		{
			WriteData("%llu", t);
		}
		return *this;
	}
	CStringStream & operator <<(long long t)
	{
#ifdef WIN32  
		WriteData("%I64d", t);
#else
		WriteData("%lld", t);
#endif
		return *this;
	}
	CStringStream & operator <<(unsigned long long t)
	{
#ifdef WIN32  
		WriteData("%I64u", t);
#else
		WriteData("%llu", t);
#endif
		return *this;
	}
	CStringStream & operator <<(float t)
	{
		WriteData("%.4f", t);
		return *this;
	}
	CStringStream & operator <<(double t)
	{
		WriteData("%.4lf", t);
		return *this;
	}
	CStringStream & operator <<(const std::string t)
	{
		WriteData("%s", t.c_str());
		return *this;
	}

	CStringStream & operator << (const BinaryBlock binary)
	{
		WriteData("%s", "[");
		for (int i=0; i<binary._len; i++)
		{
			WriteData("%02x ", (unsigned char)binary._buf[i]);
		}
		WriteData("%s", "]");
		return *this;
	}

private:
	CStringStream(){}
	CStringStream(CStringStream &){}
	char *  m_pBegin;
	char *  m_pEnd;
	char *  m_pCur;
};

#ifdef WIN32
#pragma warning(pop)
#endif

_ZSUMMER_LOG4Z_END
_ZSUMMER_END

#endif





