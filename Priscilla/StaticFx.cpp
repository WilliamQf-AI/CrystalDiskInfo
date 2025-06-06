﻿/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : https://crystalmark.info/
//      License : MIT License
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "StaticFx.h"

#if _MSC_VER <= 1310
#define ON_WM_MOUSEHOVER() \
	{ 0x2A1 /*WM_MOUSEHOVER*/, 0, 0, 0, AfxSig_vwp, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(UINT, CPoint) > (OnMouseHover)) },

#define ON_WM_MOUSELEAVE() \
	{ 0x2A3 /*WM_MOUSELEAVE*/, 0, 0, 0, AfxSig_vv, \
		(AFX_PMSG)(AFX_PMSGW) \
		(static_cast< void (AFX_MSG_CALL CWnd::*)(void) > (OnMouseLeave)) },
#endif

////------------------------------------------------
//   CStaticFx
////------------------------------------------------

CStaticFx::CStaticFx()
{
	// Control
	m_X = 0;
	m_Y = 0;
	m_RenderMode = SystemDraw;
	m_bHighContrast = FALSE;
	m_bDarkMode = FALSE;
	m_DrawFrame = FALSE;
	m_bDrawFrameEx = FALSE;
	m_FrameColor = RGB(128, 128, 128);
	m_hPal = NULL;

	// Glass
	m_GlassColor = RGB(255, 255, 255);
	m_GlassAlpha = 255;

	// Meter
	m_bMeter = FALSE;
	m_MeterRatio = 0.0;

	// Image
	m_ImageCount = 0;
	m_BkDC = NULL;
	m_bBkBitmapInit = FALSE;
	m_bBkLoad = FALSE;

	// Font
	m_TextAlign = SS_LEFT;
	m_TextColor = RGB(0, 0, 0);

	// Mouse
	m_bHover = FALSE;
	m_bFocas = FALSE;
	m_bTrackingNow = FALSE;
	m_bHandCursor = FALSE;

	// Text Format
	m_TextFormat = 0;
	m_LabelFormat = DT_LEFT | DT_TOP | DT_SINGLELINE;
	m_UnitFormat = DT_RIGHT | DT_BOTTOM | DT_SINGLELINE;

	// Margin
	m_Margin.top = 0;
	m_Margin.left = 0;
	m_Margin.bottom = 0;
	m_Margin.right = 0;
}

CStaticFx::~CStaticFx()
{
}

IMPLEMENT_DYNAMIC(CStaticFx, CStatic)

BEGIN_MESSAGE_MAP(CStaticFx, CStatic)
	//{{AFX_MSG_MAP(CStaticFx)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//------------------------------------------------
// Control
//------------------------------------------------

BOOL CStaticFx::InitControl(int x, int y, int width, int height, double zoomRatio, HPALETTE hPal, CDC* bkDC,
	LPCTSTR imagePath, int imageCount, DWORD textAlign, int renderMode, BOOL bHighContrast, BOOL bDarkMode, DWORD drawFrame)
{
	m_X = (int)(x * zoomRatio);
	m_Y = (int)(y * zoomRatio);
	m_CtrlSize.cx = (int)(width * zoomRatio);
	m_CtrlSize.cy = (int)(height * zoomRatio);
	MoveWindow(m_X, m_Y, m_CtrlSize.cx, m_CtrlSize.cy);

	m_hPal = hPal;
	m_BkDC = bkDC;
	m_ImagePath = imagePath;
	m_ImageCount = imageCount;
	m_RenderMode = renderMode;

	if (SS_LEFT <= textAlign && textAlign <= SS_RIGHT)
	{
		m_TextAlign = textAlign;
	}

	if (m_ToolTip.m_hWnd != NULL)
	{
		if (m_ToolTip.GetToolCount() != 0)
		{
			m_ToolTip.DelTool(this, 1);
		}
		CRect rect;
		GetClientRect(rect);
		m_ToolTip.AddTool(this, m_ToolTipText, rect, 1);
	}

	m_bHighContrast = bHighContrast;
	m_bDarkMode = bDarkMode;
	m_DrawFrame = drawFrame;

	if (m_bHighContrast)
	{
		ModifyStyle(SS_OWNERDRAW, m_TextAlign | SS_CENTERIMAGE);

		return TRUE;
	}
	else if (renderMode & SystemDraw)
	{
		ModifyStyle(SS_OWNERDRAW, m_TextAlign | SS_CENTERIMAGE);

		return TRUE;
	}
	else
	{
		SetBkReload();
		ModifyStyle(m_TextAlign | SS_CENTERIMAGE, SS_OWNERDRAW);
	}

	if (renderMode & OwnerDrawImage)
	{
		if (!LoadBitmap(imagePath))
		{
			ModifyStyle(SS_OWNERDRAW, m_TextAlign);
		}
	}
	else
	{
		m_ImageCount = 1;
		m_CtrlImage.Destroy();
		m_CtrlImage.Create(m_CtrlSize.cx, m_CtrlSize.cy * m_ImageCount, 32);
		m_CtrlBitmap.Detach();
		m_CtrlBitmap.Attach((HBITMAP)m_CtrlImage);
		DWORD length = m_CtrlSize.cx * m_CtrlSize.cy * m_ImageCount * 4;
		BYTE* bitmapBits = new BYTE[length];
		m_CtrlBitmap.GetBitmapBits(length, bitmapBits);

		BYTE r, g, b, a;
		if (renderMode & OwnerDrawGlass)
		{
			r = (BYTE)GetRValue(m_GlassColor);
			g = (BYTE)GetGValue(m_GlassColor);
			b = (BYTE)GetBValue(m_GlassColor);
			a = m_GlassAlpha;
		}
		else // OwnerDrawTransparent
		{
			r = 0;
			g = 0;
			b = 0;
			a = 0;
		}

		for (int y = 0; y < (int)(m_CtrlSize.cy * m_ImageCount); y++)
		{
			for (int x = 0; x < m_CtrlSize.cx; x++)
			{
				DWORD p = (y * m_CtrlSize.cx + x) * 4;
#if _MSC_VER > 1310
#pragma warning( disable : 6386 )
#endif
				bitmapBits[p + 0] = b;
				bitmapBits[p + 1] = g;
				bitmapBits[p + 2] = r;
				bitmapBits[p + 3] = a;
#if _MSC_VER > 1310
#pragma warning( default : 6386 )
#endif
			}
		}

		m_CtrlBitmap.SetBitmapBits(length, bitmapBits);
		delete[] bitmapBits;
	}

	Invalidate();

	return TRUE;
}

void CStaticFx::SetMargin(int top, int left, int bottom, int right, double zoomRatio)
{
	m_Margin.top = (int)(top * zoomRatio);
	m_Margin.left = (int)(left * zoomRatio);
	m_Margin.bottom = (int)(bottom * zoomRatio);
	m_Margin.right = (int)(right * zoomRatio);
}

CSize CStaticFx::GetSize(void)
{
	return m_CtrlSize;
}

void CStaticFx::SetDrawFrame(BOOL drawFrame)
{
	if (drawFrame && m_bHighContrast)
	{
		ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_DRAWFRAME);
	}
	else
	{
		ModifyStyleEx(WS_EX_STATICEDGE, 0, SWP_DRAWFRAME);
	}
	m_DrawFrame = drawFrame;
}

void CStaticFx::SetDrawFrameEx(BOOL bDrawFrameEx, COLORREF frameColor)
{
	m_bDrawFrameEx = bDrawFrameEx;
	m_FrameColor = frameColor;
}

void CStaticFx::SetGlassColor(COLORREF glassColor, BYTE glassAlpha)
{
	m_GlassColor = glassColor;
	m_GlassAlpha = glassAlpha;
}

void CStaticFx::SetMeter(BOOL bMeter, double meterRatio)
{
	m_bMeter = bMeter;
	if (meterRatio > 1.0)
	{
		m_MeterRatio = 1.0;
	}
	else if (meterRatio > 0)
	{
		m_MeterRatio = meterRatio;
	}
	else
	{
		m_MeterRatio = 0.0;
	}

	Invalidate();
}

void CStaticFx::SetLabelUnit(CString label, CString unit)
{
	m_Label = label;
	m_Unit = unit;
}

void CStaticFx::SetLabelUnitFormat(UINT labelFormat, UINT unitFormat)
{
	m_LabelFormat = labelFormat;
	m_UnitFormat = unitFormat;
}

void CStaticFx::SetTextFormat(UINT format)
{
	m_TextFormat = format;
}

//------------------------------------------------
// Draw Control
//------------------------------------------------

void CStaticFx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* drawDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	LoadCtrlBk(drawDC);

	DrawControl(drawDC, lpDrawItemStruct, m_CtrlBitmap, m_BkBitmap, ControlImageNormal);
}

void CStaticFx::DrawControl(CDC* drawDC, LPDRAWITEMSTRUCT lpDrawItemStruct, CBitmap& ctrlBitmap, CBitmap& bkBitmap, int no)
{
	CDC* pMemDC = new CDC;
	CBitmap* pOldMemBitmap;
	if(m_hPal && drawDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
	{
		SelectPalette( drawDC->GetSafeHdc(), m_hPal, FALSE );
		drawDC->RealizePalette();
		drawDC->SetStretchBltMode(HALFTONE);
	}
	pMemDC->CreateCompatibleDC(drawDC);
	if(m_hPal && pMemDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
	{
		SelectPalette( pMemDC->GetSafeHdc(), m_hPal, FALSE );
		pMemDC->RealizePalette();
		pMemDC->SetStretchBltMode(HALFTONE);
	}
	pOldMemBitmap = pMemDC->SelectObject(&ctrlBitmap);
	CDC* pBkDC = new CDC;
	CBitmap* pOldBkBitmap;
	pBkDC->CreateCompatibleDC(drawDC);
	if(m_hPal && pBkDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
	{
		SelectPalette( pBkDC->GetSafeHdc(), m_hPal, FALSE );
		pBkDC->RealizePalette();
		pBkDC->SetStretchBltMode(HALFTONE);
	}
	pOldBkBitmap = pBkDC->SelectObject(&bkBitmap);

	CBitmap DrawBmp;
	DrawBmp.CreateCompatibleBitmap(drawDC, m_CtrlSize.cx, m_CtrlSize.cy);
	CDC* pDrawBmpDC = new CDC;
	CBitmap* pOldDrawBitmap;
	pDrawBmpDC->CreateCompatibleDC(drawDC);
	if(m_hPal && pDrawBmpDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
	{
		SelectPalette( pDrawBmpDC->GetSafeHdc(), m_hPal, FALSE );
		pDrawBmpDC->RealizePalette();
		pDrawBmpDC->SetStretchBltMode(HALFTONE);
	}
	pOldDrawBitmap = pDrawBmpDC->SelectObject(&DrawBmp);

	int color = drawDC->GetDeviceCaps(BITSPIXEL) * drawDC->GetDeviceCaps(PLANES);

	if (!m_CtrlImage.IsNull())
	{
		if (m_CtrlImage.GetBPP() == 32)
		{
			CBitmap* bk32Bitmap;
			CImage bk32Image;
			if (color == 32)
			{
				bk32Bitmap = &bkBitmap;
			}
			else
			{
				bk32Image.Create(m_CtrlSize.cx, m_CtrlSize.cy, 32);
				::StretchBlt(bk32Image.GetDC(), 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, *pBkDC, 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
				bk32Bitmap = CBitmap::FromHandle((HBITMAP)bk32Image);
			}

			BITMAP CtlBmpInfo, DstBmpInfo;
			bk32Bitmap->GetBitmap(&DstBmpInfo);
			DWORD DstLineBytes = DstBmpInfo.bmWidthBytes;
			DWORD DstMemSize = DstLineBytes * DstBmpInfo.bmHeight;
			ctrlBitmap.GetBitmap(&CtlBmpInfo);
			DWORD CtlLineBytes = CtlBmpInfo.bmWidthBytes;
			DWORD CtlMemSize = CtlLineBytes * CtlBmpInfo.bmHeight;

			if ((DstBmpInfo.bmWidthBytes != CtlBmpInfo.bmWidthBytes)
			||  (DstBmpInfo.bmHeight != CtlBmpInfo.bmHeight / m_ImageCount))
			{
				// Error Check //
			}
			else
			{
				BYTE* DstBuffer = new BYTE[DstMemSize];
				bk32Bitmap->GetBitmapBits(DstMemSize, DstBuffer);
				BYTE* CtlBuffer = new BYTE[CtlMemSize];
				ctrlBitmap.GetBitmapBits(CtlMemSize, CtlBuffer);

				if (m_bMeter)
				{
					int meter = (int)(m_CtrlSize.cx * m_MeterRatio);
					int baseY;
					baseY = m_CtrlSize.cy;
					for (LONG py = 0; py < DstBmpInfo.bmHeight; py++)
					{
						int dn = py * DstLineBytes;
						int cn = (baseY + py) * CtlLineBytes;
						for (LONG px = 0; px < meter; px++)
						{
							BYTE a = CtlBuffer[cn + 3];
							BYTE na = 255 - a;
							DstBuffer[dn + 0] = (BYTE)((CtlBuffer[cn + 0] * a + DstBuffer[dn + 0] * na) / 255);
							DstBuffer[dn + 1] = (BYTE)((CtlBuffer[cn + 1] * a + DstBuffer[dn + 1] * na) / 255);
							DstBuffer[dn + 2] = (BYTE)((CtlBuffer[cn + 2] * a + DstBuffer[dn + 2] * na) / 255);
							dn += (DstBmpInfo.bmBitsPixel / 8);
							cn += (CtlBmpInfo.bmBitsPixel / 8);
						}
						cn -= baseY * CtlLineBytes;
						for (LONG px = meter; px < DstBmpInfo.bmWidth; px++)
						{
							BYTE a = CtlBuffer[cn + 3];
							BYTE na = 255 - a;
							DstBuffer[dn + 0] = (BYTE)((CtlBuffer[cn + 0] * a + DstBuffer[dn + 0] * na) / 255);
							DstBuffer[dn + 1] = (BYTE)((CtlBuffer[cn + 1] * a + DstBuffer[dn + 1] * na) / 255);
							DstBuffer[dn + 2] = (BYTE)((CtlBuffer[cn + 2] * a + DstBuffer[dn + 2] * na) / 255);
							dn += (DstBmpInfo.bmBitsPixel / 8);
							cn += (CtlBmpInfo.bmBitsPixel / 8);
						}
					}
				}
				else
				{
					int baseY = m_CtrlSize.cy * no;
					for (LONG py = 0; py < DstBmpInfo.bmHeight; py++)
					{
						int dn = py * DstLineBytes;
						int cn = (baseY + py) * CtlLineBytes;
						for (LONG px = 0; px < DstBmpInfo.bmWidth; px++)
						{
#if _MSC_VER > 1310
#pragma warning( disable : 6385 )
#pragma warning( disable : 6386 )
#endif
							BYTE a = CtlBuffer[cn + 3];
							BYTE na = 255 - a;
							DstBuffer[dn + 0] = (BYTE)((CtlBuffer[cn + 0] * a + DstBuffer[dn + 0] * na) / 255);
							DstBuffer[dn + 1] = (BYTE)((CtlBuffer[cn + 1] * a + DstBuffer[dn + 1] * na) / 255);
							DstBuffer[dn + 2] = (BYTE)((CtlBuffer[cn + 2] * a + DstBuffer[dn + 2] * na) / 255);
							dn += (DstBmpInfo.bmBitsPixel / 8);
							cn += (CtlBmpInfo.bmBitsPixel / 8);
#if _MSC_VER > 1310
#pragma warning( default : 6386 )
#pragma warning( default : 6385 )
#endif
						}
					}
				}

				if (color == 32)
				{
					DrawBmp.SetBitmapBits(DstMemSize, DstBuffer);
				}
				else
				{
					bk32Bitmap->SetBitmapBits(DstMemSize, DstBuffer);
					::StretchBlt(pDrawBmpDC->GetSafeHdc(), 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, bk32Image.GetDC(), 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
					bk32Image.ReleaseDC();
				}
				DrawString(pDrawBmpDC, lpDrawItemStruct);
				drawDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);

				delete[] DstBuffer;
				delete[] CtlBuffer;
			}
		}
		else
		{
			if (m_bMeter)
			{
				int meter = (int)(m_CtrlSize.cx * (m_MeterRatio));
				pDrawBmpDC->StretchBlt(meter, 0, m_CtrlSize.cx - meter, m_CtrlSize.cy, pMemDC, meter, m_CtrlSize.cy * 0, m_CtrlSize.cx - meter, m_CtrlSize.cy, SRCCOPY);
				pDrawBmpDC->StretchBlt(0, 0, meter, m_CtrlSize.cy, pMemDC, 0, m_CtrlSize.cy * 1, meter, m_CtrlSize.cy, SRCCOPY);
				DrawString(pDrawBmpDC, lpDrawItemStruct);
				drawDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
			}
			else
			{
				pDrawBmpDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pMemDC, 0, m_CtrlSize.cy* no, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
				DrawString(pDrawBmpDC, lpDrawItemStruct);
				drawDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
			}
		}
	}
	else
	{
		pDrawBmpDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pBkDC, 0, m_CtrlSize.cy* no, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
		DrawString(pDrawBmpDC, lpDrawItemStruct);
		drawDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, pDrawBmpDC, 0, 0, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
	}


	if (m_DrawFrame == Border::UNDERLINE)
	{
		HGDIOBJ oldPen;
		POINT point;
		CPen pen1;
		COLORREF frameColor;
		if (m_bDarkMode){frameColor = RGB(0x29, 0x2B, 0x2F);} // Windows 11 color
		else{	 		 frameColor = RGB(0xCC, 0xCC, 0xCC);}
		pen1.CreatePen(PS_SOLID, 1, frameColor);

		oldPen = SelectObject(drawDC->m_hDC, pen1);
		MoveToEx(drawDC->m_hDC, 0, m_CtrlSize.cy - 1, &point);
		LineTo(drawDC->m_hDC, m_CtrlSize.cx - 1, m_CtrlSize.cy - 1);
		LineTo(drawDC->m_hDC, 0, m_CtrlSize.cy - 1);
		SelectObject(drawDC->m_hDC, oldPen);

		pen1.DeleteObject();
	}
	else if (m_DrawFrame)
	{
		HGDIOBJ oldPen;
		POINT point;
		CPen pen1; pen1.CreatePen(PS_SOLID, 1, RGB(0xF8, 0xF8, 0xF8));
		CPen pen2; pen2.CreatePen(PS_SOLID, 1, RGB(0x98, 0x98, 0x98));

		oldPen = SelectObject(drawDC->m_hDC, pen1);
		MoveToEx(drawDC->m_hDC, 0, m_CtrlSize.cy - 1, &point);
		LineTo(drawDC->m_hDC, m_CtrlSize.cx - 1, m_CtrlSize.cy - 1);
		LineTo(drawDC->m_hDC, m_CtrlSize.cx - 1, 0);
		LineTo(drawDC->m_hDC, m_CtrlSize.cx - 1, m_CtrlSize.cy - 1);
		SelectObject(drawDC->m_hDC, pen2);
		MoveToEx(drawDC->m_hDC, 0, m_CtrlSize.cy - 2, &point);
		LineTo(drawDC->m_hDC, 0, 0);
		LineTo(drawDC->m_hDC, m_CtrlSize.cx - 1, 0);
		SelectObject(drawDC->m_hDC, oldPen);

		pen1.DeleteObject();
		pen2.DeleteObject();
	}

	pDrawBmpDC->SelectObject(&pOldDrawBitmap);
	pDrawBmpDC->DeleteDC();
	delete pDrawBmpDC;
	pMemDC->SelectObject(&pOldMemBitmap);
	pMemDC->DeleteDC();
	delete pMemDC;
	pBkDC->SelectObject(&pOldBkBitmap);
	pBkDC->DeleteDC();
	delete pBkDC;

	if (m_bDrawFrameEx)
	{
		CBrush brush;
		brush.CreateSolidBrush(m_FrameColor);
		drawDC->FrameRect(&(lpDrawItemStruct->rcItem), &brush);
		brush.DeleteObject();
	}
}

void CStaticFx::DrawString(CDC* drawDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString title;
	GetWindowText(title);

	if (title.IsEmpty())
	{
		return;
	}

	drawDC->SetBkMode(TRANSPARENT);
	CRect rect = (CRect)(lpDrawItemStruct->rcItem);
	CRect rectControl = (CRect)(lpDrawItemStruct->rcItem);
	rect.top += m_Margin.top;
	rect.left += m_Margin.left;
	rect.bottom -= m_Margin.bottom;
	rect.right -= m_Margin.right;

	CRect rectI;
	CSize extent;
	HGDIOBJ oldFont = drawDC->SelectObject(m_Font);
	if ((m_RenderMode & OwnerDrawTransparent) && m_bDarkMode)
	{
		SetTextColor(drawDC->m_hDC, RGB(255, 255, 255));
	}
	else
	{
		SetTextColor(drawDC->m_hDC, m_TextColor);
	}
	extent = drawDC->GetTextExtent(title);

	if (m_bMeter && rect.Width() < extent.cx)
	{
		title.Replace(_T(","), _T("."));
		int score = _tstoi((LPCTSTR)title);
		title.Format(_T("%d"), score);
		extent = drawDC->GetTextExtent(title);
	}

	if (m_TextFormat != 0)
	{
		drawDC->DrawText(title, title.GetLength(), rect, m_TextFormat);
		drawDC->SelectObject(oldFont);

		oldFont = drawDC->SelectObject(m_FontToolTip);
		drawDC->DrawText(m_Label, m_Label.GetLength(), rect, m_LabelFormat);
		drawDC->DrawText(m_Unit, m_Unit.GetLength(), rect, m_UnitFormat);
	}
	else if (!m_Label.IsEmpty())
	{
		drawDC->DrawText(title, title.GetLength(), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		drawDC->SelectObject(oldFont);

		oldFont = drawDC->SelectObject(m_FontToolTip);
		drawDC->DrawText(m_Label, m_Label.GetLength(), rect, m_LabelFormat);
		drawDC->DrawText(m_Unit, m_Unit.GetLength(), rect, m_UnitFormat);
	}
	else if (m_TextAlign == SS_LEFT)
	{
		drawDC->DrawText(title, title.GetLength(), rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
	else if (m_TextAlign == SS_RIGHT)
	{
		drawDC->DrawText(title, title.GetLength(), rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		drawDC->DrawText(title, title.GetLength(), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	drawDC->SelectObject(oldFont);
}

//------------------------------------------------
// Image
//------------------------------------------------

BOOL CStaticFx::LoadBitmap(LPCTSTR fileName)
{
	if (m_bHighContrast) { return FALSE; }
	if (fileName == NULL) { return FALSE; }

	m_CtrlImage.Destroy();
	m_CtrlImage.Load(fileName);
	if (m_CtrlImage.IsNull()) { return FALSE; }

	return LoadBitmap((HBITMAP)m_CtrlImage);
}

BOOL CStaticFx::LoadBitmap(HBITMAP hBitmap)
{
	if (m_bHighContrast) { return FALSE; }

	m_CtrlBitmap.Detach();
	m_CtrlBitmap.Attach(hBitmap);

	return SetBitmap(m_CtrlBitmap);
}

void CStaticFx::SetBkReload(void)
{
	m_bBkBitmapInit = FALSE;
	m_bBkLoad = FALSE;
}

BOOL CStaticFx::SetBitmap(CBitmap& bitmap)
{
	if (m_bHighContrast) { return FALSE; }

	BITMAP	bitmapInfo;
	bitmap.GetBitmap(&bitmapInfo);
	if (m_CtrlSize.cx != bitmapInfo.bmWidth
	||  m_CtrlSize.cy != bitmapInfo.bmHeight / m_ImageCount)
	{
		ModifyStyle(SS_OWNERDRAW, 0);
		return FALSE;
	}
	else
	{
		ModifyStyle(0, SS_OWNERDRAW);
		return TRUE;
	}
}

void CStaticFx::LoadCtrlBk(CDC* drawDC)
{
	if (m_bHighContrast) { SetBkReload(); return; }

	if (m_BkBitmap.m_hObject != NULL)
	{
		BITMAP bitmapInfo;
		m_BkBitmap.GetBitmap(&bitmapInfo);
		if (bitmapInfo.bmBitsPixel != drawDC->GetDeviceCaps(BITSPIXEL))
		{
			SetBkReload();
		}
	}

	if (&m_CtrlBitmap != NULL)
	{
		if (!m_bBkBitmapInit)
		{
			m_BkBitmap.DeleteObject();
			m_BkBitmap.CreateCompatibleBitmap(drawDC, m_CtrlSize.cx, m_CtrlSize.cy);
			m_bBkBitmapInit = TRUE;
		}

		if (!m_bBkLoad)
		{
			CBitmap* pOldBitmap;
			CDC* pMemDC = new CDC;
			pMemDC->CreateCompatibleDC(drawDC);
			pOldBitmap = pMemDC->SelectObject(&m_BkBitmap);
			pMemDC->StretchBlt(0, 0, m_CtrlSize.cx, m_CtrlSize.cy, m_BkDC, m_X, m_Y, m_CtrlSize.cx, m_CtrlSize.cy, SRCCOPY);
			pMemDC->SelectObject(pOldBitmap);
			pMemDC->DeleteDC();
			delete pMemDC;
			m_bBkLoad = TRUE;
		}
	}
}

//------------------------------------------------
// Font
//------------------------------------------------

void CStaticFx::SetFontEx(CString face, int size, int sizeToolTip, double zoomRatio, double fontRatio,
     COLORREF textColor, LONG fontWeight, BYTE fontRender)
{
	LOGFONT logFont = { 0 };
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfHeight = (LONG)(-1 * size * zoomRatio * fontRatio);
	logFont.lfQuality = fontRender;
	logFont.lfWeight = fontWeight;
	if (face.GetLength() < 32)
	{
		wsprintf(logFont.lfFaceName, _T("%s"), (LPCTSTR)face);
	}
	else
	{
		wsprintf(logFont.lfFaceName, _T(""));
	}

	m_Font.DeleteObject();
	m_Font.CreateFontIndirect(&logFont);
	SetFont(&m_Font);

	logFont.lfHeight = (LONG)(-1 * sizeToolTip * zoomRatio * fontRatio);
	m_FontToolTip.DeleteObject();
	m_FontToolTip.CreateFontIndirect(&logFont);

	m_TextColor = textColor;

	if (m_ToolTip.m_hWnd != NULL)
	{
		m_ToolTip.SetFont(&m_FontToolTip);
	}
}

//------------------------------------------------
// Mouse
//------------------------------------------------

void CStaticFx::SetHandCursor(BOOL bHandCuror)
{
	m_bHandCursor = bHandCuror;
}

void CStaticFx::OnMouseMove(UINT nFlags, CPoint point)
{
#if _MSC_VER <= 1310
	typedef BOOL(WINAPI* Func_TrackMouseEvent)(LPTRACKMOUSEEVENT);
	static Func_TrackMouseEvent p_TrackMouseEvent = NULL;
	static BOOL bInit_TrackMouseEvent = FALSE;

	if (bInit_TrackMouseEvent && p_TrackMouseEvent == NULL)
	{
		return; // TrackMouseEvent is not available
	}
	else
	{
		HMODULE hModule = GetModuleHandle(_T("user32.dll"));
		if (hModule)
		{
			p_TrackMouseEvent = (Func_TrackMouseEvent)GetProcAddress(hModule, "TrackMouseEvent");
		}
	}

	if (p_TrackMouseEvent != NULL)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTrackingNow = p_TrackMouseEvent(&tme);
	}
	bInit_TrackMouseEvent = TRUE;
#else
	if (!m_bTrackingNow)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTrackingNow = _TrackMouseEvent(&tme);
	}
#endif

	CStatic::OnMouseMove(nFlags, point);
}

void CStaticFx::OnMouseHover(UINT nFlags, CPoint point)
{
#if _MSC_VER > 1310
	CStatic::OnMouseHover(nFlags, point);
#endif

	m_bHover = TRUE;
	Invalidate();
}

void CStaticFx::OnMouseLeave()
{
#if _MSC_VER > 1310
	CStatic::OnMouseLeave();
#endif

	m_bTrackingNow = FALSE;
	m_bHover = FALSE;
	Invalidate();
}

void CStaticFx::OnSetfocus()
{
	m_bFocas = TRUE;
	Invalidate();
}

void CStaticFx::OnKillfocus()
{
	m_bFocas = FALSE;
	Invalidate();
}

BOOL CStaticFx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	HCURSOR hCursor = NULL;
	if (m_bHandCursor)
	{
		hCursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);
		if (hCursor)
		{
			::SetCursor(hCursor);
		}
	}
	else
	{
		hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		if (hCursor)
		{
			::SetCursor(hCursor);
		}
	}

	return TRUE;
}

//------------------------------------------------
// ToolTip
//------------------------------------------------

void CStaticFx::SetToolTipText(LPCTSTR text)
{
	if (text == NULL) { return; }

	InitToolTip();
	m_ToolTipText = text;
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rect;
		GetClientRect(rect);
		m_ToolTip.AddTool(this, m_ToolTipText, rect, 1);
	}
	else
	{
		m_ToolTip.UpdateTipText(m_ToolTipText, this, 1);
	}

	SetToolTipActivate(TRUE);
}

void CStaticFx::SetToolTipActivate(BOOL bActivate)
{
	if (m_ToolTip.GetToolCount() == 0) { return; }
	m_ToolTip.Activate(bActivate);
}

void CStaticFx::SetToolTipWindowText(LPCTSTR pText)
{
	SetToolTipText(pText);
	SetWindowText(pText);
}

CString CStaticFx::GetToolTipText()
{
	return m_ToolTipText;
}

void CStaticFx::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		m_ToolTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOANIMATE | TTS_NOFADE);
		m_ToolTip.Activate(FALSE);
		m_ToolTip.SetFont(&m_FontToolTip);
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 1024);
		m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 8000);
		m_ToolTip.SetDelayTime(TTDT_INITIAL, 500);
		m_ToolTip.SetDelayTime(TTDT_RESHOW, 100);
	}
}

BOOL CStaticFx::PreTranslateMessage(MSG* pMsg)
{
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);

	return CStatic::PreTranslateMessage(pMsg);
}

BOOL CStaticFx::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}