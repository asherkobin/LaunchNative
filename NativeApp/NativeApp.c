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

    Status = NtRequestPort(
        &ServerPort,
        &PortMessage);

    NtClose(ServerPort);
    
    return Status;
}