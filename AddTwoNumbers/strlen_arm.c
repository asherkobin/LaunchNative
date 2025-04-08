extern int add_two_numbers2(int a, int b);

#include <Windows.h>
#include <strsafe.h>

int main()
{
	int a = -9;
	int b = 425;
	int sum = add_two_numbers2(a, b);

	WCHAR buffer[256];
	StringCchPrintfW(buffer, 256, L"XXXX %d + %d = %d\n", a, b, sum);
	OutputDebugString(buffer);
}
