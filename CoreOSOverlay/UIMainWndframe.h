#pragma once
#include "data/UIString.h"
#include "ui/CoreWidget.h"
#include <list>

#define  MAINLEFTBGWIDTH 480
#define  MAINBTN_X 21
#define  DEFAULTLISTTOP 112
#define  DEFAULLISTBOTTOM 78
#define  FRIENDLISTLEFT 181
#define  FRIENDLISTRIGHT 460
#define  RESOLUTION_SIGN 1024

class CGameToBackTip : public CWindowImpl<CGameToBackTip>
{
public:
	CGameToBackTip();
	~CGameToBackTip();
	DECLARE_WND_CLASS(_T("CGameToBackTip"))

	BEGIN_MSG_MAP_EX(CGameToBackTip)
		MSG_WM_CREATE(OnCreate)
		//	MSG_WM_SIZE(OnSize)
		//MSG_WM_TIMER(OnTimer)
		END_MSG_MAP()

		void SetStrText(_tstring &strText1,_tstring &strTex2);

protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

class core_msg_cookie;
class CUIMainWndframe
{
public:
	CUIMainWndframe(void);
	~CUIMainWndframe(void);
	void Init(HWND hwnd);
	void InitMainUI();
	//virtual void ShowBrowserMainWnd(LPCTSTR lpUrl) {};
	void OnNewsClick(ISonicBase *, LPVOID pReserve);
	void OnBillingClick(ISonicBase *, LPVOID pReserve);
	void OnSupportClick(ISonicBase *, LPVOID pReserve);
	void OnWebClick(ISonicBase *, LPVOID pReserve);
	void OnCommunityClick(ISonicBase *, LPVOID pReserve);
	void OnMediaClick(ISonicBase *, LPVOID pReserve);
	void OnSettingClick(ISonicBase *, LPVOID pReserve);
	void OnBacktoGame(ISonicBase*,LPVOID pReserve);
	void OnArcIcon(ISonicBase*,LPVOID pReserve);
	void OnClickBtnSetStatus(ISonicString *pStatus, LPVOID pReserve);
	void OnClickedBtnAvatar(ISonicString *pStatus, LPVOID pReserve);

	void HandleSelfAvatar(LPCTSTR lpPath);
	void HandleSelfPresence(UINT iStatus);
	void UpdateAvatarPaint(ISonicImage* pImg,UINT iStatus);
	void UpdateStatus(UINT iStatus);

	void OnBacktoGameMouseEnter(ISonicBase*,LPVOID pReserve);
	void OnBacktoGameMouseLeave(ISonicBase*,LPVOID pReserve);
	

	void SetSystemDate();
	void SetSystemTime();
	void SetStatus();

	void SetBackToGameTip();
	void ShowBacktoGameTip();
	void HideGameBackTip(){m_GameBackTip.ShowWindow(SW_HIDE);}
    void Resize();

private:
    void DrawLeftMainBg(ISonicPaint * pPaint, LPVOID);
    void CalMainBtn(int & nHeight,int & nGap,int &nTopPoint); 
    void IntiSatusStr();


    void InitCommonUI();
    void InitLowResolutionUI();
    void InitHighResolutionUI();
    void InitTextUI();
	void InitIMUI();

    _tstring GetToolTipText();

    void ShowByResolution(BOOL bLowResolution);

private:
	ISonicString *m_pSystemTime;
	ISonicString *m_pSystemDate;

	ISonicPaint* m_pAvatarPaint;	
	ISonicString * m_pBackToGame;
	ISonicImage* m_pAvatar;
	ISonicString* m_pStatusStr;
	ISonicString* m_pNickName;

    ISonicString * m_pBgBottom;
    ISonicString * m_pGameNameTitle;
    ISonicString * m_pGameName;
    
    // less than 1024
    ISonicString * m_pNewsBtn;
    ISonicString * m_pWeb;
    ISonicString * m_pCommunityBtn;
    ISonicString * m_pMedia;
    ISonicString * m_pChargeBtn;
    ISonicString * m_pSupportBtn;
    ISonicString * m_pBgIcon;

    
    //more than 1024
    ISonicString * m_pNewsBtn_big;
    ISonicString * m_pWeb_big;
    ISonicString * m_pCommunityBtn_big;
    ISonicString * m_pMedia_big;
    ISonicString * m_pChargeBtn_big;
    ISonicString * m_pSuportBtn_big;
    ISonicString * m_pSetting;
    ISonicString * m_pBgIcon_big;


    std::list<ISonicBaseWnd*> m_LowResolutionUI;
    std::list<ISonicBaseWnd*> m_HighResolutionUI;

    ISonicPaint  * m_pPaintMainBg;

    ISonicPaint *  m_pPaint;
    core_msg_cookie *m_pMsgCookie;
	CGameToBackTip m_GameBackTip;
	HWND m_hWnd;
	_tstring m_szStatus[9];
    _tstring m_szBackToGameTip;
    _tstring m_szNickName;
};

