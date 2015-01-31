#pragma once
#include "tstring/tstring.h"
using String::_tstring;

class CPageItem
{
public:
	CPageItem(void);
	~CPageItem(void);

public:
	ISonicString *m_pScBtnClose;
	ISonicString *m_pScSelect;
	_tstring     m_strTitle;
};
