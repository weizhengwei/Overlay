#pragma once 

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 PDL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// PDL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef PDL_EXPORTS
#define PDL_API extern "C" __declspec(dllexport)
#else
#define PDL_API extern "C" __declspec(dllimport)
#endif

// 定义PDL.dll的版本号
#define PDL_VERSION_INITIALIZE						_T("1")
#define PDL_VERSION_20130418						_T("2")
#define PDL_VERSION_CURRENT							PDL_VERSION_20130418

// 定义通知消息
#define	WM_PASSIVE_DOWNLOAD_NOTIFY_MSG				(WM_USER + 1890)	// 被动下载线程通知消息（wParam为线程索引,lParam为通知消息文本串）
#define	WM_ACTIVE_DOWNLOAD_NOTIFY_MSG				(WM_USER + 1891)	// 主动下载线程通知消息（同上）

#define	MAXCOUNT_PASSIVE_DOWNLOAD_THREAD			10					// 被动下载线程最大个数
#define	MAXCOUNT_ACTIVE_DOWNLOAD_THREAD				10					// 主动下载线程最大个数

// 定义任务状态
#define PDL_STATUS_INIT								0X00000000			// 初始状态
#define PDL_STATUS_DOWNLOAD_JSON					0X00000010			// 下载JSON文件
#define PDL_STATUS_PREPARE							0X00000020			// 准备运行环境（包括创建PDL文件，下载速度检测等）
#define PDL_STATUS_DOWNLOAD_INITPACKAGE				0X00000030			// 下载初始包
#define PDL_STATUS_UNCOMPRESS_INITPACKAGE			0X00000031			// 解压缩初始包
#define PDL_STATUS_DOWNLOAD_MINPACKAGE				0X00000050			// 下载最小包
#define PDL_STATUS_RUN								0X00000100			// 任务运行（指已经开始正常的边下边玩了）
#define PDL_STATUS_COMPLETED						0X00001000			// 任务已完成
#define PDL_STATUS_PAUSED							0X40000000			// 任务已暂停（该值为一个标志值，必须与前面的值一起使用，表明任务暂停在对应的阶段）
#define PDL_STATUS_STOPPED							0X80000000			// 任务已停止（该值为一个标志值，必须与前面的值一起使用，表明任务停止在对应的阶段）

// 定义配置ID
// 以下为文本值的配置
#define PDL_CFGID_TEXT_GAMEHOMEDIR					1					// 所有游戏的主目录（该值只能获取，不能设置）
// 以下为整型值的配置
#define PDL_CFGID_INT_BEGIN							1000				// 整型配置ID开始（系统保留）
#define PDL_CFGID_INT_PASSIVE_DOWNLOAD_THREADCOUNT	1001				// 被动下载线程数
#define PDL_CFGID_INT_ACTIVE_DOWNLOAD_THREADCOUNT	1002				// 主动下载线程数
#define PDL_CFGID_INT_ACTIVE_DOWNLOAD_MAXSPEED		1003				// 主动下载最大速度(0：自动限速(缺省值 )，1：不限速，其他则是限制速度值，KB/S)
#define PDL_CFGID_INT_PACKAGEDOWNLOADTHREADCOUNT	1004				// 初始包和最小包下载线程数
#define PDL_CFGID_INT_RUNFORMINPACKAGE				1005				// 为获取最小包信息运行

// 定义错误码
#define PDL_ERROR_SUCCESS											0		// 成功
#define PDL_ERROR_NEEDUPDATE										1		// PDL.dll需要更新

#define PDL_ERROR_DOWNLOADJSON_NETWORK								10		// 下载JSON文件失败，原因是网络故障
#define PDL_ERROR_DOWNLOADJSON_DISKFULL								11		// 下载JSON文件失败，原因是磁盘空间满
#define PDL_ERROR_DOWNLOADJSON_ACCESS_DENIED						12		// 下载JSON文件失败，原因是权限不够
#define PDL_ERROR_DOWNLOADJSON_DATAERROR							13		// 下载JSON文件失败，原因是文件数据有误

#define PDL_ERROR_CREATE_GAMEDIR_ACCESS_DENIED						20		// 创建游戏目录失败，原因是权限不够
#define PDL_ERROR_CREATE_GAMEDIR_INVALID_NAME						21		// 创建游戏目录失败，原因是目录名含有错误字符
#define PDL_ERROR_CREATE_GAMEDIR_TOOLONG							22		// 创建游戏目录失败，原因是目录名超长
#define PDL_ERROR_CREATE_GAMEDIR_PATH_NOT_FOUND						23		// 创建游戏目录失败，原因是上级目录不存在
#define PDL_ERROR_CREATE_GAMEDIR_INSUFFICIENT_DISK_SPACE			24		// 创建游戏目录失败，原因是磁盘空间不足

#define PDL_ERROR_REGISTERDRIVER_QUERYDOSDEVICE						30		// 注册驱动失败之查询dos device失败
#define PDL_ERROR_REGISTERDRIVER_COMMUNICATION						31		// 注册驱动失败之连接通信端口失败
#define PDL_ERROR_REGISTERDRIVER_SENDMESSAGE						32		// 注册驱动失败之向端口发送消息失败
#define PDL_ERROR_REGISTERDRIVER_INSTANCE_ALTITUDE_COLLISION		33		// 注册驱动失败之该盘上已指定了相同attitude的Pwepdl

#define PDL_ERROR_INIT_FILEMANAGE_JSONDATA							40		// 初始化文件管理模块失败，原因是JSON文件中的数据不对，建议清除所有数据从头开始
#define PDL_ERROR_INIT_FILEMANAGE_DISKFULL							41		// 初始化文件管理模块失败，原因是磁盘空间满
#define PDL_ERROR_INIT_FILEMANAGE_ACCESS_DENIED						42		// 初始化文件管理模块失败，原因是权限不够
#define PDL_ERROR_INIT_FILEMANAGE_PDLDATA_CHECKFILE					43		// 初始化文件管理模块失败，原因是装载PDL数据校验文件失败，建议清除所有数据从头开始

#define PDL_ERROR_DOWNLOAD_INITPACKAGE_NETWORK						50		// 下载初始包失败，原因是网络故障（此时回调函数如果返回TRUE则继续重试，否则退出）
#define PDL_ERROR_DOWNLOAD_INITPACKAGE_DISKFULL						51		// 下载初始包失败，原因是磁盘空间满
#define PDL_ERROR_DOWNLOAD_INITPACKAGE_ACCESS_DENIED				52		// 下载初始包失败，原因是权限不够

#define PDL_ERROR_UNCOMPRESS_INITPACKAGE_DATAERROR					60		// 解压缩初始包失败，原因是数据不正确
#define PDL_ERROR_UNCOMPRESS_INITPACKAGE_DISKFULL					61		// 解压缩初始包失败，原因是磁盘空间满
#define PDL_ERROR_UNCOMPRESS_INITPACKAGE_ACCESS_DENIED				62		// 解压缩初始包失败，原因是权限不够

#define PDL_ERROR_DOWNLOAD_MINPACKAGE_NETWORK						70		// 下载最小包失败，原因是网络故障（此时回调函数如果返回TRUE则继续重试，否则退出）
#define PDL_ERROR_DOWNLOAD_MINPACKAGE_DISKERROR						71		// 下载最小包失败，原因是磁盘错误
#define PDL_ERROR_DOWNLOAD_MINPACKAGE_CHECKFAILED					72		// 下载最小包失败，原因是数据校验失败

// 定义DllPDLInitialize的返回值
#define PDL_INIT_ERR_SUCCESS										0					// 成功
#define PDL_INIT_ERR_PDLDIR											1					// PDL目录不存在或无法创建
#define PDL_INIT_ERR_PDLDIR_ACCESS_DENIED							2					// PDL目录权限不够（无法创建配置文件等）
#define PDL_INIT_ERR_LOADDRIVER_ALREADY_EXISTS						20					// 加载驱动失败，原因是已加载过
#define PDL_INIT_ERR_LOADDRIVER_FILE_NOT_FOUND						21					// 加载驱动失败，原因是驱动文件不存在
#define PDL_INIT_ERR_LOADDRIVER_BAD_DRIVER							22					// 加载驱动失败，原因是驱动文件不正确
#define PDL_INIT_ERR_LOADDRIVER_INVALID_IMAGE_HASH					23					// 加载驱动失败，原因是驱动文件签名错误、或者未签名
#define PDL_INIT_ERR_LOADDRIVER_ACCESS_DENIE						24					// 加载驱动失败，原因是权限不够
#define PDL_INIT_ERR_LOADDRIVER_UNKOWN_ERROR						25					// 加载驱动失败，原因是未知错误

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 定义游戏任务接口类
class IGameTask
{
public:

	// 获取游戏名称
	virtual LPCTSTR GetGameName() = 0;

	// 获取任务状态，返回值见宏定义，如：PDL_STATUS_RUNNING
	virtual DWORD GetStatus() = 0;

	// 查询任务是否正在运行（注意：暂停状态也属于运行状态）
	virtual BOOL IsRunning() = 0;

	// 查询任务是否处于暂停状态
	virtual BOOL IsPaused() = 0;

	// 查询任务是否可以被暂停（在准备运行环境完成后才能被暂停）
	virtual BOOL CanBePaused() = 0;

	// 查询任务是否已经完成
	virtual BOOL IsCompleted() = 0;

	// 获取任务当前状态的进度，返回值为0-10000则表示正确的进度，-1表示没有对应的进度信息，否则为错误。
	// bCombineBeforeRun：为TRUE表示合并初始状态到运行前状态，此时将初始状态到运行前的所有状态合并为一个进度
	// 为FALSE时则是当前状态的进度
	virtual int GetProgress(__in BOOL bCombineBeforeRun = TRUE) = 0;

	// 获取剩余秒数（计算到启动游戏进程时），为MAXDWORD时则表示无效
	virtual DWORD GetRemainSeconds() = 0;

	// 获取下载速度（单位KB/S）
	virtual DWORD GetDownloadSpeed() = 0;

	// 设置自定义数据
	virtual void SetCustomData(__in DWORD dwCustomData) = 0;

	// 获取自定义数据
	virtual DWORD GetCustomData() = 0;

	// 保留接口
	virtual DWORD Res1() = 0;
	virtual DWORD Res2(__in DWORD dwRes) = 0;
	virtual DWORD Res3(__in DWORD dwRes1, __in DWORD dwRes2) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 定义回调函数，BOOL PDLCallback(...)
// pIGameTask：游戏任务接口指针，lpCaller:调用者指针，该值为DllPDLInitialize中传入的对应值，方便调用者使用
// dwType：类型，见宏定义，如：PDL_CBTYPE_STATUS_CHANGED，dwValue：对应类型下的值。
// 返回值只对特定类型有效，具体见类型定义说明
typedef BOOL (*FNPDLCALLBACK)(__in IGameTask *pIGameTask, __in LPVOID lpCaller, __in DWORD dwType, __in DWORD dwValue);
// 定义类型
#define PDL_CBTYPE_ERROR								0				// 错误，此时dwValue为错误码，见宏定义，如：PDL_ERROR_SUCCESS
#define PDL_CBTYPE_STATUS_CHANGED						1				// 状态改变，此时dwValue为状态值
#define PDL_CBTYPE_PREPARING							10				// 正在准备运行环境，此时dwValue无效
#define PDL_CBTYPE_DOWNLOADING_INITPACKAGE				20				// 正在下载初始包，此时dwValue为进度值（0-10000（转换为百分比时有两位小数），以下同）
#define PDL_CBTYPE_UNCOMPRESSING_INITPACKAGE			21				// 正在解压缩初始包，此时dwValue为进度值
#define PDL_CBTYPE_DOWNLOADING_MINPACKAGE				30				// 正在下载最小包，此时dwValue为进度值
#define PDL_CBTYPE_NEEDLAUNCHGAME						50				// 需要启动游戏，此时dwValue为LPCTSTR(待启动的可执行文件全路径)，临时有效
#define PDL_CBTYPE_RUNNING								80				// 正在运行，此时dwValue为进度值
#define PDL_CBTYPE_COMPLETED							100				// 已完成
#define PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_BEGIN		200				// 被动下载开始，此时dwValue为预计需要现在的时间，单位：毫秒（暂未提供）
#define PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_END			201				// 被动下载结束（暂未提供）
#define PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_FAILED		210				// 被动下载失败，原因是网络故障，此时返回值有效，如果返回TRUE则表示请求重下，否则退出下载；此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_DISKERROR	211				// 被动下载失败，原因是磁盘错误，此时返回值有效，如果返回TRUE则表示请求重试，否则退出；此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_NETWORK_PASSIVE_DOWNLOAD_CHECKFAILED	212				// 被动下载失败，原因是数据校验失败，此时返回值有效，如果返回TRUE则表示请求重下，否则退出下载；此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_FAILED		310				// 主动下载失败，原因是网络故障，此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_DISKERROR	311				// 主动下载失败，原因是磁盘错误，此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_NETWORK_ACTIVE_DOWNLOAD_CHECKFAILED	312				// 主动下载失败，原因是数据校验失败，此时dwValue为LPCTSTR（错误的调试信息）
#define PDL_CBTYPE_PASSIVE_DOWNLOAD_REPORT				400				// 被动下载信息上报，此时dwValue为LPCTSTR（上报文本，临时有效）

#define PROGRESS_VALUE_ZERO (0)
#define PROGRESS_VALUE_MAX  (10000)

// 根据当前值和总值制作进度值
#define MAKE_PROGRESS_VALUE(nCurValue, nTotalValue) (int)((double(nCurValue) / double(nTotalValue)) * double(PROGRESS_VALUE_MAX))


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 初始化，lpszPDLDir：PDL文件目录，如果为NULL，则使用当前调用者所在的目录，
// fnPDLCallback：回调函数指针，为NULL则无效
// lpCaller：调用（即调用DllPDLInitialize函数）者（通常为类对象）指针，如果fnPDLCallback不为NULL则不能为NULL
// hNotifyWnd：接收通知消息的句柄（用于调试信息的输出），具体消息信息见宏定义
// 返回值见宏定义，如：PDL_INIT_ERR_SUCCESS
PDL_API DWORD DllPDLInitialize(__in LPCTSTR lpszPDLDir, __in FNPDLCALLBACK fnPDLCallback, __in LPVOID lpCaller, __in HWND hNotifyWnd = NULL);
typedef DWORD (*FNDLLPDLINITIALIZE)(__in LPCTSTR lpszPDLDir, __in FNPDLCALLBACK fnPDLCallback, __in LPVOID lpCaller, __in HWND hNotifyWnd);

// 反初始化
PDL_API void DllPDLUninitialize();
typedef void (*FNDLLPDLUNINITIALIZE)();

// 创建游戏任务，lpszJsonFilename：任务对应的Json文件名（不含路径），该值为任务的唯一区分
// lpszJsonURL：Json文件对应的下载地址，当Json文件不存在或者需要更新时会根据此URL下载
// lpszGameDir：游戏保存的目录
// ppIGameTask：保存创建的任务对象指针
// 返回FALSE则表示创建失败
PDL_API BOOL DllPDLCreateGameTask(__in LPCTSTR lpszJsonFilename, __in LPCTSTR lpszJsonURL, __in LPCTSTR lpszGameDir, __out IGameTask **ppIGameTask);
typedef DWORD (*FNDLLPDLCREATEGAMETASK)(__in LPCTSTR lpszJsonFilename, __in LPCTSTR lpszJsonURL, __in LPCTSTR lpszGameDir, __out IGameTask **ppIGameTask);

// 销毁游戏任务，ppIGameTask：创建的任务对象指针的指针
PDL_API void DllPDLDestroyGameTask(__in IGameTask **ppIGameTask);
typedef void (*FNDLLPDLDESTROYGAMETASK)(__in IGameTask **ppIGameTask);

// 启动游戏任务，pIGameTask：指定的游戏任务接口指针
// 该函数是个异步函数，通过回调函数PDLCallback可获取相关信息
PDL_API void DllPDLStartGameTask(__in IGameTask *pIGameTask);
typedef void (*FNDLLPDLSTARTGAMETASK)(__in IGameTask *pIGameTask);

// 停止游戏任务，pIGameTask：指定的游戏任务接口指针，如果为NULL，则为当前正在运行的游戏任务
PDL_API void DllPDLStopGameTask(__in IGameTask *pIGameTask);
typedef void (*FNDLLPDLSTOPGAMETASK)(__in IGameTask *pIGameTask);

// 暂停游戏任务，pIGameTask：指定的游戏任务接口指针，如果为NULL，则为当前正在运行的游戏任务
PDL_API void DllPDLSuspendGameTask(__in IGameTask *pIGameTask);
typedef void (*FNDLLPDLSUSPENDGAMETASK)(__in IGameTask *pIGameTask);

// 继续游戏任务，pIGameTask：指定的游戏任务接口指针
PDL_API void DllPDLResumeGameTask(__in IGameTask *pIGameTask);
typedef void (*FNDLLPDLRESUMEGAMETASK)(__in IGameTask *pIGameTask);

// 清理游戏任务，将删除该任务相关数据，如果bIncludeGameFiles为TRUE则还删除游戏包含的文件，如果任务正在运行则返回FALSE
PDL_API BOOL DllPDLCleanGameTask(__in IGameTask *pIGameTask, __in BOOL bIncludeGameFiles = FALSE);
typedef BOOL (*FNDLLPDLCLEANGAMETASK)(__in IGameTask *pIGameTask, __in BOOL bIncludeGameFiles);

// 检测最大下载速度，返回每秒平均字节数，为MAXDWORD则是测测失败，可能是URL有误等
// nDetectTime：检测时间（单位秒），lpszDetectURL：检测使用的URL，为NULL时系统直接使用配置文件中的信息，nDetectThreadCount：检测时同时发起的下载线程数
PDL_API DWORD DllPDLDetectMaxDownloadSpeed(__in int nDetectTime = 10, __in LPCTSTR lpszDetectURL = NULL, __in int nDetectThreadCount = 5);
typedef DWORD (*FNDLLPDLDETECTMAXDOWNLOADSPEED)(__in int nDetectTime, __in LPCTSTR lpszDetectURL, __in int nDetectThreadCount);

// 获取配置值，dwConfigID为配置ID，见宏定义，如：PDL_CFGID_GAMEHOMEDIR
// lpReturnedValue：接收文本配置值，如果配置为整型值则必须传入NULL
// dwSize：如果lpReturnedValue为NULL，则忽略，否则为lpReturnedValue的空间大小
// 当配置值为整形时直接返回配置值，否则返回配置值文本的长度
PDL_API int DllPDLGetConfigValue(__in DWORD dwConfigID, __out LPTSTR lpReturnedValue, __in DWORD dwSize);
typedef int (*FNDLLPDLGETCONFIGVALUE)(__in DWORD dwConfigID, __out LPTSTR lpReturnedValue, __in DWORD dwSize);

// 设置配置值，dwConfigID为配置ID，见宏定义，如：PDL_CFGID_GAMEHOMEDIR
// nValue：当配置值为整形时，该变量为配置值，否则无效
// lpValue：当配置值为整形时，该变量必须为NULL，否则为文本配置值
// 成功则返回TRUE，失败则返回FALSE
PDL_API BOOL DllPDLSetConfigValue(__in DWORD dwConfigID, __in int nValue, __in LPCTSTR lpValue);
typedef BOOL (*FNDLLPDLSETCONFIGVALUE)(__in DWORD dwConfigID, __in int nValue, __in LPCTSTR lpValue);
