#define PHNT_NO_INLINE_INIT_STRING

#include <phnt_windows.h>
#include <phnt.h>

typedef int (*PSPRINTF)(char* buffer, const char* format, ...);
PSPRINTF __sprintf = NULL;

void InitNtFunctions()
{
	HMODULE NtDllHandle;
	UNICODE_STRING NtDllName;
	STRING FunctionName;

	RtlInitUnicodeString(&NtDllName, L"ntdll.dll");
	LdrGetDllHandle(NULL, NULL, &NtDllName, &NtDllHandle);
	RtlInitString(&FunctionName, "sprintf");
	LdrGetProcedureAddress(NtDllHandle, &FunctionName, 0, (PVOID*)&__sprintf);
}

void TestASM()
{
	STRING String;
	RtlInitAnsiString(&String, "425-443-8700");}

void WaitForEnterKeyPress()
{
	WCHAR Buffer[2];
	DWORD cchRead = 0;

	while (TRUE)
	{
		ReadConsole(GetStdHandle(STD_INPUT_HANDLE), Buffer, 2, &cchRead, NULL);

		if (cchRead == 2 && Buffer[0] == L'\r' && Buffer[1] == L'\n')
			return;
	} 
}

void DebugPrint(char* DebugString)
{
	EXCEPTION_RECORD ExceptionRecord;

	ExceptionRecord.ExceptionAddress = NULL;
	ExceptionRecord.ExceptionCode = DBG_PRINTEXCEPTION_C;
	ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
	ExceptionRecord.NumberParameters = 2;
	ExceptionRecord.ExceptionRecord = NULL;
	ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR) strlen(DebugString) + 1;
	ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR) DebugString;

	RtlRaiseException(&ExceptionRecord);
}

void DebugPrintErrorMessage(ULONG ErrorCode)
{
	if (ErrorCode == ERROR_SUCCESS)
	{
		DebugPrint("Status: ERROR_SUCCESS\n");
	}
	else
	{
		char Buffer[256];
		HMODULE KernelBaseDllHandle;
		UNICODE_STRING KernelBaseDllName;
		MESSAGE_RESOURCE_ENTRY* ResourceEntry;
		NTSTATUS FindMessageStatus;

		RtlInitUnicodeString(&KernelBaseDllName, L"KernelBase.dll");
		LdrGetDllHandle(NULL, NULL, &KernelBaseDllName, &KernelBaseDllHandle);

		FindMessageStatus = RtlFindMessage(KernelBaseDllHandle, (ULONG_PTR) RT_MESSAGETABLE, 0, ErrorCode, &ResourceEntry);

		if (FindMessageStatus == STATUS_SUCCESS && ResourceEntry->Flags & MESSAGE_RESOURCE_UNICODE)
		{
			ANSI_STRING Message = { 0 };
			UNICODE_STRING UnicodeMessage;

			RtlInitUnicodeString(&UnicodeMessage, (LPCWSTR) ResourceEntry->Text);
			RtlUnicodeStringToAnsiString(&Message, &UnicodeMessage, TRUE);
			__sprintf(Buffer, "Status:  0x%x\nMessage: %s", ErrorCode, Message.Buffer);
			RtlFreeAnsiString(&Message);
		}
		else
		{
			__sprintf(Buffer, "Status:  0x%x\nMessage: %s", ErrorCode, "MESSAGE_RESOURCE_ENTRY Not Found");
		}

		DebugPrint(Buffer);
	}
}

#define BUFFER_LEN 256

int main()
{
	//InitNtFunctions();
	TestASM();

	LPWSTR Buffer = NULL;
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BUFFER_LEN,
		L"LaunchNativeFileMapping");

	if (!hMapFile)
	{
		DebugPrintErrorMessage(GetLastError());
	}
	else
	{
		Buffer = MapViewOfFile(
			hMapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			BUFFER_LEN);
	
		ZeroMemory(Buffer, BUFFER_LEN);
		CopyMemory(Buffer, L"Bruce Kobin", (10 + 1) * 2);
		
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), Buffer, 11, NULL, NULL);
		WaitForEnterKeyPress();
		
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), Buffer, 11, NULL, NULL);
		WaitForEnterKeyPress();

		UnmapViewOfFile(Buffer);

		CloseHandle(hMapFile);
	}

	return 0;
}