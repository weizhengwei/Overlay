#include "log/Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

FileLog g_theLog ;

void LogBase::Output(const char* str)
{
	printf("%s\n", str);
}

void LogBase::Log(Level level, const char* fmt, ...)
{
	if (level > _logLevel)
		return;
		
	char buf[LOGLENGTH];

	const static char* levelName[] = {
		"F", 
		"E",
		"W", 
		"I", 
		"D", 
	};

	time_t t;
	time(&t);
	tm* now = localtime(&t);
	int n = snprintf(buf, sizeof(buf) - 1, "%d-%02d-%02d %02d:%02d:%02d[%010u]<%s> ", 
		1900 + now->tm_year, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, 
		GetTickCount(), levelName[level]);

	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buf + n, sizeof(buf) - 1 - n, fmt, vl);
	va_end(vl);

	buf[sizeof(buf) -1] = 0;
	Output(buf);
}

//////////////////////////////////////////////////////////////////

bool FileLog::Init(const std::string& logfile)
{
	_logFile = fopen(logfile.c_str(), "a+");
	if(_logFile == NULL)
	{
		fprintf(stderr,"ERROR: couldn't open logfile: %s\n", strerror(errno));
		return false;
	}
	_stopFlag = false;
	return Thread::start();
}

void FileLog::Uninit(int code /*= 0*/)
{
	if(_logFile != NULL)
	{
		fclose(_logFile);
	}
	_stopFlag = true;
	Thread::stop(code);
}

void FileLog::Enter(const char* content)
{
	LogRecord* logrec = new LogRecord();
	strncpy(logrec->content, content, LOGLENGTH);
	logrec->content[LOGLENGTH-1] = '\0';
	_logQueue.enter(logrec);
}

long FileLog::run()
{
	int isFlush = 0;

	while (!_stopFlag)
	{
		LogRecord* curLog = _logQueue.leave();
		if (NULL == curLog)
		{
			continue ;
		}

		fprintf(_logFile, "%s\n", curLog->content);
		fflush(_logFile);
		delete curLog;
	}

	return 0;
}

void FileLog::Output(const char* str)
{
	Enter(str);
}

