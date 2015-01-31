// HttpDownloader 接口头文件
#pragma once 
#include <Windows.h>
#include <vector>

using namespace std;

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 HTTPDOWNLOADER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// HTTPDOWNLOADER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef HTTPDOWNLOADER_EXPORTS
#define HTTPDOWNLOADER_API EXTERN_C __declspec(dllexport)
#else
#define HTTPDOWNLOADER_API EXTERN_C __declspec(dllimport)
#endif

// 定义任务状态
typedef enum
{
	HD_TASK_STATUS_INIT = 0,				// 初始状态
	HD_TASK_STATUS_CONNECTING,				// 正在连接
	HD_TASK_STATUS_DOWNLOADING,				// 正在下载（只有该状态才能被暂停）
	HD_TASK_STATUS_SUSPENDED,				// 已暂停
	HD_TASK_STATUS_STOPPING,				// 正在停止
	HD_TASK_STATUS_STOPPED,					// 已停止
	HD_TASK_STATUS_COMPLETED,				// 已完成
	HD_TASK_STATUS_ERROR					// 错误状态，此时可通过函数GetErrorNumber获取错误码
}HDTaskStatus;

// 定义错误码
typedef enum
{
	HD_ERROR_SUCCESS = 0,					// 成功
	HD_ERROR_INVALID_URL,					// URL错误
	HD_ERROR_CANNOT_CONNECT,				// 连接不上下载服务器
	HD_ERROR_AUTH_CERT_NEEDED,				// 需要认证
	HD_ERROR_LOGIN_FAILURE,					// 登录错误
	HD_ERROR_SERVER_FILENOTFOUND,			// 文件不存在
	HD_ERROR_SERVER_FILEMODIFIED,			// 文件已改变（比如文件大小、时间与以前下载保存的不一样）
	HD_ERROR_SERVER_FORBIDDEN,				// 不允许访问
	HD_ERROR_SERVER_TIMEOUT,				// 超时（并超过重试次数）
	HD_ERROR_FILE_INSUFFICIENTDISKSPACE,	// 磁盘空间不够
	HD_ERROR_FILE_ACCESS_DENIED,			// 权限不够，无法访问保存文件
	HD_ERROR_FILE_DISK_ERROR,				// 磁盘错误，无法访问保存文件
	HD_ERROR_FILE_MD5FILENOTFOUND,			// MD5文件未找到
	HD_ERROR_FILE_MD5FILEERROR,				// MD5文件数据错误
	HD_ERROR_MEMORY_INSUFFICIENT,			// 内存空间不够
	HD_ERROR_THREAD_CREATEFAILED,			// 创建新线程失败
	HD_ERROR_NAME_NOT_RESOLVED=20,			// 域名解析失败（因为以前HD_ERROR_DEBUG用的是16，所以这里设置为20跳过该值以避免和以前的值混淆，2013-08-27）

	HD_ERROR_DEBUG=99,						// 获取错误调试信息（由16改为99）

	HD_ERROR_LAST							// 最后错误码（仅供系统使用）
}HDErrorNumber;

#define WM_HTTPDOWN_NOTIFY		WM_USER + 97				// 通知消息ID，用于调试

// 定义HTTP选项值
typedef enum 
{
	HD_HTTP_OPTION_USERNAME = 0,			// 登录用户名，TCHAR[]，缺省为空串
	HD_HTTP_OPTION_PASSWORD,				// 登录用密码，TCHAR[]，缺省为空串
	HD_HTTP_OPTION_PROXY,					// 代理信息，WINHTTP_PROXY_INFO
	HD_HTTP_OPTION_PROXY_USERNAME,			// 代理用户名，TCHAR[]，缺省为空串
	HD_HTTP_OPTION_PROXY_PASSWORD,			// 代理密码，TCHAR[]，缺省为空串
	HD_HTTP_OPTION_USER_AGENT,				// UserAgent信息，TCHAR[]，缺省为空串
	HD_HTTP_OPTION_CONTENT_MD5,				// 请求MD5值，类型待定（暂不提供）
	HD_HTTP_OPTION_CONTENT_ENCODING,		// 下载内容编码，值可用"gzip"，TCHAR[]，缺省为空串

	HD_HTTP_OPTION_COUNT					// 选项总数（仅供系统使用）
}HDHttpOption;

// 定义缺省值
#define DEFAULT_IGNORE					MAXDWORD				// 缺省忽略值
#define DEFAULT_TIMEOUT_CONNECT			(30 * 1000)				// 缺省的连接超时值单位：毫秒
#define DEFAULT_TIMEOUT_SEND			(30 * 1000)				// 缺省的发送超时值单位：毫秒
#define DEFAULT_TIMEOUT_RECEIVE			(30 * 1000)				// 缺省的接收超时值单位：毫秒
#define DEFAULT_RETRY_TIMES				10						// 缺省的重试次数值
#define DEFAULT_RETRY_DELAYTIME			5000					// 缺省的重试时间间隔值（单位：毫秒）

// ===========================================================================================================
// 任务接口类
// ===========================================================================================================
class IHttpDownTask
{
public: // 设置接口，需要在开始下载前进行设置

	// 设置下载限速，dwLimitSpeed：下载限速（单位：字节/秒），MAXDWORD为不限速
	virtual void SetLimitSpeed(__in DWORD dwLimitSpeed) = 0;
	// 获取当前限速值，（单位：字节/秒），0为不限速
	virtual DWORD GetLimitSpeed() = 0;

	// 设置相关超时值（单位：毫秒），dwConnectTimeout：连接超时值，dwSendTimeout发送超时值，dwReceiveTimeout：接收超时值；各值为DEFAULT_IGNORE则表示不修改当前的设置值
	virtual void SetTimeouts(__in DWORD dwConnectTimeout = DEFAULT_TIMEOUT_CONNECT, __in DWORD dwSendTimeout = DEFAULT_TIMEOUT_SEND, __in DWORD dwReceiveTimeout = DEFAULT_TIMEOUT_RECEIVE) = 0;
	// 获取超时设置，lpSendTimeout：保存发送超时值，lpReceiveTimeout：保存接收超时值，返回连接超时值
	virtual DWORD GetTimeouts(__out LPDWORD lpSendTimeout = NULL, __out LPDWORD lpReceiveTimeout = NULL) = 0;

	// 设置重试信息，dwRetryTimes：重试次数（为DEFAULT_IGNORE则不修改当前值），dwDelayTime：重试延迟时间（单位：毫秒，为DEFAULT_IGNORE则不修改当前值）
	virtual void SetRetryInfo(__in DWORD dwRetryTimes = DEFAULT_RETRY_TIMES, __in DWORD dwDelayTime = DEFAULT_RETRY_DELAYTIME) = 0;
	// 获取重试信息，lpDelayTime：保存重试延迟时间，返回重试次数
	virtual DWORD GetRetryInfo(__out LPDWORD lpDelayTime = NULL) = 0;

	// 设置HTTP选项值，option：选项定义，lpBuffer：选项值缓冲区（由选项定义确定类型），dwBufferLength：缓冲区长度
	virtual BOOL SetHttpOption(__in HDHttpOption option, __in LPVOID lpBuffer, __in DWORD dwBufferLength) = 0;
	// 获取HTTP选项值
	virtual BOOL GetHttpOption(__in HDHttpOption option, __in LPVOID lpBuffer, __inout LPDWORD lpdwBufferLength) = 0;

	// 设置下载线程个数，在调用Start后才生效
	virtual void SetThreadCount(__in DWORD dwThreadCount = 5) = 0;

	// 获取下载线程个数，bOnlyRunning：为TRUE时则表示获取正在运行的下载线程数，否则为设置的下载线程数
	virtual DWORD GetThreadCount(BOOL bOnlyRunning = FALSE) = 0;

public:	// 操作接口
	
	// 开始下载
	virtual void Start() = 0;
	// 停止下载
	virtual void Stop() = 0;
	// 暂停下载
	virtual void Suspend() = 0;
	// 继续下载
	virtual void Resume() = 0;

	// 清除下载信息（目前是删除idx文件），必须在停止状态才能调用该函数，清除以后再次开始下载时会从头下载
	// bDelDownloadFile：是否删除下载文件
	virtual void Clean(BOOL bDelDownloadFile = FALSE) = 0;

public:	// 获取信息接口

	// 获取任务名，对于单个文件的任务，则返回文件名（不含路径），对于目录任务，则返回目录名（不含上级目录）
	// 注意：任务名可能重复，即不同的任务可能有相同的任务名，所以不能作为任务的唯一区分
	virtual LPCTSTR GetTaskName() = 0;

	// 获取任务状态
	virtual HDTaskStatus GetStatus() = 0;
	// 获取进度值（0-10000，为MAXDWORD时表示当前无法计算进度，其他值为错误） 
	virtual DWORD GetProgress() = 0;
	// 获取剩余秒数，为MAXDORD时表示当前无法计算（比如暂停中）
	virtual DWORD GetRemainSeconds() = 0;
	// 获取当前下载速度，单位：字节/秒
	virtual DWORD GetDownloadSpeed() = 0;
	// 获取已下载大小
	virtual UINT64 GetDownloadedSize() = 0;
	// 获取总下载大小
	virtual UINT64 GetTotalSize() = 0;

	// 是否是需要MD5校验的任务
	virtual BOOL IsMD5Check() = 0;

	// 是否处于正在下载状态
	virtual BOOL IsDownloading() = 0;
	// 是否处于暂停状态
	virtual BOOL IsSuspended() = 0;
	// 是否处于停止状态
	virtual BOOL IsStopped() = 0;
	// 是否已完成下载
	virtual BOOL IsCompleted() = 0;

	// 查询当前状态下是否可以启动任务
	virtual BOOL CanStart() = 0;
	// 查询当前状态下是否可以停止任务
	virtual BOOL CanStop() = 0;
	// 查询当前状态下是否可以继续任务
	virtual BOOL CanResume() = 0;
	// 查询当前状态下是否可以暂停任务
	virtual BOOL CanSuspend() = 0;
	// 查询当前状态下是否可以清除任务数据
	virtual BOOL CanClean() = 0;

	// 获取错误码（见宏定义）
	virtual DWORD GetErrorNumber() = 0;

public:	// 用于单个文件下载的接口

	// 获取下载地址
	virtual LPCTSTR GetURL() = 0;
	// 获取保存文件路径（作为任务的唯一区分）
	virtual LPCTSTR GetSavePath() = 0;

public:	// 用于整个目录下载的接口

	// 获取下载基地址
	virtual LPCTSTR GetBaseURL() = 0;
	// 获取文件相对路径集合
	virtual vector<LPCTSTR>& GetFilePaths() = 0;
	// 获取保存目录（作为任务的唯一区分）
	virtual LPCTSTR GetSaveDir() = 0;

public:	// 自定义数据

	// 设置自定义数据
	virtual void SetCustomData(__in DWORD dwCustomData) = 0;

	// 获取自定义数据
	virtual DWORD GetCustomData() = 0;

public: // 调试信息接口

	// 设置通知消息窗口，hNotifyWnd：接收通知消息的句柄，nMessageID：消息ID，具体消息信息见宏定义
	// 如果hNotifyWnd为NULL或者nMessageID为WM_NULL，则取消通知
	virtual void SetNotifyWnd(__in HWND hNotifyWnd, __in UINT nMessageID = WM_HTTPDOWN_NOTIFY) = 0;
	
	// 根据错误码获取错误信息，dwErrorNumber：错误码（见宏定义），如果为HD_ERROR_LAST，则直接获取当前的错误信息
	virtual LPCTSTR GetErrorMsg(__in HDErrorNumber errorNumber = HD_ERROR_LAST) = 0;

public:	// 保留接口
	
	virtual DWORD Reserve1(DWORD dwRes) = 0;
	virtual DWORD Reserve2(DWORD dwRes, DWORD dwRes2) = 0;
	virtual DWORD Reserve3(DWORD dwRes, DWORD dwRes2, DWORD dwRes3) = 0;
};

// ===========================================================================================================
// 定义回调函数，BOOL HttpDownCallback(...)
// pIHttpDownTask：任务接口指针，lpCaller:调用者指针，该值为HttpDownInitialize中传入的对应值，方便调用者使用
// type：类型，见定义，如：HD_CBTYPE_STATUS_CHANGED，dwValue：对应类型下的值。
// 返回值只对特定类型有效，具体见类型定义说明
// ===========================================================================================================
// 定义类型
typedef enum
{
	HD_CBTYPE_ERROR = 0,					// 错误，此时dwValue为错误码，见定义，如：HD_ERROR_NETWORK_URL
	HD_CBTYPE_STATUS_CHANGED,				// 状态改变，此时dwValue为状态值
	HD_CBTYPE_PROGRESS_CHANGED,				// 进度值改变，此时dwValue为进度值
	HD_CBTYPE_COMPLETE						// 任务完成
}HDCallbackType;

typedef BOOL (*FNHTTPDOWNCALLBACK)(__in IHttpDownTask *pIHttpDownTask, __in LPVOID lpCaller, __in HDCallbackType type, __in DWORD dwValue);

// ===========================================================================================================
// 导出函数
// ===========================================================================================================
// 定义版本号，最高字节为接口类IHttpDownTask的版本，第二字节为导出函数版本，低字为功能版本
#define HTTPDOWN_VERSION_1								0X01010001							// 第一版，2013-05-23
#define HTTPDOWN_VERSION_CURRENT						HTTPDOWN_VERSION_1					// 当前版本

// 定义HttpDownCreateTask的返回值
#define HD_CREATETASK_ERROR_SUCCESS						0			// 成功
#define HD_CREATETASK_ERROR_ALREADYEXIST				10			// 任务已存在
#define HD_CREATETASK_ERROR_URL							20			// URL错误
#define HD_CREATETASK_ERROR_SAVEPATH					21			// 保存文件路径错误
#define HD_CREATETASK_ERROR_INSUFFICIENTMEMORY			30			// 内存不足，无法创建任务对象
#define HD_CREATETASK_ERROR_NOTINITIALIZED				40			// HttpDownloader未初始化

// 初始化，必须首先调用，返回FALSE则表示当前库的版本与所需的版本不匹配，不能调用
// dwHttpDownVersion：功能库的版本号，必须传入HTTPDOWN_VERSION_CURRENT，以便于确定是否可以使用当前库
// fnHttpDownCallback：回调函数指针，为NULL则无效
// lpCaller：调用（即调用HttpDownInitialize函数）者（通常为类对象）指针，如果fnHttpDownCallback不为NULL则不能为NULL
HTTPDOWNLOADER_API BOOL HttpDownInitialize(__in DWORD dwHttpDownVersion, __in FNHTTPDOWNCALLBACK fnHttpDownCallback, __in LPVOID lpCaller);
typedef BOOL (*FNHTTPDOWNINITIALIZE)(__in DWORD dwHttpDownVersion, __in FNHTTPDOWNCALLBACK fnHttpDownCallback, __in LPVOID lpCaller);

// 反初始化
HTTPDOWNLOADER_API void HttpDownUninitialize();
typedef void (*FNHTTPDOWNUNINITIALIZE)();

// 创建任务（下载单个文件），lpszURL：下载地址，lpszSavePath：保存路径
// ppIHttpDownTask：在创建成功或者任务已存在时保存对应的任务对象指针，其他则为NULL
// bMD5Check：是否使用MD5校验
// 返回值见宏定义，如：HD_CREATETASK_ERROR_SUCCESS，
HTTPDOWNLOADER_API DWORD HttpDownCreateTask(__in LPCTSTR lpszURL, __in LPCTSTR lpszSavePath, __out IHttpDownTask **ppIHttpDownTask, __in BOOL bMD5Check);
typedef DWORD (*FNHTTPDOWNCREATETASK)(__in LPCTSTR lpszURL, __in LPCTSTR lpszSavePath, __out IHttpDownTask **ppIHttpDownTask, __in BOOL bMD5Check);

// 创建任务（下载整个目录），lpszBaseURL：下载基地址，vctFilePaths：文件相对路径（与lpszBaseURL组合成一个具体的URL）集合，
// lpszSaveDir：保存目录，ppIHttpDownTask：保存创建的任务对象指针，返回值见宏定义
HTTPDOWNLOADER_API DWORD HttpDownCreateTaskEx(__in LPCTSTR lpszBaseURL, __in vector<LPCTSTR>vctFilePaths, __in LPCSTR lpszSaveDir, __out IHttpDownTask **ppIHttpDownTask);
typedef DWORD (*FNHTTPDOWNCREATETASKEX)(__in LPCTSTR lpszBaseURL, __in vector<LPCTSTR>vctFilePaths, __in LPCSTR lpszSaveDir, __out IHttpDownTask **ppIHttpDownTask);

// 销毁任务（注意：该函数并不删除下载的文件及IDX文件），ppIHttpDownTask：任务指针地址
HTTPDOWNLOADER_API void HttpDownDestoryTask(__in IHttpDownTask **ppIHttpDownTask);
typedef void (*FNHTTPDOWNDESTORYTASK)(__in IHttpDownTask **ppIHttpDownTask);

// 获取当前全局总下载速度，单位：字节/秒
HTTPDOWNLOADER_API DWORD HttpDownGetTotalSpeed();
typedef DWORD (*FNHTTPDOWNGETTOTALSPEED)();

// 设置全局下载限速，dwSpeed：速度值，单位：字节/秒，为MAXDWORD则表示不限速  
// 暂不提供该功能 
HTTPDOWNLOADER_API void HttpDownSetLimitSpeed(__in DWORD dwSpeed = MAXDWORD);
typedef void (*FNHTTPDOWNSETLIMITSPEED)(DWORD dwSpeed);

// 获取全局下载限速，返回速度值，单位：字节/秒，为MAXDWORD则表示不限速
HTTPDOWNLOADER_API DWORD HttpDownGetLimitSpeed();
typedef DWORD (*FNHTTPDOWNGETLIMITSPEED)();
