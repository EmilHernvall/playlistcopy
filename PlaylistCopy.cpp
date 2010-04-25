#include "stdafx.h"
#include "PlaylistCopy.h"

#define ID_PLAYLISTEDIT 100
#define ID_PLAYLISTBUTTON 101
#define ID_TARGETEDIT 102
#define ID_TARGETBUTTON 103
#define ID_MOVEBUTTON 104
#define ID_COPYBUTTON 105

typedef struct PlaylistEntry_ {
	CHAR szPath[MAX_PATH+1];
	struct PlaylistEntry_ *next;
} PlaylistEntry;

HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_playlistEdit;
HWND g_targetEdit;
HWND g_copyButton;
HWND g_moveButton;
HWND g_status;

LPCTSTR szTitle = TEXT("PlaylistCopy (By Emil Hernvall - aderyn@gmail.com)");
LPCTSTR szWindowClass = TEXT("PlaylistCopyClass");

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

VOID CopyPlaylist(LPTSTR szPlaylist, LPTSTR szTarget, BOOLEAN bMove);
VOID ParsePlaylist(LPSTR szPlaylist, PlaylistEntry **result);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	LPVOID comToken = NULL;

	CoInitialize(comToken);

	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra	= 0;
	wcex.cbWndExtra	= 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAYLISTCOPY));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInst = hInstance;

	g_hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 460, 220, NULL, NULL, hInstance, NULL);

	if (!g_hWnd) {
		return FALSE;
	}

	// playlist
	HWND playlistLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("Playlist:"), 
		WS_VISIBLE | WS_CHILD, 10, 10, 400, 20, g_hWnd, NULL, hInstance, NULL);

	SetWindowText(playlistLabel, TEXT("Playlist:"));

	g_playlistEdit = CreateWindowEx(0, TEXT("EDIT"), TEXT(""), 
		WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 40, 400, 20, g_hWnd, (HMENU)ID_PLAYLISTEDIT, hInstance, NULL);

	HWND playlistButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("..."), 
		WS_VISIBLE | WS_CHILD, 420, 40, 20, 20, g_hWnd, (HMENU)ID_PLAYLISTBUTTON, hInstance, NULL);

	EnableWindow(g_playlistEdit, FALSE);

	// target
	HWND targetLabel = CreateWindowEx(0, TEXT("STATIC"), TEXT("Target folder:"), 
		WS_VISIBLE | WS_CHILD, 10, 70, 400, 20, g_hWnd, NULL, hInstance, NULL);

	SetWindowText(targetLabel, TEXT("Target folder:"));

	g_targetEdit = CreateWindowEx(0, TEXT("EDIT"), TEXT(""), 
		WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 100, 400, 20, g_hWnd, (HMENU)ID_TARGETEDIT, hInstance, NULL);

	HWND targetButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("..."), 
		WS_VISIBLE | WS_CHILD, 420, 100, 20, 20, g_hWnd, (HMENU)ID_TARGETBUTTON, hInstance, NULL);

	EnableWindow(g_targetEdit, FALSE);

	// actions

	g_moveButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Move"), 
		WS_VISIBLE | WS_CHILD, 10, 130, 100, 30, g_hWnd, (HMENU)ID_MOVEBUTTON, hInstance, NULL);

	g_copyButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("Copy"), 
		WS_VISIBLE | WS_CHILD, 120, 130, 100, 30, g_hWnd, (HMENU)ID_COPYBUTTON, hInstance, NULL);

	g_status = CreateWindowEx(0, TEXT("STATIC"), NULL, 
		WS_VISIBLE | WS_CHILD, 10, 170, 400, 20, g_hWnd, NULL, hInstance, NULL);

	SetWindowText(g_status, TEXT("Not in progress"));

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_PLAYLISTBUTTON:
			{
				OPENFILENAME ofn;
				TCHAR szFile[MAX_PATH+1];

				szFile[0] = '\0';
				
				ZeroMemory(&ofn, sizeof(ofn));

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = szFile;
				ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);
				ofn.lpstrFilter = L"Winamp Playlist\0*.m3u\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.lpstrTitle = TEXT("Select playlist");

				if (GetOpenFileName(&ofn)) {
					SetWindowText(g_playlistEdit, szFile);
				}
			}
			break;
		case ID_TARGETBUTTON:
			{
				TCHAR szFile[MAX_PATH+1];
				BROWSEINFO binfo;
				PIDLIST_ABSOLUTE res;

				binfo.lpszTitle = NULL;
				binfo.hwndOwner = hWnd;
				binfo.pidlRoot = NULL;
				binfo.pszDisplayName = szFile;
				binfo.ulFlags = BIF_NEWDIALOGSTYLE;
				binfo.lpfn = NULL;
				binfo.lParam = NULL;
				binfo.iImage = 0;

				res = SHBrowseForFolder(&binfo);
				if (res == NULL) {
					return 0;
				}

				if (SHGetPathFromIDList(res, szFile)) {
					SetWindowText(g_targetEdit, szFile);
				}
			}
			break;
		case ID_MOVEBUTTON:
			{
				TCHAR szPlaylist[MAX_PATH+1];
				TCHAR szTarget[MAX_PATH+1];

				GetWindowText(g_playlistEdit, szPlaylist, sizeof(szPlaylist)/sizeof(*szPlaylist));
				GetWindowText(g_targetEdit, szTarget, sizeof(szTarget)/sizeof(*szTarget));

				CopyPlaylist(szPlaylist, szTarget, true);
			}
			break;
		case ID_COPYBUTTON:
			{
				TCHAR szPlaylist[MAX_PATH+1];
				TCHAR szTarget[MAX_PATH+1];

				EnableWindow(g_copyButton, false);
				EnableWindow(g_moveButton, false);

				GetWindowText(g_playlistEdit, szPlaylist, sizeof(szPlaylist)/sizeof(*szPlaylist));
				GetWindowText(g_targetEdit, szTarget, sizeof(szTarget)/sizeof(*szTarget));

				CopyPlaylist(szPlaylist, szTarget, false);

				EnableWindow(g_copyButton, true);
				EnableWindow(g_moveButton, true);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
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

VOID GetDirectoryName(LPSTR szPath, LPSTR szDirectory, DWORD dwSize)
{
	CHAR tmp;
	LPSTR szFilePtr;

	szFilePtr = strrchr(szPath, '\\');
	tmp = szFilePtr[1];
	szFilePtr[1] = '\0';
	strcpy_s(szDirectory, dwSize, szPath);
	szFilePtr[1] = tmp;
}

VOID CopyPlaylist(LPTSTR szPlaylistMB, LPTSTR szTargetMB, BOOLEAN bMove)
{
	CHAR szPlaylist[MAX_PATH+1], szTarget[MAX_PATH+1], szTargetFile[MAX_PATH+1];
	CHAR szStatus[2*MAX_PATH+1];
	LPSTR szFileName;
	PlaylistEntry *lpPlaylist, *current, *prev = NULL;
	DWORD dwPlaylistLen, dwTargetLen;

	dwPlaylistLen = _tcslen(szPlaylistMB);
	if (dwPlaylistLen == 0) {
		MessageBox(g_hWnd, TEXT("You have to choose a playlist."), TEXT("Error"), 0);
		return;
	}

	dwTargetLen = _tcslen(szTargetMB);
	if (dwTargetLen == 0) {
		MessageBox(g_hWnd, TEXT("You have to choose a target directory."), TEXT("Error"), 0);
		return;
	}

	WideCharToMultiByte(CP_ACP, 0, szPlaylistMB, -1, szPlaylist, sizeof(szPlaylist), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, szTargetMB, -1, szTarget, sizeof(szTarget), NULL, NULL);

	ParsePlaylist(szPlaylist, &lpPlaylist);
	if (lpPlaylist == NULL) {
		return;
	}

	for (current = lpPlaylist; current != NULL; current = current->next) {
		szFileName = strrchr(current->szPath, '\\');

		ZeroMemory(szTargetFile, sizeof(szTargetFile));
		strcat_s(szTargetFile, sizeof(szTargetFile), szTarget);
		strcat_s(szTargetFile, sizeof(szTargetFile), szFileName);

		if (strcmp(szTargetFile, current->szPath) == 0) {
			continue;
		}

		ZeroMemory(szStatus, sizeof(szStatus));
		if (bMove) {
			strcat_s(szStatus, sizeof(szStatus), "Moving ");
		} else {
			strcat_s(szStatus, sizeof(szStatus), "Copying ");
		}
		strcat_s(szStatus, sizeof(szStatus), szFileName);
		SetWindowTextA(g_status, szStatus);

		if (bMove) {
			MoveFileA(current->szPath, szTargetFile);
		} else {
			CopyFileA(current->szPath, szTargetFile, true);
		}

		if (prev != NULL) {
			delete prev;
		}
		prev = current;
	}

	if (prev != NULL) {
		delete prev;
	}

	ZeroMemory(szStatus, sizeof(szStatus));
	strcat_s(szStatus, sizeof(szStatus), "Not in progress");
	SetWindowTextA(g_status, szStatus);
}

VOID ParsePlaylist(LPSTR szPlaylist, PlaylistEntry **result)
{
	LPSTR szLineBreak, szPtr;
	CHAR szPath[MAX_PATH+1];
	CHAR szBuffer[10], szWorkBuffer[MAX_PATH+1], szRemaining[MAX_PATH+1];
	CHAR szCurrentFile[2*MAX_PATH+1];
	DWORD dwRead;
	HANDLE fileHandle;
	PlaylistEntry *current = NULL, *next = NULL;

	GetDirectoryName(szPlaylist, szPath, sizeof(szPath));

	fileHandle = CreateFileA(szPlaylist, GENERIC_READ, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (fileHandle == NULL) {
		MessageBox(g_hWnd, TEXT("Failed to open playlist."), TEXT("Error"), 0);
		return;
	}

	ZeroMemory(szBuffer, sizeof(szBuffer));
	ZeroMemory(szRemaining, sizeof(szRemaining));
	ZeroMemory(szWorkBuffer, sizeof(szWorkBuffer));

	while (ReadFile(fileHandle, szBuffer, sizeof(szBuffer)/sizeof(*szBuffer)-1, &dwRead, NULL)) {
		if (dwRead == 0) {
			break;
		}

		szBuffer[dwRead] = '\0';

		ZeroMemory(szWorkBuffer, sizeof(szRemaining));

		strcat_s(szWorkBuffer, sizeof(szWorkBuffer), szRemaining);
		strcat_s(szWorkBuffer, sizeof(szWorkBuffer), szBuffer);

		szPtr = szWorkBuffer;
		do {
			szLineBreak = strstr(szPtr, "\r\n");
			if (szLineBreak == NULL) {
				break;
			}

			*szLineBreak = '\0';

			ZeroMemory(szCurrentFile, sizeof(szCurrentFile));

			if (szPtr[1] != ':' || szPtr[2] != '\\') {
				strcat_s(szCurrentFile, sizeof(szCurrentFile), szPath);
			}

			strcat_s(szCurrentFile, sizeof(szCurrentFile), szPtr);
			
			next = new PlaylistEntry;
			ZeroMemory(next, sizeof(PlaylistEntry));

			strcat_s(next->szPath, sizeof(next->szPath), szCurrentFile);
			next->next = current;

			current = next;

			szPtr = szLineBreak + 2;
		} while (szLineBreak != NULL);

		int len = strlen(szPtr);
		memcpy(szRemaining, szPtr, len);
		szRemaining[len] = '\0';
	}

	CloseHandle(fileHandle);

	*result = current;
}
