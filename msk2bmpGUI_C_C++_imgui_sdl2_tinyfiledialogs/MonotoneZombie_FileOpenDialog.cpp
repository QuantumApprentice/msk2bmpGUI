#include "MonotoneZombie_FileOpenDialog.h"
#include <Windows.h>

FileDir SelectFile()
{
	IFileDialog* pFD = nullptr;
	SUCCEEDED(CoInitialize(nullptr));
	auto HRES = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFD));

	if (SUCCEEDED(HRES))
	{
		DWORD Flags;
		HRES = pFD->GetOptions(&Flags);

		if (SUCCEEDED(HRES))
		{
			pFD->SetOptions(FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
		}
		else return FileDir();

		FINALLY{
			if (pFD)
				pFD->Release();
		}FINALLYOVER;

		IShellItem* pItem = nullptr;

		WCHAR CurrentDirectory[256];
		GetCurrentDirectoryW(256, CurrentDirectory);

		HRES = pFD->GetFolder(&pItem);
		if (SUCCEEDED(HRES))
		{
			HRES = SHCreateItemFromParsingName(CurrentDirectory, nullptr, IID_PPV_ARGS(&pItem));
			pFD->SetDefaultFolder(pItem);
		}

		FINALLY{
			if (pItem)
				pItem->Release();
		}FINALLYOVER;

		HRES = pFD->Show(nullptr);
		if (SUCCEEDED(HRES))
		{
			IShellItem* pItem = nullptr;
			HRES = pFD->GetResult(&pItem);
			if (SUCCEEDED(HRES))
			{
				FINALLY{
				if (pItem)
					pItem->Release();
				}FINALLYOVER;

				PWSTR FilePath;
				HRES = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);
				if (SUCCEEDED(HRES))
				{
					size_t length = wcslen(FilePath);
					FileDir	Dir;
					BOOL DefaultCharUsed = 0;
					CCHAR	DChar = ' ';
					auto res = WideCharToMultiByte(CP_UTF8, 0, FilePath, length, Dir.str, 256, nullptr, nullptr);
					if (!res)
					{
						//IErrorInfo*	INFO = nullptr;
						auto Err = GetLastError();
						std::cout << Err;
					}
					else
					{
						Dir.str[length] = '\0';
						Dir.Valid = true;
						CoTaskMemFree(FilePath);
						return Dir;
					}
				}
			}
		}
	}
	return FileDir();
}