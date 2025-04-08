extern int arm_strlen(const char* str);

#include <windows.h>
#include <strsafe.h>

int main()
{
	char* str = NULL;// "";
	int cch = arm_strlen(str);

	char buffer[256];
	StringCchPrintfA(buffer, 256, "arm_strlen: \"%s\" = %d\n", str, cch);
	OutputDebugStringA(buffer);

	return cch;
}
