#include "ipc/ipc.h"
#include "candidate_window.h"
#include "defines/defines.h"
#include "defines/globals.h"
#include "utils/common_utils.h"
#include "utils/window_utils.h"
#include <debugapi.h>
#include <minwindef.h>
#include <string>
#include <windef.h>
#include <winnt.h>
#include <winuser.h>
#include "ime_engine/shuangpin/pinyin_utils.h"
#include <d2d1.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <wrl.h>
// #include "fmt/xchar.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "dwmapi.lib")

using namespace Microsoft::WRL;

ID2D1Factory *pD2DFactory = nullptr;
ID2D1HwndRenderTarget *pRenderTarget = nullptr;
ID2D1SolidColorBrush *pBrush = nullptr;
IDWriteFactory *pDWriteFactory = nullptr;
IDWriteTextFormat *pTextFormat = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

bool InitD2DAndDWrite()
{
    // Direct2D
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr))
        return false;

    // DirectWrite
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (FAILED(hr))
        return false;

    // TextFormat
    hr = pDWriteFactory->CreateTextFormat( //
        L"Noto Sans SC",                   //
        nullptr,                           //
        DWRITE_FONT_WEIGHT_NORMAL,         //
        DWRITE_FONT_STYLE_NORMAL,          //
        DWRITE_FONT_STRETCH_NORMAL,        //
        18.0f,                             // Font size
        L"zh-cn",                          //
        &pTextFormat                       //
    );

    if (FAILED(hr))
        return false;

    return true;
}

bool InitD2DRenderTarget(HWND hwnd)
{
    if (!pD2DFactory)
        return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(                             //
        D2D1::RenderTargetProperties(                                             //
            D2D1_RENDER_TARGET_TYPE_DEFAULT,                                      //
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED) //
            ),                                                                    //
        D2D1::HwndRenderTargetProperties(                                         //
            hwnd,                                                                 //
            D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top),                  //
            D2D1_PRESENT_OPTIONS_IMMEDIATELY                                      //
            ),                                                                    //
        &pRenderTarget                                                            //
    );

    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
    }

    return SUCCEEDED(hr);
}

bool IsHalfWidth(wchar_t ch)
{
    return (ch >= L'0' && ch <= L'9')    //
           || (ch >= L'A' && ch <= L'Z') //
           || (ch >= L'a' && ch <= L'z') //
           || ch == L'.'                 //
           || ch == L'\''                //
           || ch == L' ';
}

int GetMaxLenWordIndex()
{
    int maxLen = 0;
    int maxIndex = 0;
    int i = 0;
    for (auto candWord : Global::CandidateWordList)
    {
        if (candWord.length() > maxLen)
        {
            maxLen = candWord.length();
            maxIndex = i;
        }
        i++;
    }

    return maxIndex;
}

float MeasureTextWidth(IDWriteFactory *pDWriteFactory, IDWriteTextFormat *pTextFormat, const std::wstring &text)
{
    ComPtr<IDWriteTextLayout> pTextLayout;

    HRESULT hr = pDWriteFactory->CreateTextLayout( //
        text.c_str(),                              //
        static_cast<UINT32>(text.length()),        //
        pTextFormat,                               //
        1000.0f,                                   // 最大允许宽度（足够大以避免换行）
        1000.0f,                                   // 最大允许高度
        &pTextLayout                               //
    );

    if (FAILED(hr) || !pTextLayout)
        return 0.0f;

    DWRITE_TEXT_METRICS metrics;
    hr = pTextLayout->GetMetrics(&metrics);
    if (FAILED(hr))
        return 0.0f;

    return metrics.width; // 返回精确宽度（单位：像素）
}

void PaintCandidates(HWND hwnd, std::wstring &text)
{
    if (!pRenderTarget)
        return;

    pRenderTarget->BeginDraw();

    pRenderTarget->Clear(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f));

    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));

    std::vector<std::wstring> lines;
    size_t start = 0;
    size_t end = text.find(L'\n');
    while (end != std::wstring::npos)
    {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find(L'\n', start);
    }
    lines.push_back(text.substr(start)); //

    float lineHeight = 26.0f; //
    float x = 8.0f;           //
    float y = 0.0f;           //

    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i == 1)
        {
            float radius = 6.0f;
            float width = MeasureTextWidth(pDWriteFactory, pTextFormat, lines[i]);
            if (width < 56)
            {
                width = 56;
            }
            D2D1_ROUNDED_RECT roundedRect = {
                D2D1::RectF(                //
                    x - 3.0f,               //
                    y,                      //
                    x + width + 5.0f,       //
                    y + lineHeight - 1.0f), //
                radius,                     //
                radius                      //
            };
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LightBlue, 0.3f));
            pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Pink, 1.0f));
        }
        else
        {
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
        }

        pRenderTarget->DrawText(                       //
            lines[i].c_str(),                          //
            static_cast<UINT32>(lines[i].length()),    //
            pTextFormat,                               //
            D2D1::RectF(x, y, 590.0f, y + lineHeight), //
            pBrush                                     //
        );

        // Update y coordinate
        y += lineHeight;
    }

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
        InitD2DRenderTarget(hwnd);
    }

    ValidateRect(hwnd, nullptr);
}

LRESULT RegisterCandidateWindowMessage()
{

    WM_SHOW_MAIN_WINDOW = RegisterWindowMessage(L"WM_SHOW_MAIN_WINDOW");
    WM_HIDE_MAIN_WINDOW = RegisterWindowMessage(L"WM_HIDE_MAIN_WINDOW");
    WM_MOVE_CANDIDATE_WINDOW = RegisterWindowMessage(L"WM_MOVE_CANDIDATE_WINDOW");
    return 0;
}

LRESULT RegisterCandidateWindowClass(WNDCLASSEX &wcex, HINSTANCE hInstance)
{
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL; // Prevent background painting, otherwise it will be flickering
    // wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(                             //
            NULL,                               //
            L"Call to RegisterClassEx failed!", //
            L"Windows Desktop Guided Tour",     //
            NULL                                //
        );                                      //
        return 1;
    }
    return 0;
}

int CreateCandidateWindow(HINSTANCE hInstance)
{
    if (!InitD2DAndDWrite())
        return -1;

    DWORD dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE; //
    HWND hWnd = CreateWindowEx(                                                            //
        dwExStyle,                                                                         //
        szWindowClass,                                                                     //
        lpWindowName,                                                                      //
        WS_POPUP,                                                                          //
        300,                                                                               //
        1500,                                                                              //
        (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),                                 //
        (26.0f * 9 * 1.5 + ::CandidateWndMargin),                                          //
        nullptr,                                                                           //
        nullptr,                                                                           //
        hInstance,                                                                         //
        nullptr                                                                            //
    );                                                                                     //

    if (!hWnd)
    {
        MessageBox(                          //
            NULL,                            //
            L"Call to CreateWindow failed!", //
            L"Windows Desktop Guided Tour",  //
            NULL                             //
        );                                   //
        return 1;
    }
    else
    {
        // Set the window to be fully transparent
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        MARGINS mar = {-1};
        DwmExtendFrameIntoClientArea(hWnd, &mar);
    }

    ::global_hwnd = hWnd;

    SetWindowPos(                                          //
        hWnd,                                              //
        HWND_TOPMOST,                                      //
        300,                                               //
        1500,                                              //
        (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH), //
        (26.0f * 9 * 1.5 + ::CandidateWndMargin),          //
        SWP_SHOWWINDOW);                                   //

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    OutputDebugString(L"CreateCandidateWindow");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Release resources
    if (pBrush)
        pBrush->Release();
    if (pRenderTarget)
        pRenderTarget->Release();
    if (pDWriteFactory)
        pDWriteFactory->Release();
    if (pTextFormat)
        pTextFormat->Release();
    if (pD2DFactory)
        pD2DFactory->Release();
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_SHOW_MAIN_WINDOW)
    {
        int caretX = Global::Point[0];
        int caretY = Global::Point[1];
        POINT pt = {caretX, caretY};

        ::ReadDataFromSharedMemory(0b100000);
        // TODO: rewrite InflateCandidateWindow(str);
        int maxIndex = GetMaxLenWordIndex();
        int wndWidth = MeasureTextWidth(   //
                           pDWriteFactory, //
                           pTextFormat,    //
                           L"1. " + Global::CandidateWordList[maxIndex]) *
                           1.5 +
                       20 * 1.5;
        int wndHeight = (26.0f * (Global::CandidateWordList.size() + 1) * 1.5 + ::CandidateWndMargin * 1.5);
        if (wndWidth < 125)
        {
            wndWidth = 125;
        }
        SetWindowPos(                 //
            hWnd,                     //
            nullptr,                  //
            0,                        //
            0,                        //
            wndWidth,                 //
            wndHeight,                //
            SWP_NOMOVE | SWP_NOZORDER //
        );
        InvalidateRect(hWnd, NULL, FALSE);
        ShowWindow(hWnd, SW_SHOWNOACTIVATE);
        return 0;
    }
    if (message == WM_HIDE_MAIN_WINDOW)
    {
        ShowWindow(hWnd, SW_HIDE);
        // TODO: rewrite UpdateHtmlContentWithJavaScript(webview, L"");
        return 0;
    }
    if (message == WM_MOVE_CANDIDATE_WINDOW)
    {
        int caretX = Global::Point[0];
        int caretY = Global::Point[1];

        RECT rc;
        GetClientRect(hWnd, &rc);

        POINT pt = {caretX, caretY};
        int wndWidth = rc.right - rc.left;
        int wndHeight = rc.bottom - rc.top;

        POINT adjustedPos = {caretX, caretY};
        AdjustCandidateWindowPosition( //
            &pt,                       //
            wndWidth,                  //
            wndHeight,                 //
            &adjustedPos               //
        );

        SetWindowPos(                 //
            hWnd,                     //
            nullptr,                  //
            adjustedPos.x,            //
            adjustedPos.y,            //
            0,                        //
            0,                        //
            SWP_NOSIZE | SWP_NOZORDER //
        );

        return 0;
    }

    switch (message)
    {

    case WM_ERASEBKGND:
        return 1;

    case WM_CREATE:
        if (!InitD2DRenderTarget(hWnd))
        {
            // TODO: Error handle
        }
        return 0;

    case WM_MOUSEMOVE: {
        float x = (float)LOWORD(lParam);
        float y = (float)HIWORD(lParam);

        RECT rc;
        GetClientRect(hWnd, &rc);
        if (x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom)
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        break;
    }

    case WM_SIZE: {
        if (pRenderTarget)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            pRenderTarget->Resize(D2D1::SizeU(width, height));
        }
        return 0;
    }

    case WM_PAINT: {
        std::wstring embeded_pinyin = string_to_wstring(                             //
            PinyinUtil::pinyin_segmentation(wstring_to_string(Global::PinyinString)) //
        );
        std::wstring str = embeded_pinyin + L"," + Global::CandidateString;
        boost::algorithm::replace_all(str, ",", "\n");
        PaintCandidates(hWnd, str);
        return 0;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}