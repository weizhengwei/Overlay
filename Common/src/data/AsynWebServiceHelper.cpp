#include "stdafx.h"
#include "data/WebService.h"
#include "data/AsynWebServiceHelper.h"




DWORD WINAPI WebServiceWorkThread(LPVOID lpThis);

AsynWebServiceHelper&	AsynWebServiceHelper::getInstance() {
	
	static AsynWebServiceHelper sHelper;

	return sHelper;
}

AsynWebServiceHelper::Response AsynWebServiceHelper::MakeResponse(WPARAM wparam, LPARAM lParam) {
	Response* pDeleteResponse = reinterpret_cast<Response*>(wparam);
	Response response = *pDeleteResponse;

	delete pDeleteResponse;

	return response;
}

AsynWebServiceHelper::AsynWebServiceHelper() 
	:_hThread(NULL){
	
}

bool AsynWebServiceHelper::Initilize() {
	if(_hThread != NULL) {
		//ASSERT(FALSE);
		return NULL;
	}

	_hThread = CreateThread(NULL, 0, WebServiceWorkThread, NULL, 0, 0);

	return _hThread != NULL;
}

bool AsynWebServiceHelper::Uninitilize() {
	if(_hThread != NULL) {
	/* 	TerminateThread(_hThread, 0);
		_hThread = NULL;

		_taskQueque.destroy();*/
	}

	return true;
}

bool AsynWebServiceHelper::SendRequest(REQUEST_METHOD method, const Request& req, DWORD dwTimeOut) {
	
	if (_hThread == NULL){
		OutputDebugString(_T("Error : Helper::SendRequest you must initilize AsynWebServiceHelper first"));
		return false;
	}

	if (method < 0 || method >= REQUEST_METHOD_END) {
		OutputDebugString(_T("Error : Helper::SendRequest method not invalid"));
		return false;
	}
	WebRquestTask* pTask = new WebRquestTask;
	pTask->eMothod = method;
	pTask->request = req;
	pTask->dwTimeOut = dwTimeOut;

	_taskQueque.enter(pTask);

	return true;
}

bool AsynWebServiceHelper::_doHttpRequest(const WebRquestTask* pTask, CWebService::_RESULT_DATA* lpResult) {

	bool bRet = false;

	CWebService  service;

	switch (pTask->eMothod) {
		case HTTP_GET:
			bRet = service.GetFieldService(pTask->request.data.sUrl, pTask->request.data.sCookie, pTask->request.data.sRequestData, lpResult, pTask->dwTimeOut);
			break;
		case HTTP_POST:
			bRet = service.PostFieldService(pTask->request.data.sUrl, pTask->request.data.sCookie, pTask->request.data.sRequestData, lpResult,pTask->dwTimeOut);
			break;
		case HTTPS_GET:
			bRet = service.HttpsGetService(pTask->request.data.sUrl, pTask->request.data.sCookie,pTask->request.data.sRequestData, lpResult);
			break;
		case HTTPS_POST:
			bRet = service.HttpsPostService(pTask->request.data.sUrl,pTask->request.data.sCookie, pTask->request.data.sRequestData, lpResult);
			break;
		default:
			OutputDebugString(_T("Error : Helper::WebServiceWorkThread unhandled task.eMothond"));
			break;
	}

	return bRet;
}

bool AsynWebServiceHelper::_protectedHttpRequest(const WebRquestTask* pTask, CWebService::_RESULT_DATA* lpResult)
{
	bool bRet = false;
	__try {

		bRet = _doHttpRequest(pTask, lpResult);

	} __except(EXCEPTION_EXECUTE_HANDLER) { 

		_reportSHEError(pTask);

	}

	return bRet;	
}

void  AsynWebServiceHelper::_reportSHEError(const WebRquestTask* pTask)
{
	_tstring sErrorMsg;
	sErrorMsg.Format(5, _T("Error : catch an SHE exception in %s(), URL: %s, method type : %d, cookie : %s, request data: %s\n"),
		_tstring(__FUNCTION__).c_str(), pTask->request.data.sUrl.c_str(), pTask->eMothod, 
		pTask->request.data.sCookie.c_str(), pTask->request.data.sRequestData.c_str());

	OutputDebugString(sErrorMsg.c_str());
}

DWORD WINAPI AsynWebServiceHelper::WebServiceWorkThread(LPVOID lpParameter) {	

	do 
	{
		WebRquestTask* pTask = getInstance()._taskQueque.leave();
		
		try {
			
			CWebService::_RESULT_DATA result = {0, "", "", NULL};		
			
			bool bRet = _protectedHttpRequest(pTask, &result);
			
			Response* pResponse = new Response;
			pResponse->bSuccess = bRet;
			pResponse->dwRequestTag = pTask->request.dwReqTag;
			pResponse->sResponseData = result.sResult;

			if(!PostMessage(pTask->request.hTargetWnd, 
				pTask->request.uTargetMsg, reinterpret_cast<WPARAM>(pResponse), 0)) {
				
				delete pResponse;

			}
		} catch(std::bad_alloc& ) {
			OutputDebugString(_T("Error : AsynWebServiceHelper::WebServiceWorkThread occur bad alloc"));
		} catch(...)
		{
			delete pTask;
			throw;
		}

		delete pTask;

	} while (true);
	
	OutputDebugString(_T("Error : AsynWebServiceHelper::WebServiceWorkThread, this code cannot be run!!!"));

	return -1;
}

