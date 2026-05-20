#include <windows.h>
#include "WindowsProject1.h"
#include "framework.h"
#include "Game.h"
#include "Settings.h"

#include <string>
#include <vector>
#include <sstream>
#include <codecvt>
#include <locale>

static void UpdateResult();
static void UpdateBalance();
static void UpdateBetDisplay();
static void UpdateHandsInstant(bool hideDealer);
static void UpdateHandsAnimatedStep();
static void StartDealAnimation(HWND hwnd);
static void RefreshAll(bool hideDealer);
static void AddHistoryRecord(const std::string& rec);
static void OnRoundFinished();
static void CreateGuiControls(HWND hwnd);
static void LayoutControls(HWND hwnd);

static Game game;

static HWND hEditPromo, hButtonPromo;
static HWND hStaticBalance, hStaticBet;
static HWND hEditBet, hButtonSetBet;
static HWND hButtonStart, hButtonHit, hButtonStand, hButtonDouble, hButtonSplit, hButtonExit;
static HWND hStaticPlayer, hStaticDealer, hStaticResult;
static HWND hListHistory;
static HWND hGroupControls, hGroupTable, hGroupHistory, hButtonSettings;

static HFONT hFontTitle;
static HFONT hFontNormal;

static bool gAnimating = false;
static UINT_PTR gAnimTimer = 0;
static int gAnimStep = 0;
static const int gAnimStepMax = 20;

static std::string gPlayerRaw;
static std::string gDealerRaw;

static std::wstring gPlayerWide;
static std::wstring gDealerWide;

static HBRUSH hBgBrush = CreateSolidBrush(RGB(10, 10, 25));
static HBRUSH hPanelBrush = CreateSolidBrush(RGB(35, 35, 70));
static HBRUSH hButtonBrush = CreateSolidBrush(RGB(50, 50, 50));


static COLORREF clrTextMain = RGB(0, 255, 200);
static COLORREF clrTextWarn = RGB(255, 80, 120);

static std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(str);
}

static std::string wstring_to_utf8(const std::wstring& ws)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.to_bytes(ws);
}

static void ApplyFont(HWND h, HFONT f)
{
	SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE);
}

static void UpdateBalance()
{
	std::wstring text = L"Balance: $" + std::to_wstring(game.getBalance());
	SetWindowTextW(hStaticBalance, text.c_str());
}

static void UpdateBetDisplay()
{
	std::wstring text = L"Bet: " + std::to_wstring(game.getCurrentBet());
	SetWindowTextW(hStaticBet, text.c_str());
}

static std::wstring JoinHands(const std::vector<std::string>& hands)
{
	std::wstring result;

	for (const auto& h : hands)
	{
		for (char c : h)
			result.push_back((wchar_t)c);

		result.push_back(L'\n');
	}

	return result;
}

static void BuildHandsStrings(bool hideDealerForAnim)
{
	gPlayerWide = JoinHands(game.getPlayerHandsStrings());

	gDealerRaw = game.getDealerString(hideDealerForAnim);
	if (gDealerRaw.empty())
		gDealerRaw = "No cards yet";

	gDealerWide = utf8_to_wstring(gDealerRaw);
}

static void UpdateHandsInstant(bool hideDealer)
{
	SetWindowTextW(hStaticPlayer,
		(L"Player hand:\n" + JoinHands(game.getPlayerHandsStrings())).c_str());

	bool hide = hideDealer && game.getState() != RoundState::Finished;

	std::string dealerStr = game.getDealerString(hide);
	if (dealerStr.empty()) dealerStr = "No cards yet";

	SetWindowTextW(hStaticDealer,
		(L"Dealer hand:\n" + utf8_to_wstring(dealerStr)).c_str());
}

static void UpdateHandsAnimatedStep()
{
	if (!gAnimating)
		return;

	int lenP = (int)gPlayerWide.size();
	int lenD = (int)gDealerWide.size();

	int showP = lenP * gAnimStep / gAnimStepMax;
	int showD = lenD * gAnimStep / gAnimStepMax;

	if (showP < 0) showP = 0;
	if (showP > lenP) showP = lenP;
	if (showD < 0) showD = 0;
	if (showD > lenD) showD = lenD;

	std::wstring playerText = L"Player hand:\n" + gPlayerWide.substr(0, showP);
	std::wstring dealerText;

	if (gDealerWide.empty())
		dealerText = L"Dealer hand:\nNo cards yet";
	else
		dealerText = L"Dealer hand:\n" + gDealerWide.substr(0, showD);

	SetWindowTextW(hStaticPlayer, playerText.c_str());
	SetWindowTextW(hStaticDealer, dealerText.c_str());

	gAnimStep++;
	if (gAnimStep > gAnimStepMax)
	{
		gAnimating = false;
		if (gAnimTimer)
		{
			KillTimer(GetParent(hStaticPlayer), gAnimTimer);
			gAnimTimer = 0;
		}

		bool hideDealer = game.getState() != RoundState::Finished;
		UpdateHandsInstant(hideDealer);

		UpdateBalance();
		UpdateBetDisplay();
		UpdateResult();

		if (game.getState() == RoundState::Finished)
		{
			OnRoundFinished();
		}
	}
}

static void StartDealAnimation(HWND hwnd)
{
	BuildHandsStrings(true);
	gAnimating = true;
	gAnimStep = 0;
	if (gAnimTimer)
		KillTimer(hwnd, gAnimTimer);
	gAnimTimer = SetTimer(hwnd, 1, 60, NULL);
}

static void UpdateResult()
{
	std::string r = game.getLastResult();
	if (r.empty())
	{
		SetWindowTextW(hStaticResult, L"");
		return;
	}

	std::wstring w = utf8_to_wstring(r);

	int count = (int)SendMessageW(hListHistory, LB_GETCOUNT, 0, 0);
	if (count > 0)
	{
		wchar_t buf[512];
		SendMessageW(hListHistory, LB_GETTEXT, count - 1, (LPARAM)buf);

		if (w == buf)
		{
			SetWindowTextW(hStaticResult, L"");
			return;
		}
	}

	AddHistoryRecord(r);
	SetWindowTextW(hStaticResult, L"");
}

static void RefreshAll(bool hideDealer)
{
	UpdateBalance();
	UpdateBetDisplay();

	if (!gAnimating)
		UpdateHandsInstant(hideDealer);

	if (!gAnimating)
		UpdateResult();

	if (Settings::hintsEnabled() && game.getState() == RoundState::InProgress)
	{
		const auto& hands = game.getPlayerHandsStrings();
		if (!hands.empty())
		{
			int dealerUp = game.getDealerUpCardValue();
			std::string hint = Settings::getHint(game.getPlayerHand(0), dealerUp);
			if (hint != "Round started")
				AddHistoryRecord(hint);
		}
	}
}


static void AddHistoryRecord(const std::string& rec)
{
	std::wstring w = utf8_to_wstring(rec);
	SendMessageW(hListHistory, LB_ADDSTRING, 0, (LPARAM)w.c_str());
	int count = (int)SendMessageW(hListHistory, LB_GETCOUNT, 0, 0);
	if (count > 0)
		SendMessageW(hListHistory, LB_SETTOPINDEX, count - 1, 0);
}

static void OnRoundFinished()
{
	UpdateBalance();
	UpdateBetDisplay();
	SetWindowTextW(hStaticResult, L"");
}


static void CreateGuiControls(HWND hwnd)
{
	hFontTitle = CreateFontW(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

	hFontNormal = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

	hGroupControls = CreateWindowW(L"STATIC", L"Controls",
		WS_CHILD | WS_VISIBLE,
		10, 10, 360, 240,
		hwnd, NULL, NULL, NULL);

	hGroupTable = CreateWindowW(L"STATIC", L"Table",
		WS_CHILD | WS_VISIBLE,
		380, 10, 520, 420,
		hwnd, NULL, NULL, NULL);

	hGroupHistory = CreateWindowW(L"STATIC", L"History",
		WS_CHILD | WS_VISIBLE,
		10, 260, 360, 290,
		hwnd, NULL, NULL, NULL);

	hEditPromo = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
		25, 40, 200, 28, hwnd, (HMENU)ID_EDIT_PROMO, NULL, NULL);

	hButtonPromo = CreateWindowW(L"BUTTON", L"Apply promo",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		235, 40, 120, 28, hwnd, (HMENU)ID_BUTTON_PROMO, NULL, NULL);

	hStaticBalance = CreateWindowW(L"STATIC", L"Balance: 0",
		WS_CHILD | WS_VISIBLE,
		25, 80, 320, 28, hwnd, (HMENU)ID_STATIC_BALANCE, NULL, NULL);

	hStaticBet = CreateWindowW(L"STATIC", L"Bet: 0",
		WS_CHILD | WS_VISIBLE,
		25, 115, 320, 28, hwnd, (HMENU)ID_STATIC_BET, NULL, NULL);

	hEditBet = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
		25, 150, 200, 28, hwnd, (HMENU)ID_EDIT_BET, NULL, NULL);

	hButtonSetBet = CreateWindowW(L"BUTTON", L"Set bet",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		235, 150, 120, 28, hwnd, (HMENU)ID_BUTTON_SETBET, NULL, NULL);

	hButtonStart = CreateWindowW(L"BUTTON", L"Start round",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		25, 190, 160, 36, hwnd, (HMENU)ID_BUTTON_START, NULL, NULL);

	hButtonSettings = CreateWindowW(L"BUTTON", L"Settings",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		195, 190, 160, 36, hwnd, (HMENU)ID_BUTTON_SETTINGS, NULL, NULL);

	hButtonHit = CreateWindowW(L"BUTTON", L"Hit",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		400, 350, 100, 36, hwnd, (HMENU)ID_BUTTON_HIT, NULL, NULL);

	hButtonStand = CreateWindowW(L"BUTTON", L"Stand",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		510, 350, 100, 36, hwnd, (HMENU)ID_BUTTON_STAND, NULL, NULL);

	hButtonDouble = CreateWindowW(L"BUTTON", L"Double",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		620, 350, 100, 36, hwnd, (HMENU)ID_BUTTON_DOUBLE, NULL, NULL);

	hButtonSplit = CreateWindowW(L"BUTTON", L"Split",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		730, 350, 100, 36, hwnd, (HMENU)ID_BUTTON_SPLIT, NULL, NULL);

	hButtonExit = CreateWindowW(L"BUTTON", L"Exit",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		730, 400, 100, 36, hwnd, (HMENU)ID_BUTTON_EXIT, NULL, NULL);

	hStaticPlayer = CreateWindowW(L"STATIC", L"Player hand:",
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		400, 40, 430, 140, hwnd, (HMENU)ID_STATIC_PLAYER, NULL, NULL);

	hStaticDealer = CreateWindowW(L"STATIC", L"Dealer hand:",
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		400, 190, 430, 140, hwnd, (HMENU)ID_STATIC_DEALER, NULL, NULL);

	hStaticResult = CreateWindowW(L"STATIC", L"",
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		400, 300, 430, 36, hwnd, (HMENU)ID_STATIC_RESULT, NULL, NULL);

	hListHistory = CreateWindowW(L"LISTBOX", L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		25, 300, 320, 240, hwnd, (HMENU)ID_LIST_HISTORY, NULL, NULL);

	HWND titleControls[] = { hGroupControls, hGroupTable, hGroupHistory };
	for (HWND c : titleControls) ApplyFont(c, hFontTitle);

	HWND normals[] = {
		hEditPromo, hButtonPromo, hStaticBalance, hStaticBet,
		hEditBet, hButtonSetBet, hButtonStart, hButtonSettings,
		hButtonHit, hButtonStand, hButtonDouble, hButtonSplit, hButtonExit,
		hStaticPlayer, hStaticDealer, hStaticResult, hListHistory
	};
	for (HWND c : normals) ApplyFont(c, hFontNormal);

	LayoutControls(hwnd);
}

static void LayoutControls(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	int margin = 10;

	int leftW = 360;
	int controlsH = 240;
	int historyH = rc.bottom - controlsH - 3 * margin;

	SetWindowPos(hGroupControls, NULL,
		margin, margin,
		leftW, controlsH,
		SWP_NOZORDER);

	SetWindowPos(hGroupHistory, NULL,
		margin, controlsH + 2 * margin,
		leftW, historyH,
		SWP_NOZORDER);

	int tableX = leftW + 2 * margin;
	int tableW = rc.right - tableX - margin;
	int tableH = rc.bottom - 2 * margin;

	SetWindowPos(hGroupTable, NULL,
		tableX, margin,
		tableW, tableH,
		SWP_NOZORDER);

	int x = margin + 15;
	int y = margin + 30;

	SetWindowPos(hEditPromo, NULL, x, y, 200, 28, SWP_NOZORDER);
	SetWindowPos(hButtonPromo, NULL, x + 210, y, 120, 28, SWP_NOZORDER);

	y += 40;
	SetWindowPos(hStaticBalance, NULL, x, y, 300, 28, SWP_NOZORDER);

	y += 35;
	SetWindowPos(hStaticBet, NULL, x, y, 300, 28, SWP_NOZORDER);

	y += 35;
	SetWindowPos(hEditBet, NULL, x, y, 200, 28, SWP_NOZORDER);
	SetWindowPos(hButtonSetBet, NULL, x + 210, y, 120, 28, SWP_NOZORDER);

	y += 40;
	SetWindowPos(hButtonStart, NULL, x, y, 160, 36, SWP_NOZORDER);
	SetWindowPos(hButtonSettings, NULL, x + 170, y, 160, 36, SWP_NOZORDER);

	int tx = tableX + 15;
	int ty = margin + 30;

	SetWindowPos(hStaticPlayer, NULL, tx, ty, tableW - 30, 140, SWP_NOZORDER);

	ty += 150;
	SetWindowPos(hStaticDealer, NULL, tx, ty, tableW - 30, 140, SWP_NOZORDER);

	ty += 150;
	SetWindowPos(hStaticResult, NULL, tx, ty, tableW - 30, 36, SWP_NOZORDER);

	int btnCount = 5;
	int btnH = 36;
	int gap = 10;

	int totalWidth = tableW - 30;
	int btnW = (totalWidth - (btnCount - 1) * gap) / btnCount;

	int btnX = tx;
	int btnY = rc.bottom - btnH - margin;

	SetWindowPos(hButtonHit, NULL, btnX, btnY, btnW, btnH, SWP_NOZORDER);
	SetWindowPos(hButtonStand, NULL, btnX + (btnW + gap), btnY, btnW, btnH, SWP_NOZORDER);
	SetWindowPos(hButtonDouble, NULL, btnX + 2 * (btnW + gap), btnY, btnW, btnH, SWP_NOZORDER);
	SetWindowPos(hButtonSplit, NULL, btnX + 3 * (btnW + gap), btnY, btnW, btnH, SWP_NOZORDER);
	SetWindowPos(hButtonExit, NULL, btnX + 4 * (btnW + gap), btnY, btnW, btnH, SWP_NOZORDER);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		CreateGuiControls(hwnd);
		RefreshAll(true);
		return 0;

	case WM_SIZE:
		LayoutControls(hwnd);
		return 0;

	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, hBgBrush);
		return 1;
	}

	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, clrTextMain);
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)hPanelBrush;
	}

	case WM_CTLCOLORBTN:
	{
		HDC hdc = (HDC)wParam;
		SetTextColor(hdc, clrTextMain);
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)hButtonBrush;
	}

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
		FillRect(dis->hDC, &dis->rcItem, hButtonBrush);

		SetTextColor(dis->hDC, clrTextMain);
		SetBkMode(dis->hDC, TRANSPARENT);

		wchar_t text[64];
		GetWindowTextW(dis->hwndItem, text, 64);

		DrawTextW(dis->hDC, text, -1, (RECT*)&dis->rcItem,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		HPEN pen = CreatePen(PS_SOLID, 3, clrTextMain);
		HGDIOBJ oldPen = SelectObject(dis->hDC, pen);
		HGDIOBJ oldBrush = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));

		Rectangle(dis->hDC,
			dis->rcItem.left,
			dis->rcItem.top,
			dis->rcItem.right,
			dis->rcItem.bottom);

		SelectObject(dis->hDC, oldPen);
		SelectObject(dis->hDC, oldBrush);
		DeleteObject(pen);

		return TRUE;
	}

	case WM_TIMER:
		if (wParam == 1)
			UpdateHandsAnimatedStep();
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BUTTON_PROMO:
		{
			wchar_t buf[128];
			GetWindowTextW(hEditPromo, buf, 128);
			std::wstring ws = buf;
			std::string code = wstring_to_utf8(ws);

			std::string res = game.applyPromoCode(code);

			if (code == "404")
			{
				DestroyWindow(hwnd);
				return 0;
			}

			AddHistoryRecord(res);

			std::wstring wres = utf8_to_wstring(res);
			MessageBoxW(hwnd, wres.c_str(), L"Promo", MB_OK);
			RefreshAll(true);
		}
		return 0;

		case ID_BUTTON_SETBET:
		{
			wchar_t buf[64];
			GetWindowTextW(hEditBet, buf, 64);
			int bet = _wtoi(buf);

			game.resetRound();

			if (game.startRound(bet))
				StartDealAnimation(hwnd);

			RefreshAll(true);
		}
		return 0;

		case ID_BUTTON_START:
		{
			int bet = game.getCurrentBet();
			if (bet <= 0) bet = 10;

			game.resetRound();

			if (game.startRound(bet))
				StartDealAnimation(hwnd);

			RefreshAll(true);
		}
		return 0;

		case ID_BUTTON_HIT:
			game.hit();
			if (game.getState() == RoundState::Finished)
			{
				RefreshAll(false);
				OnRoundFinished();
			}
			else RefreshAll(true);
			return 0;

		case ID_BUTTON_STAND:
			game.stand();
			if (game.getState() == RoundState::Finished)
			{
				RefreshAll(false);
				OnRoundFinished();
			}
			else RefreshAll(true);
			return 0;

		case ID_BUTTON_DOUBLE:
			game.doubleDown();
			if (game.getState() == RoundState::Finished)
			{
				RefreshAll(false);
				OnRoundFinished();
			}
			else RefreshAll(true);
			return 0;

		case ID_BUTTON_SPLIT:
			game.split();
			RefreshAll(true);
			return 0;

		case ID_BUTTON_EXIT:
			DestroyWindow(hwnd);
			return 0;

		case ID_BUTTON_SETTINGS:
		{
			int choice = MessageBoxW(
				hwnd,
				L"Choose option:\n\nYes = Toggle hints\nNo = Toggle animations\nCancel = Change theme",
				L"Settings",
				MB_YESNOCANCEL | MB_ICONQUESTION
			);

			if (choice == IDYES)
			{
				bool newState = !Settings::hintsEnabled();
				Settings::enableHints(newState);
				AddHistoryRecord(newState ? "Hints enabled" : "Hints disabled");
			}
			else if (choice == IDNO)
			{
				gAnimating = !gAnimating;
				AddHistoryRecord(gAnimating ? "Animations enabled" : "Animations disabled");
			}
			else if (choice == IDCANCEL)
			{
				int theme = MessageBoxW(
					hwnd,
					L"Select theme:\n\nYes = Cyberpunk\nNo = Classic Green\nCancel = Light",
					L"Themes",
					MB_YESNOCANCEL | MB_ICONQUESTION
				);

				if (theme == IDYES)
				{
					hBgBrush = CreateSolidBrush(RGB(10, 10, 25));
					hPanelBrush = CreateSolidBrush(RGB(35, 35, 70));
					hButtonBrush = CreateSolidBrush(RGB(50, 50, 50));
					clrTextMain = RGB(0, 255, 200);
					AddHistoryRecord("Theme: Cyberpunk");
				}
				else if (theme == IDNO)
				{
					hBgBrush = CreateSolidBrush(RGB(0, 80, 0));
					hPanelBrush = CreateSolidBrush(RGB(0, 120, 0));
					hButtonBrush = CreateSolidBrush(RGB(0, 100, 0));
					clrTextMain = RGB(255, 255, 255);
					AddHistoryRecord("Theme: Classic Green");
				}
				else
				{
					hBgBrush = CreateSolidBrush(RGB(230, 230, 230));
					hPanelBrush = CreateSolidBrush(RGB(250, 250, 250));
					hButtonBrush = CreateSolidBrush(RGB(210, 210, 210));
					clrTextMain = RGB(0, 0, 0);
					AddHistoryRecord("Theme: Light");
				}

				InvalidateRect(hwnd, NULL, TRUE);
			}

			RefreshAll(true);
		}
		return 0;
		}

		return 0;

	case WM_DESTROY:
		if (gAnimTimer)
			KillTimer(hwnd, gAnimTimer);
		DeleteObject(hFontTitle);
		DeleteObject(hFontNormal);
		DeleteObject(hBgBrush);
		DeleteObject(hPanelBrush);
		DeleteObject(hButtonBrush);
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"BlackjackWindow";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = hBgBrush;

	RegisterClassW(&wc);

	HWND hwnd = CreateWindowW(
		CLASS_NAME,
		L"Blackjack - Casino",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		920, 640,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}