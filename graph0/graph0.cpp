#include "windows.h"
#include "framework.h"
#include "gdiplus.h"

#include "graph0.h"
#include "graph.h"
#include "painter.h"
#include "IOcontroller.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

Paint::Painter PAINTER;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO: Place code here.

  /* Initialize GDI+ */
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_GRAPH0, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPH0));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  Gdiplus::GdiplusShutdown(gdiplusToken);
  return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = (HICON)LoadImage(wcex.hInstance, L"custom.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GRAPH0);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = (HICON)LoadImage(wcex.hInstance, L"custom.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

  return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, defW, defH, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd)
  {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case ID_FILE_STARTDRAWINGTHEGRAPH:
      try {
        Math::Graph graph(
          Math::Graph::AdjListFromMatrix(IOcontroller::ReadMatrix("graph3.txt"))
        );
        PAINTER.Reset();
        graph.Lay().Render(PAINTER);

        //graphics update
        RECT rect;
        GetClientRect(hWnd, &rect);
        InvalidateRect(hWnd, &rect, true);
      }
      catch (...) {
        MessageBox(hWnd, L"An error occurred while reading the graph!",
          L"Input error", MB_OK);
      }
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  break;
  case WM_GETMINMAXINFO:
  {
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    lpMMI->ptMinTrackSize.x = minW;
    lpMMI->ptMinTrackSize.y = minH;
  }
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    RECT rect;
    GetClientRect(hWnd, &rect);
    PAINTER.Draw(hdc, rect.right - rect.left, rect.bottom - rect.top);
    EndPaint(hWnd, &ps);
  }
  break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}