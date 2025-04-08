#include <windows.h>
#include <strsafe.h>

extern int arm_strlen(const char* str);

int xmain()
{
	char* str_list[] =
	{
		"ARM64 Assembly",
		"Visual Studio 2022",
		"The silver spot price today is $30.38 USD.",
		"The gold spot price today is $3,013.80 USD.",
		"Charles Leclerc is from Monaco",
		"",
		".",
		" ",
		"pi = 3.14",
		"macOS Sequoia 15.3.2"
	};

	int num_strs = sizeof(str_list) / 8;

	for (int i = 0; i <= num_strs - 1; i++)
	{
		char* str = str_list[i];
		int cch = arm_strlen(str);

		char buffer[256];
		StringCchPrintfA(buffer, 256, "arm_strlen: \"%s\" = %d\n", str, cch);
		OutputDebugStringA(buffer);
	}

	return 1;
}
