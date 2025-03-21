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

void DebugPrintStatus(NTSTATUS Status)
{
	char Buffer[256];
	HMODULE NtDllHandle;
	UNICODE_STRING NtDllName;
	ULONG DosStatus = RtlNtStatusToDosError(Status);
	MESSAGE_RESOURCE_ENTRY* ResourceEntry;
	NTSTATUS FindMessageStatus;

	RtlInitUnicodeString(&NtDllName, L"KernelBase.dll");
	LdrGetDllHandle(NULL, NULL, &NtDllName, &NtDllHandle);
	
	FindMessageStatus = RtlFindMessage(NtDllHandle, (ULONG_PTR) RT_MESSAGETABLE, 0, DosStatus, &ResourceEntry);
	
	if (FindMessageStatus == STATUS_SUCCESS && ResourceEntry->Flags & MESSAGE_RESOURCE_UNICODE)
	{
		ANSI_STRING Message = { 0 };
		UNICODE_STRING UnicodeMessage;

		RtlInitUnicodeString(&UnicodeMessage, (LPCWSTR) ResourceEntry->Text);
		RtlUnicodeStringToAnsiString(&Message, &UnicodeMessage, TRUE);
		__sprintf(Buffer, "Status:  0x%x\nMessage: %s", Status, Message.Buffer);
		RtlFreeAnsiString(&Message);
	}
	else
	{
		__sprintf(Buffer, "Status:  0x%x\nMessage: %s", Status, "MESSAGE_RESOURCE_ENTRY Not Found");
	}
	
	DebugPrint(Buffer);
}

void DebugPrintNtStatus(NTSTATUS Status)
{
	char Buffer[256];
	HMODULE NtDllHandle;
	UNICODE_STRING NtDllName;
	MESSAGE_RESOURCE_ENTRY* ResourceEntry;
	NTSTATUS FindMessageStatus;

	RtlInitUnicodeString(&NtDllName, L"ntdll.dll");
	LdrGetDllHandle(NULL, NULL, &NtDllName, &NtDllHandle);

	FindMessageStatus = RtlFindMessage(NtDllHandle, (ULONG_PTR) RT_MESSAGETABLE, 0, Status, &ResourceEntry);

	if (FindMessageStatus == STATUS_SUCCESS && ResourceEntry->Flags & MESSAGE_RESOURCE_UNICODE)
	{
		ANSI_STRING Message = { 0 };
		UNICODE_STRING UnicodeMessage;

		RtlInitUnicodeString(&UnicodeMessage, (LPCWSTR) ResourceEntry->Text);
		RtlUnicodeStringToAnsiString(&Message, &UnicodeMessage, TRUE);
		__sprintf(Buffer, "Status:  0x%x\nMessage: %s", Status, Message.Buffer);
		RtlFreeAnsiString(&Message);
	}
	else
	{
		__sprintf(Buffer, "Status:  0x%x\nMessage: %s", Status, "MESSAGE_RESOURCE_ENTRY Not Found");
	}

	DebugPrint(Buffer);
}

void DebugPrintReturnStatus(char *FunctionName, NTSTATUS Status)
{
	SIZE_T BufferLen = strlen(FunctionName);
	char *Buffer = RtlAllocateHeap(RtlGetCurrentPeb()->ProcessHeap, HEAP_ZERO_MEMORY, BufferLen + 1);

	if (!Buffer)
	{
		return;
	}

	for (int i = 0; i < BufferLen; i++)
	{
		Buffer[i] = '=';
	}

	DebugPrint(Buffer);
	DebugPrint("\n");
	DebugPrint(FunctionName);
	DebugPrint("\n");
	DebugPrint(Buffer);
	DebugPrint("\n");
	DebugPrintStatus(Status);

	RtlFreeHeap(RtlGetCurrentPeb()->ProcessHeap, 0, Buffer);
}

void DebugPrintProcessInformation(RTL_USER_PROCESS_INFORMATION *ProcessInformation)
{
	PROCESS_BASIC_INFORMATION BasicInformation;

	NTSTATUS Status = NtQueryInformationProcess(
		ProcessInformation->ProcessHandle,
		ProcessBasicInformation,
		&BasicInformation,
		sizeof(BasicInformation),
		NULL);

	if (!Status)
	{
		DebugPrintReturnStatus("Process ExitStatus", BasicInformation.ExitStatus);
	}
	else
	{
		DebugPrint("NtQueryInformationProcess Failed!");
	}
}

NTSTATUS LpcServerThread(PVOID ThreadParameter)
{
	NTSTATUS Status;
	HANDLE LpcServerPort = (HANDLE) ThreadParameter;
	PORT_MESSAGE PortMessage;
	HANDLE LpcClientPort;
	
	Status = NtListenPort(LpcServerPort, &PortMessage);
	DebugPrintReturnStatus("NtListenPort", Status);
	
	Status = NtAcceptConnectPort(&LpcClientPort, NULL, &PortMessage, TRUE, NULL, NULL);
	DebugPrintReturnStatus("NtAcceptConnectPort", Status);

	Status = NtCompleteConnectPort(LpcClientPort);
	DebugPrintReturnStatus("NtCompleteConnectPort", Status);

	Status = NtReplyWaitReceivePort(LpcClientPort, NULL, NULL, &PortMessage);
	DebugPrintReturnStatus("NtReplyWaitReceivePort", Status);

	NtClose(LpcClientPort);

	return Status;
}

void InitLpcServer()
{
	HANDLE Port;
	NTSTATUS Status;
	UNICODE_STRING PortName;
	OBJECT_ATTRIBUTES ObjectAttributes;;
	HANDLE LpcServerThreadHandle;

	RtlInitUnicodeString(&PortName, L"\\??\\LaunchNative");
	InitializeObjectAttributes(&ObjectAttributes, &PortName, 0, NULL, NULL);

	Status = NtCreatePort(
		&Port,
		&ObjectAttributes,
		0,
		sizeof(PORT_MESSAGE),
		0);

	DebugPrintReturnStatus("NtCreatePort", Status);

	Status = RtlCreateUserThread(
		NtCurrentProcess(),
		NULL,
		TRUE,
		0,
		0,
		0,
		LpcServerThread,
		Port,
		&LpcServerThreadHandle,
		NULL);

	DebugPrintReturnStatus("RtlCreateUserThread", Status);

	Status = NtResumeThread(LpcServerThreadHandle, NULL);
	
	/*
	
	THREAD_BASIC_INFORMATION ThreadBasicInfo;

	Status = NtWaitForSingleObject(LpcServerThreadHandle, FALSE, NULL);

	Status = NtQueryInformationThread(
		LpcServerThreadHandle,
		ThreadBasicInformation,
		&ThreadBasicInfo,
		sizeof(THREAD_BASIC_INFORMATION),
		NULL);

	DebugPrintReturnStatus("NtQueryInformationThread", Status);
	DebugPrintReturnStatus("LpcServerThread ExitStatus", ThreadBasicInfo.ExitStatus);
	*/
}

int main()
{
	NTSTATUS Status;

	PCWSTR NativeAppPath = L"\\??\\C:\\Users\\Asher\\Source\\LaunchNative\\ARM64\\Debug\\NativeApp.exe";
	UNICODE_STRING ImageName;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	RTL_USER_PROCESS_INFORMATION ProcessInformation;

	InitNtFunctions();
	InitLpcServer();

	RtlInitUnicodeString(&ImageName, NativeAppPath);

	Status = RtlCreateProcessParameters(
		&ProcessParameters,
		&ImageName,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	DebugPrintReturnStatus("RtlCreateProcessParameters", Status);

	Status = RtlCreateUserProcess(
		&ImageName,
		0,
		ProcessParameters,
		NULL, NULL, NULL, FALSE, NULL, NULL,
		&ProcessInformation);

	DebugPrintReturnStatus("RtlCreateUserProcess", Status);

	RtlDestroyProcessParameters(ProcessParameters);

	NtResumeThread(ProcessInformation.ThreadHandle, NULL);
	NtWaitForSingleObject(ProcessInformation.ProcessHandle, FALSE, NULL);
	
	DebugPrintProcessInformation(&ProcessInformation);

	NtClose(ProcessInformation.ThreadHandle);
	NtClose(ProcessInformation.ProcessHandle);

	return 0;
}
