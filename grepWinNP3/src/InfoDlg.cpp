// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2009, 2011-2014, 2020 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "InfoDlg.h"
#include "StringUtils.h"
#include "DPIAware.h"
#include "maxpath.h"

#include <memory>
#include <mshtmhst.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoDlg::CInfoDlg()
{
}

CInfoDlg::~CInfoDlg()
{
}

//Function which takes input of An HTML Resource Id
BOOL CInfoDlg::ShowDialog(HWND hParent, UINT idAboutHTMLID, HINSTANCE hInstance)
{
    //Load the IE Specific MSTML Interface DKK
    HINSTANCE hinstMSHTML = LoadLibrary(TEXT("mshtml.dll"));
    BOOL bSuccess = FALSE;
    if (hinstMSHTML)
    {
        SHOWHTMLDIALOGEXFN  *pfnShowHTMLDialog;
        //Locate The Function ShowHTMLDialog in the Loaded mshtml.dll
        pfnShowHTMLDialog = (SHOWHTMLDIALOGEXFN *)GetProcAddress(hinstMSHTML, "ShowHTMLDialogEx");
        if (pfnShowHTMLDialog)
        {
            auto lpszModule = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
            //Get The Application Path
            if (GetModuleFileName(hInstance, lpszModule.get(), MAX_PATH_NEW))
            {
                //Add the IE Res protocol
                auto strResourceURL = std::make_unique<wchar_t[]>(MAX_PATH_NEW);
                swprintf_s(strResourceURL.get(), MAX_PATH_NEW, L"res://%s/%u", lpszModule.get(), idAboutHTMLID);
                auto iLength       = wcslen(strResourceURL.get());
                auto lpWideCharStr = std::make_unique<wchar_t[]>(iLength + 1);
                //Attempt to Create the URL Moniker to the specified in the URL String
                IMoniker *pmk;
                if (SUCCEEDED(CreateURLMoniker(NULL, strResourceURL.get(), &pmk)))
                {
                    //Invoke the ShowHTMLDialog function by pointer
                    //passing the HWND of your Application , the Moniker,
                    //the remaining parameters can be set to NULL
                    auto opts = CStringUtils::Format(L"dialogHeight:%dpx; dialogWidth:%dpx; resizable:yes",
                                                     CDPIAware::Instance().Scale(hParent, 600),
                                                     CDPIAware::Instance().Scale(hParent, 480));
                    pfnShowHTMLDialog(NULL, pmk, HTMLDLG_MODELESS, NULL, opts.data(), NULL);
                    bSuccess = TRUE;
                }
            }
        }
        FreeLibrary(hinstMSHTML);
    }
    return bSuccess;
}
