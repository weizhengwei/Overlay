#pragma once

#include <SDKDDKVer.h>
#include <atlbase.h>
#include <string>
#include <tchar.h>
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "unzip.h"
#include "tstring/tstring.h"
using namespace String ;

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192*8)
#define MAXFILENAME (256)

//#ifdef _WIN32
//#define USEWIN32IOAPI
//#include "iowin32.h"
//#endif

const INT32 ERROR_UNZIP     = -1;
const INT32 UNZIP_COMPLETE = 0xEFFFFFFF;
const INT32 UNZ_STOP        = -200;
typedef struct ParamData{
    const char* write_filename;
}*LPParamData;

//nProgress£º0-10000£¬UNZIP_COMPELETE
typedef void (*ZIP_CALLBACK)(INT32 nProgress, LPVOID lpCaller);

#ifdef CC_UNZIPF_EXPORTS
#define CC_UNZIPF __declspec(dllexport)
#else
#define CC_UNZIPF __declspec(dllimport)
#endif

class CC_UNZIPF CUnZip
{
public:
    CUnZip(void);
    ~CUnZip(void);
public:
    /*
    *input:  zipfile--    .zip file name contain the absolute path,
             unzip_path-- the absolute path unzip to.
    *return: 0---ok, non-zero---error.
    */
    int unzip(const TCHAR* zipfile, const TCHAR* unzip_path, ZIP_CALLBACK callback = NULL, LPVOID lpCaller = NULL);
    BOOL stop_unzip();
    BOOL pause_unzip();
    BOOL restart_unzip();
private:
    void change_file_date(const char* filename, uLong dosdate, tm_unz tmu_date);
    int mymkdir(_tstring dirname);
    int do_extract_currentfile(unzFile uf,const int* popt_extract_without_path, int* popt_overwrite, const char* password);
    int do_extract(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* password);
    int do_extract_onefile(unzFile uf,const char* filename,int opt_extract_without_path,int opt_overwrite,const char* password);
    wchar_t* utf2unicode(const char *szU8, wchar_t *dst, int len);
# ifdef _WIN32
    DWORD WINAPI io_thread(LPVOID);
# endif
private:
    ZIP_CALLBACK unzip_callback_progress;
    LPVOID m_lpCaller;
    ZPOS64_T total_file_num;
    ZPOS64_T unziped_file_num;
    ZPOS64_T zipfile_size;
    ZPOS64_T cur_zipfile_size;
    int progress;
    Bytef buf[WRITEBUFFERSIZE];
    unzFile zip_fd;
    volatile BOOL stop_flag;
    volatile BOOL pause_flag;
};
