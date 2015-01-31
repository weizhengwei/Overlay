#pragma once

#include "im/im_ui_secondary.h"
#include "im/im_constant.h"
#include <list>
#include "tstring/tstring.h"
using std::list;

class COverlayImImpl
{
public:
	COverlayImImpl(void);
	~COverlayImImpl(void);

	typedef struct _IMMESSAGE_INFO
	{
		_tstring _szUserName; //friend's name 
		_tstring _szSenderNick;//sender's NikeName 
		int nTimeCur;          // insernt time
	}IMMESSAGE_INFO;

	static COverlayImImpl* GetInstance()
	{
		static COverlayImImpl Instance;
		return &Instance;
	}
	static CUIChatMgrSecondary* GetUIChatMgr()
	{
		static CUIChatMgrSecondary UIChatMgr;
		return &UIChatMgr;
	}
	BOOL InsertImMessage(_IMMESSAGE_INFO & ImMessage);
	im_dialog_params GetDefaultTabMgrParams();
	void RemoveImMessage(LPCTSTR lpUserName);
	void ShowFriendDlg(CRect &rc);
	void ShowStatusDlg(CRect &rc,BOOL bShow = TRUE);
	bool InitIM();
	void ShowOverlayChatDlg(BOOL bShow);
	//_ITEM_BASE_INFO GetUserInfo(){return m_UserInfo;}
	void ShowChatDlgForMsg(LPCTSTR lpUserName);
	
private:
	list<COverlayImImpl::_IMMESSAGE_INFO> m_ImMessage;
//	_ITEM_BASE_INFO m_UserInfo;
	
};
