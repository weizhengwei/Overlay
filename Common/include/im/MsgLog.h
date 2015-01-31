/**
 * @file      MsgLog.h
   @brief     The header file for chat message DB I/O.
   @author    xushenghui@pwrd.com
   @date      2012/08/29
   @version   01.00.00
 */

/* 
 *  Copyright(C) 2012. Perfect World Entertainment Inc
 *
 *  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
 *  its licensors and is protected by copyright and other intellectual 
 *  property laws and treaties.  We and our licensors own all rights, 
 *  title and interest in and to the SOURCE CODE, including all copyright 
 *  and other intellectual property rights in the SOURCE CODE.
 */

/*
 * $LastChangedBy: xushenghui@pwrd.com
 * $LastChangedDate: 
 * $Revision: 
 * $Id: 2
 * $notes:
 */

#pragma once

#include <list>
#include "data/sqlite3.h"
#include "thread/thread.h"
#include "thread/WQueue.h"
#include "tstring/tstring.h"
#include "im/im_constant.h"

using namespace String;

class CMsgLog : public Thread
{
public:
	CMsgLog(void);
	~CMsgLog(void);
	struct _msg_log_element_internal
	{
		_tstring _szUserName; //friend's name 
		_tstring  _szRecvNick;//receiver's NikeName.
		_tstring _szSenderNick;//sender's NikeName 
		_tstring _szTime;//msg time
		_tstring _szDate;//msg date
		_tstring _szSecTime;
		_tstring _szContent;//msg
	};
public:
	std::list<MSG_LOG_ELEMENT>	ReadLog(LPCTSTR lpszUserName,UINT countNew);

	BOOL AppendLog(LPCTSTR lpszUserName, LPCTSTR lpszRecvNickName, LPCSTR lpszSenderNickName, 
		LPCSTR lpszTime, LPCSTR lpszDate, LPCTSTR  lpszContent);

	void ClearLog( LPCTSTR lpszUserName );
	bool Init(LPCTSTR lpLogDir,LPCTSTR lpUserName);
	void UnInit();
protected://method
	virtual long run();
	BOOL CheckTableExist(LPCTSTR lpszTableName);
	int  GetTableColumNum(LPCTSTR lpszTableName);
	BOOL CreateTable(LPCTSTR lpszTableName);
	BOOL DeleteTable(LPCTSTR lpszTableName);
	BOOL RenameTable(LPCTSTR lpszOldTable,LPCTSTR lpszNewTable);
	BOOL CopyTable(LPCTSTR lpszOldTable,LPCTSTR lpszNewTable);
	void CheckAndCreateTable(LPCTSTR lpszUserName);
    _tstring GetTableName(LPCTSTR lpszUserName);
private://variables
	 sqlite3* m_pDB;
	 Mutex    m_lockDB;
	 _tstring m_sUserName;
	 bool   m_bStop ;
	 WQueue<_msg_log_element_internal*> m_wTaskQueue ;
};