#pragma once

#include <string>
#include <windows.h>

inline WCHAR szWindowClass[] = L"global_candidate_window";
inline WCHAR szTitle[] = L"Global Candidate Window";
inline WCHAR lpWindowName[] = L"fanycandidatewindow";
inline int CandidateWndMargin = 6;

LRESULT RegisterCandidateWindowMessage();
LRESULT RegisterCandidateWindowClass(WNDCLASSEX &, HINSTANCE);
int CreateCandidateWindow(HINSTANCE);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void PaintCandidates(HWND, std::wstring &);