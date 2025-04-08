extern int arm_strlen(const char* str);

#include <windows.h>
#include <strsafe.h>

int main()
{
	char *str_list[] =
	{
		"DUMMY",
		"asdf",
		".",
		"The silver spot price today is $30.38 USD.",
		NULL,
		"Charles Leclerc is from Monaco",
		"",
		"pi = 3.14",
		"The gold spot price today is $3,013.80 USD.",
		"asdfasdfsadfsdaf"
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
