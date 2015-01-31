#pragma once

#include "thread/WQueue.h"
#include "tstring/tstring.h"

class AsynWebServiceHelper {
	public:
		struct RequestData {
			_tstring sUrl;
			_tstring sCookie;
			_tstring sRequestData;
		};

		struct Response {
			BOOL		bSuccess;
			DWORD		dwRequestTag;
			_tstring	sResponseData;
		};

		struct Request {
			HWND	hTargetWnd;
			UINT	uTargetMsg;
			DWORD	dwReqTag;
			RequestData data;

			Request()
				:hTargetWnd(NULL), uTargetMsg(0), dwReqTag(0){
			}
		};

		enum REQUEST_METHOD{
			HTTP_GET, 
			HTTP_POST, 
			HTTPS_GET, 
			HTTPS_POST,
			REQUEST_METHOD_END
		};

	public: // static function 
		static AsynWebServiceHelper&	getInstance();
		static Response MakeResponse(WPARAM wparam, LPARAM lParam);

	public:
		bool Initilize();
		bool Uninitilize();

		bool SendRequest(REQUEST_METHOD method, const Request& req, DWORD dwTimeOut=0);	

	private:
		AsynWebServiceHelper();
		AsynWebServiceHelper(const AsynWebServiceHelper& );
		bool operator = (const AsynWebServiceHelper& );

	private:
		static DWORD WINAPI WebServiceWorkThread(LPVOID lpParameter);
	
	private:
		struct WebRquestTask {
			REQUEST_METHOD eMothod;
			DWORD dwTimeOut;
			Request request;
		};

		static bool _doHttpRequest(const WebRquestTask* pTask, CWebService::_RESULT_DATA* lpResult);
		static bool _protectedHttpRequest(const WebRquestTask* pTask, CWebService::_RESULT_DATA* lpResult);
		static void _reportSHEError(const WebRquestTask* pTask);
	private:

		typedef WQueue<WebRquestTask*> TaskQueue;
		
		TaskQueue	_taskQueque;
		HANDLE		_hThread;
};

