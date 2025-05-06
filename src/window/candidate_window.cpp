#include "boost/algorithm/string/split.hpp"
#include "ipc/ipc.h"
#include "candidate_window.h"
#include "defines/defines.h"
#include "defines/globals.h"
#include "spdlog/spdlog.h"
#include "utils/common_utils.h"
#include <debugapi.h>
#include <minwindef.h>
#include <string>
#include <windef.h>
#include <winnt.h>
#include <winuser.h>
#include "ime_engine/shuangpin/pinyin_utils.h"
// Direct2D
#include <d2d1.h>
#include <dwrite.h>
#include <dwmapi.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "dwmapi.lib")

// 全局变量
ID2D1Factory *pD2DFactory = nullptr;
ID2D1HwndRenderTarget *pRenderTarget = nullptr;
ID2D1SolidColorBrush *pBrush = nullptr;
IDWriteFactory *pDWriteFactory = nullptr;
IDWriteTextFormat *pTextFormat = nullptr;

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// 初始化 Direct2D 和 DirectWrite
bool InitD2DAndDWrite()
{

    // D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &g_pD2DFactory);
    // 初始化 Direct2D 工厂
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr))
        return false;

    // 初始化 DirectWrite 工厂
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                             reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (FAILED(hr))
        return false;

    // 创建文本格式
    hr = pDWriteFactory->CreateTextFormat( //
        L"微软雅黑",                       // 字体
        nullptr,                           // 字体集合
        DWRITE_FONT_WEIGHT_NORMAL,         //
        DWRITE_FONT_STYLE_NORMAL,          //
        DWRITE_FONT_STRETCH_NORMAL,        //
        18.0f,                             // 字体大小
        L"zh-cn",                          // 本地化
        &pTextFormat                       // 输出文本格式对象
    );
    if (FAILED(hr))
        return false;

    return true;
}

// 初始化 Direct2D 渲染目标
bool InitD2DRenderTarget(HWND hwnd)
{
    if (!pD2DFactory)
        return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                     D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top),
                                         D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &pRenderTarget);

    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
    }

    return SUCCEEDED(hr);
}

// 绘制内容
void OnPaint(HWND hwnd)
{
    if (!pRenderTarget)
        return;

    pRenderTarget->BeginDraw();

    // 清除背景为暗色
    pRenderTarget->Clear(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f));

    // 绘制文本（使用 TextLayout 支持自动换行和行距设置）
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));

    std::wstring text = L"ll'zi\n1. 量子\n2. 笔画\n3. 拼音\n4. 汉字\n5. 牛魔\n6. 可恶\n7. 不是\n8. 好吧";
    // 分割文本为多行
    std::vector<std::wstring> lines;
    size_t start = 0;
    size_t end = text.find(L'\n');
    while (end != std::wstring::npos)
    {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find(L'\n', start);
    }
    lines.push_back(text.substr(start)); // 添加最后一行

    // 设置行间距（可选）
    float lineHeight = 26.0f; // 假设每行高度为 28（你可以调整）
    float x = 8.0f;           // X 起始位置
    float y = 5.0f;           // Y 起始位置

    // 绘制文本，每行设置背景色
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i == 1)
        {
            // 设置圆角矩形的半径
            float radius = 6.0f; // 圆角半径（你可以调整）

            // 创建圆角矩形
            D2D1_ROUNDED_RECT roundedRect = {
                D2D1::RectF(x - 3.0f, y, x + 140.0f / 1.5 - 2, y + lineHeight - 1.0f), // 矩形区域
                radius,                                                                // 圆角半径
                radius                                                                 // 圆角半径
            };

            // 设置背景色
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LightBlue, 0.3f)); // 设置半透明背景色

            // 绘制圆角矩形背景
            pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Pink, 1.0f)); // 设置文本颜色
        }
        else
        {
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f)); // 设置文本颜色
        }

        // 绘制文本
        pRenderTarget->DrawText(lines[i].c_str(),                          // 文本内容
                                static_cast<UINT32>(lines[i].length()),    // 文本长度
                                pTextFormat,                               // 文本格式
                                D2D1::RectF(x, y, 590.0f, y + lineHeight), // 绘制区域
                                pBrush                                     // 画刷
        );

        // 更新 y 坐标以控制行间距
        y += lineHeight;
    }

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        InitD2DRenderTarget(hwnd);
    }
}

void PaintCandidates(HWND hwnd, std::wstring &text)
{
    if (!pRenderTarget)
        return;

    pRenderTarget->BeginDraw();

    // 清除背景为暗色
    pRenderTarget->Clear(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f));

    // 绘制文本（使用 TextLayout 支持自动换行和行距设置）
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));

    // 分割文本为多行
    std::vector<std::wstring> lines;
    size_t start = 0;
    size_t end = text.find(L'\n');
    while (end != std::wstring::npos)
    {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find(L'\n', start);
    }
    lines.push_back(text.substr(start)); // 添加最后一行

    // 设置行间距（可选）
    float lineHeight = 26.0f; // 假设每行高度为 28（你可以调整）
    float x = 8.0f;           // X 起始位置
    float y = 5.0f;           // Y 起始位置

    // 绘制文本，每行设置背景色
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i == 1)
        {
            // 设置圆角矩形的半径
            float radius = 6.0f; // 圆角半径（你可以调整）

            // 创建圆角矩形
            D2D1_ROUNDED_RECT roundedRect = {
                D2D1::RectF(x - 3.0f, y, x + 140.0f / 1.5 - 2, y + lineHeight - 1.0f), // 矩形区域
                radius,                                                                // 圆角半径
                radius                                                                 // 圆角半径
            };

            // 设置背景色
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LightBlue, 0.3f)); // 设置半透明背景色

            // 绘制圆角矩形背景
            pRenderTarget->FillRoundedRectangle(roundedRect, pBrush);
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Pink, 1.0f)); // 设置文本颜色
        }
        else
        {
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f)); // 设置文本颜色
        }

        // 绘制文本
        pRenderTarget->DrawText(lines[i].c_str(),                          // 文本内容
                                static_cast<UINT32>(lines[i].length()),    // 文本长度
                                pTextFormat,                               // 文本格式
                                D2D1::RectF(x, y, 590.0f, y + lineHeight), // 绘制区域
                                pBrush                                     // 画刷
        );

        // 更新 y 坐标以控制行间距
        y += lineHeight;
    }

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        InitD2DRenderTarget(hwnd);
    }
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
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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

    DWORD dwExStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW;      //
    HWND hWnd = CreateWindowEx(                              //
        dwExStyle,                                           //
        szWindowClass,                                       //
        lpWindowName,                                        //
        WS_POPUP,                                            //
        300,                                                 //
        1500,                                                //
        (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
        (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
        nullptr,                                             //
        nullptr,                                             //
        hInstance,                                           //
        nullptr                                              //
    );                                                       //

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

    SetWindowPos(                                            //
        hWnd,                                                //
        HWND_TOPMOST,                                        //
        300,                                                 //
        1500,                                                //
        (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
        (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
        SWP_SHOWWINDOW);                                     //

    // MoveWindow(                                       //
    //     hWnd,                                         //
    //     100,                                          //
    //     100,                                          //
    //     (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH),  //
    //     (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH), //
    //     TRUE                                          //
    // );                                                //
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    OutputDebugString(L"CreateCandidateWindow");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 释放资源
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

        if (caretY < -900)
        {
            MoveWindow(                                              //
                hWnd,                                                //
                caretX,                                              //
                caretY,                                              //
                (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
                (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
                TRUE                                                 //
            );                                                       //
        }
        else
        {
            MoveWindow(                                              //
                hWnd,                                                //
                caretX,                                              //
                caretY,                                              //
                (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
                (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
                TRUE                                                 //
            );                                                       //
        }
        ShowWindow(hWnd, SW_SHOWNOACTIVATE);
        ::ReadDataFromSharedMemory(0b100000);
        std::wstring embeded_pinyin = string_to_wstring(                             //
            PinyinUtil::pinyin_segmentation(wstring_to_string(Global::PinyinString)) //
        );
        std::wstring str = embeded_pinyin + L"," + Global::CandidateString;
        boost::algorithm::replace_all(str, ",", "\n");
        // TODO: rewrite InflateCandidateWindow(str);
        PaintCandidates(hWnd, str);
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
        if (caretY < -900)
        {
            MoveWindow(                                              //
                hWnd,                                                //
                caretX,                                              //
                caretY,                                              //
                (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
                (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
                TRUE                                                 //
            );                                                       //
        }
        else
        {
            MoveWindow(                                              //
                hWnd,                                                //
                caretX,                                              //
                caretY,                                              //
                (::CANDIDATE_WINDOW_WIDTH * 1.3 + ::SHADOW_WIDTH),   //
                (::CANDIDATE_WINDOW_HEIGHT * 1.32 + ::SHADOW_WIDTH), //
                TRUE                                                 //
            );                                                       //
        }
        return 0;
    }

    switch (message)
    {
    case WM_CREATE:
        InitD2DRenderTarget(hWnd);
        OnPaint(hWnd);
        ValidateRect(hWnd, nullptr);
        return 0;

    // case WM_PAINT:
    // case WM_DISPLAYCHANGE:
    //     return 0;
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
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}