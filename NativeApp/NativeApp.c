#define PHNT_NO_INLINE_INIT_STRING

#include <phnt_windows.h>
#include <phnt.h>

NTSTATUS NtProcessStartup(PPEB peb)
{
    NTSTATUS Status;
    UNICODE_STRING PortName;
    HANDLE ServerPort;
    ULONG MaxMessageLength = 0;
    SECURITY_QUALITY_OF_SERVICE QoS;
    PORT_MESSAGE PortMessage;

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
    InitializeMessageHeader(&PortMessage, sizeof(PortMessage), 0);
    Status = NtRequestWaitReplyPort(
        ServerPort,
        &PortMessage,
        &PortMessage);
    /*
    Status = NtRequestPort(
        ServerPort,
        &PortMessage);
        */
    NtClose(ServerPort);
    
    return Status;
}