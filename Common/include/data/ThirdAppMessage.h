#pragma  once
#include "data/IThirdApp.h"
#include "tstring/tstring.h"
//////////////////////////////////////////////////////////////////////////
enum ThirdAppMsgType
{
	TAMT_InvalideMsg = 0,
	TAMT_QueryToken,
	TAMT_QueryCookie,
	TAMT_QueryIsUrlInfo,
	TAMT_QueryUrlRelatedApp,
	TAMT_SendArcMessageToGame,
};

//////////////////////////////////////////////////////////////////////////
class ThirdAppMsgHeader
{
public:
	~ThirdAppMsgHeader(){}

	virtual ThirdAppMsgType getMsgType() = 0;
};

//////////////////////////////////////////////////////////////////////////
template<typename ReturnType,ThirdAppMsgType msgType=TAMT_InvalideMsg>	
class ThirdAppMessageComm : public ThirdAppMsgHeader
{
public:
	ThirdAppMessageComm(ReturnType type) : m_retVal(type){}
	virtual ThirdAppMsgType getMsgType(){return msgType;}
	virtual ReturnType& GetRetValue(){return m_retVal;}
	virtual void SetRetValue(const ReturnType& value){m_retVal=value;}
private:
	ReturnType m_retVal;
};

#define DEF_MESSAGE_PARAM_0(MsgType, MsgName, RetType) \
	typedef ThirdAppMessageComm<RetType, MsgType> MsgName;

#define DEF_MESSAGE_PARAM_1(MsgType, MsgName,Param1T, Param1Name, RetType) \
class MsgName : public ThirdAppMessageComm< RetType, MsgType >	  \
{\
public:\
	MsgName(Param1T p1, RetType defaultRet):ThirdAppMessageComm(defaultRet),m_param1(p1){}\
	virtual ~##MsgName(){}\
	virtual Param1T Get##Param1Name(){return m_param1;}\
private:\
	Param1T m_param1;\
};

#define DEF_MESSAGE_PARAM_2(MsgType, MsgName, Param1T, Param1Name,Param2T, Param2Name,RetType)  \
class MsgName : public ThirdAppMessageComm< RetType, MsgType >	\
{\
public:\
	MsgName(Param1T p1, Param2T p2,RetType defaultRet):ThirdAppMessageComm(defaultRet),m_param1(p1), m_param2(p2){}\
	virtual ~##MsgName(){}\
	virtual  Param1T Get##Param1Name(){return m_param1;}\
	virtual  Param2T Get##Param2Name(){return m_param2;}\
private:\
	 Param1T m_param1;\
	 Param2T m_param2;\
};

class IThirdApp;

//DEF_MESSAGE_PARAM_0(TAMT_QueryToken, QueryTokenMsg, bool)
DEF_MESSAGE_PARAM_1(TAMT_QueryToken, QueryTokenMsg, DWORD, GameId, bool)
DEF_MESSAGE_PARAM_1(TAMT_QueryCookie,QueryCookieMsg, core_msg_cookie*, CoreCookieMsg, bool);
DEF_MESSAGE_PARAM_1(TAMT_QueryIsUrlInfo, QueryIsUrlInfoMsg, _tstring, Url, bool);
DEF_MESSAGE_PARAM_1(TAMT_QueryUrlRelatedApp, QueryUrlRelatedAppMsg, _tstring, Url, IThirdApp*)
DEF_MESSAGE_PARAM_1(TAMT_SendArcMessageToGame, SendArcMessageToGameMsg, core_msg_header*, ArcMsg, bool)
