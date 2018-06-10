// original code : C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\winui\shell\appplatform\commonfiledialog\Backup

//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

// Indices of file types
#define INDEX_WORDDOC 1
#define INDEX_WEBPAGE 2
#define INDEX_TEXTDOC 3

// Controls
#define CONTROL_GROUP           2000
#define CONTROL_RADIOBUTTONLIST 2
#define CONTROL_RADIOBUTTON1    1
#define CONTROL_RADIOBUTTON2    2       // It is OK for this to have the same ID as CONTROL_RADIOBUTTONLIST,
// because it is a child control under CONTROL_RADIOBUTTONLIST

// IDs for the Task Dialog Buttons
#define IDC_BASICFILEOPEN                       100
#define IDC_ADDITEMSTOCUSTOMPLACES              101
#define IDC_ADDCUSTOMCONTROLS                   102
#define IDC_SETDEFAULTVALUESFORPROPERTIES       103
#define IDC_WRITEPROPERTIESUSINGHANDLERS        104
#define IDC_WRITEPROPERTIESWITHOUTUSINGHANDLERS 105

namespace imajuscule {
    using namespace StorageStuff;

/* File Dialog Event Handler *****************************************************************************************************/

class CDialogEventHandler : public IFileDialogEvents,
    public IFileDialogControlEvents
{
public:
    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDialogEventHandler, IFileDialogEvents),
            QITABENT(CDialogEventHandler, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IFileDialogEvents methods
    IFACEMETHODIMP OnFileOk(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd);
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

    // IFileDialogControlEvents methods
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem);
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

    CDialogEventHandler() : _cRef(1) { };
private:
    ~CDialogEventHandler() { };
    long _cRef;
};

// IFileDialogEvents methods
// This method gets called when the file-type is changed (combo-box selection changes).
// For sample sake, let's react to this event by changing the properties show.
HRESULT CDialogEventHandler::OnTypeChange(IFileDialog *pfd)
{
    IFileSaveDialog *pfsd;
    HRESULT hr = pfd->QueryInterface(&pfsd);
    if (SUCCEEDED(hr))
    {
        UINT uIndex;
        hr = pfsd->GetFileTypeIndex(&uIndex);   // index of current file-type
        if (SUCCEEDED(hr))
        {
            IPropertyDescriptionList *pdl = NULL;

            switch (uIndex)
            {
            case INDEX_WORDDOC:
                // When .doc is selected, let's ask for some arbitrary property, say Title.
                hr = PSGetPropertyDescriptionListFromString(L"prop:System.Title", IID_PPV_ARGS(&pdl));
                if (SUCCEEDED(hr))
                {
                    // FALSE as second param == do not show default properties.
                    hr = pfsd->SetCollectedProperties(pdl, FALSE);
                    pdl->Release();
                }
                break;

            case INDEX_WEBPAGE:
                // When .html is selected, let's ask for some other arbitrary property, say Keywords.
                hr = PSGetPropertyDescriptionListFromString(L"prop:System.Keywords", IID_PPV_ARGS(&pdl));
                if (SUCCEEDED(hr))
                {
                    // FALSE as second param == do not show default properties.
                    hr = pfsd->SetCollectedProperties(pdl, FALSE);
                    pdl->Release();
                }
                break;

            case INDEX_TEXTDOC:
                // When .txt is selected, let's ask for some other arbitrary property, say Author.
                hr = PSGetPropertyDescriptionListFromString(L"prop:System.Author", IID_PPV_ARGS(&pdl));
                if (SUCCEEDED(hr))
                {
                    // TRUE as second param == show default properties as well, but show Author property first in list.
                    hr = pfsd->SetCollectedProperties(pdl, TRUE);
                    pdl->Release();
                }
                break;
            }
        }
        pfsd->Release();
    }
    return hr;
}

// IFileDialogControlEvents
// This method gets called when an dialog control item selection happens (radio-button selection. etc).
// For sample sake, let's react to this event by changing the dialog title.
HRESULT CDialogEventHandler::OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem)
{
    IFileDialog *pfd = NULL;
    HRESULT hr = pfdc->QueryInterface(&pfd);
    if (SUCCEEDED(hr))
    {
        if (dwIDCtl == CONTROL_RADIOBUTTONLIST)
        {
            switch (dwIDItem)
            {
            case CONTROL_RADIOBUTTON1:
                hr = pfd->SetTitle(L"Longhorn Dialog");
                break;

            case CONTROL_RADIOBUTTON2:
                hr = pfd->SetTitle(L"Vista Dialog");
                break;
            }
        }
        pfd->Release();
    }
    return hr;
}

// Instance creation helper
HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    CDialogEventHandler *pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
    HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pDialogEventHandler->QueryInterface(riid, ppv);
        pDialogEventHandler->Release();
    }
    return hr;
}

/* Utility Functions *************************************************************************************************************/
/*
// A helper function that converts UNICODE data to ANSI and writes it to the given file.
// We write in ANSI format to make it easier to open the output file in Notepad.
HRESULT _WriteDataToFile(HANDLE hFile, PCWSTR pszDataIn)
{
    // First figure out our required buffer size.
    DWORD cbData = WideCharToMultiByte(CP_ACP, 0, pszDataIn, -1, NULL, 0, NULL, NULL);
    HRESULT hr = (cbData == 0) ? HRESULT_FROM_WIN32(GetLastError()) : S_OK;
    if (SUCCEEDED(hr))
    {
        // Now allocate a buffer of the required size, and call WideCharToMultiByte again to do the actual conversion.
        char *pszData = new (std::nothrow) CHAR[cbData];
        hr = pszData ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = WideCharToMultiByte(CP_ACP, 0, pszDataIn, -1, pszData, cbData, NULL, NULL)
                ? S_OK
                : HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
            {
                DWORD dwBytesWritten = 0;
                hr = WriteFile(hFile, pszData, cbData - 1, &dwBytesWritten, NULL)
                    ? S_OK
                    : HRESULT_FROM_WIN32(GetLastError());
            }
            delete[] pszData;
        }
    }
    return hr;
}

/* Common File Dialog Snippets ***************************************************************************************************/


bool Open(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, std::string & sPath)
{
    bool bRet = false;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if_A(SUCCEEDED(hr))
    {
        // CoCreate the File Open Dialog object.
        IFileDialog *pfd = NULL;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if_A(SUCCEEDED(hr))
        {
            // Create an event handling object, and hook it up to the dialog.
            IFileDialogEvents *pfde = NULL;
            hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
            if_A(SUCCEEDED(hr))
            {
                // Hook up the event handler.
                DWORD dwCookie;
                hr = pfd->Advise(pfde, &dwCookie);
                if_A(SUCCEEDED(hr))
                {
                    // Set the options on the dialog.
                    DWORD dwFlags;

                    // Before setting, always get the options first in order not to override existing options.
                    hr = pfd->GetOptions(&dwFlags);
                    if_A(SUCCEEDED(hr))
                    {
                        bool bFilter = true;

                        FILEOPENDIALOGOPTIONS  opts = dwFlags | FOS_FORCEFILESYSTEM;

                        if (k == FileSystemOperation::Kind::OP_DIR)
                        {
                            opts |= FOS_PICKFOLDERS;
                            bFilter = false;
                        }

                        hr = pfd->SetOptions(opts);
                        if_A(SUCCEEDED(hr))
                        {
                            std::vector<std::wstring *> allocated;
                            COMDLG_FILTERSPEC * rgSpec = NULL;
                            if (bFilter)
                            {
                                size_t nElems;
                                if (extensions.empty())
                                {
                                    nElems = 1;
                                    rgSpec = new COMDLG_FILTERSPEC[nElems];

                                    rgSpec[0].pszName = L"Any";
                                    rgSpec[0].pszSpec = L"*.*";
                                }
                                else
                                {
                                    nElems = extensions.size();
                                    rgSpec = new COMDLG_FILTERSPEC[nElems];
                                    int i = 0;
                                    for (auto & ext : extensions)
                                    {
                                        std::wstring * WfileExt = new std::wstring;

                                        WfileExt->append(L"*.");
                                        WfileExt->append(ext.begin(), ext.end());

                                        allocated.push_back(WfileExt);

                                        rgSpec[i].pszName = L"";
                                        rgSpec[i].pszSpec = WfileExt->c_str();
                                    }
                                }

                                // Set the file types to display only. Notice that, this is a 1-based array.
                                hr = pfd->SetFileTypes(nElems, rgSpec);
                            }

                            if_A(SUCCEEDED(hr))
                            {
                                // Show the dialog
                                hr = pfd->Show(NULL);
                                if(SUCCEEDED(hr))
                                {
                                    // Obtain the result, once the user clicks the 'Open' button.
                                    // The result is an IShellItem object.
                                    IShellItem *psiResult;
                                    hr = pfd->GetResult(&psiResult);
                                    if_A(SUCCEEDED(hr))
                                    {
                                        // We are just going to print out the name of the file for sample sake.
                                        PWSTR pszFilePath = NULL;
                                        hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                        if_A(SUCCEEDED(hr))
                                        {
                                            string_cast(pszFilePath, CP_ACP, sPath);

                                            bRet = true;
                                            CoTaskMemFree(pszFilePath);
                                        }
                                        psiResult->Release();
                                    }
                                }
                            }

                            if (rgSpec)
                                delete[] rgSpec;
                            for (auto a : allocated)
                            {
                                delete a;
                            }
                        }
                    }
                    // Unhook the event handler.
                    pfd->Unadvise(dwCookie);
                }
                pfde->Release();
            }
            pfd->Release();
        }

        CoUninitialize();
    }
    return bRet;

}

bool BasicFileOpen(std::string & sFilePath, std::string & fileName, const std::string & fileExt)
{
    bool bRet = false;

    // CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Create an event handling object, and hook it up to the dialog.
        IFileDialogEvents *pfde = NULL;
        hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
        if (SUCCEEDED(hr))
        {
            // Hook up the event handler.
            DWORD dwCookie;
            hr = pfd->Advise(pfde, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Set the options on the dialog.
                DWORD dwFlags;

                // Before setting, always get the options first in order not to override existing options.
                hr = pfd->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // In this case, get shell items only for file system items.
                    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                    if (SUCCEEDED(hr))
                    {
                        std::wstring WfileExt = std::wstring(fileExt.begin(), fileExt.end());

                        std::wstring W1(L"*.");
                        W1.append(WfileExt);

                        std::wstring W2(WfileExt);
                        W2.append(L" File (*.");
                        W2.append(WfileExt);
                        W2.append(L")");

                        const COMDLG_FILTERSPEC c_rgSaveTypes2[] =
                        {
                            { W2.c_str(), W1.c_str() }
                        };

                        // Set the file types to display only. Notice that, this is a 1-based array.
                        hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes2), c_rgSaveTypes2);
                        if (SUCCEEDED(hr))
                        {
                            // Show the dialog
                            hr = pfd->Show(NULL);
                            if (SUCCEEDED(hr))
                            {
                                // Obtain the result, once the user clicks the 'Open' button.
                                // The result is an IShellItem object.
                                IShellItem *psiResult;
                                hr = pfd->GetResult(&psiResult);
                                if (SUCCEEDED(hr))
                                {
                                    // We are just going to print out the name of the file for sample sake.
                                    PWSTR pszFilePath = NULL;
                                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                    if (SUCCEEDED(hr))
                                    {
                                        string_cast(pszFilePath, CP_ACP, sFilePath);
                                        // We are just going to print out the name of the file for sample sake.
                                        PWSTR pszFilePath2 = NULL;
                                        hr = psiResult->GetDisplayName(SIGDN_PARENTRELATIVE, &pszFilePath2);
                                        if (SUCCEEDED(hr))
                                        {
                                            string_cast(pszFilePath2, CP_ACP, fileName);

                                            bRet = true;
                                            CoTaskMemFree(pszFilePath2);
                                        }
                                        CoTaskMemFree(pszFilePath);
                                    }
                                    psiResult->Release();
                                }
                            }
                        }
                    }
                }
                // Unhook the event handler.
                pfd->Unadvise(dwCookie);
            }
            pfde->Release();
        }
        pfd->Release();
    }
    return bRet;
}

    FileSystemOperation::Nature fFileSystemOperation(FileSystemOperation::Kind k, const std::vector<std::string> & extensions, const std::string & title, std::function<void(OperationResult, const std::string &)> f, std::function<void(void)> fEnd)
    {
        std::string path;
        if (Open(k, extensions, path))
            f(OperationResult::SUCCESS, path);
        else
            f(OperationResult::CANCELED, std::string());
        fEnd();

        return FileSystemOperation::Nature::BLOCKING;
    }
}
