#define LPC_COMMAND_REQUEST_NOREPLY  0x00000000
#define LPC_COMMAND_REQUEST_REPLY    0x00000001
#define LPC_COMMAND_STOP             0x00000002

// This is the data structure transferred through LPC.
// Every structure must begin with PORT_MESSAGE, and must NOT be
// greater that MAX_LPC_DATA

typedef struct _TRANSFERRED_MESSAGE
{
    PORT_MESSAGE Header;

    ULONG   Command;
    WCHAR   MessageText[48];

} TRANSFERRED_MESSAGE, * PTRANSFERRED_MESSAGE;