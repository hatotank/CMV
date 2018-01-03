/*
  CMV -よくカーソルを見失う人の為に-
  2015/09/14 Tubo
*/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#define HOT_KEY_ID 100001
#define ID_TRAY (101)
#define MSG_TRAY WM_USER

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProcSet(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProcVer(HWND, UINT, WPARAM, LPARAM);
int MakeRTrayMenu(HWND);
void SetTaskTray(HWND, PNOTIFYICONDATA);
void DlgSetIni(HWND,long,long);

HWND hWnd;
HMENU hMenu,hSubMenu;
HINSTANCE hInst;

char szClassName[] = "cmv";
char szVersion[] = "Version 1.00 20150914";
char g_lpPos[256];
char g_lpPosX[256];
char g_lpPosY[256];
char g_lpUsrHotkey[256];
char g_lpUsrHotkeyId[256];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASS wc;
    hInst = hInstance;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;

    RegisterClass(&wc);

    hWnd = CreateWindow(szClassName,
                        szClassName,
                        WS_CAPTION | WS_SYSMENU | WS_DLGFRAME,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        400,
                        300,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    ShowWindow(hWnd, SW_HIDE); //メインウィンドウは非表示
    UpdateWindow(hWnd);

    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int sc_x,sc_y;
    int pos_x,pos_y;
    
    static NOTIFYICONDATA ni;
    long lRtnCd;
    DWORD dwHotkey;

    switch(msg){
        case WM_CREATE:
            //タスクトレイメニュー
            hMenu = LoadMenu(hInst,MAKEINTRESOURCE(IDR_TRAY));

            //INIから設定取得
            lRtnCd = GetPrivateProfileString("cmv","position",   "0",  g_lpPos,        sizeof(g_lpPos),        ".\\cmv.ini");
            lRtnCd = GetPrivateProfileString("cmv","pos_x",      "100",g_lpPosX,       sizeof(g_lpPosX),       ".\\cmv.ini");
            lRtnCd = GetPrivateProfileString("cmv","pos_y",      "100",g_lpPosY,       sizeof(g_lpPosY),       ".\\cmv.ini");
            lRtnCd = GetPrivateProfileString("cmv","usrhotkey",  "0",  g_lpUsrHotkey,  sizeof(g_lpUsrHotkey),  ".\\cmv.ini");
            lRtnCd = GetPrivateProfileString("cmv","usrhotkeyid","0",  g_lpUsrHotkeyId,sizeof(g_lpUsrHotkeyId),".\\cmv.ini");

            //位置調節の値チェック
            pos_y = atoi(g_lpPosY);
            pos_x = atoi(g_lpPosX);
            if(pos_y < 1 || pos_y > GetSystemMetrics(SM_CYSCREEN)-1){
                sprintf(g_lpPosY,"%d", 100);
            }
            if(pos_x < 1 || pos_x > GetSystemMetrics(SM_CXSCREEN)-1){
                sprintf(g_lpPosX,"%d", 100);
            }

            //ホットキー設定
            if(atoi(g_lpUsrHotkey) != 1){
                RegisterHotKey(hWnd,HOT_KEY_ID,MOD_CONTROL | MOD_ALT,0x51);
            }else{
                dwHotkey = atoi(g_lpUsrHotkeyId);
                if(dwHotkey != 0){
                    BYTE lo,hi,keyMod = 0;
                    lo = LOBYTE(dwHotkey);
                    hi = HIBYTE(dwHotkey);
                    if((hi & 0x1) != 0) keyMod = keyMod | MOD_SHIFT;
                    if((hi & 0x2) != 0) keyMod = keyMod | MOD_CONTROL;
                    if((hi & 0x4) != 0) keyMod = keyMod | MOD_ALT;
                    RegisterHotKey(hWnd,HOT_KEY_ID,keyMod,lo);
                }
            }

            //タスクトレイ格納
            SetTaskTray(hWnd, &ni);
            break;

        //ホットキー
        case WM_HOTKEY:
            switch(wParam){
                case HOT_KEY_ID:
                    sc_x = GetSystemMetrics(SM_CXSCREEN);
                    sc_y = GetSystemMetrics(SM_CYSCREEN);
                    pos_x = atoi(g_lpPosX);
                    pos_y = atoi(g_lpPosY);
                    switch(atoi(g_lpPos)){
                        case 0: //左上
                            SetCursorPos(pos_x,pos_y);
                            break;
                        case 1: //右上
                            SetCursorPos(sc_x-pos_x,pos_y);
                            break;
                        case 2: //右下
                            SetCursorPos(sc_x-pos_x,sc_y-pos_y);
                            break;
                        case 3: //左下
                            SetCursorPos(pos_x,sc_y-pos_y);
                            break;
                        case 4: //中央
                            SetCursorPos((int)sc_x/2,(int)sc_y/2);
                            break;
                        default:
                            SetCursorPos((int)sc_x/2,(int)sc_y/2);
                            break;
                    }
                    break;
            }
            break;
            
        case MSG_TRAY:
            if(wParam == ID_TRAY){
                switch(lParam){
                    case WM_RBUTTONDOWN:
                        MakeRTrayMenu(hWnd);
                        break;
                }
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case IDM_SETUP:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), hWnd, (DLGPROC)DlgProcSet);
                    break;
                case IDM_VERSION:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_VERSION), hWnd, (DLGPROC)DlgProcVer);
                    break;
                case IDM_END:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            UnregisterHotKey(hWnd,HOT_KEY_ID); //ホットキー解除
            Shell_NotifyIcon(NIM_DELETE, &ni);
            DestroyMenu(hMenu);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

//ダイアログプロシージャ
BOOL CALLBACK DlgProcSet(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static long radioPos,radioHotKey;
    static int tpos_x,tpos_y;
    static int pos,hotkey;
    char buf[80];
    bool tpos_yflg = true,tpos_xflg = true;
    LPNMUPDOWN lpnmud;
    HWND hHotkeyWnd;
    DWORD dwHotkey;

    switch(msg){
        case WM_INITDIALOG:
            //コモンコントロール初期化
            InitCommonControls();
            //ラジオボタン処理
            pos = atoi(g_lpPos);
            hotkey = atoi(g_lpUsrHotkey);
            switch(pos){
                case 0: radioPos = IDC_RADIO_UL; break;
                case 1: radioPos = IDC_RADIO_UR; break;
                case 2: radioPos = IDC_RADIO_LR; break;
                case 3: radioPos = IDC_RADIO_LL; break;
                case 4: radioPos = IDC_RADIO_C;  break;
            }
            switch(hotkey){
                case 0: radioHotKey = IDC_RADIO_HOTKEY_DEF; break;
                case 1: radioHotKey = IDC_RADIO_HOTKEY_USR; break;
            }
            DlgSetIni(hDlgWnd,radioPos,radioHotKey);

            //位置調節
            tpos_y = atoi(g_lpPosY);
            sprintf(buf,"%d",tpos_y);
            SetDlgItemText(hDlgWnd,IDC_EDIT_H,buf);

            tpos_x = atoi(g_lpPosX);
            sprintf(buf,"%d",tpos_x);
            SetDlgItemText(hDlgWnd,IDC_EDIT_W,buf);

            //ホットキーの値取得
            hHotkeyWnd = GetDlgItem(hDlgWnd, IDC_HOTKEY_USR);
            dwHotkey = SendMessage(hHotkeyWnd, HKM_SETHOTKEY, atoi(g_lpUsrHotkeyId), 0);
            break;

        case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDC_RADIO_UL: //左上
                    radioPos = IDC_RADIO_UL;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    pos = 0;
                    break;
                case IDC_RADIO_UR: //右上
                    radioPos = IDC_RADIO_UR;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    pos = 1;
                    break;
                case IDC_RADIO_LR: //右下
                    radioPos = IDC_RADIO_LR;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    pos = 2;
                    break;
                case IDC_RADIO_LL: //左下
                    radioPos = IDC_RADIO_LL;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    pos = 3;
                    break;
                case IDC_RADIO_C:  //中央
                    radioPos = IDC_RADIO_C;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    pos = 4;
                    break;

                case IDC_RADIO_HOTKEY_DEF: //デフォ
                    radioHotKey = IDC_RADIO_HOTKEY_DEF;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    hotkey = 0;
                    break;
                case IDC_RADIO_HOTKEY_USR: //ユーザ
                    radioHotKey = IDC_RADIO_HOTKEY_USR;
                    DlgSetIni(hDlgWnd,radioPos,radioHotKey);
                    hotkey = 1;
                    break;

                case IDC_EDIT_H: //エディット(高さ)
                    if(HIWORD(wp) == EN_UPDATE){
                        GetDlgItemText(hDlgWnd,IDC_EDIT_H,buf,sizeof(buf));
                        tpos_y = atoi(buf);
                    }
                    break;
                case IDC_EDIT_W: //エディット(幅)
                    if(HIWORD(wp) == EN_UPDATE){
                        GetDlgItemText(hDlgWnd,IDC_EDIT_W,buf,sizeof(buf));
                        tpos_x = atoi(buf);
                    }
                    break;

                case IDOK:
                    //基本位置
                    sprintf(g_lpPos,"%d",pos);

                    //位置調節処理(範囲チェック)
                    //縦(y軸)
                    if(tpos_y < 1){
                        tpos_y = 1;
                    }
                    if(tpos_y > GetSystemMetrics(SM_CYSCREEN)){
                        tpos_y = GetSystemMetrics(SM_CYSCREEN)-1;
                    }
                    sprintf(g_lpPosY,"%d",tpos_y);
                    //横(x軸)
                    if(tpos_x < 1){
                        tpos_x = 1;
                    }
                    if(tpos_x > GetSystemMetrics(SM_CXSCREEN)){
                        tpos_x = GetSystemMetrics(SM_CXSCREEN)-1;
                    }
                    sprintf(g_lpPosX,"%d",tpos_x);

                    //ホットキー関係処理
                    UnregisterHotKey(hWnd,HOT_KEY_ID);
                    sprintf(g_lpUsrHotkey,"%d",hotkey);
                    hHotkeyWnd = GetDlgItem(hDlgWnd,IDC_HOTKEY_USR);
                    dwHotkey = SendMessage(hHotkeyWnd,HKM_GETHOTKEY,0,0);
                    if(atoi(g_lpUsrHotkey) == 1 && dwHotkey > 0){
                        BYTE lo,hi,keyMod = 0;
                        lo = LOBYTE(dwHotkey);
                        hi = HIBYTE(dwHotkey);
                        //取得コードが設定用と異なる？
                        //ALT:4     -> 1
                        //CONTROL:2 -> 2
                        //SHIFT:1   -> 4
                        //WIN:8     -> 8
                        if((hi & 0x1) != 0) keyMod = keyMod | MOD_SHIFT;
                        if((hi & 0x2) != 0) keyMod = keyMod | MOD_CONTROL;
                        if((hi & 0x4) != 0) keyMod = keyMod | MOD_ALT;
                        RegisterHotKey(hWnd,HOT_KEY_ID,keyMod,lo);
                        sprintf(g_lpUsrHotkeyId,"%d",dwHotkey);
                    }else{
                        sprintf(g_lpUsrHotkey,"%d",0);
                        RegisterHotKey(hWnd,HOT_KEY_ID,MOD_CONTROL | MOD_ALT,0x51);
                    }

                    //INI設定
                    WritePrivateProfileString("cmv","position",   g_lpPos,        ".\\cmv.ini");
                    WritePrivateProfileString("cmv","pos_x",      g_lpPosX,       ".\\cmv.ini");
                    WritePrivateProfileString("cmv","pos_y",      g_lpPosY,       ".\\cmv.ini");
                    WritePrivateProfileString("cmv","usrhotkey",  g_lpUsrHotkey,  ".\\cmv.ini");
                    WritePrivateProfileString("cmv","usrhotkeyId",g_lpUsrHotkeyId,".\\cmv.ini");

                    EndDialog(hDlgWnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlgWnd, IDCANCEL);
                    break;
            }
            break;

        case WM_NOTIFY:
            //スピンコントロール(縦)
            if(wp == (WPARAM)IDC_SPIN_H){
                lpnmud = (LPNMUPDOWN)lp;
                if(lpnmud->hdr.code == UDN_DELTAPOS){
                    GetDlgItemText(hDlgWnd,IDC_EDIT_H,buf,sizeof(buf));
                    tpos_y = atoi(buf);
                    //0以下の場合は強制的に1をセット
                    if(tpos_y < 1){
                        tpos_y = 1;
                        tpos_yflg = false;
                    }
                    //画面サイズを超えている場合は画面サイズ-1をセット
                    if(tpos_y > GetSystemMetrics(SM_CYSCREEN)){
                        tpos_y = GetSystemMetrics(SM_CYSCREEN)-1;
                        tpos_yflg = false;
                    }
                    //数値の加算減算処理
                    if(tpos_yflg){
                        if(lpnmud->iDelta < 0 && tpos_y < GetSystemMetrics(SM_CYSCREEN)-1){
                            tpos_y++;
                        }else if(lpnmud->iDelta > 0 && tpos_y > 1){
                            tpos_y--;
                        }
                    }
                    sprintf(buf,"%d",tpos_y);
                    sprintf(g_lpPosY,"%d",tpos_y);
                    SetDlgItemText(hDlgWnd,IDC_EDIT_H,buf);
                }
            }
            //スピンコントロール(横)
            if(wp == (WPARAM)IDC_SPIN_W){
                lpnmud = (LPNMUPDOWN)lp;
                if(lpnmud->hdr.code == UDN_DELTAPOS){
                    GetDlgItemText(hDlgWnd,IDC_EDIT_W,buf,sizeof(buf));
                    tpos_x = atoi(buf);
                    //0以下の場合は強制的に1をセット
                    if(tpos_x < 1){
                        tpos_x = 1;
                        tpos_xflg = false;
                    }
                    //画面サイズを超えている場合は画面サイズ-1をセット
                    if(tpos_x > GetSystemMetrics(SM_CXSCREEN)){
                        tpos_x = GetSystemMetrics(SM_CXSCREEN)-1;
                        tpos_xflg = false;
                    }
                    //数値の加算減算処理
                    if(tpos_xflg){
                        if((lpnmud->iDelta) < 0 && tpos_x < GetSystemMetrics(SM_CXSCREEN)-1){
                            tpos_x++;
                        }else if((lpnmud->iDelta) > 0 && tpos_x > 1){
                            tpos_x--;
                        }
                    }
                    sprintf(buf,"%d",tpos_x);
                    sprintf(g_lpPosX,"%d",tpos_x);
                    SetDlgItemText(hDlgWnd,IDC_EDIT_W,buf);
                }
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlgWnd, IDOK);
            return TRUE;
    }
    return FALSE;
}

//ダイアログプロシージャ(バージョン情報)
BOOL CALLBACK DlgProcVer(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg){
        case WM_INITDIALOG:
            //バージョン情報
            SetDlgItemText(hDlgWnd, IDC_VERSION, szVersion);
            break;
        case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
                    EndDialog(hDlgWnd, IDOK);
                    break;
            }
            break;
        case WM_CLOSE:
            EndDialog(hDlgWnd, IDOK);
            return TRUE;
    }
    return FALSE;
}

//タスクトレイセット
void SetTaskTray(HWND hWnd, PNOTIFYICONDATA ni)
{
    HICON hIcon;

    hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_CMV1));
    ni->cbSize = sizeof(NOTIFYICONDATA);
    ni->hIcon = hIcon;
    ni->hWnd = hWnd;
    ni->uCallbackMessage = MSG_TRAY;
    ni->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    ni->uID = ID_TRAY;
    strcpy(ni->szTip, szClassName);

    Shell_NotifyIcon(NIM_ADD, ni);

    return;
}

//タスクトレイ用メニュー
int MakeRTrayMenu(HWND hWnd)
{
    POINT pt;

    hSubMenu = GetSubMenu(hMenu, 0);
    GetCursorPos(&pt);
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hSubMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);

    return 0;
}

//メイン用ラジオボタン設定
void DlgSetIni(HWND hDlgWnd, long radioPos, long radioHotkey)
{
    //基本位置
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_UL),BM_SETCHECK,BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_UR),BM_SETCHECK,BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_LR),BM_SETCHECK,BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_LL),BM_SETCHECK,BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_C) ,BM_SETCHECK,BST_UNCHECKED,0);
    //選択
    SendMessage(GetDlgItem(hDlgWnd,radioPos),BM_SETCHECK,BST_CHECKED,0);

    //ホットキー
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_HOTKEY_DEF),BM_SETCHECK,BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hDlgWnd,IDC_RADIO_HOTKEY_USR),BM_SETCHECK,BST_UNCHECKED,0);
    //選択
    SendMessage(GetDlgItem(hDlgWnd,radioHotkey),BM_SETCHECK,BST_CHECKED,0);
}
