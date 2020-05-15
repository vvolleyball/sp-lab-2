#include <iostream>
#include "locale.h" 
#include <stdio.h>
#include "TCHAR.h"
#include "windows.h"
#include <shlwapi.h>
#include <wchar.h>

bool PrintDirectory(TCHAR tCurrentDirectory[]);
bool ChangeDirectory(TCHAR* tCurrentDirectory, TCHAR* tNewPath);
bool CopySomeFile(TCHAR* tExistFilePath, TCHAR* tNewFilePath);
bool CreateSomeDirectory(TCHAR* tNewPath);
bool DeleteEmptyDirectoryOrFile(TCHAR* tPathToDeleting);
bool PrintInfoAboutFile(TCHAR* tFilePath);
void ShowError();
void FixPath(TCHAR tNewPath[]);