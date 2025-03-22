#define PHNT_NO_INLINE_INIT_STRING

#include <phnt_windows.h>
#include <phnt.h>

typedef struct _NATIVE_APP_MESSAGE
{
    PORT_MESSAGE PortMessage;
    WCHAR MessageText[32];

} NATIVE_APP_MESSAGE, * PNATIVE_APP_MESSAGE;

NTSTATUS NtProcessStartup(PPEB peb)
{
    NTSTATUS Status;
    UNICODE_STRING PortName;
    HANDLE ServerPort;
    ULONG MaxMessageLength = 0;
    SECURITY_QUALITY_OF_SERVICE QoS;
    NATIVE_APP_MESSAGE NativeAppMessage;

    RtlInitUnicodeString(&PortName, L"\\??\\LaunchNative");

    QoS.Length = sizeof(QoS);
    QoS.ImpersonationLevel = SecurityImpersonation;
    QoS.EffectiveOnly = FALSE;
    QoS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;

    Status = NtConnectPort(
        &ServerPort,
        &PortName,
        &QoS,
        NULL,
        NULL,
        &MaxMessageLength,
        NULL,
        NULL);
    /*
    Status = NtRequestWaitReplyPort(
        ServerPort,
        &PortMessage,
        &PortMessage);
        */
#define InitializeMessageHeader(ph, l, t)                              \
{                                                                      \
    (ph)->u1.s1.TotalLength      = (USHORT)(l);                        \
    (ph)->u1.s1.DataLength       = (USHORT)(l - sizeof(PORT_MESSAGE)); \
    (ph)->u2.s2.Type             = (USHORT)(t);                        \
    (ph)->u2.s2.DataInfoOffset   = 0;                                  \
    (ph)->ClientId.UniqueProcess = NULL;                               \
    (ph)->ClientId.UniqueThread  = NULL;                               \
    (ph)->MessageId              = 0;                                  \
    (ph)->ClientViewSize         = 0;                                  \
}
    InitializeMessageHeader(&NativeAppMessage.PortMessage, sizeof(NativeAppMessage), 0);

    NativeAppMessage.MessageText[0] = L'A';
    NativeAppMessage.MessageText[1] = L's';
    NativeAppMessage.MessageText[2] = L'h';
    NativeAppMessage.MessageText[3] = L'e';
    NativeAppMessage.MessageText[4] = L'r';
    NativeAppMessage.MessageText[5] = L'\0';

   // wcscpy(LpcMessage->MessageText, L"Message text through LPC");

    Status = NtRequestWaitReplyPort(
        ServerPort,
        &NativeAppMessage.PortMessage,
        &NativeAppMessage.PortMessage);
    /*
    Status = NtRequestPort(
        ServerPort,
        &PortMessage);
        */
    NtClose(ServerPort);
    
    return Status;
}