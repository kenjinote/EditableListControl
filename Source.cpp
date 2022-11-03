#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "comctl32")

#include <windows.h>
#include <commctrl.h>

TCHAR szClassName[] = TEXT("EditableListControl");

WNDPROC defaultEditWndProc;

LRESULT CALLBACK listviewEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_APP: // 編集結果を親ウィンドウ（リストビュー）に反映
		{
			HWND hParent = GetParent(hWnd);
			if (hParent) {
				RECT rect = {};
				GetWindowRect(hWnd, &rect);
				ScreenToClient(hParent, (LPPOINT)&rect.left);
				ScreenToClient(hParent, (LPPOINT)&rect.right);
				LVHITTESTINFO hi = {};
				hi.pt.x = (rect.right + rect.left) / 2;
				hi.pt.y = (rect.bottom + rect.top) / 2;
				ListView_HitTest(hParent, &hi);
				const int nRow = hi.iItem;
				ListView_SubItemHitTest(hParent, &hi);
				const int nColumn = hi.iSubItem;
				WCHAR szText[1024];
				GetWindowText(hWnd, szText, _countof(szText));
				ListView_SetItemText(hParent, nRow, nColumn, szText);
				return 0;
			}
		}
		break;
	case WM_CHAR:
		if (wParam == VK_RETURN) {
			SendMessage(hWnd, WM_APP, 0, 0);
			ShowWindow(hWnd, SW_HIDE);
			SetFocus(GetParent(hWnd));
			return 0;
		} else if (wParam == VK_ESCAPE) {
			ShowWindow(hWnd, SW_HIDE);
			SetFocus(GetParent(hWnd));
			return 0;
		}
		break;
	case WM_KILLFOCUS:
		SendMessage(hWnd, WM_APP, 0, 0);
		ShowWindow(hWnd, SW_HIDE);
		break;
	default:
		break;
	}
	return CallWindowProc(defaultEditWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hList;
	static HWND hEdit;
	static RECT editRect;
	static int nRow;
	static int nColumn;
	switch (msg)
	{
	case WM_CREATE:
		InitCommonControls();
		hList = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, 0, WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 0, 512, 512, hWnd, (HMENU)1000, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		if (hList) {
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_TWOCLICKACTIVATE);
			hEdit = CreateWindowEx(0, WC_EDIT, 0, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, hList, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			if (hEdit) {
				HFONT lvFont = (HFONT)SendMessage(hList, WM_GETFONT, 0, 0);
				SendMessage(hEdit, WM_SETFONT, (WPARAM)lvFont, TRUE);
				defaultEditWndProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)listviewEditProc);
			}
			LV_COLUMN lvcol = {};
			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 128;
			lvcol.pszText = TEXT("変数名");
			lvcol.iSubItem = 0;
			SendMessage(hList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvcol);
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 128;
			lvcol.pszText = TEXT("値");
			lvcol.iSubItem = 1;
			SendMessage(hList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvcol);
			SendMessage(hWnd, WM_APP, 0, 0);
			{
				LV_ITEM item = {};
				item.mask = LVIF_TEXT;
				int index = 0;
				WCHAR szText[1024];
				for (int i = 0; i < 16; i++) {
					wsprintf(szText, L"item %d", i);
					item.pszText = (LPWSTR)szText;
					item.iItem = index;
					item.iSubItem = 0;
					SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&item);
					wsprintf(szText, L"sub item %d", i);
					item.pszText = (LPWSTR)szText;
					item.iItem = index;
					item.iSubItem = 1;
					SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&item);
					index++;
				}
			}
		}
		break;
	case WM_APP: // nRowとnColumnをセットした後でWM_APPを呼ぶとテキストボックスを表示して編集状態にする
		{
			WCHAR szText[1024] = {};
			RECT rect = {};
			ListView_GetSubItemRect(hList, nRow, nColumn, LVIR_LABEL, &rect);
			ListView_GetItemText(hList, nRow, nColumn, szText, _countof(szText));
			if (hEdit) {
				MoveWindow(hEdit, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
				ShowWindow(hEdit, SW_SHOW);
				SetWindowText(hEdit, szText);
				SendMessage(hEdit, EM_SETSEL, 0, -1);
				SetFocus(hEdit);
			}
		}
		break;
	case WM_NOTIFY:
		if (LOWORD(wParam) == 1000)
		{
			switch (((LV_DISPINFO*)lParam)->hdr.code)
			{
			case NM_DBLCLK:
				{
					LVHITTESTINFO hi = {};
					GetCursorPos(&hi.pt);
					ScreenToClient(hList, &hi.pt);
					hi.pt.x = 4;
					nRow = ListView_HitTest(hList, &hi);
					nColumn = ((LPNMITEMACTIVATE)lParam)->iSubItem;
					SendMessage(hWnd, WM_APP, 0, 0);
				}
				break;
			case LVN_KEYDOWN:
				if (((NMLVKEYDOWN*)lParam)->wVKey == VK_F2) {
					nRow = ListView_GetNextItem(hList, -1, LVNI_FOCUSED);
					nColumn = 1;
					SendMessage(hWnd, WM_APP, 0, 0);
				}
				break;
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("EditableListControl"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
