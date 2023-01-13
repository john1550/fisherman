#pragma once
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

class DialogTemplate
{
public:
    LPCDLGTEMPLATE Template() const { return reinterpret_cast<LPCDLGTEMPLATE>(&v[0]); }
    void AlignToDword();
    void Write(LPCVOID pvWrite, DWORD cbWrite);
    template<typename T> void Write(T t) { Write(&t, sizeof(T)); }
    void WriteString(LPCWSTR psz);
    void Clear() { v.clear(); }

private:
    vector<BYTE> v;

};

// Messagebox types
enum class MsgType
{
    OUTMSG,
    INTEXT,
    ININT,
    INFLOAT
};

// Button combinations
constexpr auto UD_OKONLY = 1;
constexpr auto UD_OKCANCEL = 3;
constexpr auto UD_OKCANCELRETRY = 7;
constexpr auto UD_YESNO = 11;           //Use yes/no instead of ok/cancel
constexpr auto UD_YESNORETRY = 15;


constexpr auto UD_EDIT = 1000;          // Edit control ID
constexpr auto UD_EDITLIMIT = 100;      // Max user input length +1
constexpr auto UD_TITLELIMIT = 50;      // Max title length
constexpr auto UD_STATICLIMIT = 200;    //Max length static text
constexpr auto UD_BTNWIDTH = 50;        //Button width
constexpr auto UD_BTNHEIGHT = 14;       //Button height
constexpr auto UD_INTERSPC = 7;         //Space between controls
constexpr auto UD_EDTHEIGHT = 12;       //Edit control height
constexpr auto UD_TXTHEIGHT = 28;       //Static text height

class UserDialog
{
public:
    //Getters and setters
    LPCWSTR GetTitle() const { return wsTitle.c_str(); }
    LPCWSTR GetStatic() const { return wsStatic.c_str(); }
    LPCWSTR GetEdit() const { return wsEdit.c_str(); }
    bool SetTitle(const wstring newTitle)
    {
        bool ret = true;
        if (newTitle.size() <= UD_TITLELIMIT)
            wsTitle = newTitle;
        else
            ret = false;
        return ret;
    }
    bool SetStatic(const wstring newStatic)
    {
        bool ret = true;
        if (newStatic.size() <= UD_STATICLIMIT)
            wsStatic = newStatic;
        else
            ret = false;
        return ret;
    }
    bool SetEdit(const wstring newEdit)
    {
        bool ret = true;
        if (newEdit.size() <= UD_EDITLIMIT)
            wsEdit = newEdit;
        else
            ret = false;
        return ret;
    }
    void SetButton(WORD button)
    {
        wButtons = button;
    }
    void ResetButton(WORD button)
    {
        wButtons = UD_OKONLY;
    }
    WORD Show(MsgType msgType, HWND hWnd);
    
private:
    wstring wsTitle = L"User Dialog Box";
    wstring wsStatic = L"Dialog static text";
    wstring wsEdit;
    WORD wButtons = UD_OKONLY;

    //Member function declarations
    static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam);
    bool BuildDlgTemplate(MsgType msgType, DialogTemplate* dlgTemplate);

};