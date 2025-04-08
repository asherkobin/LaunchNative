#include <windows.h>
#include <strsafe.h>

extern int arm_strlen_unicode(wchar_t * str);

int main()
{
	wchar_t* str_list[] =
	{
		L"ARM64 Assembly",
		L"Visual Studio 2022",
		L"The silver spot price today is $30.38 USD.",
		L"The gold spot price today is $3,013.80 USD.",
		L"Charles Leclerc is from Monaco",
		L"",
		L".",
		L" ",
		L"pi = 3.14",
		L"macOS Sequoia 15.3.2"
	};

	int num_strs = sizeof(str_list) / 8;

	for (int i = 0; i <= num_strs - 1; i++)
	{
		wchar_t* str = str_list[i];
		int cch = arm_strlen_unicode(str);

		wchar_t buffer[256];
		StringCchPrintfW(buffer, 256, L"arm_strlen: \"%s\" = %d\n", str, cch);
		OutputDebugStringW(buffer);
	}

	return 1;
}
