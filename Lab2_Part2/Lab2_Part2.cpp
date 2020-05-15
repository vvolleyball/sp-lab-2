#include "pch.h"

using namespace std;

int main()
{
	setlocale(LC_ALL, "RUS");
	TCHAR tCurrentPath[MAX_PATH]{'\0'};
	GetCurrentDirectory(MAX_PATH, tCurrentPath);
	TCHAR tCommand[MAX_PATH]{ '\0' };
	TCHAR* pch;
	TCHAR tCmd[MAX_PATH]{ '\0' };
	TCHAR tFirstPath[MAX_PATH]{ '\0' };
	TCHAR tSecondPath[MAX_PATH]{ '\0' };

	for (;;) {
		cout << "\n\t[Файловый менеджер]" << endl;
		cout << "Введите команду для взаимодействия с файловым менеджером: " << endl;
		fgetws(tCommand, MAX_PATH, stdin); // get all command string
		pch = _wcstok(tCommand, L" ");
		int i = 0;
		while (pch != NULL) {
			if(i == 0)_tcscpy(tCmd, pch); // get command
			if (i == 1)_tcscpy(tFirstPath, pch); // get first path
			if (i == 2)_tcscpy(tSecondPath, pch); // get second path
			pch = _wcstok(NULL, L" ");
			i++;
		}
		FixPath(tCmd);
		if (_tcscmp(L"changedirectory", tCmd) == 0) // command example: changedirectory [directoryPath]
		{
			if (!ChangeDirectory(tCurrentPath, tFirstPath))
				ShowError();
		}
		else if (_tcscmp(L"printdirectory", tCmd) == 0)  // command example: printdirectory 
		{
			if (!PrintDirectory(tCurrentPath))
				ShowError();
		}
		else if (_tcscmp(L"copyfile", tCmd) == 0)  // command example: copyfile [file] [newPathForFile]
		{
			if (!CopySomeFile(tFirstPath, tSecondPath))
				ShowError();
		}
		else if (_tcscmp(L"createdirectory", tCmd) == 0) // command example: createdirectory [directoryPath]
		{
			if (!CreateSomeDirectory(tFirstPath))
				ShowError();
		}
		else if (_tcscmp(L"delete", tCmd) == 0) // command example: delete [directoryPath/filePath]
		{
			if (!DeleteEmptyDirectoryOrFile(tFirstPath))
				ShowError();
		}
		else if (_tcscmp(L"printinfo", tCmd) == 0)  // command example: delete [file]
		{
			if (!PrintInfoAboutFile(tFirstPath))
				ShowError();
		}
		else {
			cout << "Нету такой команды" << endl;
			continue;
		}
	}
}

bool ChangeDirectory(TCHAR* tCurrentDirectory, TCHAR* tNewPath)  // function change current directory
{
	cout << "\nСтарый путь: ";
	wcout << tCurrentDirectory << endl;
	FixPath(tNewPath);
	DWORD dw = GetFileAttributes(tNewPath);
	if (dw != FILE_ATTRIBUTE_DIRECTORY)
	{
		cout << "Вы ввели путь не к директории!" << endl;
		return false;
	}
	wcscpy(tCurrentDirectory, tNewPath);
	cout << "Новый путь: ";
	wcout << tCurrentDirectory << endl;
	return true;
}

bool PrintDirectory(TCHAR tCurrentDirectory[]) // function print all data(files) from current directory
{
	TCHAR tBuff[MAX_PATH];
	WIN32_FIND_DATAW id;
	SYSTEMTIME st;
	_sntprintf(tBuff, MAX_PATH, L"%s\\*\0", tCurrentDirectory);
	HANDLE hFiles = FindFirstFile(tBuff, &id);
	if (hFiles != INVALID_HANDLE_VALUE)
	{
		wcout << "\n" << tCurrentDirectory << endl;
		do
		{
			if ((wcscmp(L".", id.cFileName) == 0) || (wcscmp(L"..", id.cFileName) == 0))
				continue;
			wcout << " - " << id.cFileName ;
			cout << "  (File size: " << ((id.nFileSizeHigh * MAXDWORD) + id.nFileSizeLow) / 1024 << " KB  ";
			if (FileTimeToSystemTime(&id.ftCreationTime, &st))
				wprintf_s(L"Creation time: %d-%d-%d, %d:%d)\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
		} while (FindNextFile(hFiles, &id));
		FindClose(hFiles);
		return true;
	}
	return false;
}

bool CopySomeFile(TCHAR* tExistFilePath, TCHAR* tNewFilePath) // copy file
{
	FixPath(tExistFilePath);
	DWORD dw = GetFileAttributes(tExistFilePath);
	if (dw == INVALID_FILE_ATTRIBUTES)
	{
		cout << "Данного файла не существует" << endl;
		return false;
	}
	FixPath(tNewFilePath);
	if (!CopyFile(tExistFilePath, tNewFilePath, TRUE))
	{
		ShowError();
		return false;
	}
	return true;
}

bool CreateSomeDirectory(TCHAR* tNewPath) // function create directory
{
	FixPath(tNewPath);
	if (!CreateDirectory(tNewPath, NULL))
		return false;
	return true;
}

bool DeleteEmptyDirectoryOrFile(TCHAR* tPathToDeleting) // function delete empty directory or file
{
	FixPath(tPathToDeleting);
	if (!RemoveDirectory(tPathToDeleting))
	{
		if (!DeleteFile(tPathToDeleting))
			return false;
	}
	return true;
}

bool PrintInfoAboutFile(TCHAR* tFilePath) // print all info about file
{
	WIN32_FIND_DATA id;
	SYSTEMTIME st;
	FixPath(tFilePath);
	if (!FindFirstFile(tFilePath, &id))
		return false;
	cout << "\n\t Информация о файле: "; wcout << id.cFileName << endl;
	cout << "Альтернативное наименование файла: "; wcout << id.cAlternateFileName << endl;
	cout << "Размер файла: " << ((id.nFileSizeHigh * MAXDWORD) + id.nFileSizeLow) / 1024 << " KB" << endl;
	if (FileTimeToSystemTime(&id.ftCreationTime, &st))
		wprintf_s(L"Дата создания: %d-%d-%d, %d:%d\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
	if (FileTimeToSystemTime(&id.ftLastAccessTime, &st))
		wprintf_s(L"Дата последнего доступа: %d-%d-%d, %d:%d\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
	if (FileTimeToSystemTime(&id.ftLastWriteTime, &st))
		wprintf_s(L"Дата последней записи: %d-%d-%d, %d:%d\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
	cout << "Атрибуты файла: " << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE)
		cout << " - Archive" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_COMPRESSED)
		cout << " - Compressed" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_HIDDEN)
		cout << " - Hidden" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
		cout << " - Normal" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_OFFLINE)
		cout << " - Unenable file data" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_READONLY)
		cout << " - Readonly";
	if (id.dwFileAttributes == FILE_ATTRIBUTE_SYSTEM)
		cout << " - System" << endl;
	if (id.dwFileAttributes == FILE_ATTRIBUTE_TEMPORARY)
		cout << " - Temporary" << endl;
	return true;
}

void FixPath(TCHAR tNewPath[]) // function for fix path of file or directory
{
	if (wcschr(tNewPath, '\n'))
		tNewPath[(wcschr(tNewPath, '\n') - tNewPath)] = '\0'; // change last \n on \0
}

void ShowError()
{
	LPVOID lpMsgBuf;
	DWORD dwLastError = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwLastError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("\n%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
	return;
}