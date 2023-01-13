#include "userdlgs.h"

//---------------------------------------------------------------------
// DialogTemplate class member functions
//---------------------------------------------------------------------

void DialogTemplate::AlignToDword()
{
	if (v.size() % 4) Write(NULL, 4 - (v.size() % 4));
}

void DialogTemplate::Write(LPCVOID pvWrite, DWORD cbWrite)
{
    v.insert(v.end(), cbWrite, 0);
    if (pvWrite) CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
}

void DialogTemplate::WriteString(LPCWSTR psz)
{
    Write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR));
}
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// UserDialog class member functions
//---------------------------------------------------------------------

// Dialog box callback function

 INT_PTR CALLBACK UserDialog::DlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm)
    {
    case WM_INITDIALOG:
        //Get UserDialog instance pointer from lParam and save in window long.
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            wchar_t szUserInput[UD_EDITLIMIT];
            UserDialog* uD;
            //Recover UserDialog instance pointer
            uD = reinterpret_cast<UserDialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
            if (GetDlgItemText(hwnd, UD_EDIT, szUserInput, UD_EDITLIMIT)) uD->wsEdit = szUserInput;
        }
        case IDRETRY:
        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            break;
        }
    }
    return FALSE;
}

 //---------------------------------------------------------------------

// Build the dialog template
 bool UserDialog::BuildDlgTemplate(MsgType msgType, DialogTemplate* dlgTemplate)
 {
     HDC hdc = GetDC(NULL);
     if (hdc)
     {
         NONCLIENTMETRICSW ncm = { sizeof(ncm) };
         if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
         {
             // Clear any existing template
             dlgTemplate->Clear();

             // Write out the extended dialog template header
             dlgTemplate->Write<WORD>(1); // dialog version
             dlgTemplate->Write<WORD>(0xFFFF); // extended dialog template
             dlgTemplate->Write<DWORD>(0); // help ID
             dlgTemplate->Write<DWORD>(0); // extended style
             dlgTemplate->Write<DWORD>(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME);
             if (msgType == MsgType::OUTMSG)
                 if ((wButtons & UD_OKCANCELRETRY) == 1)    //OK or Yes button only
                     dlgTemplate->Write<WORD>(2);           // number of controls no input one button
                 else
                     if (wButtons & 4)                      // Has a retry button
                         dlgTemplate->Write<WORD>(4);
                     else
                         dlgTemplate->Write<WORD>(3);
             else
                 if ((wButtons & UD_OKCANCELRETRY) == 1)    //OK or Yes button only
                     dlgTemplate->Write<WORD>(3);           // number of controls no input one button
                 else
                     if (wButtons & 4)                      //Has a retry button
                         dlgTemplate->Write<WORD>(5);
                     else
                         dlgTemplate->Write<WORD>(4);
             dlgTemplate->Write<WORD>(150); // X
             dlgTemplate->Write<WORD>(100); // Y
             dlgTemplate->Write<WORD>(5*UD_INTERSPC + 4*UD_BTNWIDTH); // width
             dlgTemplate->Write<WORD>(4*UD_INTERSPC + UD_BTNHEIGHT + UD_EDTHEIGHT + UD_TXTHEIGHT); // height
             dlgTemplate->WriteString(L""); // no menu
             dlgTemplate->WriteString(L""); // default dialog class
             dlgTemplate->WriteString(wsTitle.c_str()); // title
             if (ncm.lfMessageFont.lfHeight < 0)
             {
                 ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight,
                     72, GetDeviceCaps(hdc, LOGPIXELSY));
             }
             dlgTemplate->Write<WORD>((WORD)ncm.lfMessageFont.lfHeight); // point
             dlgTemplate->Write<WORD>((WORD)ncm.lfMessageFont.lfWeight); // weight
             dlgTemplate->Write<BYTE>(ncm.lfMessageFont.lfItalic); // Italic
             dlgTemplate->Write<BYTE>(ncm.lfMessageFont.lfCharSet); // CharSet
             dlgTemplate->WriteString(ncm.lfMessageFont.lfFaceName);

             // Write out the static text control
             dlgTemplate->AlignToDword();
             dlgTemplate->Write<DWORD>(0); // help id
             dlgTemplate->Write<DWORD>(0); // window extended style
             dlgTemplate->Write<DWORD>(WS_CHILD | WS_VISIBLE); // style
             dlgTemplate->Write<WORD>(UD_INTERSPC); // x
             dlgTemplate->Write<WORD>(UD_INTERSPC); // y
             dlgTemplate->Write<WORD>(3 * UD_INTERSPC + 4 * UD_BTNWIDTH); // width
             dlgTemplate->Write<WORD>(UD_TXTHEIGHT); // height
             dlgTemplate->Write<DWORD>(-1); // control ID
             dlgTemplate->Write<DWORD>(0x0082FFFF); // static
             dlgTemplate->WriteString(wsStatic.c_str()); // text
             dlgTemplate->Write<WORD>(0); // no extra data
             if (msgType != MsgType::OUTMSG)
             {
                 // Write out the input text area
                 dlgTemplate->AlignToDword();
                 dlgTemplate->Write<DWORD>(0); // help id
                 dlgTemplate->Write<DWORD>(0); // window extended style
                 dlgTemplate->Write<DWORD>(WS_CHILD | WS_VISIBLE
                     | WS_GROUP | WS_TABSTOP); // style
                 dlgTemplate->Write<WORD>(UD_INTERSPC); // x
                 dlgTemplate->Write<WORD>(UD_TXTHEIGHT + 2*UD_INTERSPC); // y
                 dlgTemplate->Write<WORD>(3 * UD_INTERSPC + 4 * UD_BTNWIDTH); // width
                 dlgTemplate->Write<WORD>(UD_EDTHEIGHT); // height
                 dlgTemplate->Write<DWORD>(UD_EDIT); // control ID
                 dlgTemplate->Write<DWORD>(0x0081FFFF); // edit
                 dlgTemplate->WriteString(wsEdit.c_str()); // text
                 dlgTemplate->Write<WORD>(0); // no extra data
             }
              // Write out the button control(s)

             //OK/YES button first
             dlgTemplate->AlignToDword();
             dlgTemplate->Write<DWORD>(0); // help id
             dlgTemplate->Write<DWORD>(0); // window extended style
             dlgTemplate->Write<DWORD>(WS_CHILD | WS_VISIBLE |
                 WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
             dlgTemplate->Write<WORD>(4 * UD_INTERSPC + 3 * UD_BTNWIDTH); // x
             dlgTemplate->Write<WORD>(3 * UD_INTERSPC + UD_EDTHEIGHT + UD_TXTHEIGHT); // y
             dlgTemplate->Write<WORD>(UD_BTNWIDTH); // width
             dlgTemplate->Write<WORD>(UD_BTNHEIGHT); // height
             dlgTemplate->Write<DWORD>(IDOK); // control ID
             dlgTemplate->Write<DWORD>(0x0080FFFF); // button
             if (wButtons & 8)                      //Using Yes/No labels
                 dlgTemplate->WriteString(L"Yes"); // text
             else
                 dlgTemplate->WriteString(L"OK");
             dlgTemplate->Write<WORD>(0); // no extra data

              //Cancel/No button second
             if (wButtons & 2)              //Has a Cancel button
             {
                 dlgTemplate->AlignToDword();
                 dlgTemplate->Write<DWORD>(0); // help id
                 dlgTemplate->Write<DWORD>(0); // window extended style
                 dlgTemplate->Write<DWORD>(WS_CHILD | WS_VISIBLE |
                     WS_GROUP | WS_TABSTOP); // style
                 dlgTemplate->Write<WORD>(3 * UD_INTERSPC + 2 * UD_BTNWIDTH); // x
                 dlgTemplate->Write<WORD>(3 * UD_INTERSPC + UD_EDTHEIGHT + UD_TXTHEIGHT); // y
                 dlgTemplate->Write<WORD>(UD_BTNWIDTH); // width
                 dlgTemplate->Write<WORD>(UD_BTNHEIGHT); // height
                 dlgTemplate->Write<DWORD>(IDCANCEL); // control ID
                 dlgTemplate->Write<DWORD>(0x0080FFFF); // button
                 if (wButtons & 8)          //Using Yes/No labels
                     dlgTemplate->WriteString(L"No"); // text
                 else
                     dlgTemplate->WriteString(L"Cancel");
                 dlgTemplate->Write<WORD>(0); // no extra data

                 //Finally Retry button if required
                 if (wButtons & 4)              //Has a Retry button
                 {
                     dlgTemplate->AlignToDword();
                     dlgTemplate->Write<DWORD>(0); // help id
                     dlgTemplate->Write<DWORD>(0); // window extended style
                     dlgTemplate->Write<DWORD>(WS_CHILD | WS_VISIBLE |
                         WS_GROUP | WS_TABSTOP); // style
                     dlgTemplate->Write<WORD>(2 * UD_INTERSPC + UD_BTNWIDTH); // x
                     dlgTemplate->Write<WORD>(3 * UD_INTERSPC + UD_EDTHEIGHT + UD_TXTHEIGHT); // y
                     dlgTemplate->Write<WORD>(UD_BTNWIDTH); // width
                     dlgTemplate->Write<WORD>(UD_BTNHEIGHT); // height
                     dlgTemplate->Write<DWORD>(IDRETRY); // control ID
                     dlgTemplate->Write<DWORD>(0x0080FFFF); // button
                     dlgTemplate->WriteString(L"Retry"); // text
                     dlgTemplate->Write<WORD>(0); // no extra data
                 }
             }
             return true;
         }
     }
     return false;
 }

 WORD UserDialog::Show(MsgType msgType, HWND hWnd)
 {
     DialogTemplate dT;

     if (BuildDlgTemplate(msgType, &dT))
         return DialogBoxIndirectParam(GetModuleHandle(NULL), dT.Template(), hWnd, &DlgProc, reinterpret_cast<LPARAM>(this));
     else
         return 0;
 }

