#ifndef _H_SHARED_STRING_TSTRING
#define _H_SHARED_STRING_TSTRING

#include <string>
#include <list>
#include "SafeStrFunc.h"

namespace String
{

#define _tANSICHAR(x)       /*Share::*/String::_tstringA(x).c_str()
#define _tUNICODECHAR(x)    /*Share::*/String::_tstringW(x).c_str()
#define _tUTF8CHAR(x)       /*Share::*/String::_tstring(x).toUTF8().c_str()

class _tstringA;
class _tstringW;

class _tstringA : public std::string
{
public:
    typedef std::list<_tstringA> SplitList;

    _tstringA();
    virtual ~_tstringA();

public:
    _tstringA& fromUTF8(const char* pszUTF8);
    std::string toUTF8() const;

    _tstringA& fromBASE64(const char* pszBASE64);
    std::string toBASE64() const;

    _tstringA& toNarrowString() const;
    std::wstring toWideString() const;

public:
    _tstringA& MakeLower();
	_tstringA  MakeLowerConst()  const;
    _tstringA& MakeUpper();
	_tstringA  MakeUpperConst() const;

    int CompareNoCase(const _tstringA& str) const;

    unsigned int Replace(const _tstringA& strOld, const _tstringA& strNew);
    unsigned int Replace(wchar_t chOld, wchar_t chNew);
    unsigned int Replace(char chOld, char chNew);

    std::string Right(size_t nCount) const;
    std::string Left(size_t nCount) const;
    std::string Mid(size_t iFirst, size_t nCount) const;

    size_t Find(char ch, unsigned int nStart = 0) const;
    size_t Find(const char* pszStr, unsigned int nStart = 0) const;
    size_t Find(const _tstringA& str, unsigned int nStart = 0) const;

    size_t ReverseFind(char ch) const;

    SplitList Split(const _tstringA& strKeyword) const;

public:
    //实现sprintf功能
    void Format(uint32 uParamCnt, const char* pszFormat, ...);
    void Format(uint32 uParamCnt, const wchar_t* pwszFormat, ...);

public:

    //重载=号函数
    using std::string::operator =;

    _tstringA& operator = (char ch);
    _tstringA& operator = (wchar_t ch);
    _tstringA& operator = (const char* pszStr);
    _tstringA& operator = (const wchar_t* pszwStr);
    _tstringA& operator = (const _tstringA& str);
    _tstringA& operator = (const _tstringW& wstr);
    _tstringA& operator = (const std::string& str);
    _tstringA& operator = (const std::wstring& wstr);

public:

    _tstringA& operator = (int n);
    _tstringA& operator = (unsigned int n);
    _tstringA& operator = (long n);
    _tstringA& operator = (unsigned long n);
    _tstringA& operator = (double d);
    _tstringA& operator = (__int64 n);
    _tstringA& operator = (unsigned __int64 n);

public:

    //重载<<
    _tstringA& operator << (const _tstringA& str);

public:
    
    //数据类型转换
    int8 ConvToInt8() const;
    uint8 ConvToUInt8() const;
    int16 ConvToInt16() const;
    uint16 ConvToUInt16() const;
    Int32 ConvToInt32() const;
    uint32 ConvToUInt32() const;
    int64 ConvToInt64() const;
    uint64 ConvToUInt64() const;
    double ConvToDouble() const;
    float ConvToFloat() const;


    const _tstringA& operator >> (int8& n) const;
    const _tstringA& operator >> (uint8& n) const;
    const _tstringA& operator >> (int16& n) const;
    const _tstringA& operator >> (uint16& n) const;
    const _tstringA& operator >> (Int32& n) const;
    const _tstringA& operator >> (uint32& n) const;
    const _tstringA& operator >> (int64& n) const;
    const _tstringA& operator >> (uint64& n) const;
    const _tstringA& operator >> (int& n) const;
    const _tstringA& operator >> (double& n) const;
    const _tstringA& operator >> (float& n) const;

public:
    //构造函数
    _tstringA(wchar_t ch);
    _tstringA(char ch);
    _tstringA(const char* pszStr);
    _tstringA(const wchar_t* pwszStr);
    _tstringA(const _tstringA& str);
    _tstringA(const _tstringW& wstr);
    _tstringA(const std::string& str);
    _tstringA(const std::wstring& wstr);
    _tstringA(int n);
    _tstringA(unsigned int n);
    _tstringA(long n);
    _tstringA(unsigned long n);
    _tstringA(double d);
    _tstringA(__int64 n);
    _tstringA(unsigned __int64 n);
};

class _tstringW : public std::wstring
{
public:
    typedef std::list<_tstringA> SplitList;

    _tstringW();
    virtual ~_tstringW();

public:
    _tstringW& fromUTF8(const char* pszUTF8);
    std::string toUTF8() const;

    _tstringW& fromBASE64(const char* pszBASE64);
    std::string toBASE64() const;

    std::string toNarrowString() const;
    _tstringW& toWideString() const;

public:
    _tstringW& MakeLower();
	_tstringW  MakeLowerConst() const;
    _tstringW& MakeUpper();
	_tstringW  MakeUpperConst() const;

    int CompareNoCase(const _tstringW& str) const;

    unsigned int Replace(const _tstringW& strOld, const _tstringW& strNew);
    unsigned int Replace(wchar_t chOld, wchar_t chNew);
    unsigned int Replace(char chOld, char chNew);

    std::wstring Right(size_t nCount) const;
    std::wstring Left(size_t nCount) const;
    std::wstring Mid(size_t iFirst, size_t nCount) const;

    size_t Find(wchar_t ch, unsigned int nStart = 0) const;
    size_t Find(const wchar_t* pwszch, unsigned int nStart = 0) const;
    size_t Find(const _tstringW& str, unsigned int nStart = 0) const;

    size_t ReverseFind(wchar_t ch) const;

    SplitList Split(const _tstringW& strKeyword) const;

public:
    //实现sprintf功能
    void Format(uint32 uParamCnt, const char* pszFormat, ...);
    void Format(uint32 uParamCnt, const wchar_t* pwszFormat, ...);

public:

    //重载=号函数
    using std::wstring::operator =;

    _tstringW& operator = (char ch);
    _tstringW& operator = (wchar_t ch);
    _tstringW& operator = (const char* pszStr);
    _tstringW& operator = (const wchar_t* pwszStr);
    _tstringW& operator = (const _tstringA& str);
    _tstringW& operator = (const _tstringW& wstr);
    _tstringW& operator = (const std::string& str);
    _tstringW& operator = (const std::wstring& wstr);

public:

    _tstringW& operator = (int n);
    _tstringW& operator = (unsigned int n);
    _tstringW& operator = (long n);
    _tstringW& operator = (unsigned long n);
    _tstringW& operator = (double d);
    _tstringW& operator = (__int64 n);
    _tstringW& operator = (unsigned __int64 n);

public:

    //重载<<
    _tstringW& operator << (const _tstringW& str);

public:
    
    //数据类型转换
    int8 ConvToInt8() const;
    uint8 ConvToUInt8() const;
    int16 ConvToInt16() const;
    uint16 ConvToUInt16() const;
    Int32 ConvToInt32() const;
    uint32 ConvToUInt32() const;
    int64 ConvToInt64() const;
    uint64 ConvToUInt64() const;
    double ConvToDouble() const;
    float ConvToFloat() const;


    const _tstringW& operator >> (int8& n) const;
    const _tstringW& operator >> (uint8& n) const;
    const _tstringW& operator >> (int16& n) const;
    const _tstringW& operator >> (uint16& n) const;
    const _tstringW& operator >> (Int32& n) const;
    const _tstringW& operator >> (uint32& n) const;
    const _tstringW& operator >> (int64& n) const;
    const _tstringW& operator >> (uint64& n) const;
    const _tstringW& operator >> (int& n) const;
    const _tstringW& operator >> (double& n) const;
    const _tstringW& operator >> (float& n) const;

public:
    //构造函数
    _tstringW(wchar_t ch);
    _tstringW(char ch);
    _tstringW(const char* pStr);
    _tstringW(const wchar_t* pwStr);
    _tstringW(const _tstringA& str);
    _tstringW(const _tstringW& wstr);
    _tstringW(const std::string& str);
    _tstringW(const std::wstring& wstr);
    _tstringW(int n);
    _tstringW(unsigned int n);
    _tstringW(long n);
    _tstringW(unsigned long n);
    _tstringW(double d);
    _tstringW(__int64 n);
    _tstringW(unsigned __int64 n);

};

#ifdef _UNICODE
typedef _tstringW _tstring;
#else
typedef _tstringA _tstring;
#endif //_UNICODE

//NAMESPACEEND(Share)
//NAMESPACEEND(String)
};

#endif //_H_SHARED_STRING_TSTRING