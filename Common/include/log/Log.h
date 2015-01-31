#pragma once

#include "thread/WQueue.h"
#include "thread/Thread.h"

const int LOGLENGTH = 2048;

enum Level {

	LogFault, 
	LogError,
	LogWarning, 
	LogInfo, 
	LogDebug, 
};

struct LogRecord {
	char content[LOGLENGTH];
};

class LogBase {
public:
	LogBase()
	{
		_logLevel = LogDebug;
	}

	void SetLevel(Level level)
	{
		_logLevel = level;
	}

	Level GetLevel() const 
	{
		return _logLevel;
	}

	void Log(Level level, const char* fmt, ...);

protected:
	virtual void Output(const char* str);

protected:
	Level		_logLevel;
};

class FileLog : public Thread, public LogBase
{
public:
	FileLog() : _logFile(NULL), _stopFlag(true) {}
	virtual ~FileLog(){}

	bool Init(const std::string& logfile);
	void Uninit(int code = 0);
	void Enter(const char* content);
	virtual long run();
	
	virtual void Output(const char* str);

protected:
	bool   _stopFlag;
	FILE*  _logFile;
	WQueue<LogRecord*> _logQueue;
};

extern FileLog g_theLog;

