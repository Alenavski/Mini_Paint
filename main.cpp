#include <Windows.h>
#include <string>
#include <vector>

static CHAR szWindowClass[] = "Mini_Paint";
static CHAR szTitle[] = "Application for paint";
std::vector<POINT> points;

typedef struct _TMainWindow
{
    HWND hWnd;
    std::string text;
    HDC hdcBack;
    HBITMAP hbmBack;
    RECT rcClient;
    POINT from, to;
    COLORREF pen, brush;
    INT Figure;
    INT LineWidth;
    BOOL isPaint;
} TMainWindow, * PMainWindow;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID DrawFigure(PMainWindow pSelf, BOOL isMem);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof wcex);
    wcex.cbSize = sizeof wcex;
    wcex.lpfnWndProc = WndProc;
    wcex.cbWndExtra = sizeof(PMainWindow);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorA(NULL,IDC_WAIT);
    wcex.hbrBackground = (HBRUSH)(COLOR_HIGHLIGHT+1);
    wcex.lpszClassName = szWindowClass;
    wcex.cbClsExtra = 0;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hIconSm = LoadIcon(wcex.hInstance,IDI_APPLICATION);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpszMenuName = NULL;

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, "Call to RegisterClassEx failed!", szTitle, NULL);
        return 1;
    }

    HWND hWnd = CreateWindowEx(0, szWindowClass, szTitle, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_SIZEBOX | WS_MAXIMIZEBOX), 0, 0, 1000, 800, NULL, NULL, hInstance, NULL);
    if (!hWnd)
    {
        MessageBox(NULL, "Call to CreateWindow failed!", szTitle, NULL);
        return 1;
    }

    HMENU hm = CreateMenu();
    HMENU hmFigure = CreateMenu();
    HMENU hmColorPen = CreateMenu();
    HMENU hmColorBrush = CreateMenu();
    HMENU hmLineWidth = CreateMenu();
    AppendMenu(hm, MF_STRING | MF_POPUP, (UINT_PTR)hmFigure, "Figure");
    {
        AppendMenu(hmFigure, MF_STRING, 10, "Line");
        AppendMenu(hmFigure, MF_STRING, 20, "PolyLine");
        AppendMenu(hmFigure, MF_STRING, 30, "Rectangle");
        AppendMenu(hmFigure, MF_STRING, 40, "Polygon");
        AppendMenu(hmFigure, MF_STRING, 50, "Ellipse");
        AppendMenu(hmFigure, MF_STRING, 60, "Text");
    }
    AppendMenu(hm, MF_STRING | MF_POPUP, (UINT_PTR)hmColorPen, "Color of Pen");
    {
        AppendMenu(hmColorPen, MF_STRING, 1, "Red");
        AppendMenu(hmColorPen, MF_STRING, 2, "Green");
        AppendMenu(hmColorPen, MF_STRING, 3, "Blue");
        AppendMenu(hmColorPen, MF_STRING, 4, "White");
        AppendMenu(hmColorPen, MF_STRING, 5, "Black");
        AppendMenu(hmColorPen, MF_STRING, 6, "Cyan");
        AppendMenu(hmColorPen, MF_STRING, 7, "Majenta");
        AppendMenu(hmColorPen, MF_STRING, 8, "Yellow");
    }
    AppendMenu(hm, MF_STRING | MF_POPUP, (UINT_PTR)hmColorBrush, "Color of Brush");
    {
        AppendMenu(hmColorBrush, MF_STRING, 11, "Red");
        AppendMenu(hmColorBrush, MF_STRING, 12, "Green");
        AppendMenu(hmColorBrush, MF_STRING, 13, "Blue");
        AppendMenu(hmColorBrush, MF_STRING, 14, "White");
        AppendMenu(hmColorBrush, MF_STRING, 15, "Black");
        AppendMenu(hmColorBrush, MF_STRING, 16, "Cyan");
        AppendMenu(hmColorBrush, MF_STRING, 17, "Majenta");
        AppendMenu(hmColorBrush, MF_STRING, 18, "Yellow");
    }
    AppendMenu(hm, MF_STRING | MF_POPUP, (UINT_PTR)hmLineWidth, "Width");
    {
        AppendMenu(hmLineWidth, MF_STRING, 25, "5");
        AppendMenu(hmLineWidth, MF_STRING, 35, "7");
        AppendMenu(hmLineWidth, MF_STRING, 45, "8");
        AppendMenu(hmLineWidth, MF_STRING, 55, "11");
    }
    SetMenu(hWnd, hm);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0 ,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PMainWindow pSelf;
    HANDLE hOld;

    PAINTSTRUCT ps;
    HDC hdc;

    if (WM_CREATE == message)
    {
        pSelf = (PMainWindow)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TMainWindow));
        SetWindowLongPtr(hWnd, 0, (LONG_PTR)pSelf);
        if (pSelf != NULL)
        {
            pSelf->hWnd = hWnd;
            pSelf->pen = (COLORREF)RGB(0, 255, 255);
            pSelf->brush = (COLORREF)RGB(255,255,255);
            SetRect(&pSelf->rcClient, 0, 0, 1000, 700);

            hdc = GetDC(hWnd);
            pSelf->hdcBack = CreateCompatibleDC(hdc);
            pSelf->hbmBack = CreateCompatibleBitmap(hdc, pSelf->rcClient.right - pSelf->rcClient.left, pSelf->rcClient.bottom - pSelf->rcClient.top);
            ReleaseDC(hWnd, hdc);
            SaveDC(pSelf->hdcBack);
            SelectObject(pSelf->hdcBack, pSelf->hbmBack);

            RECT r;
            CopyRect(&r, &pSelf->rcClient);
            FillRect(pSelf->hdcBack, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
            AdjustWindowRect(&r, ((LPCREATESTRUCT)lParam)->style, FALSE);
            SetWindowPos(hWnd, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOZORDER);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    pSelf = (PMainWindow)GetWindowLongPtr(hWnd, 0);
    if (!pSelf)
        return DefWindowProc(hWnd, message, wParam,lParam);

    WORD param = LOWORD(wParam);
    switch (message)
    {
        case WM_COMMAND:
            if (param % 10 == 0)
            {
                pSelf->Figure = param;
            }

            if ((param % 10 != 0) && (param > 20))
            {
                pSelf->LineWidth = param / 5;
            }

            switch (param)
            {
                case 1:
                    pSelf->pen = (COLORREF)RGB(255,0,0);
                    break;
                case 2:
                    pSelf->pen = (COLORREF)RGB(0, 255, 0);
                    break;
                case 3:
                    pSelf->pen = (COLORREF)RGB(0,0,255);
                    break;
                case 4:
                    pSelf->pen = (COLORREF)RGB(255,255,255);
                    break;
                case 5:
                    pSelf->pen = (COLORREF)RGB(0,0,0);
                    break;
                case 6:
                    pSelf->pen = (COLORREF)RGB(0,255,255);
                    break;
                case 7:
                    pSelf->pen = (COLORREF)RGB(255,0,255);
                    break;
                case 8:
                    pSelf->pen = (COLORREF)RGB(255,255,0);
                    break;
                case 11:
                    pSelf->brush = (COLORREF)RGB(255,0,0);
                    break;
                case 12:
                    pSelf->brush = (COLORREF)RGB(0, 255, 0);
                    break;
                case 13:
                    pSelf->brush = (COLORREF)RGB(0,0,255);
                    break;
                case 14:
                    pSelf->brush = (COLORREF)RGB(255,255,255);
                    break;
                case 15:
                    pSelf->brush = (COLORREF)RGB(0,0,0);
                    break;
                case 16:
                    pSelf->brush = (COLORREF)RGB(0,255,255);
                    break;
                case 17:
                    pSelf->brush = (COLORREF)RGB(255,0,255);
                    break;
                case 18:
                    pSelf->brush = (COLORREF)RGB(255,255,0);
                    break;
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, 0, 0, pSelf->rcClient.right - pSelf->rcClient.left, pSelf->rcClient.bottom - pSelf->rcClient.top, pSelf->hdcBack, 0, 0, SRCCOPY);
            DrawFigure(pSelf, FALSE);
            EndPaint(hWnd, &ps);
            return 0;
        case WM_KEYDOWN:
            if (pSelf->isPaint && pSelf->Figure == 60)
            {
                if (wParam == VK_BACK && pSelf->text.length() != 0)
                {
                    pSelf->text.pop_back();
                }
                else
                {
                    pSelf->text.push_back(wParam);
                }

                InvalidateRect(hWnd, &pSelf->rcClient, FALSE);
            }
            break;
        case WM_LBUTTONDOWN:
            if (!pSelf->isPaint)
            {
                pSelf->from.x = LOWORD(lParam);
                pSelf->from.y = HIWORD(lParam);
                if (pSelf->Figure == 40)
                {
                    points.push_back(pSelf->from);
                }
                pSelf->isPaint = true;
            }
            else if (pSelf->Figure == 20 || pSelf->Figure == 40)
            {
                pSelf->to.x = LOWORD(lParam);
                pSelf->to.y = HIWORD(lParam);
                DrawFigure(pSelf, TRUE);
                pSelf->from.x = LOWORD(lParam);
                pSelf->from.y = HIWORD(lParam);
                if (pSelf->Figure == 40)
                {
                    points.push_back(pSelf->from);
                }
            }
            break;
        case WM_RBUTTONDOWN:
            if (pSelf->isPaint)
            {
                if (pSelf->Figure == 40)
                {
                    if (points.size() >= 3)
                    {
                        pSelf->to.x = LOWORD(lParam);
                        pSelf->to.y = HIWORD(lParam);
                        points.push_back(pSelf->to);
                        pSelf->from = points[points.size()-1];
                        pSelf->to.x = points[0].x;
                        pSelf->to.y = points[0].y;
                        DrawFigure(pSelf, TRUE);
                        pSelf->isPaint = false;
                        points.clear();
                        InvalidateRect(hWnd, &pSelf->rcClient, FALSE);
                    }
                    break;
                }
                pSelf->to.x = LOWORD(lParam);
                pSelf->to.y = HIWORD(lParam);
                DrawFigure(pSelf, TRUE);
                pSelf->isPaint = false;
                if (pSelf->Figure == 60)
                {
                    pSelf->text.clear();
                }
            }
            break;
        case WM_MOUSEMOVE:
            if (pSelf->isPaint)
            {
                pSelf->to.x = LOWORD(lParam);
                pSelf->to.y = HIWORD(lParam);
                InvalidateRect(hWnd, &pSelf->rcClient, FALSE);
            }
            break;
        case WM_DESTROY:
            DeleteObject(pSelf->hbmBack);
            RestoreDC(pSelf->hdcBack, -1);
            DeleteDC(pSelf->hdcBack);

            HeapFree(GetProcessHeap(), 0, pSelf);

            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

VOID DrawFigure(PMainWindow pSelf, BOOL isMem)
{
    if (pSelf->isPaint)
    {
        HDC hdc;
        if (!isMem)
        {
            hdc = GetDC(pSelf->hWnd);
        }
        else
        {
            hdc = pSelf->hdcBack;
        }
        SelectObject(hdc, CreateSolidBrush(pSelf->brush));
        SelectObject(hdc, CreatePen(PS_SOLID, pSelf->LineWidth, pSelf->pen));

        switch (pSelf->Figure)
        {
            case 10:
                MoveToEx(hdc, pSelf->from.x, pSelf->from.y, NULL);
                LineTo(hdc, pSelf->to.x, pSelf->to.y);
                break;
            case 20:
                MoveToEx(hdc, pSelf->from.x, pSelf->from.y, NULL);
                LineTo(hdc, pSelf->to.x, pSelf->to.y);
                break;
            case 30:
                Rectangle(hdc, pSelf->from.x, pSelf->from.y, pSelf->to.x, pSelf->to.y);
                break;
            case 40:
                Polygon(hdc, &points[0], points.size());
                break;
            case 50:
                Ellipse(hdc, pSelf->from.x, pSelf->from.y, pSelf->to.x, pSelf->to.y);
                break;
            case 60:
                SetBkColor(hdc, pSelf->brush);
                SetTextColor(hdc, pSelf->pen);
                TextOut(hdc, pSelf->from.x, pSelf->from.y, (LPCSTR)&pSelf->text[0], pSelf->text.size());
                break;
            default:
                break;
        }
        if (!isMem)
        {
            ReleaseDC(pSelf->hWnd, hdc);
        }
    }
}