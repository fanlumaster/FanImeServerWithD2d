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
#include <winuser.h>
#include "ime_engine/shuangpin/pinyin_utils.h"

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
    DWORD dwExStyle = WS_EX_TOOLWINDOW | //
                      WS_EX_NOACTIVATE | //
                      WS_EX_TOPMOST;     //
    HWND hWnd = CreateWindowEx(          //
        dwExStyle,                       //
        szWindowClass,                   //
        lpWindowName,                    //
        // WS_OVERLAPPEDWINDOW,             //
        WS_POPUP,                                     //
        100,                                          //
        100,                                          //
        (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH),  //
        (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH), //
        nullptr,                                      //
        nullptr,                                      //
        hInstance,                                    //
        nullptr                                       //
    );                                                //

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

    ::global_hwnd = hWnd;

    SetWindowPos(                                     //
        hWnd,                                         //
        HWND_TOPMOST,                                 //
        100,                                          //
        100,                                          //
        (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH),  //
        (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH), //
        SWP_SHOWWINDOW);                              //

    MoveWindow(                                       //
        hWnd,                                         //
        100,                                          //
        100,                                          //
        (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH),  //
        (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH), //
        TRUE                                          //
    );                                                //
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    OutputDebugString(L"CreateCandidateWindow");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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
            MoveWindow(                                             //
                hWnd,                                               //
                caretX,                                             //
                caretY,                                             //
                (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH) * 1.5,  //
                (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH) * 1.5, //
                TRUE                                                //
            );                                                      //
        }
        else
        {
        }
        ShowWindow(hWnd, SW_SHOWNOACTIVATE);
        ::ReadDataFromSharedMemory(0b100000);
        std::wstring embeded_pinyin = string_to_wstring(                             //
            PinyinUtil::pinyin_segmentation(wstring_to_string(Global::PinyinString)) //
        );
        std::wstring str = embeded_pinyin + L"," + Global::CandidateString;
        // TODO: rewrite InflateCandidateWindow(str);
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
            MoveWindow(                                             //
                hWnd,                                               //
                caretX,                                             //
                caretY,                                             //
                (::CANDIDATE_WINDOW_WIDTH + ::SHADOW_WIDTH) * 1.5,  //
                (::CANDIDATE_WINDOW_HEIGHT + ::SHADOW_WIDTH) * 1.5, //
                TRUE                                                //
            );                                                      //
        }
        else
        {
            std::shared_ptr<std::pair<int, int>> properPos = std::make_shared<std::pair<int, int>>();
            // TODO: move window to proper position, consider the Monitor edges
        }
        return 0;
    }

    switch (message)
    {
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}