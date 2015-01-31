/**
 * @file      MsgLog.cpp
   @brief     The implementation file for chat message DB I/O.
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

#include "stdafx.h"

#include "log/local.h"
#include "data/Utility.h"
#include "im/MsgLog.h"
#include <gloox/md5.h>
using namespace gloox;
#pragma comment(lib,"lib/sqlite3.lib")

#define  CHATLOGFIELDNUM 8
#define  DB_PREFIX _T("chat")

CMsgLog::CMsgLog(void)
{
   m_bStop = true;
   m_pDB = NULL;
}

CMsgLog::~CMsgLog(void)
{
}

bool CMsgLog::Init(LPCTSTR lpLogDir,LPCTSTR lpUserName)
{
	//Set chatlog.db file path
	_tstring szFolder(lpLogDir);
	if (szFolder.Right(1) != _T("\\"))
	{
		szFolder.append(_T("\\"));
	}
	m_sUserName.append(lpUserName);
	
    MD5 md5;
	_tstring sUserMd5;
	md5.feed(m_sUserName.toNarrowString( ).c_str( ));
	md5.finalize( );
	sUserMd5.fromUTF8(md5.hex().c_str());

	_tstring sChatDBName;
	sChatDBName.Format(3,_T("%s%s_%s.db"),szFolder.c_str(),DB_PREFIX,sUserMd5.c_str());
	sChatDBName.Replace('\\','/');

	///check if chatlog.db exists
	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(sChatDBName.c_str( )))
	{
		CUtility::MakeDir(szFolder.c_str( ));
	}
	int nRes=sqlite3_open_v2(sChatDBName.toUTF8().c_str(),&m_pDB,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL);
	if(SQLITE_OK!=nRes)
	{
		m_pDB=NULL;
		Log(_T("open chat log db fail,error code is %d"),nRes);
		return false;
	}	
    
	//start work thread.
	m_bStop = false;
	return Thread::start();
}

void CMsgLog::UnInit( )
{
	m_bStop = true;
	_msg_log_element_internal* pMsg=NULL;
	m_wTaskQueue.enter(pMsg);//ensure a null record in order to invoke the write DB thread and this record will not insert into db.
	Thread::wait();
}

BOOL CMsgLog::CheckTableExist(LPCTSTR lpszTableName)
{
	int nRes = SQLITE_OK;
	int  nColNum=0;
	sqlite3_stmt   *pstmt=NULL;
	_tstring   szQuery;
	do 
	{
		m_lockDB.lock( );
		szQuery.Format(1,_T("select count(*) from sqlite_master where type= \"table\" and name=\"%s\";"),lpszTableName);
		nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nRes=sqlite3_step(pstmt);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nColNum = sqlite3_column_int(pstmt,0);

	} while (FALSE);
	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock();
	
	if (nRes == SQLITE_OK && nColNum >0)
	{
		return TRUE;
	}
	return FALSE;

}
int	 CMsgLog::GetTableColumNum(LPCTSTR lpszTableName)
{
	//get table info;
	char       *errMsg=NULL;
	int           nRow,nColumn;
	char       **dbResult; 
	_tstring  szQuery;

	m_lockDB.lock();
	szQuery.Format(1,_T("pragma table_info(%s);"),lpszTableName);
	int nRes=sqlite3_get_table( m_pDB,szQuery.toUTF8().c_str(),&dbResult, &nRow, &nColumn, &errMsg);
	sqlite3_free_table(dbResult);
	szQuery.clear( );
	m_lockDB.unlock();
	
	return nRes == SQLITE_OK ? nRow:-1;
}
BOOL CMsgLog::CreateTable(LPCTSTR lpszTableName)
{
	int  nRes=0;
	sqlite3_stmt   *pstmt=NULL;
	_tstring   szQuery;
	do 
	{
		m_lockDB.lock( );
		szQuery.Format(1,_T("CREATE TABLE IF NOT EXISTS  %s(keyIndex integer PRIMARY KEY,szType char(32),szTime char(20), szContent varchar(450),Reserved1 integer,Reserved2 integer,szReserved1 varchar(512),szReserved2 varchar(512));"),lpszTableName);
		nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nRes=sqlite3_step(pstmt);
	} while (FALSE);
	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock();
	return nRes == SQLITE_OK;
}
BOOL CMsgLog:: DeleteTable(LPCTSTR lpszTableName)
{
	int  nRes=0;
	sqlite3_stmt   *pstmt=NULL;
	_tstring   szQuery;
	do 
	{
		m_lockDB.lock( );
		szQuery.Format(1,_T("drop table %s;"),lpszTableName);
		nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nRes=sqlite3_step(pstmt);
	} while (FALSE);
	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock( );
	return nRes == SQLITE_OK;
}
BOOL CMsgLog::RenameTable(LPCTSTR lpszOldTable,LPCTSTR lpszNewTable)
{
	int  nRes=0;
	sqlite3_stmt   *pstmt=NULL;
	_tstring   szQuery;
	do 
	{	m_lockDB.lock( );
		szQuery.Format(2,_T("alter table %s rename to %s;"),lpszOldTable,lpszNewTable);
		nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nRes=sqlite3_step(pstmt);
	} while (FALSE);
	
	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock( );
	
	return nRes == SQLITE_OK;
}
BOOL CMsgLog::CopyTable(LPCTSTR lpszOldTable,LPCTSTR lpszNewTable)
{
	int  nRes=0;
	sqlite3_stmt   *pstmt=NULL;
	_tstring   szQuery;
	do 
	{
		m_lockDB.lock( );
		szQuery.Format(2,_T("insert into  %s select null,szType,szTime,szContent,null,null,null,null from %s;"),lpszNewTable,lpszOldTable);
		nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			break;
		}
		nRes=sqlite3_step(pstmt);
	} while (FALSE);

	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock( );

	return nRes == SQLITE_OK;
}
void CMsgLog::CheckAndCreateTable(LPCTSTR lpszUserName)
{
	_tstring sLogTableName = GetTableName(lpszUserName);

	BOOL bExist = CheckTableExist(sLogTableName.c_str());
	if (bExist)
	{
       int nFieldCount = GetTableColumNum(sLogTableName.c_str());
	   if (CHATLOGFIELDNUM == nFieldCount)
	   {
		   return; //means the tables have the same structures.
	   }
	   else
	   {
		   //create new table and move the data to new table from old table.
		   _tstring szLogTableTemp = sLogTableName + _T("_temp");
		   if (RenameTable(sLogTableName.c_str(),szLogTableTemp.c_str()))
		   {
			   if (CreateTable(sLogTableName.c_str()))
			   {
				   if (CopyTable(szLogTableTemp.c_str(),sLogTableName.c_str()))
				   {
					   DeleteTable(szLogTableTemp.c_str()); 
					   return;
				   }
			   }
		   }
		  	Log(_T("update table  %s fail"),sLogTableName.c_str());
	   }
	}
	//create table directly.
   CreateTable(sLogTableName.c_str());
}

BOOL CMsgLog::AppendLog(LPCTSTR lpszUserName, LPCTSTR lpszRecvNickName, LPCSTR lpszSenderNickName, 
						LPCSTR lpszTime, LPCSTR lpszDate, LPCTSTR  lpszContent)
{
	_msg_log_element_internal   *pElement=new _msg_log_element_internal;
	pElement->_szUserName.assign(lpszUserName) ;
	pElement->_szRecvNick.assign(lpszRecvNickName);
	pElement->_szSenderNick.fromUTF8(lpszSenderNickName);
	pElement->_szTime.fromUTF8(lpszTime);
	pElement->_szDate.fromUTF8(lpszDate);
	pElement->_szContent=lpszContent;
	pElement->_szContent.reserve(pElement->_szContent.size()*2) ;
	pElement->_szContent.Replace(_T("\'"),_T("\'\'")); //if not replaced, ' cannot be read from log file.
	m_wTaskQueue.enter(pElement);
	return true;
}

std::list<MSG_LOG_ELEMENT> CMsgLog::ReadLog(LPCTSTR lpszUserName,UINT countNew)
{
	
   std::list<MSG_LOG_ELEMENT> LogList;
   LogList.clear();

   _tstring sLogTableName = GetTableName(lpszUserName);

	sqlite3_stmt *pstmt=NULL;
	char *errMsg=NULL;
	int nRow,nColumn;
	char **dbResult; 
	_tstring szQuery;
	do 
	{
		m_lockDB.lock();
		szQuery.Format(1,_T("select szType,szTime,szContent,szReserved1 from %s ORDER BY keyIndex DESC limit 0,100;"),sLogTableName.c_str());

		int nRes=sqlite3_get_table( m_pDB,szQuery.toUTF8().c_str(),&dbResult, &nRow, &nColumn, &errMsg);
		if(SQLITE_OK!=nRes)
		{
			_tstringA sError(errMsg);
			Log(_T("read log from table %s failed,and the error string is %s"),sLogTableName.c_str(),sError.toWideString().c_str());
			break;
		}
		int nIndex=nColumn;
		if(nIndex>=0)
		{
			int nCount= nRow>countNew ? countNew:nRow;
			MSG_LOG_ELEMENT   msgElement;
			for(int i=0;i<nCount;i++)
			{
				_tstring sTmp;
				lstrcpyn(msgElement._szSenderNick,sTmp.fromUTF8(dbResult[nIndex++]).c_str(),MAX_PATH-1);
				lstrcpyn(msgElement._szTime,sTmp.fromUTF8(dbResult[nIndex++]).c_str(),MAX_PATH-1);
				lstrcpyn(msgElement._szContent,sTmp.fromUTF8(dbResult[nIndex++]).c_str(),MAX_PATH-1);
				lstrcpyn(msgElement._szDate,sTmp.fromUTF8(dbResult[nIndex++]).c_str(),MAX_PATH-1);
				LogList.push_front(msgElement);
			}
		}

	} while (FALSE);	
	sqlite3_free_table(dbResult);
	szQuery.clear( );
	m_lockDB.unlock();

	return LogList;
}

void CMsgLog::ClearLog(  LPCTSTR lpszUserName )
{
	_tstring sLogTableName = GetTableName(lpszUserName);

    sqlite3_stmt   *pstmt=NULL;
	_tstring  szQuery;
	do 
	{
		m_lockDB.lock( );
		szQuery.Format(1,_T("delete from %s"),sLogTableName.c_str());
		int nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
		if(SQLITE_OK!=nRes)
		{
			Log(_T("prepare drop log %s fail"),sLogTableName.c_str());
			break;
		}
		sqlite3_step(pstmt);
	
	} while (FALSE);

	sqlite3_finalize(pstmt);
	szQuery.clear( );
	m_lockDB.unlock( );
}

long CMsgLog::run()
{
	while (!m_bStop)
	{
		_msg_log_element_internal  *pElement=(_msg_log_element_internal *)m_wTaskQueue.leave();
		if (pElement)
		{
           _tstring sTableName = GetTableName(pElement->_szUserName.c_str());

			CheckAndCreateTable(pElement->_szUserName.c_str());	

	        int nRes=0;
			sqlite3_stmt *pstmt=NULL;
			_tstring szQuery;
			do 
			{	m_lockDB.lock( );
				szQuery.Format(5, _T("insert into %s values(NULL,\'%s\',\'%s\',\'%s\',NULL,NULL,\'%s\',NULL)"), 
					sTableName.c_str(), 
					pElement->_szSenderNick.c_str( ), 
					pElement->_szTime.c_str(), 
					pElement->_szContent.c_str(), 
					pElement->_szDate.c_str()
					);

				nRes=sqlite3_prepare_v2( m_pDB,szQuery.toUTF8().c_str(),szQuery.toUTF8().length(),&pstmt,NULL);
				if(SQLITE_OK!=nRes)
				{
					break;
				}
				nRes=sqlite3_step(pstmt);
			} while (FALSE);
			sqlite3_finalize(pstmt);
			szQuery.clear( );
			m_lockDB.unlock( );
			
			delete pElement ; 
			pElement=NULL;	
		}
	}

	if (m_pDB)
	{
		sqlite3_close(m_pDB);
		m_pDB=NULL;
	}
	return 0;
}

_tstring CMsgLog::GetTableName(LPCTSTR lpszUserName)
{
	_tstring sLogTableName;
	sLogTableName.Format(2,_T("%s_%s"),lpszUserName,m_sUserName.c_str());

	MD5  md5;
	md5.feed(sLogTableName.toNarrowString( ).c_str( ));
	md5.finalize( );
	_tstring smd5;
	smd5.fromUTF8(md5.hex().c_str());
	sLogTableName.Format(2,_T("%s_%s"),DB_PREFIX,smd5.c_str());
    
    return sLogTableName;
}