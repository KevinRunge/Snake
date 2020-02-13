// Snake.cpp : Defines the entry point for the application.
//

#include "MainGame.h"
#include "Snake.h"
#include "resource.h"
#include "framework.h"
#include <algorithm>
#include <atlstr.h>
#include <windows.h>
#include <mmsystem.h>

#define MAX_LOADSTRING 100
#define LINETHICKNESS 4
// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd = NULL;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
Game TheGame;									// Instance of the game

DIFFICULTY g_eDifficulty = EASY;

static int iStart = 30;
static int iEnd = iStart + TranslateGameToDisplay(GRIDSIZE);

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Startup(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GameOver(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

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
    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ROUNDSNAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BACKGROUND);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_BLOCKSNAKE));

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

   DWORD dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);
   g_hWnd = CreateWindowW(szWindowClass, szTitle, dwStyle,
      0, 0, 480, 520, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   SetTimer(g_hWnd, IDS_TIMER, g_eDifficulty, (TIMERPROC)NULL);     // no timer callback 
   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);
   DialogBox(hInst, MAKEINTRESOURCE(IDD_START), g_hWnd, Startup);
   return TRUE;
}

void DrawGivenGEWithColor(const GridElement& GE, HDC hdc, const HBRUSH hbrBrush)
{
	// Get XY pair in game coordinates
	const IntPair XYPair = GE.GetXY();
	// Translate to display coordinates
	const IntPair DisplayPair = TranslateGameToDisplay(XYPair);
	// Get XY values of the pixel (note: pixel is translated to square of size TranslateGameToDisplay())
	const int iLeft = iStart + DisplayPair.first;
	const int iRight = iLeft + TranslateGameToDisplay(1);
	const int iTop = iStart + DisplayPair.second;
	const int iBottom = iTop + TranslateGameToDisplay(1);
	RECT rPixel;
	SetRect(&rPixel, iLeft, iTop, iRight, iBottom);
	FillRect(hdc, &rPixel, hbrBrush);
}

CString GetScoreString()
{
	//Get score and make it into a string
	// formatting example: 00005
	const int iScore = TheGame.GetScore();
	CString strScore;
	strScore.Format(_T("%05d\0"), iScore);
	return strScore;
}

void DrawSnake(HWND hWnd, HDC hdc)
{
	// Define the brushes
	const HBRUSH hbrBlack  = CreateSolidBrush(RGB(  0,   0,   0));
	const HBRUSH hbrOrange = CreateSolidBrush(RGB(255, 180,   0));

	// Get snake vector
	const std::vector<GridElement>* const pSnakeVector = TheGame.GetSnake();
	// Draw each element of the snake
	std::for_each(pSnakeVector->begin(), pSnakeVector->end(), [&](const GridElement& GE){DrawGivenGEWithColor(GE, hdc, hbrBlack);});

	//Get the food point
	const GridElement& GEFood = TheGame.GetFoodRef();
	if (GEFood.GetActive())
	{
		DrawGivenGEWithColor(GEFood,hdc,hbrOrange);
	}

	// Draw score:
	CString strScore = GetScoreString();
	
	// Determine window rect
	RECT rcWindow;
	GetClientRect(hWnd,&rcWindow);
	rcWindow.top++;
	rcWindow.bottom--;
	rcWindow.left+=iStart;
	rcWindow.right--;
	SetTextColor(hdc, RGB(0,0,0));

	DrawText(hdc, strScore, -1, &rcWindow, DT_LEFT | DT_TOP | DT_SINGLELINE);
	DeleteObject(hbrBlack);
	DeleteObject(hbrOrange);
}

void DrawGameOver(HWND hWnd, HDC hdc)
{
	RECT rcWindow;
	GetClientRect(hWnd,&rcWindow);
	DrawText(hdc, _T("GAME OVER"), -1, &rcWindow, DT_CENTER | DT_SINGLELINE);
}

void DrawPause(HWND hWnd, HDC hdc)
{
	const HBRUSH hbrGrey  = CreateSolidBrush(RGB( 150, 150, 150));
	// Get window rect
	RECT rcWindow;
	GetClientRect(hWnd,&rcWindow);
	const int iMiddleLR = (rcWindow.right - rcWindow.left) / 2;
	const int iMiddleUD = (rcWindow.bottom - rcWindow.top) / 2;
	const int iOffset = 10;
	const int iHeight = 50;
	const int iWidth  = 20; 
	RECT rcLeftBar;
	SetRect(&rcLeftBar, iMiddleLR - iOffset - iWidth, iMiddleUD - iHeight, iMiddleLR - iOffset, iMiddleUD + iHeight);
	RECT rcRightBar;
	SetRect(&rcRightBar, iMiddleLR + iOffset, iMiddleUD - iHeight, iMiddleLR + iOffset + iWidth, iMiddleUD + iHeight);
	
	FillRect(hdc, &rcLeftBar, hbrGrey);
	FillRect(hdc, &rcRightBar, hbrGrey);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_KEYDOWN  - process keys pressed
//  WM_COMMAND  - process the application menu
//	WM_TIMER	- every X seconds
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
HMENU hmenu = GetMenu(hWnd);
    switch (message)
    {
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:

			// Process the LEFT ARROW key. 
			TheGame.SetDirection(Game::DIRECTION::LEFT);
			break;

		case VK_RIGHT:

			// Process the RIGHT ARROW key. 
			TheGame.SetDirection(Game::DIRECTION::RIGHT);
			break;

		case VK_UP:

			// Process the UP ARROW key. 
			TheGame.SetDirection(Game::DIRECTION::UP);
			break;

		case VK_DOWN:

			// Process the DOWN ARROW key. 
			TheGame.SetDirection(Game::DIRECTION::DOWN);
			break;

		case VK_ESCAPE:

			// Process the ESC key. 
			TheGame.TogglePause();
			break;

			// Process other non-character keystrokes. 
		default:
			return -1;
			break;
		}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDM_RESTART:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_START), hWnd, Startup);
				break;
			}
			case IDM_DIFFICULTY_EASY:
			{
				UINT state = GetMenuState(hmenu,IDM_DIFFICULTY_EASY, MF_BYCOMMAND);
				if (state == MF_UNCHECKED)
				{
					g_eDifficulty = EASY;
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY,   MF_CHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD, MF_UNCHECKED);
				}
				SetTimer(hWnd, IDS_TIMER, g_eDifficulty, (TIMERPROC)NULL);     // no timer callback 
				break;
			}
			case IDM_DIFFICULTY_MEDIUM:
			{
				UINT state = GetMenuState(hmenu,IDM_DIFFICULTY_MEDIUM, MF_BYCOMMAND);
				if (state == MF_UNCHECKED)
				{
					g_eDifficulty = MEDIUM;
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY, MF_UNCHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM,   MF_CHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD, MF_UNCHECKED);
				}
				SetTimer(hWnd, IDS_TIMER, g_eDifficulty, (TIMERPROC)NULL);     // no timer callback 
				break;
			}
			case IDM_DIFFICULTY_HARD:
			{
				UINT state = GetMenuState(hmenu,IDM_DIFFICULTY_HARD, MF_BYCOMMAND);
				if (state == MF_UNCHECKED)
				{
					g_eDifficulty = HARD;
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY, MF_UNCHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD,   MF_CHECKED);
				}
				SetTimer(hWnd, IDS_TIMER, g_eDifficulty, (TIMERPROC)NULL);     // no timer callback 
				break;
			}
			case IDM_OPTIONS_SOUND:
			{
				UINT state = GetMenuState(hmenu,IDM_OPTIONS_SOUND, MF_BYCOMMAND);
				if (state == MF_UNCHECKED) // Go to enabled mode
				{
					TheGame.SetSoundEnabled(true);
					// Play snakejazz when playing, not paused, not game over
					PlaySnakeJazz(TheGame.IsPlaying());
					CheckMenuItem(hmenu,   IDM_OPTIONS_SOUND, MF_CHECKED);
				}
				else if (state == MF_CHECKED) // Go to disabled mode
				{
					TheGame.SetSoundEnabled(false);
					PlaySnakeJazz(false);
					CheckMenuItem(hmenu,   IDM_OPTIONS_SOUND, MF_UNCHECKED);
				}
				break;
			}
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
				return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_TIMER:
		TheGame.Play();
		if(TheGame.IsGameOver())
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_GAMEOVER), hWnd, GameOver);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Add any drawing code that uses hdc here...
			HPEN hpen;
			hpen = CreatePen(PS_SOLID, LINETHICKNESS, RGB(0, 0, 0));
			SelectObject(hdc, hpen);
			Rectangle(hdc, iStart - LINETHICKNESS/2, iStart - LINETHICKNESS/2, iEnd, iEnd);
			if (!TheGame.IsGameOver())
			{
				DrawSnake(hWnd, hdc);
				if (TheGame.GetPaused())
				{
					DrawPause(hWnd,hdc);
				}
			}
			DeleteObject(hpen);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return -1;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	{
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BLOCKSNAKE));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        return (INT_PTR)TRUE;
	}
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for Startup box.
INT_PTR CALLBACK Startup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Get menu from main window:
	HMENU hmenu = GetMenu(g_hWnd);

	UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	{
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BLOCKSNAKE));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		CheckRadioButton(hDlg,IDC_SETEASY , IDC_SETHARD, IDC_SETEASY);
        return (INT_PTR)TRUE;
	}
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_SETEASY:
				g_eDifficulty = EASY;
			break;
			
			case IDC_SETMEDIUM:
				g_eDifficulty = MEDIUM;
			break;
			
			case IDC_SETHARD:
				g_eDifficulty = HARD;
			break;
			
			case IDSTART:
				switch (g_eDifficulty)
				{
					case EASY:
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY,   MF_CHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD, MF_UNCHECKED);
					break;
					case MEDIUM:
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY, MF_UNCHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM,   MF_CHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD, MF_UNCHECKED);
					break;
					case HARD:
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_EASY, MF_UNCHECKED);
					CheckMenuItem(hmenu, IDM_DIFFICULTY_MEDIUM, MF_UNCHECKED);
					CheckMenuItem(hmenu,   IDM_DIFFICULTY_HARD,   MF_CHECKED);
					break;

				}
				SetTimer(g_hWnd, IDS_TIMER, g_eDifficulty, (TIMERPROC)NULL);     // no timer callback 
				EndDialog(hDlg, LOWORD(wParam));
				TheGame.InitialiseGame();
				return (INT_PTR)TRUE;
			break;
		}
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK GameOver(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	{
		HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BLOCKSNAKE));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		HWND hWndScoreTextBox = GetDlgItem(hDlg,IDC_SCORE);
		CString strScore = GetScoreString();
		SendMessage(hWndScoreTextBox, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)strScore);
        return (INT_PTR) TRUE;
		break;
	}
	case WM_COMMAND:
        if (LOWORD(wParam) == IDRESTART)
        {
            EndDialog(hDlg, LOWORD(wParam));
			DialogBox(hInst, MAKEINTRESOURCE(IDD_START), g_hWnd, Startup);
            return (INT_PTR)TRUE;
        }
        break;
	default:
    return (INT_PTR)FALSE;
	}
	return (INT_PTR)FALSE;
}

void PlaySnakeJazz(const bool bPlay)
{
	if (bPlay && TheGame.GetSoundEnabled())
	{
		PlaySound(MAKEINTRESOURCE(IDA_SNAKEJAZZ),NULL,SND_ASYNC | SND_LOOP | SND_RESOURCE);
	}
	else
	{
		PlaySound(NULL,NULL,NULL);
	}
}

IntPair TranslateGameToDisplay(const IntPair Coordinates)
{
	const int iX = TranslateGameToDisplay(Coordinates.first);
	const int iY = TranslateGameToDisplay(Coordinates.second);
	return std::make_pair(iX, iY);
}