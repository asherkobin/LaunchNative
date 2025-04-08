#include <Windows.h>
#include <strsafe.h>

int main()
{
	WCHAR str[] = L"The spot price of Silver is $30.38 USD.";
	int cch = strlen(str);

	WCHAR buffer[256];
	StringCchPrintfW(buffer, 256, L"strlen: %s = %d\n", str, cch);
	OutputDebugString(buffer);

	return cch;
}
