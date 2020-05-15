#include "pch.h"

using namespace std;

int main()
{
	DWORD dwCountRecords = 0, dwFileSize = 0;
	HANDLE hRecordFile = NULL;
	BOOL bDeleteFlag = FALSE;
	if (!InitializeProgrammData(&hRecordFile, &dwFileSize, &dwCountRecords))
	{
		cout << "Не может инициализировать файл для записи!" << endl;
		return 1;
	}

	setlocale(LC_ALL, "RUS");
	CHAR cKey;
	for (;;) {
		cout << "\n\t [MENU]" << endl;
		cout << " 1 - создать запись" << endl;
		cout << " 2 - модифицировать запись" << endl;
		cout << " 3 - удалить запись" << endl;
		cout << " 4 - вывести заголовок" << endl;
		cout << " 5 - закончить работу" << endl;
		cout << "Выберите операцию: ";
		cin >> cKey;
		switch (cKey)
		{
			case '1': // create record
			{
				CHAR Buff[TEXTFIELD_SIZE] = { 0 };
				cout << "\nВведите данные(не больше 80 символов) для записи или введите '0' для заполнения записи нулями:\n";
				cin >> Buff;
				if (sizeof(Buff) <= TEXTFIELD_SIZE) {
					if (!CreateRecordAndWriteInFile(hRecordFile, &dwFileSize, Buff, &dwCountRecords))
						cout << "\nОШИБКА! Не может добавить новую запись!";
				}
				else
					cout << "\nОШИБКА! Вы ввели слишком много данных!";
			} break;

			case '2': // modify record
			{
				DWORD dwIdToModify;
				cout << "\nВведите id записи, которую нужно изменить: ";
				cin >> dwIdToModify;
				if (dwIdToModify >= dwCountRecords)
				{
					cout << "\nОШИБКА! Такого id не существует!";
					continue;
				}
				else
				{
					if (!ModifyRecord(hRecordFile, dwIdToModify))
					{
						cout << "\nОШИБКА! Не удается изменить запись!";
						continue;
					}
				}
			} break;

			case '3': // delete record
			{
				DWORD dwIdToDelete;
				cout << "\nВведите id записи, которую нужно удалить: ";
				cin >> dwIdToDelete;
				if (dwIdToDelete >= dwCountRecords)
				{
					cout << "\nОШИБКА! Такого id не существует!";
					continue;
				}
				else
				{
					if (!DeleteRecord(hRecordFile, dwIdToDelete, &dwCountRecords, &dwFileSize))
						cout << "\nОШИБКА! Не удается удалить запись!";
					continue;
				}
			} break;

			case '4': // print header and all records
			{
				if (!PrintRecordsFile(hRecordFile, dwCountRecords))
				{
					cout << "\nОШИБКА! Не может вывести заголоаок!";
					continue;
				}
			} break;

			case '5': // exit
			{
				CloseHandle(hRecordFile);
				return 0;
			} break;

			default: {
				cout << "\nВы ввели неверный ключ!";
			} continue;
		}
	}
}

bool WriteOrGetHeader(HANDLE hRecordFile, DWORD dwCreateOrGetHeader, P_RECORDFILE_HEADER pHeader, BOOL bClearFile)
{
	DWORD dwBytes = 0;
	if (SetFilePointer(hRecordFile, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	if (dwCreateOrGetHeader == 1) // create header
	{
		if ((WriteFile(hRecordFile, pHeader, sizeof(RECORDFILE_HEADER), &dwBytes, NULL) == false) || 
			(dwBytes != sizeof(RECORDFILE_HEADER)))
			return false;

		if (bClearFile)
		{
			if (SetEndOfFile(hRecordFile) == false)
				return false;
		}
	}
	else if (dwCreateOrGetHeader == 2) // read header
	{
		if ((ReadFile(hRecordFile, pHeader, sizeof(RECORDFILE_HEADER), &dwBytes, NULL) == false) || 
			(dwBytes != sizeof(RECORDFILE_HEADER)))
			return false;
	}
	else
		return false;

	return true;
}

bool InitializeProgrammData(HANDLE* hRecordFile, DWORD* dwFileSize, DWORD* dwCountRecords)
{
	// open file
	*hRecordFile = CreateFile(RECORDFILE,
		GENERIC_ALL,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (*hRecordFile == INVALID_HANDLE_VALUE)
		return false;

	// get file size
	*dwFileSize = GetFileSize(*hRecordFile, NULL);
	if (*dwFileSize == INVALID_FILE_SIZE)
		return false;

	if (*dwFileSize <= sizeof(RECORDFILE_HEADER))
	{
		RECORDFILE_HEADER header;
		header.wNotNullRecordsCount = 0;
		header.wRecordFileSize = sizeof(RECORDFILE_HEADER);
		if (!WriteOrGetHeader(*hRecordFile, 1, &header, TRUE)) // create header
		{
			return false;
		}
		*dwFileSize = sizeof(RECORDFILE_HEADER);
	}

	// get count records in file
	*dwCountRecords = (*dwFileSize - sizeof(RECORDFILE_HEADER)) / sizeof(USERRECORD);
	return true;
}

bool CreateRecordAndWriteInFile(HANDLE hRecordsFile, DWORD* dwFileSize, CHAR* recText, DWORD* dwRecordId)
{
	// init record to write
	BOOL isNullRecord = false;
	USERRECORD usRecord;
	if (strcmp(recText, "0") == 0)
	{
		memset(usRecord.cText, '\0', sizeof(usRecord.cText));
		isNullRecord = true;
	}
	else
	{
		strcpy_s(usRecord.cText, TEXTFIELD_SIZE, recText);
		usRecord.cText[TEXTFIELD_SIZE] = '\0';
		isNullRecord = false;
	}
	usRecord.wChangeRecordCounter = 0;
	usRecord.wRecordId = *dwRecordId;

	SYSTEMTIME st = { 0 };
	FILETIME ft = { 0 };
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	usRecord.fltmCreationTime = ft;

	// write record and update header
	if (!WriteRecordToFile(hRecordsFile, usRecord, sizeof(RECORDFILE_HEADER) + (*dwRecordId) * sizeof(USERRECORD)))
		return false;

	// read header
	RECORDFILE_HEADER header = { 0 };
	WriteOrGetHeader(hRecordsFile, 2, &header, FALSE);
	header.wRecordFileSize += sizeof(USERRECORD);
	header.wNotNullRecordsCount = isNullRecord ? header.wNotNullRecordsCount : ++header.wNotNullRecordsCount;
	WriteOrGetHeader(hRecordsFile, 1, &header, FALSE);

	// update programm variables about file with records
	(*dwRecordId)++;
	(*dwFileSize) += sizeof(USERRECORD);

	return true;
}

bool WriteRecordToFile(HANDLE hRecordFile, USERRECORD usRecord, DWORD dwOffset)
{
	if (SetFilePointer(hRecordFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	DWORD dwCountWritenBytes = 0;
	if ((WriteFile(hRecordFile, &usRecord, sizeof(usRecord), &dwCountWritenBytes, NULL) == false) || (dwCountWritenBytes != sizeof(usRecord)))
		return false;

	return true;
}

bool DeleteRecord(HANDLE hRecordFile, DWORD dwIdToDelete, DWORD* dwCountRecords, DWORD* dwFileSize)
{
	RECORDFILE_HEADER header;
	DWORD dwLastId = *dwCountRecords - 1;
	CHAR cBuff[TEXTFIELD_SIZE + 1] = { 0 };

	// read header
	if (!WriteOrGetHeader(hRecordFile, 2, &header, FALSE))
		return false;

	// read last record
	USERRECORD urBuff = { 0 };
	DWORD dwBytes = 0;

	if (SetFilePointer(hRecordFile, sizeof(RECORDFILE_HEADER) + sizeof(USERRECORD) * dwLastId, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	if ((ReadFile(hRecordFile, &urBuff, sizeof(USERRECORD), &dwBytes, NULL) == false) || (dwBytes != sizeof(USERRECORD)))
		return false;

	urBuff.wRecordId = dwIdToDelete;

	// write last record to deleted record
	if (!WriteRecordToFile(hRecordFile, urBuff, (sizeof(RECORDFILE_HEADER) + sizeof(USERRECORD) * dwIdToDelete)))
		return false;

	// update header and programm variables
	header.wNotNullRecordsCount = urBuff.cText[0] == '\0' ? header.wNotNullRecordsCount : header.wNotNullRecordsCount - 1;
	header.wRecordFileSize = header.wRecordFileSize - sizeof(USERRECORD);
	(*dwCountRecords)--;
	(*dwFileSize) -= sizeof(USERRECORD);

	// write new header to file
	if (!WriteOrGetHeader(hRecordFile, 1, &header, FALSE))
		return false;
	return true;
}

bool PrintRecordsFile(HANDLE hRecordFile, DWORD dwCountRecords)
{
	RECORDFILE_HEADER header = { 0 };
	USERRECORD recordsBuff = { 0 };
	DWORD dwReadedBytes = 0;

	// read header
	if (!WriteOrGetHeader(hRecordFile, 2, &header, FALSE))
		return false;

	// output header data
	cout << "\n********* Заголовок *********" << endl;
	cout << "* Количество ненулевых записей: " << header.wNotNullRecordsCount << endl;
	cout << "* Размер файла: " << header.wRecordFileSize << " Bytes" << endl;

	// output record data
	while ((ReadFile(hRecordFile, &recordsBuff, sizeof(USERRECORD), &dwReadedBytes, NULL) != false) && (dwReadedBytes == sizeof(USERRECORD)))
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&recordsBuff.fltmCreationTime, &st);
		cout << "********* Запись *********" << endl;
		cout << "* ID:" << recordsBuff.wRecordId << endl;
		printf("* Дата создания: %d-%d-%d, %d:%d\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
		cout << "* Количество изменений: " << recordsBuff.wChangeRecordCounter << endl;
		cout << "* Текст: " << recordsBuff.cText << endl;
	}
	return true;
}

bool ModifyRecord(HANDLE hRecordFile, DWORD  dwIdToModify)
{
	RECORDFILE_HEADER header;
	CHAR cBuff[TEXTFIELD_SIZE + 1] = { 0 };

	// read header
	if (!WriteOrGetHeader(hRecordFile, 2, &header, FALSE))
		return false;

	// get new text
	cout << "Введите новые данные(не больше 80 символов) для записи или введите '0' для заполнения записи нулями:\n";
	cin >> cBuff;
	cBuff[TEXTFIELD_SIZE] = '\0';

	// read record for modify
	USERRECORD urBuff = { 0 };
	DWORD dwBytes = 0;
	BOOL isNullRecord = false;

	if (SetFilePointer(hRecordFile, sizeof(RECORDFILE_HEADER) + sizeof(USERRECORD) * dwIdToModify, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return false;

	if ((ReadFile(hRecordFile, &urBuff, sizeof(USERRECORD), &dwBytes, NULL) == false) || (dwBytes != sizeof(USERRECORD)))
		return false;

	isNullRecord = urBuff.cText[0] == '\0' ? true : false;

	// modify record
	if (strcmp(cBuff, "0") == 0)
		memset(urBuff.cText, '\0', sizeof(urBuff.cText));
	else
	{
		strcpy_s(urBuff.cText, TEXTFIELD_SIZE, cBuff);
		urBuff.cText[80] = '\0';
	}
	urBuff.wChangeRecordCounter++;

	// write record to file
	if (!WriteRecordToFile(hRecordFile, urBuff, (sizeof(RECORDFILE_HEADER) + sizeof(USERRECORD) * dwIdToModify)))
		return false;

	// update header
			// was null, will be not null
	if (isNullRecord && (strcmp(cBuff, "0") != 0)) header.wNotNullRecordsCount++;
	// was not null, will be null
	if (!isNullRecord && (strcmp(cBuff, "0") == 0)) header.wNotNullRecordsCount--;

	if (!WriteOrGetHeader(hRecordFile, 1, &header, FALSE))
		return false;

	return true;
}