#include <windows.h>
#include <string>
#include <d2d1.h>
#include <math.h>
#pragma comment(lib, "d2d1")

#include "BaseWindow.h"
#include "resource.h"
#include "userdlgs.h"


#define PI 3.14159265

using namespace std;

// Current version of code
wstring curVersion = L"V1.0";

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    D2D1_ELLIPSE            ellipse;

    D2D1_POINT_2F           ballPosn = {};
    float                   ballDirection = 30;
    float                   xDelta = 0.0;
    float                   yDelta = 0.0;
    int16_t                 ballSize = 10;
    int16_t                 ballSpeed = 15;
    bool                    ballRunning = false;
    uint16_t                timerRate = 15;

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"MainWindow"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HICON AppIconHandle() const { return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)); }
};

// Recalculate drawing layout when the size of the window changes.

void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        if (!ballPosn.x && !ballPosn.y)
        {
            D2D1_SIZE_F size = pRenderTarget->GetSize();
            ballPosn.x = size.width / 2;
            ballPosn.y = size.height / 2;
        }
        else
        {
            if (ballRunning)
            {
                D2D1_SIZE_F size = pRenderTarget->GetSize();

                //Check if we have reached left edge
                if (ballPosn.x <= ballSize)
                {
                    ballPosn.x = ballSize;
                    if (ballDirection <= 180 && ballDirection > 90)
                        //Descending
                        ballDirection = 180 - ballDirection;
                    else
                        //Ascending
                        ballDirection = 540 - ballDirection;
                }
                //Check iif right edge reached
                if (ballPosn.x >= (size.width - ballSize))
                {
                    ballPosn.x = (size.width - ballSize);
                    if (ballDirection >= 270 && ballDirection < 360)
                        //Ascending
                        ballDirection = 540 - ballDirection;
                    else
                        //Descending
                        ballDirection = 180 - ballDirection;
                }

                if (ballPosn.y <= ballSize)
                {
                    ballPosn.y = ballSize;
                    ballDirection = 360 - ballDirection;
                }

                if (ballPosn.y >= (size.height - ballSize))
                {
                    ballPosn.y = (size.height - ballSize);
                    ballDirection = 360 - ballDirection;
                }

                //Get a new ball position
                ballPosn.x += (ballSpeed * cos(ballDirection * PI / 180));
                ballPosn.y += (ballSpeed * sin(ballDirection * PI / 180));

            }
        }
        ellipse = D2D1::Ellipse(ballPosn, ballSize, ballSize);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        pRenderTarget->FillEllipse(ellipse, pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;

    HMENU myMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
    if (!win.Create(L"Bouncing Ball", WS_OVERLAPPEDWINDOW, 0, myMenu, 500, 500))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_MENU_EXIT:
            DestroyWindow(m_hwnd);
            break;

        case ID_MENU_START:
            ballRunning = true;
            SetTimer(m_hwnd, 1, timerRate, NULL);
            break;

        case ID_MENU_STOP:
            ballRunning = false;
            KillTimer(m_hwnd, 1);
            break;

        case ID_MENU_SPEED:
        {
            UserDialog* myDlg = new UserDialog;
            myDlg->SetTitle(L"2D Graphics Demo Ball Speed");
            myDlg->SetStatic(L"Enter new ball speed (1 - 50)");
            myDlg->SetButton(UD_OKCANCEL);
            myDlg->SetEdit(to_wstring(ballSpeed));
            if (myDlg->Show(MsgType::INTEXT, m_hwnd) == IDOK)
            {
                int16_t nSpeed = ballSpeed;
                try
                {
                    ballSpeed = stoi(myDlg->GetEdit());
                }
                catch (...) {}
                if (ballSpeed < 1 or ballSpeed > 50) ballSpeed = nSpeed;
            }
            delete myDlg;
            break;
        }

        case ID_MENU_SIZE:
        {
            UserDialog* myDlg = new UserDialog;
            myDlg->SetTitle(L"2D Graphics Demo Ball Size");
            myDlg->SetStatic(L"Enter new ball size (4 to 50)");
            myDlg->SetButton(UD_OKCANCEL);
            myDlg->SetEdit(to_wstring(ballSize));
            if (myDlg->Show(MsgType::INTEXT, m_hwnd) == IDOK)
            {
                int16_t nSize = ballSize;
                try
                {
                    ballSize = stoi(myDlg->GetEdit());
                }
                catch (...) {}
                if (ballSize < 4 or ballSize > 50) ballSize = nSize;
            }
            delete myDlg;
            break;
        }
        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }
        return 0;

    case WM_SIZE:
        Resize();
        return 0;

    case WM_TIMER:
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
        SetTimer(m_hwnd, 1, timerRate, NULL);
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}