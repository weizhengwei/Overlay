#include "StdAfx.h"
#include "SSEFunc.h"

BOOL CSSEFunc::IsSupportSSE()
{
#ifdef _WIN64
	return FALSE;
#else
	static int nSupportSSE = -1;
	if(nSupportSSE == -1)
	{
		__try 
		{
			__asm 
			{
				xorpd xmm0, xmm0        // executing SSE instruction
			}
			nSupportSSE = TRUE;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) 
		{
			nSupportSSE = FALSE;
		}
	}
	return nSupportSSE == TRUE;
#endif
}


void CSSEFunc::BlendWithAlpha( LPBYTE pDest, LPBYTE pSrc, int nSize )
{
	if(IsSupportSSE())
	{
		BlendWithAlphaSSE(pDest, pSrc, nSize);
	}
	else
	{
		BlendWithAlphaNormal(pDest, pSrc, nSize);
	}
}

void CSSEFunc::BlendWithAlphaNormal( LPBYTE pDest, LPBYTE pSrc, int nSize )
{
	if(nSize <= 0 || nSize % 4)
	{
		return;
	}
	for(int i = 0; i < nSize; i += 4)
	{
		BYTE bAlpha = *(pSrc + 3);
		*pDest++ = *pDest * (255 - bAlpha) / 255 + *pSrc++;
		*pDest++ = *pDest * (255 - bAlpha) / 255 + *pSrc++;
		*pDest++ = *pDest * (255 - bAlpha) / 255 + *pSrc++;
		pSrc++;
		pDest++;
	}
}

void CSSEFunc::BlendWithAlphaSSE( LPBYTE pDest, LPBYTE pSrc, int nSize )
{
#ifndef _WIN64
	int nSSELen = nSize >> 4;
	if(nSSELen)
	{
		DWORD dwMask = 0xff000000;
		__asm
		{
			push ecx
			push esi
			push edi
			mov esi, [pSrc]
			mov edi, [pDest]
			mov ecx, [nSSELen]
			xorps xmm0, xmm0
			movups xmm7, [dwMask]
			shufps xmm7, xmm7, 0
blend_alpha_process:
			movups xmm1, [esi]
			movaps xmm6, xmm1
			andps xmm6, xmm7
			xorps xmm6, xmm7
			movaps xmm2, xmm1
			punpcklbw xmm1, xmm0
			punpckhbw xmm2, xmm0
			movaps xmm3, xmm6
			punpcklbw xmm3, xmm0
			movaps xmm4, xmm3
			psrld xmm4, 10h
			orps xmm3, xmm4
			shufps xmm3, xmm3, 0f5h
			movaps xmm4, xmm6
			punpckhbw xmm4, xmm0
			movaps xmm5, xmm4
			psrld xmm5, 10h
			orps xmm4, xmm5
			shufps xmm4, xmm4, 0f5h
			movups xmm5, [edi]
			movaps xmm6, xmm5
			punpcklbw xmm5, xmm0
			punpckhbw xmm6, xmm0
			pmullw xmm5, xmm3
			psrlw xmm5, 8
			paddw xmm1, xmm5
			packuswb xmm1, xmm0
			pmullw xmm6, xmm4
			psrlw xmm6, 8
			paddw xmm2, xmm6
			packuswb xmm2, xmm0
			movlhps xmm1, xmm2
			movups [edi], xmm1
			add esi, 10h
			add edi, 10h
			dec ecx
			jnz blend_alpha_process
			pop edi
			pop esi
			pop ecx
			emms
		}
	}
	nSSELen = nSSELen << 4;
	CSSEFunc::BlendWithAlphaNormal(pDest + nSSELen, pSrc + nSSELen, nSize - nSSELen);
#endif
}
