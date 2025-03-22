#include <phnt_windows.h>
#include <phnt.h>

typedef int (*PSPRINTF)(char* buffer, const char* format, ...);
PSPRINTF __sprintf = NULL;

typedef struct _NATIVE_APP_MESSAGE
{
	PORT_MESSAGE PortMessage;
	char MessageText[32];

} NATIVE_APP_MESSAGE, *PNATIVE_APP_MESSAGE;

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
	if (Status == STATUS_SUCCESS)
	{
		DebugPrint("Status: STATUS_SUCCESS\n");
	}
	else
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

int main()
{
	NTSTATUS Status;
	PCWSTR NativeAppPath = L"\\??\\C:\\Users\\Asher\\Source\\LaunchNative\\ARM64\\Debug\\NativeApp.exe";
	UNICODE_STRING ImageName;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	RTL_USER_PROCESS_INFORMATION ProcessInformation;
	HANDLE ServerPort = NULL;
	HANDLE ClientPort = NULL;
	UNICODE_STRING LpcPortName;
	OBJECT_ATTRIBUTES ObjectAttributes;
	NATIVE_APP_MESSAGE NativeAppMessage;
	ULONG MessageType = 0;
	
	InitNtFunctions();

	RtlInitUnicodeString(&LpcPortName, L"\\??\\LaunchNative");
	InitializeObjectAttributes(&ObjectAttributes, &LpcPortName, 0, NULL, NULL);

	Status = NtCreatePort(
		&ServerPort,
		&ObjectAttributes,
		0,
		sizeof(NATIVE_APP_MESSAGE),
		0);

	DebugPrintReturnStatus("NtCreatePort", Status);

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
		NULL, NULL, NULL, TRUE, NULL, NULL,
		&ProcessInformation);

	DebugPrintReturnStatus("RtlCreateUserProcess", Status);

	RtlDestroyProcessParameters(ProcessParameters);

	NtResumeThread(ProcessInformation.ThreadHandle, NULL);

	while (MessageType != LPC_PORT_CLOSED)
	{
		Status = NtReplyWaitReceivePort(
			ServerPort,
			NULL,
			NULL,
			&NativeAppMessage.PortMessage);

		MessageType = NativeAppMessage.PortMessage.u2.s2.Type;

		switch (MessageType)
		{
			case LPC_CONNECTION_REQUEST:
				DebugPrint("** LPC_CONNECTION_REQUEST **\n");
				Status = NtAcceptConnectPort(&ClientPort, NULL, &NativeAppMessage.PortMessage, TRUE, NULL, NULL);
				DebugPrintReturnStatus("NtAcceptConnectPort", Status);
				Status = NtCompleteConnectPort(ClientPort);
				DebugPrintReturnStatus("NtCompleteConnectPort", Status);
				break;
			case LPC_DATAGRAM: // NtRequestPort
				DebugPrint("** LPC_DATAGRAM **\n");
				DebugPrint(NativeAppMessage.MessageText);
				DebugPrint("\n");
				break;
			case LPC_REQUEST: // NtRequestWaitReplyPort
				DebugPrint("** LPC_REQUEST **\n");
				DebugPrint(NativeAppMessage.MessageText);
				DebugPrint("\n");
				NativeAppMessage.MessageText[0] = L'K';
				NativeAppMessage.MessageText[1] = L'o';
				NativeAppMessage.MessageText[2] = L'b';
				NativeAppMessage.MessageText[3] = L'i';
				NativeAppMessage.MessageText[4] = L'n';
				NativeAppMessage.MessageText[5] = L'\0';
				Status = NtReplyPort(ServerPort, &NativeAppMessage.PortMessage);
				DebugPrintReturnStatus("NtReplyPort", Status);
				// NtReplyWaitReplyPort
				break;
			case LPC_PORT_CLOSED:
				DebugPrint("** LPC_PORT_CLOSED **\n");
				NtWaitForSingleObject(ProcessInformation.ProcessHandle, FALSE, NULL);
				DebugPrintProcessInformation(&ProcessInformation);
				NtClose(ProcessInformation.ThreadHandle);
				NtClose(ProcessInformation.ProcessHandle);
				break;
			default:
				DebugPrint("** Unhandled LPC Message Type **\n");
		}
	}
	
	if (ClientPort)
		NtClose(ClientPort);
	if (ServerPort)
		NtClose(ServerPort);

	return STATUS_SUCCESS;
}
