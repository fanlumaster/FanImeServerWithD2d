#include "window_utils.h"
#include "spdlog/spdlog.h"
#include <utility>

FLOAT GetWindowScale(HWND hwnd)
{
    UINT dpi = GetDpiForWindow(hwnd);
    FLOAT scale = dpi / 96.0f;
    return scale;
}

//+---------------------------------------------------------------------------
//
// GetMonitorCoordinates
//
//----------------------------------------------------------------------------

MonitorCoordinates GetMonitorCoordinates()
{
    MonitorCoordinates coordinates;
    HWND hwnd = GetForegroundWindow();
    FLOAT scale = GetWindowScale(hwnd);
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (!hMonitor)
    {
#ifdef FANY_DEBUG
        spdlog::error("Failed to get monitor.");
#endif
        return coordinates;
    }

    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
    if (GetMonitorInfo(hMonitor, &monitorInfo))
    {
        int width = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
        int height = (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
        coordinates.left = monitorInfo.rcMonitor.left;
        coordinates.top = monitorInfo.rcMonitor.top;
        coordinates.right = coordinates.left + width;
        coordinates.bottom = coordinates.top + height;
    }
    else
    {
#ifdef FANY_DEBUG
        spdlog::error("Failed to get monitor info.");
#endif
    }
    return coordinates;
}

int AdjustCandidateWindowPosition(                  //
    const POINT *point,                             //
    const std::pair<double, double> &containerSize, //
    std::shared_ptr<std::pair<int, int>> properPos  //
)
{
    HWND hwnd = GetForegroundWindow();
    FLOAT scale = GetWindowScale(hwnd);

    properPos->first = point->x;
    properPos->second = point->y + 3;
    MonitorCoordinates coordinates = GetMonitorCoordinates();
#ifdef FANY_DEBUG
    spdlog::info("Proper position: {}, {}", properPos->first, properPos->second);
    spdlog::info("Coordinates: {}, {}, {}, {}", coordinates.left, coordinates.top, coordinates.right,
                 coordinates.bottom);
#endif
    if (properPos->first < coordinates.left)
    {
        properPos->first = coordinates.left + 2;
    }
    if (properPos->second < coordinates.top)
    {
        properPos->second = coordinates.top + 2;
    }
    int containerSizeX = containerSize.first * scale;
    int containerSizeY = containerSize.second * scale;
    if (properPos->first + containerSizeX > coordinates.right)
    {
        properPos->first = coordinates.right - containerSizeX - 2;
    }
    if (properPos->second + 267 * scale > coordinates.bottom)
    {
        properPos->second = properPos->second - containerSizeY - 30 - 2;
    }
    return 0;
}

int AdjustCandidateWindowPosition( //
    const POINT *position,         //
    const int width,               //
    const int height,              //
    POINT *adjustedPos             //
)
{
    HWND hwnd = GetForegroundWindow();
    MonitorCoordinates coordinates = GetMonitorCoordinates();

    static int maxHeight = height;
    if (height > maxHeight)
    {
        maxHeight = height;
    }

    if (position->x < coordinates.left)
    {
        adjustedPos->x = coordinates.left + 2;
    }
    if (position->x + width > coordinates.right)
    {
        adjustedPos->x = coordinates.right - width - 2;
    }

    if (position->y < coordinates.top)
    {
        adjustedPos->y = coordinates.top + 2;
    }
    if (position->y + maxHeight > coordinates.bottom)
    {
        adjustedPos->y = position->y - height - 30 - 2;
    }
    return 0;
}