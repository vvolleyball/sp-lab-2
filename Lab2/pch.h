#include <iostream>
#include "locale.h" 
#include "windows.h"
#include <stdio.h>

#define TEXTFIELD_SIZE 80

CONST WCHAR RECORDFILE[] = L"RecordBase.rb";

// struct for records in file
typedef struct
{
	WORD wRecordId; // record id(number)
	FILETIME fltmCreationTime; // record creation time 
	CHAR cText[TEXTFIELD_SIZE + 1]; // record text
	WORD wChangeRecordCounter; // record changing count
} USERRECORD, * P_USERRECORD;

// struct for header in file
typedef struct
{
	WORD wNotNullRecordsCount; // count of not null records in file
	WORD wRecordFileSize; // full size of file
} RECORDFILE_HEADER, * P_RECORDFILE_HEADER;

bool InitializeProgrammData(HANDLE* hRecordFile, DWORD* dwFileSize, DWORD* dwCountRecords);
bool WriteOrGetHeader(HANDLE hRecordFile, DWORD dwCreateOrGetHeader, P_RECORDFILE_HEADER pHeader, BOOL bClearFile);
bool CreateRecordAndWriteInFile(HANDLE hRecordsFile, DWORD* dwFileSize, CHAR* recText, DWORD* dwRecordId);
bool WriteRecordToFile(HANDLE hRecordFile, USERRECORD usRecord, DWORD dwOffset);
bool DeleteRecord(HANDLE hRecordFile, DWORD dwIdToDelete, DWORD* dwCountRecords, DWORD* dwFileSize);
bool PrintRecordsFile(HANDLE hRecordFile, DWORD dwCountRecords);
bool ModifyRecord(HANDLE hRecordFile, DWORD  dwIdToModify);