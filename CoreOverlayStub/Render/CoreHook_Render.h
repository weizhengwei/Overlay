#pragma once




class DirectXRender
{
public:
	DirectXRender(LPVOID pDevice);
    virtual ~DirectXRender(void);

    virtual void Render(bool Resize) = 0;
    virtual HWND GetRenderHwnd() = 0;
    virtual int  GetRenderWidth() = 0;
    virtual int  GetRenderHeight() = 0;


    virtual void Init(){};
    virtual void Uninit(){};

public:
    LPVOID m_pDevice;
	LPVOID m_pOSTexture;

};





class DirectXRenderFactory
{
public:
    static DirectXRender* CreateDirectXRender(DWORD dwVer, LPVOID pDevice);
};