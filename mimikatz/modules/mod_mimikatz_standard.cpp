/*	Benjamin DELPY `gentilkiwi`
http://blog.gentilkiwi.com
benjamin@gentilkiwi.com
Licence : http://creativecommons.org/licenses/by-nc-sa/3.0/fr/
*/
#include "mod_mimikatz_standard.h"

vector<KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND> mod_mimikatz_standard::getMimiKatzCommands()
{
	vector<KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND> monVector;
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(clearScreen,	L"cls",		L"Clears the screen (does not work in remote execution via PsExec example)"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(exit,		L"exit",	L"Exits MimiKatz"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(reponse,		L"reponse",	L"Computes the answer to the Great Question of Life, the Universe and Everything"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(cite,		L"cite",	L"Is a quote"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(version,		L"version",	L"Returns the version of mimikatz"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(sleep,		L"sleep",	L"Pauses mimikatz a number of milliseconds"));
	//monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(test,		L"test",	L"Routine test (should not be there in release..."));
	return monVector;
}

/*bool mod_mimikatz_standard::test(vector<wstring> * arguments)
{
	return true;
}*/

bool mod_mimikatz_standard::version(vector<wstring> * arguments)
{
	wcout << MIMIKATZ_FULL << L" (" << __DATE__ << L' ' << __TIME__ << L')' << endl;
	return true;
}

bool mod_mimikatz_standard::clearScreen(vector<wstring> * arguments)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = {0, 0};
	DWORD count;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hStdOut, &csbi);

	FillConsoleOutputCharacter(hStdOut, L' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
	SetConsoleCursorPosition(hStdOut, coord);

	return true;
}

bool mod_mimikatz_standard::exit(vector<wstring> * arguments)
{
	return false;
}

bool mod_mimikatz_standard::reponse(vector<wstring> * arguments)
{
	wcout << L"The answer is 42." << endl;
	return true;
}

bool mod_mimikatz_standard::cite(vector<wstring> * arguments)
{
	wcout << L"I edit the world in HEX" << endl;
	return true;
}

bool mod_mimikatz_standard::sleep(vector<wstring> * arguments)
{
	DWORD dwMilliseconds = 1000;
	if(!arguments->empty())
	{
		wstringstream z;
		z << arguments->front(); z >> dwMilliseconds;
	}
	wcout << L"Sleep : " << dwMilliseconds << L" ms... " << flush;
	Sleep(dwMilliseconds);
	wcout << L"End !" << endl;
	return true;
}
