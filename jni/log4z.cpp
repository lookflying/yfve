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

#ifndef NDEBUG

#include "log4z.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>


#ifdef WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <io.h>
#include <shlwapi.h>
#include <process.h>
#pragma comment(lib, "shlwapi")
#pragma warning(disable:4996)
#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#endif



#ifdef WIN32
__declspec(thread) char g_log4zstreambuf[LOG4Z_LOG_BUF_SIZE];
#else
__thread char g_log4zstreambuf[LOG4Z_LOG_BUF_SIZE];
#endif

_ZSUMMER_BEGIN
_ZSUMMER_LOG4Z_BEGIN

static const char *const LOG_STRING[]=
{
	"LOG_DEBUG",
	"LOG_INFO",
	"LOG_WARN",
	"LOG_ERROR",
	"LOG_ALARM",
	"LOG_FATAL",
};

static void SleepMillisecond(unsigned int ms);
static bool TimeToTm(const time_t & t, tm * tt);
static bool IsSameDay(time_t t1, time_t t2);



static void FixPath(std::string &path);
static void TrimLogConfig(std::string &str, char ignore = '\0');
static void ParseConfig(std::string file, std::map<std::string, std::map<std::string, std::string> > & outConfig);


static bool IsDirectory(std::string path);
static bool CreateRecursionDir(std::string path);
void GetProcessInfo(std::string &name, std::string &pid);
static void ShowColorText(const char *text, int level = LOG_LEVEL_DEBUG);


class CLock
{
public:
	CLock()
	{
#ifdef WIN32
		InitializeCriticalSection(&m_crit);
#else
		//m_crit = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_crit, &attr);
		pthread_mutexattr_destroy(&attr);
#endif
	}
	virtual ~CLock()
	{
#ifdef WIN32
		DeleteCriticalSection(&m_crit);
#else
		pthread_mutex_destroy(&m_crit);
#endif
	}

public:
	void Lock()
	{
#ifdef WIN32
		EnterCriticalSection(&m_crit);
#else
		pthread_mutex_lock(&m_crit);
#endif
	}
	void UnLock()
	{
#ifdef WIN32
		LeaveCriticalSection(&m_crit);
#else
		pthread_mutex_unlock(&m_crit);
#endif
	}
private:
#ifdef WIN32
	CRITICAL_SECTION m_crit;
#else
	pthread_mutex_t  m_crit;
#endif
};

class CAutoLock
{
public:
	explicit CAutoLock(CLock & lk):m_lock(lk){m_lock.Lock();}
	~CAutoLock(){m_lock.UnLock();}
private:
	CLock & m_lock;
};

class CSem
{
public:
	CSem()
	{
#ifdef WIN32
		m_hSem = NULL;
#else
		m_bCreate = false;
#endif
	}
	virtual ~CSem()
	{
#ifdef WIN32
		if (m_hSem != NULL)
		{
			CloseHandle(m_hSem);
			m_hSem = NULL;
		}
#else
		if (m_bCreate)
		{
			m_bCreate = false;
			sem_destroy(&m_semid);
		}
#endif
	}
public:
	bool Create(int initcount)
	{
		if (initcount < 0)
		{
			initcount = 0;
		}
#ifdef WIN32
		if (initcount > 64)
		{
			return false;
		}
		m_hSem = CreateSemaphore(NULL, initcount, 64, NULL);
		if (m_hSem == NULL)
		{
			return false;
		}
#else
		if (sem_init(&m_semid, 0, initcount) != 0)
		{
			return false;
		}
		m_bCreate = true;
#endif
		return true;

	}
	bool Wait(int timeout = 0)
	{
#ifdef WIN32
		if (timeout <= 0)
		{
			timeout = INFINITE;
		}
		if (WaitForSingleObject(m_hSem, timeout) != WAIT_OBJECT_0)
		{
			return false;
		}
#else
		if (timeout <= 0)
		{
			return (sem_wait(&m_semid) == 0);
		}
		else
		{
			timespec ts;
			ts.tv_sec += time(NULL) + timeout/1000;
			ts.tv_nsec += (timeout%1000)*1000000;
			return (sem_timedwait(&m_semid, &ts) == 0);
		}
#endif
		return true;
	}
	bool Post()
	{
#ifdef WIN32
		return ReleaseSemaphore(m_hSem, 1, NULL) ? true : false;
#else
		return (sem_post(&m_semid) == 0);
#endif
	}
private:
#ifdef WIN32
	HANDLE m_hSem;
#else
	sem_t m_semid;
	bool  m_bCreate;
#endif
};




#ifdef WIN32
static unsigned int WINAPI  ThreadProc(LPVOID lpParam);
#else
static void * ThreadProc(void * pParam);
#endif
class CThread
{
public:
	CThread(){m_hThreadID = 0;}
	virtual ~CThread(){}
public:
	bool Start()
	{
#ifdef WIN32
		unsigned long long ret = _beginthreadex(NULL, 0, ThreadProc, (void *) this, 0, NULL);

		if (ret == -1 || ret == 1  || ret == 0)
		{
			ShowColorText("log4z: create log4z thread error! \r\n", LOG_LEVEL_FATAL);
			return false;
		}
		m_hThreadID = ret;
#else
		pthread_t ptid = 0;
		int ret = pthread_create(&ptid, NULL, ThreadProc, (void*)this);
		if (ret != 0)
		{
			ShowColorText("log4z: create log4z thread error! \r\n", LOG_LEVEL_FATAL);
			return false;
		}
		m_hThreadID = ptid;

#endif
		return true;
	}
	bool Wait()
	{
#ifdef WIN32
		if (WaitForSingleObject((HANDLE)m_hThreadID, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
#else
		if (pthread_join((pthread_t)m_hThreadID, NULL) != 0)
		{
			return false;
		}
#endif
		return true;
	}

	virtual void Run() = 0;
	inline unsigned long long GetThreadID() {return m_hThreadID;};
private:
	unsigned long long m_hThreadID;
};

#ifdef WIN32
unsigned int WINAPI  ThreadProc(LPVOID lpParam)
{
	CThread * p = (CThread *) lpParam;
	p->Run();
	_endthreadex(0);
	return 0;
}
#else
void * ThreadProc(void * pParam)
{
	CThread * p = (CThread *) pParam;
	p->Run();
	return NULL;
}
#endif





class CLog4zFile
{
public:
	CLog4zFile()
	{
		m_file = NULL;
	}
	~CLog4zFile()
	{
		Close();
	}
	bool IsOpen()
	{
		if (m_file)
		{
			return true;
		}
		return false;
	}
	bool Open(const char *path, const char * mod)
	{
		if (m_file != NULL)
		{
			fclose(m_file);
			m_file = NULL;
		}
		m_file = fopen(path, mod);
		if (m_file == NULL)
		{
			return false;
		}
		return true;
	}
	void Close()
	{
		if (m_file != NULL)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}
	void Write(const char * data, size_t len)
	{
		if (!m_file)
		{
			return;
		}
		size_t wlen = fwrite(data, 1, len, m_file);
		if (wlen != len)
		{
			Close();
		}
	}
	void Flush()
	{
		if (!m_file)
		{
			return;
		}
		fflush(m_file);
	}
	bool ReadLine(char *buf, int count)
	{
		if (fgets(buf, count, m_file) == NULL)
		{
			return false;
		}
		return true;
	}

	const std::string ReadContent()
	{
		std::string content;

		if (!m_file)
		{
			return content;
		}
		fseek(m_file, 0, SEEK_SET);
		int beginpos = ftell(m_file);
		fseek(m_file, 0, SEEK_END);
		int endpos = ftell(m_file);
		fseek(m_file, 0, SEEK_SET);
		int filelen = endpos - beginpos;
		if (filelen > 10*1024*1024 || filelen <= 0)
		{
			return content;
		}
		content.resize(filelen+10);
		if (fread(&content[0], 1, filelen, m_file) != (size_t)filelen)
		{
			content.clear();
			return content;
		}
		content = content.c_str();
		return content;
	}
public:
	FILE *m_file;
};

struct LogData
{
	LoggerId _id;		//dest logger id
	int	_level;	//log level
	time_t _time;		//create time
	char _content[LOG4Z_LOG_BUF_SIZE]; //content
};

struct LoggerInfo 
{
	std::string _name;
	std::string _pid;
	std::string _path;
	int  _level; //filter level
	bool _display; //display to screen 
	bool _enable; //
	time_t _filetime;
	CLog4zFile	_handle; //file handle.
	LoggerInfo(){ _path = "./log/", _level = LOG_LEVEL_DEBUG; _display = true; _enable = false; _filetime=0;}
};


class CLogerManager : public CThread, public ILog4zManager
{
public:
	CLogerManager()
	{
		m_bRuning = false;
		m_lastId = LOG4Z_MAIN_LOGGER_ID;
		GetProcessInfo(m_loggers[LOG4Z_MAIN_LOGGER_ID]._name, m_loggers[LOG4Z_MAIN_LOGGER_ID]._pid);
		m_ids["Main"] = LOG4Z_MAIN_LOGGER_ID;
	}
	~CLogerManager()
	{
		Stop();
	}

	std::string GetExampleConfig()
	{
		return ""
			"[FileConfig]\n"
			"#path=./log/\n"
			"#level=DEBUG\n"
			"#display=true\n";
	}


	//! ��ȡ�����ļ�����д
	bool Config(std::string cfgPath)
	{
		std::map<std::string, std::map<std::string, std::string> > cfgKey;
		typedef std::map<std::string, std::map<std::string, std::string> > LogMap;
		ParseConfig(cfgPath, cfgKey);
		for (LogMap::iterator iter = cfgKey.begin(); iter != cfgKey.end(); ++iter)
		{
			LoggerInfo l;
			std::map<std::string, std::string>::iterator it;

			//!section name & log name
			l._name = iter->first; 

			//! path
			it = iter->second.find("path");
			if (it != iter->second.end())
			{
				l._path = it->second;
			}

			//! level
			it = iter->second.find("level");
			if (it != iter->second.end())
			{
				if (it->second == "DEBUG" || it->second == "ALL")
				{
					l._level = LOG_LEVEL_DEBUG;
				}
				else if (it->second == "INFO")
				{
					l._level = LOG_LEVEL_INFO;
				}
				else if (it->second == "WARN")
				{
					l._level = LOG_LEVEL_WARN;
				}
				else if (it->second == "ERROR")
				{
					l._level = LOG_LEVEL_ERROR;
				}
				else if (it->second == "ALARM")
				{
					l._level = LOG_LEVEL_WARN;
				}
				else if (it->second == "FATAL")
				{
					l._level = LOG_LEVEL_FATAL;
				}
			}
			//! display
			it = iter->second.find("display");
			if (it != iter->second.end())
			{
				if (it->second == "true" || it->second == "1")
				{
					l._display = true;
				}
			}
			CreateLogger(l._name, l._path, l._level, l._display);
		}
		return true;
	}

	//! ��дʽ����
	virtual LoggerId CreateLogger(std::string name,std::string path,int nLevel,bool display)
	{
		std::string _name;
		std::string _pid;
		GetProcessInfo(_name, _pid);
		if (name.length() == 0)
		{
			ShowColorText("log4z: create logger error, name is empty ! \r\n", LOG_LEVEL_FATAL);
			return -1;
		}
		TrimLogConfig(path);
		FixPath(path);

		LoggerId newID = -1;
		{
			std::map<std::string, LoggerId>::iterator iter = m_ids.find(name);
			if (iter != m_ids.end())
			{
				newID = iter->second;
			}
		}
		if (newID == -1)
		{
			if (m_lastId +1 >= LOG4Z_LOGGER_MAX)
			{
				ShowColorText("log4z: CreateLogger can not create|writeover, because loggerid need < LOGGER_MAX! \r\n", LOG_LEVEL_FATAL);
				return -1;
			}
			newID = ++ m_lastId;
			m_ids[name] = newID;
		}

		if (!path.empty())
		{
			m_loggers[newID]._path = path;
		}
		//! Main logger ID ���ܸ�� name
		if (newID > LOG4Z_MAIN_LOGGER_ID)
		{
			m_loggers[newID]._name = name;
		}
		m_loggers[newID]._pid = _pid;
		m_loggers[newID]._level = nLevel;
		m_loggers[newID]._enable = true;
		m_loggers[newID]._display = display;
		return newID;
	}


	bool Start()
	{
		m_semaphore.Create(0);
		bool ret = CThread::Start();
		return ret && m_semaphore.Wait(3000);
	}
	bool Stop()
	{
		if (m_bRuning == true)
		{
			m_bRuning = false;
			Wait();
			return true;
		}
		return false;
	}

	bool PushLog(LoggerId id, int level, const char * log)
	{
		if (id < 0 || id >= LOG4Z_LOGGER_MAX)
		{
			return false;
		}
		if (!m_bRuning || !m_loggers[id]._enable)
		{
			return false;
		}
		if (level < m_loggers[id]._level)
		{
			return true;
		}

		LogData * pLog = new LogData;
		pLog->_id =id;
		pLog->_level = level;
		pLog->_time = time(NULL);
		int len = (int) strlen(log);
		if (len >= LOG4Z_LOG_BUF_SIZE)
		{
			memcpy(pLog->_content, log, LOG4Z_LOG_BUF_SIZE);
			pLog->_content[LOG4Z_LOG_BUF_SIZE-1] = '\0';
		}
		else
		{
			memcpy(pLog->_content, log, len+1);
		}
		CAutoLock l(m_lock);
		m_logs.push_back(pLog);
		return true;
	}

	//! ����ID
	virtual LoggerId FindLogger(std::string name)
	{
		std::map<std::string, LoggerId>::iterator iter;
		iter = m_ids.find(name);
		if (iter != m_ids.end())
		{
			return iter->second;
		}
		return -1;
	}

	bool SetLoggerLevel(LoggerId nLoggerID, int nLevel)
	{
		if (nLoggerID <0 || nLoggerID >= LOG4Z_LOGGER_MAX || nLevel < LOG_LEVEL_DEBUG || nLevel >LOG_LEVEL_FATAL) return false;
		m_loggers[nLoggerID]._level = nLevel;
		return true;
	}
	bool SetLoggerDisplay(LoggerId nLoggerID, bool enable)
	{
		if (nLoggerID <0 || nLoggerID >= LOG4Z_LOGGER_MAX) return false;
		m_loggers[nLoggerID]._display = enable;
		return true;
	}
	unsigned long long GetStatusTotalWriteCount()
	{
		return m_ullStatusTotalWriteCount;
	}
	unsigned long long GetStatusTotalWriteBytes()
	{
		return m_ullStatusTotalWriteBytes;
	}
	unsigned long long GetStatusWaitingCount()
	{
		return m_logs.size();
	}
	unsigned int GetStatusActiveLoggers()
	{
		unsigned int actives = 0;
		for (int i=0; i<LOG4Z_LOGGER_MAX; i++)
		{
			if (m_loggers[i]._enable)
			{
				actives ++;
			}
		}
		return actives;
	}


protected:


	bool OpenLogger(LoggerId id)
	{
		if (id < 0 || id >m_lastId)
		{
			ShowColorText("log4z: OpenLogger can not open, invalide logger id! \r\n", LOG_LEVEL_FATAL);
			return false;
		}
		LoggerInfo * pLogger = &m_loggers[id];
		if (pLogger->_handle.IsOpen())
		{
			pLogger->_handle.Close();
		}

		tm t;
		TimeToTm(pLogger->_filetime, &t);
		char buf[100];
		sprintf(buf, "%04d_%02d", t.tm_year+1900, t.tm_mon+1);
		std::string path = pLogger->_path + buf + "/";
		if (!IsDirectory(path))
		{
			CreateRecursionDir(path);
		}

		sprintf(buf, "%s_%04d_%02d_%02d_%s.log", pLogger->_name.c_str(),  t.tm_year+1900, t.tm_mon+1, t.tm_mday, pLogger->_pid.c_str());
		path += buf;
		pLogger->_handle.Open(path.c_str(), "ab");
		return pLogger->_handle.IsOpen();
	}

	bool PopLog(LogData *& log)
	{
		CAutoLock l(m_lock);
		if (m_logs.empty())
		{
			return false;
		}
		log = m_logs.front();
		m_logs.pop_front();
		return true;
	}
	virtual void Run()
	{
		m_bRuning = true;
		m_loggers[LOG4Z_MAIN_LOGGER_ID]._enable = true;
		PushLog(0, LOG_LEVEL_ALARM, "-----------------  log4z thread started!   ----------------------------");
		for (int i=0; i<LOG4Z_LOGGER_MAX; i++)
		{
			if (m_loggers[i]._enable)
			{
				std::stringstream ss;
				ss <<" logger id=" <<i 
				   <<" path=" <<m_loggers[i]._path
				   <<" name=" <<m_loggers[i]._name
				   <<" level=" << m_loggers[i]._level
				   <<" display=" << m_loggers[i]._display;
				PushLog(0, LOG_LEVEL_ALARM, ss.str().c_str());
			}
		}
		
		m_semaphore.Post();


		LogData * pLog = NULL;
		char *pWriteBuf = new char[LOG4Z_LOG_BUF_SIZE + 512];
		int needFlush[LOG4Z_LOGGER_MAX] = {0};
		while (true)
		{
			while(PopLog(pLog))
			{
				//discard
				if (!m_loggers[pLog->_id]._enable || pLog->_level <m_loggers[pLog->_id]._level  )
				{
					delete pLog;
					pLog = NULL;
					continue;
				}

				//update file
				if (!m_loggers[pLog->_id]._handle.IsOpen() 
					|| !IsSameDay(pLog->_time, m_loggers[pLog->_id]._filetime))
				{
					m_loggers[pLog->_id]._filetime = pLog->_time;
					if (!OpenLogger(pLog->_id))
					{
						m_loggers[pLog->_id]._enable = false;
						delete pLog;
						pLog = NULL;
						ShowColorText("log4z: Run can not update file, open file false! \r\n", LOG_LEVEL_FATAL);
						continue;
					}
				}

				//record
				tm tt;
				if (!TimeToTm(pLog->_time, &tt))
				{
					memset(&tt, 0, sizeof(tt));
				}
				sprintf(pWriteBuf, "%d-%02d-%02d %02d:%02d:%02d %s %s \r\n", 
					tt.tm_year+1900, tt.tm_mon+1, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec,
					LOG_STRING[pLog->_level], pLog->_content);

				m_loggers[pLog->_id]._handle.Write(pWriteBuf, strlen(pWriteBuf));
				if (m_loggers[pLog->_id]._display)
				{
					ShowColorText(pWriteBuf, pLog->_level);
				}

				needFlush[pLog->_id] ++;

				m_ullStatusTotalWriteCount++;
				m_ullStatusTotalWriteBytes+=strlen(pWriteBuf);
				delete pLog;
				pLog = NULL;
			}

			for (int i=0; i<LOG4Z_LOGGER_MAX; i++)
			{
				if (m_loggers[i]._enable && needFlush[i] > 0)
				{
					m_loggers[i]._handle.Flush();
					needFlush[i] = 0;
				}
			}

			//! quit
			if (!m_bRuning && m_logs.empty())
			{
				break;
			}
			//! delay. 
			SleepMillisecond(100);
		}

		for (int i=0; i<LOG4Z_LOGGER_MAX; i++)
		{
			if (m_loggers[i]._enable)
			{
				m_loggers[i]._enable = false;
				m_loggers[i]._handle.Close();
			}
		}
		delete pWriteBuf;
		pWriteBuf = NULL;

	}

private:

	//! thread status.
	bool		m_bRuning;
	//! wait thread started.
	CSem		m_semaphore;


	//! logger id manager.
	std::map<std::string, LoggerId> m_ids;
	LoggerId	m_lastId;
	LoggerInfo m_loggers[LOG4Z_LOGGER_MAX];

	//! log queue
	std::list<LogData *> m_logs;
	CLock	m_lock;

	//status statistics
	unsigned long long m_ullStatusTotalWriteCount;
	unsigned long long m_ullStatusTotalWriteBytes;

};




void SleepMillisecond(unsigned int ms)
{
#ifdef WIN32
	::Sleep(ms);
#else
	usleep(1000*ms);
#endif
}

bool TimeToTm(const time_t &t, tm * tt)
{
#ifdef WIN32
	if (localtime_s(tt, &t) == 0)
	{
		return true;
	}
	return false;
#else
	if (localtime_r(&t, tt) != NULL)
	{
		return true;
	}
	return false;
#endif
}

bool IsSameDay(time_t t1, time_t t2)
{
	tm tm1;
	tm tm2;
	TimeToTm(t1, &tm1);
	TimeToTm(t2, &tm2);
	if ( tm1.tm_year == tm2.tm_year
		&& tm1.tm_yday == tm2.tm_yday)
	{
		return true;
	}
	return false;
}


void FixPath(std::string &path)
{
	if (path.empty())
	{
		return;
	}
	for (std::string::iterator iter = path.begin(); iter != path.end(); ++iter)
	{
		if (*iter == '\\')
		{
			*iter = '/';
		}
	}
	if (path.at(path.length()-1) != '/')
	{
		path += "/";
	}
}
static void TrimLogConfig(std::string &str, char ignore)
{
	if (str.empty())
	{
		return;
	}
	size_t endPos = str.length();
	int posBegin = (int)endPos;
	int posEnd = -1;

	for (size_t i = 0; i<str.length(); i++)
	{
		char ch = str[i];
		if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && ch != ignore)
		{
			posBegin = (int)i;
			break;
		}
	}
	for (size_t i = endPos; i> 0; i--)
	{
		char ch = str[i-1];
		if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && ch != ignore)
		{
			posEnd = (int)i-1;
			break;
		}
	}
	if (posBegin <= posEnd)
	{
		str = str.substr(posBegin, posEnd-posBegin+1);
	}
	else
	{
		str.clear();
	}
}

static void ParseConfig(std::string file, std::map<std::string, std::map<std::string, std::string> > & outConfig)
{
	//! read file content
	{
		CLog4zFile f;
		f.Open(file.c_str(), "r");

		if (f.IsOpen())
		{
			char buf[500];
			std::string line;
			std::string section;
			std::string key;
			std::string value;
			do 
			{
				memset(buf, 0, 500);
				if (!f.ReadLine(buf, 500-1))
				{
					break;
				}
				line = buf;

				TrimLogConfig(line);

				if (line.empty())
				{
					continue;
				}
				if (*(line.begin()) == '#')
				{
					continue;
				}
				if (*(line.begin()) == '[')
				{
					section = line;
					
					TrimLogConfig(section, '[');
					TrimLogConfig(section, ']');
					outConfig[section] = std::map<std::string, std::string>();
					continue;
				}
				size_t pos = line.find_first_of('=');
				if (pos == std::string::npos)
				{
					continue;
				}
				key = line.substr(0, pos);
				value = line.substr(pos+1);
				TrimLogConfig(key);
				TrimLogConfig(value);
				outConfig[section][key] = value;				
			} while (1);
		}
	}
}


bool IsDirectory(std::string path)
{
#ifdef WIN32
	return PathIsDirectoryA(path.c_str()) ? true : false;
#else
	DIR * pdir = opendir(path.c_str());
	if (pdir == NULL)
	{
		return false;
	}
	else
	{
		closedir(pdir);
		pdir = NULL;
		return true;
	}
#endif
}



bool CreateRecursionDir(std::string path)
{
	if (path.length() == 0) return true;
	std::string sub;
	FixPath(path);

	std::string::size_type pos = path.find('/');
	while (pos != std::string::npos)
	{
		std::string cur = path.substr(0, pos-0);
		if (cur.length() > 0 && !IsDirectory(cur))
		{
			bool ret = false;
#ifdef WIN32
			ret = CreateDirectoryA(cur.c_str(), NULL) ? true : false;
#else
			ret = (mkdir(cur.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0);
#endif
			if (!ret)
			{
				return false;
			}
		}
		pos = path.find('/', pos+1);
	}

	return true;
}

void GetProcessInfo(std::string &name, std::string &pid)
{
	name = "MainLog";
	pid = "0";
#ifdef WIN32
	
	char buf[260] = {0};
	if (GetModuleFileNameA(NULL, buf, 259) > 0)
	{
		name = buf;
	}
	std::string::size_type pos = name.rfind("\\");
	if (pos != std::string::npos)
	{
		name = name.substr(pos+1, std::string::npos);
	}
	pos = name.rfind(".");
	if (pos != std::string::npos)
	{
		name = name.substr(0, pos-0);
	}
	DWORD pidd = GetCurrentProcessId();
	sprintf(buf, "%06d", pidd);
	pid = buf;
#else
	pid_t id = getpid();
	char buf[260];
	sprintf(buf, "/proc/%d/cmdline", (int)id);
	CLog4zFile i;
	i.Open(buf, "r");
	if (!i.IsOpen())
	{
		return ;
	}
	if (i.ReadLine(buf, 259))
	{
		name = buf;
	}
	i.Close();

	std::string::size_type pos = name.rfind("/");
	if (pos != std::string::npos)
	{
		name = name.substr(pos+1, std::string::npos);
	}
	sprintf(buf, "%06d", id);
	pid = buf;
#endif
}



#ifdef WIN32

const static WORD cs_sColor[LOG_LEVEL_FATAL+1] = {
	0,
	FOREGROUND_BLUE|FOREGROUND_GREEN,
	FOREGROUND_GREEN|FOREGROUND_RED,
	FOREGROUND_RED,
	FOREGROUND_GREEN,
	FOREGROUND_RED|FOREGROUND_BLUE};
#else

const static char cs_strColor[LOG_LEVEL_FATAL+1][50] = { 
	"\e[0m",
	"\e[34m\e[1m",//hight blue
	"\e[33m", //yellow
	"\e[31m", //red
	"\e[32m", //green
	"\e[35m"};
#endif

void ShowColorText(const char *text, int level)
{
	if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL) goto showfail;
	if (level == LOG_LEVEL_DEBUG) goto showfail;
#ifndef WIN32
	printf("%s%s\e[0m", cs_strColor[level], text);
#else
	HANDLE hStd = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStd == INVALID_HANDLE_VALUE) goto showfail;

	CONSOLE_SCREEN_BUFFER_INFO oldInfo;
	if (!GetConsoleScreenBufferInfo(hStd, &oldInfo)) goto showfail;

	if (SetConsoleTextAttribute(hStd, cs_sColor[level]))
	{
		printf(text);
		SetConsoleTextAttribute(hStd, oldInfo.wAttributes);
	}
	else
	{
		goto showfail;
	}
#endif

	return;

showfail:
	printf(text);
}


ILog4zManager * ILog4zManager::GetInstance()
{
	static CLogerManager m;
	return &m;
}



_ZSUMMER_LOG4Z_END
_ZSUMMER_END

#endif //NDEBUG
