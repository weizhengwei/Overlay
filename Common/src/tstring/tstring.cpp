#include <algorithm>
#include "tstring/tstring.h"


// 用于BASE64编码、解码的常量
char pszBase64TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned int Base64Mask[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};

//NAMESPACEBEGIN(Share)
namespace String
{



//************************************************************
//功能性函数

//Format参数安全性检测
bool FormatSafeCheck(uint32 uParamCnt, const char* pszFormat)
{
    try
    {
        if (pszFormat == NULL)
        {
            return false;
        }

        _tstringA strFormat = pszFormat;
        if (strFormat.empty() || (strFormat.Right(1) == "%" && strFormat.Right(2) != "%%"))
        {
            return false;
        }
        
        size_t nPos = 0;
        uint32 nCnt = 0;
        while ((nPos = strFormat.Find('%', static_cast<unsigned int>(nPos))) != _tstring::npos)
        {
            switch (strFormat.at(nPos+1))
            {
            case '%':
                nPos += 2;
                continue;
                break;

            default:
                break;
            }

            nPos += 2;
            nCnt++;
        }

        return (nCnt == uParamCnt);
    }
    catch (...)
    {
        return false;
    }
}

bool FormatSafeCheck(uint32 uParamCnt, const wchar_t* pszFormat)
{
    try
    {
        if (pszFormat == NULL)
        {
            return false;
        }

        _tstringW strFormat = pszFormat;
        if (strFormat.empty() || (strFormat.Right(1) == L"%" && strFormat.Right(2) != L"%%"))
        {
            return false;
        }
        
        size_t nPos = 0;
        uint32 nCnt = 0;
        while ((nPos = strFormat.Find(L'%', static_cast<unsigned int>(nPos))) != _tstring::npos)
        {
            switch (strFormat.at(nPos+1))
            {
            case L'%':
                nPos += 2;
                continue;
                break;

            default:
                break;
            }

            nPos += 2;
            nCnt++;
        }

        return (nCnt == uParamCnt);
    }
    catch (...)
    {
        return false;
    }
}

//ANSIC与UNICODE互转
std::wstring& ConvtoWideString(const char* pszSrc, std::wstring& wstrDst)
{
    wstrDst.clear();
    if (pszSrc==NULL)
    {
        return wstrDst;
    }

    size_t nLen = strlen(pszSrc);
    if (nLen == 0)
    {
        return wstrDst;
    }

    wstrDst.resize(nLen);
    int nConvLength = MultiByteToWideChar(
        CP_ACP, 0, pszSrc, 
        static_cast<int>(nLen), 
        const_cast<wchar_t*>(wstrDst.c_str()), 
        static_cast<int>(wstrDst.length()));

    wstrDst.resize(nConvLength);
    return wstrDst;
}

std::wstring ConvtoWideString(const char* pszSrc)
{
    std::wstring wstrDst;
    ConvtoWideString(pszSrc, wstrDst);
    return wstrDst;
}

std::string& ConvtoNarrowString(const wchar_t* pszwSrc, std::string& strDst)
{
    strDst.clear();
    if (pszwSrc==NULL)
    {
        return strDst;
    }

    size_t nLen = wcslen(pszwSrc);
    if (nLen == 0)
    {
        return strDst;
    }

    strDst.resize(nLen*2) ;
    int nConvLength = WideCharToMultiByte(
        CP_ACP, 0, pszwSrc, static_cast<int>(nLen), 
        const_cast<char*>(strDst.c_str()), 
        static_cast<int>(strDst.length()), NULL, NULL);
    strDst.resize(nConvLength);
    return strDst;
}

std::string ConvtoNarrowString(const wchar_t* pszwSrc)
{
    std::string strDst;
    ConvtoNarrowString(pszwSrc, strDst);
    return strDst;
}

//UTF-8与UNICODE串转换功能
std::string& wcstoUTF8(const wchar_t* pwSrc, std::string& strDst)
{
    strDst.clear();
    if (pwSrc==NULL)
    {
        return strDst;
    }

    int count_bytes = 0;
    wchar_t byte_one = 0;
    wchar_t byte_other = 0x3f;    // 用于位与运算以提取位值

    unsigned char utf_one = 0;
    unsigned char utf_other = 0x80;    // 用于位或置标UTF-8编码

    size_t test_length = 0;
    size_t test_chars = 0;

    wchar_t tmp_wchar = L'\0';    // 用于宽字符位置析取和位移(右移6位)
    unsigned char tmp_char = '\0';

    const wchar_t* src_wstr = pwSrc;

    do    // 此循环可检测到字符串结尾的L'\0'并转换之
    {
        for (;;)    // 检测字节序列长度
        {
            if (src_wstr[test_chars] <= 0x7f)
            {
                count_bytes = 1;    // ASCII字符: 0xxxxxxx( ~ 01111111)
                byte_one = 0x7f;    // 用于位与运算, 提取有效位值, 下同
                utf_one = 0x0;
                break;
            }

            if ((src_wstr[test_chars] > 0x7f) && (src_wstr[test_chars] <= 0x7ff))
            {
                count_bytes = 2;    // 110xxxxx 10xxxxxx[1](最多11个1位, 简写为11*1)
                byte_one = 0x1f;    // 00011111, 下类推(1位的数量递减)
                utf_one = 0xc0;     // 11000000
                break;
            }

            if ((src_wstr[test_chars] > 0x7ff) && (src_wstr[test_chars] <= 0xffff))
            {
                count_bytes = 3;    // 1110xxxx 10xxxxxx[2](MaxBits: 16*1)
                byte_one = 0xf;     // 00001111
                utf_one = 0xe0;     // 11100000
                break;
            }

            if ((src_wstr[test_chars] > 0xffff) && (src_wstr[test_chars] <= 0x1fffff))
            {
                count_bytes = 4;    // 11110xxx 10xxxxxx[3](MaxBits: 21*1)
                byte_one = 0x7;     // 00000111
                utf_one = 0xf0;     // 11110000
                break;
            }

            if ((src_wstr[test_chars] > 0x1fffff) && (src_wstr[test_chars] <= 0x3ffffff))
            {
                count_bytes = 5;    // 111110xx 10xxxxxx[4](MaxBits: 26*1)
                byte_one = 0x3;     // 00000011
                utf_one = 0xf8;     // 11111000
                break;
            }

            if ((src_wstr[test_chars] > 0x3ffffff) && (src_wstr[test_chars] <= 0x7fffffff))
            {
                count_bytes = 6;    // 1111110x 10xxxxxx[5](MaxBits: 31*1)
                byte_one = 0x1;     // 00000001
                utf_one = 0xfc;     // 11111100
                break;
            }

            return strDst;    // 以上皆不满足则为非法序列
        }

        // 以下几行析取宽字节中的相应位, 并分组为UTF-8编码的各个字节
        tmp_wchar = src_wstr[test_chars];
        if (strDst.length() < (test_length+count_bytes))
        {
            strDst.resize(test_length+count_bytes);
        }

        for (int i = count_bytes; i > 1; i--)
        {    // 一个宽字符的多字节降序赋值
            tmp_char = (unsigned char)(tmp_wchar & byte_other);
            strDst[test_length + i - 1] = (tmp_char | utf_other);
            tmp_wchar >>= 6;
        }

        tmp_char = (unsigned char)(tmp_wchar & byte_one);
        strDst[test_length] = (tmp_char | utf_one);
        // 位值析取分组__End!
        test_length += count_bytes;
        test_chars ++;

    }while (src_wstr[test_chars] != L'\0');

    return strDst;
}

std::string wcstoUTF8(const wchar_t* pwSrc)
{
    std::string strDst;
    wcstoUTF8(pwSrc, strDst);
    return strDst;
}

std::wstring& UTF8towcs(const char* pszSrc, std::wstring& wstrDst)
{
    wstrDst.clear();
    if (pszSrc==NULL)
    {
        return wstrDst;
    }

    int count_bytes = 0;
    unsigned char byte_one = 0;
    unsigned char byte_other = 0x3f;    // 用于位与运算以提取位值
    long test_length = 0;
    size_t test_chars = 0;
    wchar_t tmp_wchar = L'\0';

    const unsigned char* src_str = reinterpret_cast<const unsigned char*>(pszSrc);

	if (src_str == NULL || (strlen((const char*)src_str) == 0))
	{
		return wstrDst ;
	}

    do    // 此循环可检测到字符串的结束符'\0'并转换之
    {
        for (;;)    // 检测字节序列长度
        {
            if (src_str[test_length] <= 0x7f){
                count_bytes = 1;    // ASCII字符: 0xxxxxxx( ~ 01111111)
                byte_one = 0x7f;    // 用于位与运算, 提取有效位值, 下同
                break;
            }
            if ( (src_str[test_length] >= 0xc0) && (src_str[test_length] <= 0xdf) ){
                count_bytes = 2;    // 110xxxxx(110 00000 ~ 110 111111) 
                byte_one = 0x1f;
                break;
            }
            if ( (src_str[test_length] >= 0xe0) && (src_str[test_length] <= 0xef) ){
                count_bytes = 3;    // 1110xxxx(1110 0000 ~ 1110 1111)
                byte_one = 0xf;
                break;
            }
            if ( (src_str[test_length] >= 0xf0) && (src_str[test_length] <= 0xf7) ){
                count_bytes = 4;    // 11110xxx(11110 000 ~ 11110 111)
                byte_one = 0x7;
                break;
            }
            if ( (src_str[test_length] >= 0xf8) && (src_str[test_length] <= 0xfb) ){
                count_bytes = 5;    // 111110xx(111110 00 ~ 111110 11)
                byte_one = 0x3;
                break;
            }
            if ( (src_str[test_length] >= 0xfc) && (src_str[test_length] <= 0xfd) ){
                count_bytes = 6;    // 1111110x(1111110 0 ~ 1111110 1)
                byte_one = 0x1;
                break;
            }
            return wstrDst;    // 以上皆不满足则为非法序列
        }
        // 以下几行析取UTF-8编码字符各个字节的有效位值
        tmp_wchar = src_str[test_length] & byte_one;
        for (int i=1; i<count_bytes; i++)
        {
            tmp_wchar <<= 6;    // 左移6位后与后续字节的有效位值"位或"赋值
            tmp_wchar = tmp_wchar | (src_str[test_length + i] & byte_other);
        }
        // 位值析取__End!
        if (wstrDst.length() < test_chars+1)
        {
            wstrDst.resize(test_chars+1);
        }
        wstrDst[test_chars] = tmp_wchar;

        test_length += count_bytes;
        test_chars ++;

    }while (src_str[test_length] != '\0');

    return wstrDst;
}

std::wstring UTF8towcs(const char* pszSrc)
{
    std::wstring wstrDst;
    UTF8towcs(pszSrc, wstrDst);
    return wstrDst;
}

//BASE64功能
std::string& cstobase64(const char* pszSrc, size_t nLength, std::string& strDst)
{
    strDst.clear();
    if (pszSrc == NULL || nLength == 0)
    {
        return strDst;
    }

    int nDigit;
    int nNumBits = 6;
    size_t nIndex = 0;
    size_t nInputSize = nLength;

    int nBitsRemaining = 0;
    long lBitsStorage =0;
    long lScratch =0;
    int nBits;
    unsigned char c;

    while (nNumBits > 0)
    {
        while ((nBitsRemaining < nNumBits) && (nIndex < nInputSize)) 
        {
            c = pszSrc[nIndex++];
            lBitsStorage <<= 8;
            lBitsStorage |= (c & 0xff);
            nBitsRemaining += 8;
        }
        if( nBitsRemaining < nNumBits ) 
        {
            lScratch = lBitsStorage << (nNumBits - nBitsRemaining);
            nBits = nBitsRemaining;
            nBitsRemaining = 0;
        }     
        else 
        {
            lScratch = lBitsStorage >> (nBitsRemaining - nNumBits);
            nBits = nNumBits;
            nBitsRemaining -= nNumBits;
        }
        nDigit = (int)(lScratch & Base64Mask[nNumBits]);
        nNumBits = nBits;
        if( nNumBits <=0 )
            break;
        
        strDst += pszBase64TAB[nDigit];
    }

    // 在后面补上=号
    while (strDst.length() % 4 != 0 )
    {
        strDst += '=';
    }

    return strDst;
}

std::string cstobase64(const char* pszSrc, size_t nLength)
{
    std::string strDst;
    cstobase64(pszSrc, nLength, strDst);
    return strDst;
}

std::string& base64tocs(const char* pszSrc, std::string& strDst)
{
    strDst.clear();
    size_t nIndex =0;
    int nDigit;
    int nDecode[256];
    size_t nSize = 0;
    int nNumBits = 6;

    if (pszSrc == NULL || (nSize = strlen(pszSrc)) == 0)
    {
        return strDst;
    }
    
    // Build Decode Table
    for (int i = 0; i < 256; i++) 
    {
        nDecode[i] = -2; // Illegal digit
    }

    for (int i=0; i < 64; i++)
    {
        nDecode[pszBase64TAB[i]] = i;
        nDecode['='] = -1;
    }

    long lBitsStorage  =0;
    int nBitsRemaining = 0;
    int nScratch = 0;
    unsigned char c;
    
    int i;
    // Decode the Input
    for (nIndex=0, i=0; nIndex<nSize; nIndex++)
    {
        c = pszSrc[nIndex];

        // 忽略所有不合法的字符
        if (c> 0x7F)
        {
            continue;
        }

        nDigit = nDecode[c];
        if( nDigit >= 0 ) 
        {
            lBitsStorage = (lBitsStorage << nNumBits) | (nDigit & 0x3F);
            nBitsRemaining += nNumBits;
            while (nBitsRemaining > 7) 
            {
                nScratch = lBitsStorage >> (nBitsRemaining - 8);
                strDst += (char)(nScratch & 0xFF);
                i++;
                nBitsRemaining -= 8;
            }
        }
    }

    return strDst;
}

std::string base64tocs(const char* pszSrc)
{
    std::string strDst;
    base64tocs(pszSrc, strDst);
    return strDst;
}

int64 StrConvToI64(const _tstringA& strSrc)
{
    return _atoi64(strSrc.c_str());
}

int64 StrConvToI64(const _tstringW& strSrc)
{
    return _wtoi64(strSrc.c_str());
}

double StrConvToDouble(const _tstringA& strSrc)
{
    return strtod(strSrc.c_str(), NULL);
}

double StrConvToDouble(const _tstringW& strSrc)
{
    return wcstod(strSrc.c_str(), NULL);
}
//************************************************************























//************************************************************
//tstringA

_tstringA::_tstringA(void)
{
}

_tstringA::~_tstringA(void)
{
}

_tstringA::_tstringA(wchar_t ch)
{
    operator = (ch);
}

_tstringA::_tstringA(char ch)
{
    operator = (ch);
}

_tstringA::_tstringA(const wchar_t* pwszStr)
{
    operator = (pwszStr);
}

_tstringA::_tstringA (const char* pszStr)
{
    operator = (pszStr);
}

_tstringA::_tstringA(const _tstringA& str)
{
    operator = (str);
}

_tstringA::_tstringA(const _tstringW& wstr)
{
    operator = (wstr);
}

_tstringA::_tstringA(const std::string& str)
{
    operator = (str);
}

_tstringA::_tstringA(const std::wstring& wstr)
{
    operator = (wstr);
}

_tstringA::_tstringA(int n)
{
    operator = (n);
}

_tstringA::_tstringA(unsigned int n)
{
    operator = (n);
}

_tstringA::_tstringA(long n)
{
    operator = (n);
}

_tstringA::_tstringA(unsigned long n)
{
    operator = (n);
}

_tstringA::_tstringA(double d)
{
    operator = (d);
}

_tstringA::_tstringA(__int64 n)
{
    operator = (n);
}

_tstringA::_tstringA(unsigned __int64 n)
{
    operator = (n);
}

unsigned int _tstringA::Replace(const _tstringA& strOld, const _tstringA& strNew)
{
    unsigned int nCount = 0;

    _tstringA::iterator it_begin = std::search(begin(), end(), strOld.begin(), strOld.end()); 

    while(it_begin != end()) 
    {
        _tstringA::iterator it_end = it_begin + strOld.size(); 
        replace(it_begin, it_end, strNew); 
        it_begin = std::search(it_begin+strNew.size(), end(), strOld.begin(), strOld.end());
        nCount++;
    }

    return nCount;
}

unsigned int _tstringA::Replace(wchar_t chOld, wchar_t chNew)
{
    unsigned int nCount = 0;
    if (chOld == chNew)
    {
        return nCount;
    }

    return Replace(_tstringA(chOld), _tstringA(chNew));
}

unsigned int _tstringA::Replace(char chOld, char chNew)
{
    unsigned int nCount = 0;
    if (chOld == chNew)
    {
        return nCount;
    }

    size_t length = this->length();
    for (size_t i=0; i<length; i++)
    {
        if (this->operator [](i) == chOld)
        {
            this->operator [](i) = chNew;
            nCount++;
        }
    }
    return nCount;
}

void _tstringA::Format(uint32 uParamCnt, const char* pszFormat, ...)
{
	try
	{
        if (!FormatSafeCheck(uParamCnt, pszFormat))
        {
            clear();
            return;
        }

		va_list args;
		va_start(args, pszFormat);
		unsigned int nLength = _vscprintf(pszFormat, args);

		resize(nLength);
		_vsnprintf_s(const_cast<char*>(this->c_str()), nLength+1, _TRUNCATE, pszFormat, args);
		va_end(args);
	}
	catch (...)
	{
        clear();
		//out error
	}
 }


void _tstringA::Format(uint32 uParamCnt, const wchar_t* pwszFormat, ...)
{
	try
	{
        if (!FormatSafeCheck(uParamCnt, pwszFormat))
        {
            clear();
            return;
        }

		va_list args;
		va_start(args, pwszFormat);
		unsigned int nLength = _vscwprintf(pwszFormat, args);

		wchar_t* pwszTmp = new wchar_t [nLength+1];
		pwszTmp[nLength] = L'\0';
		_vsnwprintf_s(pwszTmp, nLength+1, _TRUNCATE, pwszFormat, args);
		operator = (pwszTmp);
		delete [] pwszTmp;

		va_end(args);
	}
	catch (...)
	{
        clear();
		//out error
	}
}

std::string _tstringA::Right(size_t nCount) const
{
    if (nCount >= length())
    {
        return *this;
    }

    return substr(length() - nCount, nCount);
}

std::string _tstringA::Left(size_t nCount) const
{
    if (nCount >= length())
    {
        return *this;
    }
    return substr(0, nCount);
}

std::string _tstringA::Mid(size_t iFirst, size_t nCount)  const
{
    if (iFirst > length())
    {
        iFirst = length();
    }

    if (nCount + iFirst > length())
    {
        nCount = length()-iFirst;
    }

    return substr(iFirst, nCount);
}

size_t _tstringA::Find(char ch, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(ch, nStart);
}

size_t _tstringA::Find(const char* pszStr, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(pszStr, nStart);
}

size_t _tstringA::Find(const _tstringA& str, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(str.c_str(), nStart);
}

size_t _tstringA::ReverseFind(char ch)  const
{
    for (size_t nLength = length(); nLength > 0; nLength--)
    {
        if (this->at(nLength-1) == ch)
        {
            return nLength-1;
        }
    }
    return static_cast<size_t>(std::string::npos);
}

_tstringA::SplitList _tstringA::Split(const _tstringA& strKeyword) const
{
    _tstringA::SplitList list;
    size_t npos = this->Find(strKeyword);
    size_t nlastpos = 0;

    while (npos != _tstringA::npos)
    {
        list.push_back(this->Mid(nlastpos, npos-nlastpos));
        nlastpos = npos+1;
        npos = this->Find(strKeyword, static_cast<unsigned int>(nlastpos));
    }

    if (nlastpos != this->length())
    {
        list.push_back(this->Mid(nlastpos, this->length()-nlastpos));
    }

    return list;
}

_tstringA& _tstringA::operator = (char ch)
{
    resize(1);
    operator [](0) = ch;
    operator [](1) = '\0';
    return *this;
}

_tstringA& _tstringA::operator = (wchar_t ch)
{
    std::wstring str;
    str.resize(1);
    str[0] = ch;
    operator = (str);
    return *this;
}

_tstringA& _tstringA::operator = (const char* pszStr)
{
    if (pszStr==NULL)
    {
        empty();
        return *this;
    }

    std::string::operator = (pszStr);
    return *this;
}


_tstringA& _tstringA::operator = (const wchar_t* pszwStr)
{
    if (pszwStr==NULL)
    {
        empty();
        return *this;
    }

    ConvtoNarrowString(pszwStr, *this);
    return *this;
}

_tstringA& _tstringA::operator = (const _tstringA& str)
{
    if (&str != this)
    {
        operator = (str.c_str());
    }
    return *this;
}

_tstringA& _tstringA::operator = (const _tstringW& wstr)
{
    operator = (wstr.c_str());
    return *this;
}

_tstringA& _tstringA::operator = (const std::string& str)
{
    operator = (str.c_str());
    return *this;
}

_tstringA& _tstringA::operator = (const std::wstring& wstr)
{
    operator = (wstr.c_str());
    return *this;
}

_tstringA& _tstringA::operator = (int n)
{
    char szBuff[20] = {0};
    SNPRINTF(szBuff, sizeof(szBuff), "%d", n);
    operator = (szBuff);
    return *this;
}

_tstringA& _tstringA::operator = (unsigned int n)
{
    char szBuff[20] = {0};
    SNPRINTF(szBuff, sizeof(szBuff), "%u", n);
    operator = (szBuff);
    return *this;
}

_tstringA& _tstringA::operator = (long n)
{
    char szBuff[20] = {0};
    SNPRINTF(szBuff, sizeof(szBuff), "%d", n);
    operator = (szBuff);
    return *this;
}

_tstringA& _tstringA::operator = (unsigned long n)
{
    char szBuff[20] = {0};
    SNPRINTF(szBuff, sizeof(szBuff), "%u", n);
    operator = (szBuff);
    return *this;
}

_tstringA& _tstringA::operator = (double d)
{
    char szBuff[40] = {0};
    SNPRINTF(szBuff, sizeof(szBuff), "%f", d);
    operator = (szBuff);
    return *this;
}

_tstringA& _tstringA::operator = (__int64 n)
{
    _tstringA str;
    str.resize(40);
    _i64toa_s(n, const_cast<char*>(str.c_str()), str.length(), 10);
    operator = (str.c_str());
    return *this;
}

_tstringA& _tstringA::operator = (unsigned __int64 n)
{
    _tstringA str;
    str.resize(40);
    _ui64toa_s(n, const_cast<char*>(str.c_str()), str.length(), 10);
    operator = (str.c_str());
    return *this;
}

_tstringA& _tstringA::operator << (const _tstringA& str)
{
    operator += (str.c_str());
    return *this;
}

_tstringA& _tstringA::MakeLower()
{
    _tstringA::iterator itr = this->begin();
    while (itr != this->end())
    {
        if (*itr >= 'A' && *itr <= 'Z')
        {
            *itr += ('a' - 'A');
        }
        itr++;
    }
    return *this;
}


_tstringA& _tstringA::MakeUpper()
{
    _tstringA::iterator itr = this->begin();
    while (itr != this->end())
    {
        if (*itr >= 'a' && *itr <= 'z')
        {
            *itr -= ('a' - 'A');
        }
        itr++;
    }
    return *this;
}

_tstringA  _tstringA::MakeLowerConst()  const
{
	_tstringA copyStr = *this;

	return copyStr.MakeLower();
}

_tstringA  _tstringA::MakeUpperConst() const
{
	_tstringA copyStr = *this;

	return copyStr.MakeUpper();
}




int _tstringA::CompareNoCase(const _tstringA& str) const
{
    _tstringA strSrc = *this;
	_tstringA strDes = str;

    strSrc.MakeLower();
    strDes.MakeLower();
    return strSrc.compare(strDes);
}


_tstringA& _tstringA::toNarrowString() const
{
    return const_cast<_tstringA&>(*this);
}

std::wstring _tstringA::toWideString() const
{
    return ConvtoWideString(this->c_str());
}

_tstringA& _tstringA::fromUTF8(const char* pszUTF8)
{
    operator = (UTF8towcs(pszUTF8));
    return *this;
}

std::string _tstringA::toUTF8() const
{
    return wcstoUTF8(ConvtoWideString(this->c_str()).c_str());
}

_tstringA& _tstringA::fromBASE64(const char* pszBASE64)
{
    base64tocs(pszBASE64, *this);
    return *this;
}

std::string _tstringA::toBASE64() const
{
    return cstobase64(this->c_str(), this->length());
}

int8 _tstringA::ConvToInt8() const
{
    return static_cast<int8>(StrConvToI64(*this));
}

uint8 _tstringA::ConvToUInt8() const
{
    return static_cast<uint8>(StrConvToI64(*this));
}

int16 _tstringA::ConvToInt16() const
{
    return static_cast<int16>(StrConvToI64(*this));
}

uint16 _tstringA::ConvToUInt16() const
{
    return static_cast<uint16>(StrConvToI64(*this));
}

Int32 _tstringA::ConvToInt32() const
{
    return static_cast<Int32>(StrConvToI64(*this));
}

uint32 _tstringA::ConvToUInt32() const
{
    return static_cast<uint32>(StrConvToI64(*this));
}

int64 _tstringA::ConvToInt64() const
{
    return StrConvToI64(*this);
}

uint64 _tstringA::ConvToUInt64() const
{
    return static_cast<uint64>(StrConvToI64(*this));
}

double _tstringA::ConvToDouble() const
{
    return StrConvToDouble(*this);
}

float _tstringA::ConvToFloat() const
{
    return static_cast<float>(StrConvToDouble(*this));
}

const _tstringA& _tstringA::operator >> (int8& n) const
{
    n = static_cast<int8>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (uint8& n) const
{
    n = static_cast<uint8>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (int16& n) const
{
    n = static_cast<int16>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (uint16& n) const
{
    n = static_cast<uint16>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (Int32& n) const
{
    n = static_cast<Int32>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (uint32& n) const
{
    n = static_cast<uint32>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (int64& n) const
{
    n = static_cast<int64>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (uint64& n) const
{
    n = static_cast<uint64>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (int& n) const
{
    n = static_cast<int>(StrConvToI64(*this));
    return *this;
}

const _tstringA& _tstringA::operator >> (double& n) const
{
    n = StrConvToDouble(*this);
    return *this;
}

const _tstringA& _tstringA::operator >> (float& n) const
{
    n = static_cast<float>(StrConvToDouble(*this));
    return *this;
}
























































//************************************************************
//_tstringW

_tstringW::_tstringW(void)
{
}

_tstringW::~_tstringW(void)
{
}

_tstringW::_tstringW(wchar_t ch)
{
    operator = (ch);
}

_tstringW::_tstringW(char ch)
{
    operator = (ch);
}

_tstringW::_tstringW(const wchar_t* pwszStr)
{
    operator = (pwszStr);
}

_tstringW::_tstringW (const char* pszStr)
{
    operator = (pszStr);
}

_tstringW::_tstringW(const _tstringA& str)
{
    operator = (str);
}

_tstringW::_tstringW(const _tstringW& wstr)
{
    operator = (wstr);
}

_tstringW::_tstringW(const std::string& str)
{
    operator = (str);
}

_tstringW::_tstringW(const std::wstring& wstr)
{
    operator = (wstr);
}

_tstringW::_tstringW(int n)
{
    operator = (n);
}

_tstringW::_tstringW(unsigned int n)
{
    operator = (n);
}

_tstringW::_tstringW(long n)
{
    operator = (n);
}

_tstringW::_tstringW(unsigned long n)
{
    operator = (n);
}

_tstringW::_tstringW(double d)
{
    operator = (d);
}

_tstringW::_tstringW(__int64 n)
{
    operator = (n);
}

_tstringW::_tstringW(unsigned __int64 n)
{
    operator = (n);
}

unsigned int _tstringW::Replace(const _tstringW& strOld, const _tstringW& strNew)  
{
    unsigned int nCount = 0;

  //  _tstringW::iterator it_begin = std::search(begin(), end(), strOld.begin(), strOld.end()); 

  //  while(it_begin != end()) 
  //  {
  //      _tstringW::iterator it_end = it_begin + strOld.size(); 
		//replace(it_begin, it_end, strNew.c_str(), strNew.size());  //it_begin and it_end cannot be accessed after executing this code
  //      it_begin = std::search(it_begin+strNew.size(), end(), strOld.begin(), strOld.end());
  //      nCount++;
  //  }

	_tstring::size_type pos = 0 ;
	pos = Find(strOld, pos) ;
	while (pos != std::string::npos)
	{
		replace(pos,strOld.length(), strNew) ;
		pos = Find(strOld, pos+strNew.length()) ; //search for the remaining content
		++ nCount ;
	} 

    return nCount;
}

unsigned int _tstringW::Replace(wchar_t chOld, wchar_t chNew)
{
    unsigned int nCount = 0;
    if (chOld == chNew)
    {
        return nCount;
    }

    size_t length = this->length();
    for (size_t i=0; i<length; i++)
    {
        if (this->operator [](i) == chOld)
        {
            this->operator [](i) = chNew;
            nCount++;
        }
    }
    return nCount;
}

unsigned int _tstringW::Replace(char chOld, char chNew)
{
    unsigned int nCount = 0;
    if (chOld == chNew)
    {
        return nCount;
    }

    return Replace(_tstringW(chOld), _tstringW(chNew));
}

void _tstringW::Format(uint32 uParamCnt, const char* pszFormat, ...)
{
	try
	{
        if (!FormatSafeCheck(uParamCnt, pszFormat))
        {
            clear();
            return;
        }

		va_list args;
		va_start(args, pszFormat);
		unsigned int nLength = _vscprintf(pszFormat, args);

		char* pszTmp = new char [nLength+1];
		pszTmp[nLength] = '\0';
		_vsnprintf_s(pszTmp, nLength+1, _TRUNCATE, pszFormat, args);
		operator = (pszTmp);
		delete [] pszTmp;

		va_end(args);
	}
	catch ( ... )
	{
        clear();
	}
}


void _tstringW::Format(uint32 uParamCnt, const wchar_t* pwszFormat, ...)
{
	try
	{
        if (!FormatSafeCheck(uParamCnt, pwszFormat))
        {
            clear();
            return;
        }

		va_list args;
		va_start(args, pwszFormat);
		unsigned int nLength = _vscwprintf(pwszFormat, args);

		resize(nLength);
		_vsnwprintf_s(const_cast<wchar_t*>(this->c_str()), nLength+1, _TRUNCATE, pwszFormat, args);
		va_end(args);
	}
	catch ( ... )
	{
        clear();
	}
}

std::wstring _tstringW::Right(size_t nCount) const
{
    if (nCount >= length())
    {
        return *this;
    }

    return substr(length() - nCount, nCount);
}

std::wstring _tstringW::Left(size_t nCount) const
{
    if (nCount >= length())
    {
        return *this;
    }
    return substr(0, nCount);
}

std::wstring _tstringW::Mid(size_t iFirst, size_t nCount) const
{
    if (iFirst > length())
    {
        iFirst = length();
    }

    if (nCount + iFirst > length())
    {
        nCount = length()-iFirst;
    }

    return substr(iFirst, nCount);
}

size_t _tstringW::Find(wchar_t ch, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(ch, nStart);
}

size_t _tstringW::Find(const wchar_t* pwszStr, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(pwszStr, nStart);
}

size_t _tstringW::Find(const _tstringW& str, unsigned int nStart) const
{
    if (nStart > length())
    {
        return std::string::npos;
    }
    return find(str.c_str(), nStart);
}

size_t _tstringW::ReverseFind(wchar_t ch) const
{
    for (size_t nLength = length(); nLength > 0; nLength--)
    {
        if (this->at(nLength-1) == ch)
        {
            return nLength-1;
        }
    }
    return static_cast<size_t>(std::string::npos);
}

_tstringW::SplitList _tstringW::Split(const _tstringW& strKeyword) const
{
    _tstringW::SplitList list;
    size_t npos = this->Find(strKeyword);
    size_t nlastpos = 0;

    while (npos != _tstringW::npos)
    {
        list.push_back(this->Mid(nlastpos, npos-nlastpos));
        nlastpos = npos+1;
        npos = this->Find(strKeyword, static_cast<unsigned int>(nlastpos));
    }

    if (nlastpos != this->length())
    {
        list.push_back(this->Mid(nlastpos, this->length()-nlastpos));
    }

    return list;
}

_tstringW& _tstringW::operator = (char ch)
{
    std::string str;
    str.resize(1);
    str[0] = ch;
    operator = (str);
    return *this;
}

_tstringW& _tstringW::operator = (wchar_t ch)
{
    resize(1);
    operator [](0) = ch;
    operator [](1) = L'\0';
    return *this;
}

_tstringW& _tstringW::operator = (const char* pszStr)
{
    if (pszStr==NULL)
    {
        empty();
        return *this;
    }

    ConvtoWideString(pszStr, *this);
    return *this;
}


_tstringW& _tstringW::operator = (const wchar_t* pszwStr)
{
    if (pszwStr==NULL)
    {
        empty();
        return *this;
    }

    std::wstring::operator = (pszwStr);
    return *this;
}

_tstringW& _tstringW::operator = (const _tstringA& str)
{
    operator = (str.c_str());
    return *this;
}

_tstringW& _tstringW::operator = (const _tstringW& wstr)
{
    if (&wstr != this)
    {
        operator = (wstr.c_str());
    }
    return *this;
}

_tstringW& _tstringW::operator = (const std::string& str)
{
    operator = (str.c_str());
    return *this;
}

_tstringW& _tstringW::operator = (const std::wstring& wstr)
{
    operator = (wstr.c_str());
    return *this;
}

_tstringW& _tstringW::operator = (int n)
{
    wchar_t szBuff[20] = {0};
    SNPRINTF(szBuff, 20, L"%d", n);
    operator = (szBuff);
    return *this;
}

_tstringW& _tstringW::operator = (unsigned int n)
{
    wchar_t szBuff[20] = {0};
    SNPRINTF(szBuff, 20, L"%u", n);
    operator = (szBuff);
    return *this;
}

_tstringW& _tstringW::operator = (long n)
{
    wchar_t szBuff[20] = {0};
    SNPRINTF(szBuff, 20, L"%d", n);
    operator = (szBuff);
    return *this;
}

_tstringW& _tstringW::operator = (unsigned long n)
{
    wchar_t szBuff[20] = {0};
    SNPRINTF(szBuff, 20, L"%u", n);
    operator = (szBuff);
    return *this;
}

_tstringW& _tstringW::operator = (double d)
{
    wchar_t szBuff[40] = {0};
    SNPRINTF(szBuff, 40, L"%f", d);
    operator = (szBuff);
    return *this;
}

_tstringW& _tstringW::operator = (__int64 n)
{
    _tstringW str;
    str.resize(40);
    _i64tow_s(n, const_cast<wchar_t*>(str.c_str()), str.length(), 10);
    operator = (str.c_str());
    return *this;
}

_tstringW& _tstringW::operator = (unsigned __int64 n)
{
    _tstringW str;
    str.resize(40);
    _ui64tow_s(n, const_cast<wchar_t*>(str.c_str()), str.length(), 10);
    operator = (str.c_str());
    return *this;
}

_tstringW& _tstringW::operator << (const _tstringW& str)
{
    operator += (str.c_str());
    return *this;
}

_tstringW& _tstringW::MakeLower()
{
    _tstringW::iterator itr = this->begin();
    while (itr != this->end())
    {
        if (*itr >= L'A' && *itr <= L'Z')
        {
            *itr += (L'a' - L'A');
        }
        itr++;
    }
    return *this;
}

_tstringW& _tstringW::MakeUpper()
{
	_tstringW::iterator itr = this->begin();
	while (itr != this->end())
	{
		if (*itr >= L'a' && *itr <= L'z')
		{
			*itr -= (L'a' - L'A');
		}
		itr++;
	}
	return *this;
}

_tstringW  _tstringW::MakeLowerConst() const
{
	_tstringW copyStr = *this;
	
	return copyStr.MakeLower();
}

_tstringW  _tstringW::MakeUpperConst() const
{
	_tstringW copyStr = *this;

	return copyStr.MakeUpper();
}



int _tstringW::CompareNoCase(const _tstringW& str) const
{
    _tstringW strSrc = *this;
	_tstringW strDes = str;
    strSrc.MakeLower();
    strDes.MakeLower();
    return strSrc.compare(strDes);
}


std::string _tstringW::toNarrowString() const
{
    return ConvtoNarrowString(this->c_str());
}

_tstringW& _tstringW::toWideString() const
{
    return const_cast<_tstringW&>(*this);
}


_tstringW& _tstringW::fromUTF8(const char* pszUTF8)
{
    UTF8towcs(pszUTF8, *this);
    return *this;
}

std::string _tstringW::toUTF8() const 
{
    return wcstoUTF8(this->c_str());
}

_tstringW& _tstringW::fromBASE64(const char* pszBASE64)
{
    std::string strBASE64;
    base64tocs(pszBASE64, strBASE64);
    clear();
    resize(strBASE64.length()+1/2);
    memcpy(const_cast<wchar_t*>(this->c_str()), strBASE64.c_str(), strBASE64.length());
    return *this;
}

std::string _tstringW::toBASE64() const
{
    return cstobase64(reinterpret_cast<const char*>(this->c_str()), this->length()*2);
}


int8 _tstringW::ConvToInt8() const
{
    return static_cast<int8>(StrConvToI64(*this));
}

uint8 _tstringW::ConvToUInt8() const
{
    return static_cast<uint8>(StrConvToI64(*this));
}

int16 _tstringW::ConvToInt16() const
{
    return static_cast<int16>(StrConvToI64(*this));
}

uint16 _tstringW::ConvToUInt16() const
{
    return static_cast<uint16>(StrConvToI64(*this));
}

Int32 _tstringW::ConvToInt32() const
{
    return static_cast<Int32>(StrConvToI64(*this));
}

uint32 _tstringW::ConvToUInt32() const
{
    return static_cast<uint32>(StrConvToI64(*this));
}

int64 _tstringW::ConvToInt64() const
{
    return StrConvToI64(*this);
}

uint64 _tstringW::ConvToUInt64() const
{
    return static_cast<uint64>(StrConvToI64(*this));
}

double _tstringW::ConvToDouble() const
{
    return StrConvToDouble(*this);
}

float _tstringW::ConvToFloat() const
{
    return static_cast<float>(StrConvToDouble(*this));
}

const _tstringW& _tstringW::operator >> (int8& n) const
{
    n = static_cast<int8>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (uint8& n) const
{
    n = static_cast<uint8>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (int16& n) const
{
    n = static_cast<int16>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (uint16& n) const
{
    n = static_cast<uint16>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (Int32& n) const
{
    n = static_cast<Int32>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (uint32& n) const
{
    n = static_cast<uint32>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (int64& n) const
{
    n = static_cast<int64>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (uint64& n) const
{
    n = static_cast<uint64>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (int& n) const
{
    n = static_cast<int>(StrConvToI64(*this));
    return *this;
}

const _tstringW& _tstringW::operator >> (double& n) const
{
    n = StrConvToDouble(*this);
    return *this;
}

const _tstringW& _tstringW::operator >> (float& n) const
{
    n = static_cast<float>(StrConvToDouble(*this));
    return *this;
}

//************************************************************


//NAMESPACEEND(Share)
//NAMESPACEEND(String)
};