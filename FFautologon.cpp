// FFautologon - Auto-logon manipulator for Windows
// License: MIT

// Detect memory leaks (for Debug and MSVC)
#if defined(_MSC_VER) && !defined(NDEBUG) && !defined(_CRTDBG_MAP_ALLOC)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#define CLASSNAME  L"ffautologon"

inline WORD get_lang_id(void)
{
    return PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale()));
}

// localization
LPCTSTR get_text(INT id)
{
#ifdef JAPAN
    if (get_lang_id() == LANG_JAPANESE) // Japone for Japone
    {
        switch (id)
        {
        case 0: return TEXT("FFautologon バージョン 0.6 by 片山博文MZ");
        case 1:
            return TEXT("使い方: FFautologon [オプション]\n")
                   TEXT("\n")
                   TEXT("オプション\n")
                   TEXT("  -enable                   自動ログインを有効にする。\n")
                   TEXT("  -disable                  自動ログインを無効にする。\n")
                   TEXT("  -confirm                  自動ログインを変更するか確認する。\n")
                   TEXT("  -ask_password             パスワードをユーザーに問合せします。\n")
                   TEXT("  -password パスワード      パスワードを指定します。\n")
                   TEXT("  -rot13_pass ROT13PASS     ROT13パスワードを指定します。\n")
                   TEXT("  -help                     このメッセージを表示する。\n")
                   TEXT("  -version                  バージョン情報を表示する。");
        case 2: return TEXT("エラー: アクションが未指定です。\n");
        case 3: return TEXT("FFautologon");
        case 4: return TEXT("現在のユーザーは「%ls」です。自動ログオンを有効にしますか？");
        case 5: return TEXT("現在のユーザーは「%ls」です。自動ログオンを無効にしますか？");
        case 6: return TEXT("エラー: -enable と -disable を両方指定することはできません。\n");
        case 7: return TEXT("エラー: オプション -password には引数が必要です。\n");
        case 8: return TEXT("エラー: 自動ログオンの有効化に失敗しました。\n");
        case 9: return TEXT("エラー: 自動ログオンの無効化に失敗しました。\n");
        case 10: return TEXT("成功：ユーザー「%ls」の自動ログオンを有効にしました。\n");
        case 11: return TEXT("成功：ユーザー「%ls」の自動ログオンを無効にしました。\n");
        case 12: return TEXT("エラー: レジストリを開くのに失敗しました。\n");
        case 13: return TEXT("エラー: 値 DefaultUserName のセットに失敗しました。\n");
        case 14: return TEXT("エラー: 値 DefaultPassword のセットに失敗しました。\n");
        case 15: return TEXT("エラー: 値 AutoAdminLogon のセットに失敗しました。\n");
        case 16: return TEXT("エラー: オプション -rot13_pass には引数が必要です。\n");
        case 17: return TEXT("ユーザーによって処理はキャンセルされました。\n");
        }
    }
    else // The others are Let's la English
#endif
    {
        switch (id)
        {
        case 0: return TEXT("FFautologon version 0.6 by katahiromz");
        case 1:
            return TEXT("Usage: FFautologon [Options]\n")
                   TEXT("\n")
                   TEXT("Options\n")
                   TEXT("  -enable                   Enables auto-logon.\n")
                   TEXT("  -disable                  Disables auto-logon.\n")
                   TEXT("  -confirm                  Confirms for changing auto-logon.\n")
                   TEXT("  -ask_password             Queries the user for the password.\n")
                   TEXT("  -password PASSWORD        Specifies the password.\n")
                   TEXT("  -rot13_pass ROT13PASS     Specifies the ROT13 password.\n")
                   TEXT("  -help                     Displays this message.\n")
                   TEXT("  -version                  Displays version info.");
        case 2: TEXT("ERROR: No action specified\n");
        case 3: TEXT("FFautologon");
        case 4: TEXT("The current user name is '%ls'. Do you want to enable auto-logon?");
        case 5: TEXT("The current user name is '%ls'. Do you want to disable auto-logon?");
        case 6: TEXT("ERROR: Unable to specify both -enable and -disable.\n");
        case 7: TEXT("ERROR: Option -password needs an operand.\n");
        case 8: TEXT("ERROR: Failed to enable auto-logon.\n");
        case 9: TEXT("ERROR: Failed to disable auto-logon.\n");
        case 10: return TEXT("SUCCESS: Auto-logon enabled for user '%ls'.\n");
        case 11: return TEXT("SUCCESS: Auto-logon disabled for user '%ls'.\n");
        case 12: return TEXT("ERROR: Failed to open registry.\n");
        case 13: return TEXT("ERROR: Failed to set value DefaultUserName.\n");
        case 14: return TEXT("ERROR: Failed to set value DefaultPassword.\n");
        case 15: return TEXT("ERROR: Failed to set value AutoAdminLogon.\n");
        case 16: return TEXT("ERROR: Option -rot13_pass needs an operand.\n");
        case 17: return TEXT("The operation was canceled by the user.\n");
        }
    }

    assert(0);
    return nullptr;
}

typedef struct FFAUTOLOGON
{
    bool m_enable = false;
    bool m_disable = false;
    bool m_confirm = false;
    bool m_ask_password = false;
    std::wstring m_password;

    int parse_cmd_line(INT argc, LPWSTR *argv);
    int run(void);
    bool enable_auto_logon(LPCWSTR user_name, LPCWSTR password, bool enable = true);
} FFAUTOLOGON;

INT_PTR CALLBACK
PasswordDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            FFAUTOLOGON *pThis = (FFAUTOLOGON *)lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pThis);

            TCHAR user_name[128];
            DWORD length = _countof(user_name);
            GetUserName(user_name, &length);

            SetDlgItemText(hwnd, edt2, user_name);
            SetFocus(GetDlgItem(hwnd, edt1));
        }
        return FALSE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                FFAUTOLOGON *pThis = (FFAUTOLOGON *)GetWindowLongPtr(hwnd, DWLP_USER);
                TCHAR password[512];
                GetDlgItemText(hwnd, edt1, password, _countof(password));
                pThis->m_password = password;
                EndDialog(hwnd, IDOK);
            }
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

void version(void)
{
    _putts(get_text(0));
}

void usage(void)
{
    _putts(get_text(1));
}

std::wstring rot13(const std::wstring& value)
{
    std::wstring result = value;
    for (wchar_t& ch : result)
    {
        if (L'a' <= ch && ch <= L'z')
            ch = L'a' + ((ch - L'a' + 13) % 26);
        else if (L'A' <= ch && ch <= L'Z')
            ch = L'A' + ((ch - L'A' + 13) % 26);
    }
    return result;
}

bool FFAUTOLOGON::enable_auto_logon(LPCWSTR user_name, LPCWSTR password, bool enable)
{
    LPCTSTR regkey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
    LONG error;
    HKEY hKey;
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regkey, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &hKey);
    if (error)
    {
        _ftprintf(stderr, get_text(12));
        return false;
    }

    DWORD cbValue = (lstrlenW(user_name) + 1) * sizeof(WCHAR);
    error = RegSetValueEx(hKey, L"DefaultUserName", 0, REG_SZ, (BYTE*)user_name, cbValue);
    if (error)
    {
        _ftprintf(stderr, get_text(13));
        RegCloseKey(hKey);
        return false;
    }

    if (enable)
    {
        cbValue = (lstrlenW(password) + 1) * sizeof(WCHAR);
        error = RegSetValueEx(hKey, L"DefaultPassword", 0, REG_SZ, (BYTE*)password, cbValue);
        if (error)
        {
            _ftprintf(stderr, get_text(14));
            RegCloseKey(hKey);
            return false;
        }
    }
    else
    {
        RegDeleteValue(hKey, L"DefaultPassword");
    }

    WCHAR value[8];
    if (enable)
        lstrcpynW(value, L"1", _countof(value));
    else
        lstrcpynW(value, L"0", _countof(value));

    cbValue = (lstrlenW(value) + 1) * sizeof(WCHAR);
    error = RegSetValueEx(hKey, L"AutoAdminLogon", 0, REG_SZ, (BYTE*)value, cbValue);
    if (error)
    {
        _ftprintf(stderr, get_text(15));
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

int FFAUTOLOGON::parse_cmd_line(INT argc, LPWSTR *argv)
{
    if (argc <= 1)
    {
        usage();
        return 1;
    }

    for (INT iarg = 1; iarg < argc; ++iarg)
    {
        LPWSTR arg = argv[iarg];

        if (_wcsicmp(arg, L"-help") == 0 || _wcsicmp(arg, L"--help") == 0)
        {
            usage();
            return 1;
        }

        if (_wcsicmp(arg, L"-version") == 0 || _wcsicmp(arg, L"--version") == 0)
        {
            version();
            return 1;
        }

        if (_wcsicmp(arg, L"-enable") == 0 || _wcsicmp(arg, L"--enable") == 0)
        {
            m_enable = true;
            continue;
        }

        if (_wcsicmp(arg, L"-disable") == 0 || _wcsicmp(arg, L"--disable") == 0)
        {
            m_disable = true;
            continue;
        }

        if (_wcsicmp(arg, L"-confirm") == 0 || _wcsicmp(arg, L"--confirm") == 0)
        {
            m_confirm = true;
            continue;
        }

        if (_wcsicmp(arg, L"-ask_password") == 0 || _wcsicmp(arg, L"--ask_password") == 0)
        {
            m_ask_password = true;
            continue;
        }

        if (_wcsicmp(arg, L"-password") == 0 || _wcsicmp(arg, L"--password") == 0)
        {
            if (iarg + 1 < argc)
            {
                m_password = argv[++iarg];
                continue;
            }
            else
            {
                _ftprintf(stderr, get_text(7));
                return 1;
            }
        }

        if (_wcsicmp(arg, L"-rot13_pass") == 0 || _wcsicmp(arg, L"--rot13_pass") == 0)
        {
            if (iarg + 1 < argc)
            {
                m_password = rot13(argv[++iarg]);
                continue;
            }
            else
            {
                _ftprintf(stderr, get_text(16));
                return 1;
            }
        }
    }

    if (!m_enable && !m_disable)
    {
        _ftprintf(stderr, get_text(2));
        return 1;
    }

    if (m_enable && m_disable)
    {
        _ftprintf(stderr, get_text(6));
        return 1;
    }

    return 0;
}

int FFAUTOLOGON::run(void)
{
    TCHAR user_name[128];
    DWORD length = _countof(user_name);
    GetUserName(user_name, &length);

    if (m_confirm)
    {
        TCHAR text[512];

        if (m_enable)
        {
            wnsprintf(text, _countof(text), get_text(4), user_name);
            if (MessageBox(nullptr, text, get_text(3), MB_ICONINFORMATION | MB_YESNOCANCEL) != IDYES)
                return 1;
        }
        else if (m_disable)
        {
            wnsprintf(text, _countof(text), get_text(5), user_name);
            if (MessageBox(nullptr, text, get_text(3), MB_ICONINFORMATION | MB_YESNOCANCEL) != IDYES)
                return 1;
        }
        else
        {
            assert(0);
        }
    }

    if (m_ask_password && m_enable)
    {
        if (DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PASSWORD), NULL,
                           PasswordDlgProc, (LPARAM)this) == IDCANCEL)
        {
            _ftprintf(stderr, get_text(17));
            return 1;
        }
    }

    if (m_enable)
    {
        if (!enable_auto_logon(user_name, m_password.c_str(), true))
        {
            _ftprintf(stderr, get_text(8));
            return 1;
        }
        _tprintf(get_text(10), user_name);
    }
    else if (m_disable)
    {
        if (!enable_auto_logon(user_name, m_password.c_str(), false))
        {
            _ftprintf(stderr, get_text(9));
            return 1;
        }
        _tprintf(get_text(11), user_name);
    }
    else
    {
        assert(0);
    }

    return 0;
}

int wmain(INT argc, LPWSTR *argv)
{
    FFAUTOLOGON ffautologon;
    if (int ret = ffautologon.parse_cmd_line(argc, argv))
        return ret;

    return ffautologon.run();
}

#include <clocale>

int main(void)
{
    // Unicode console output support
    std::setlocale(LC_ALL, "");

    INT myargc;
    LPWSTR *myargv = CommandLineToArgvW(GetCommandLineW(), &myargc);
    INT ret = wmain(myargc, myargv);
    LocalFree(myargv);

    // Detect handle leaks (for Debug)
#if (_WIN32_WINNT >= 0x0500) && !defined(NDEBUG)
    TCHAR szText[MAX_PATH];
    wnsprintf(szText, _countof(szText), TEXT("GDI Objects: %ld, User Objects: %ld\n"),
              GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS),
              GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
    OutputDebugString(szText);
#endif

    // Detect memory leaks (for Debug and MSVC)
#if defined(_MSC_VER) && !defined(NDEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}
