/*
  Copyright (c) 2005, Dominik Reichl <dominik.reichl@t-online.de>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Neither the name of ReichlSoft nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/////////////////////////////////////////////////////////////////////////////
// Version History:
// 2005-04-17: v1.0
// - First release

#pragma once

#include "skin/UIEditBorder.h"

#ifndef TCH_STDPWCHAR
#define TCH_STDPWCHAR _T('*')
#endif

#ifndef SE_XORPAD_SIZE
#define SE_XORPAD_SIZE 32
#endif

class CUISecureEditEx : public CUIEditBorder
{
// Construction
public:
	CUISecureEditEx();
	virtual ~CUISecureEditEx();
	void EnableSecureMode(BOOL bEnable = TRUE);

	// Retrieve the currently entered password
	LPTSTR GetPassword();

	// Securely free the passwords returned by GetPassword()
	void DeletePassword(LPTSTR lpPassword);

	// Set the currently entered password, may be NULL
	void SetPassword(LPCTSTR lpPassword);

private:
	void _DeleteAll();
	void _SetMemoryEx(void *pDest, int c, size_t uCount);
	void _DeleteTPtr(LPTSTR lp, BOOL bIsArray, BOOL bIsString);
	void _ClearSelection();
	void _InsertCharacters(unsigned int uPos, LPCTSTR lpSource, unsigned int uNumChars);
	void _DeleteCharacters(unsigned int uPos, unsigned int uCount);
	void _EncryptBuffer(BOOL bEncrypt = TRUE);

	BOOL m_bSecMode;
	LPTSTR m_pXorPad;
	CPtrArray m_apChars;
	int m_nOldLen;

protected:
	afx_msg void OnEnUpdate();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus();

	DECLARE_MESSAGE_MAP()
};
