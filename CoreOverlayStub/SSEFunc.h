#pragma once

class CSSEFunc
{
public:
	static BOOL IsSupportSSE();
	static void BlendWithAlpha(LPBYTE pDest, LPBYTE pSrc, int nSize);
private:
	static void BlendWithAlphaNormal(LPBYTE pDest, LPBYTE pSrc, int nSize);
	static void BlendWithAlphaSSE(LPBYTE pDest, LPBYTE pSrc, int nSize);
};
