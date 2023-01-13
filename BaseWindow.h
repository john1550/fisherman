#pragma once
#include <windows.h>
#include <sstream>

//Flag definitions for Base Window flags
#define BWF_sbVertKeyb		0x0001		// Vertical scrollbar keyboard control
#define BWF_sbVertMouse		0x0002		// Vertical scrollbar mouse control
#define BWF_sbHorizKeyb		0x0004		// Horizontal scrollbar keyboard control
#define BWF_sbHorizMouse		0x0008		// Horizontal scrollbar mouse control
#define BWF_MaxFlag			0x0010		// Maximum possible flag value + 1

template <class DERIVED_TYPE>

class BaseWindow
{
public:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = reinterpret_cast<DERIVED_TYPE*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
			pThis = reinterpret_cast<DERIVED_TYPE*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (pThis)
		{
			if (uMsg == WM_KEYDOWN)
			{
				// If user wants keyboard scrolling, go fo it!
				if (pThis->GetFlags() & BWF_sbVertKeyb)
				{
					WPARAM wpScrollAmt;

					switch (wParam)
					{
					case VK_HOME:
						wpScrollAmt = MAKEWPARAM(SB_TOP, NULL);
						break;
					case VK_END:
						wpScrollAmt = MAKEWPARAM(SB_BOTTOM, NULL);
						break;
					case VK_UP:
						wpScrollAmt = MAKEWPARAM(SB_LINEUP, NULL);
						break;
					case VK_DOWN:
						wpScrollAmt = MAKEWPARAM(SB_LINEDOWN, NULL);
						break;
					case VK_PRIOR:
						wpScrollAmt = MAKEWPARAM(SB_PAGEUP, NULL);
						break;
					case VK_NEXT:
						wpScrollAmt = MAKEWPARAM(SB_PAGEDOWN, NULL);
						break;
					default:
						return pThis->HandleMessage(uMsg, wParam, lParam);
					}
					SendMessage(hwnd, WM_VSCROLL, wpScrollAmt, NULL);
					return 0;
				}
			}
			if (uMsg == WM_MOUSEWHEEL)
			{
				// If user wants mouse scrolling
				if (pThis->GetFlags() & BWF_sbVertMouse)
				{
					int16_t nscroll = HIWORD(wParam);
					nscroll /= WHEEL_DELTA;
					int16_t ndirection = (nscroll > 0 ? 1 : -1);
					nscroll = abs(nscroll);
					for (int i = 0; i < nscroll; i++)
					{
						WPARAM wpScrollAmt = (ndirection > 0 ? MAKEWPARAM(SB_LINEUP, NULL) : MAKEWPARAM(SB_LINEDOWN, NULL));
						SendMessage(hwnd, WM_VSCROLL, wpScrollAmt, NULL);
					}
				}
				else
					return pThis->HandleMessage(uMsg, wParam, lParam);
			}
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	BaseWindow() : m_hwnd(NULL), wBaseWindowFlags(0) {}

	BOOL Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		HMENU hMenu = 0,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		HWND hwndParent = 0
		)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc		= DERIVED_TYPE::WindowProc;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.lpszClassName	= ClassName();
		wc.hIcon			= AppIconHandle();

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hwndParent, hMenu, GetModuleHandle(NULL), this);

		// If window style includes scrollbars assume user wants keyboard and mouse control
		if (dwStyle & WS_VSCROLL) wBaseWindowFlags |= (BWF_sbVertKeyb | BWF_sbVertMouse);
		if (dwStyle & WS_HSCROLL) wBaseWindowFlags |= (BWF_sbHorizMouse | BWF_sbHorizKeyb);

		return (m_hwnd ? TRUE : FALSE);
	}

	HWND Window() const { return m_hwnd; }

	WORD GetFlags() const
	{
		return wBaseWindowFlags;
	}

	WORD SetFlag(const unsigned int flag)
	{
		if (flag < BWF_MaxFlag) wBaseWindowFlags |= flag;
		return GetFlags();
	}

	WORD ResetFlag(const unsigned int flag)
	{
		if (flag < BWF_MaxFlag) wBaseWindowFlags &= (0x0ffff - flag);
		return GetFlags();
	}

protected:

	virtual PCWSTR ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual HICON AppIconHandle() const = 0;

	HWND m_hwnd;

private:
	WORD wBaseWindowFlags = 0;
};
