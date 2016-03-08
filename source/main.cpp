/*
   AngelCode Bitmap Font Generator
   Copyright (c) 2004-2016 Andreas Jonsson
  
   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.
  
   Andreas Jonsson
   andreas@angelcode.com
*/

#include <windows.h>
#include <crtdbg.h>
#include <string>
#include <iostream>

#include "dynamic_funcs.h"
#include "charwin.h"

using namespace std;

const char *getArgValue(const char *cmdLine, string &value)
{
	cmdLine += strspn(cmdLine, " \t");
	size_t end;
	if( *cmdLine == '"' )
	{
		end = strcspn(++cmdLine, "\"");
		if( cmdLine[end] == '"' )
			value.assign(cmdLine, end);
		end++;
	}
	else
	{
		end = strcspn(cmdLine, " ");
		value.assign(cmdLine, end);
	}
	cmdLine += end;
	return cmdLine;
}

// Returns true if the GUI should be opened
bool processCmdLine(const char *cmdLine, string &configFile_)
{
	string outputFile;
	string textFile;
	vector<string> configFiles;
	vector<CFontGen*> fontGens;

	configFile_ = CCharWin::GetDefaultConfig(); // Use the last configuration from the GUI as default

	bool hasError = false;

	size_t start = strspn(cmdLine, " \t");
	if( start == 0 && *cmdLine == 0 )
	{
		// No arguments, open the GUI
		return true;
	}

	cmdLine += start;
	bool check_export = false;
	if( *cmdLine == '-' )
	{
		while( *cmdLine == '-' )
		{
			cmdLine++;
			if (*cmdLine == 'e')
			{
				check_export = true;
				cmdLine++;
			}
			else if( *cmdLine == 'o' )
				cmdLine = getArgValue(++cmdLine, outputFile);
			else if (*cmdLine == 'c')
			{
				string conf;
				cmdLine = getArgValue(++cmdLine, conf);
				if (configFiles.size() == 0)
					configFile_ = conf;
				configFiles.push_back(conf);
			}
			else if( *cmdLine == 't' )
				cmdLine = getArgValue(++cmdLine, textFile);
			else
			{
				hasError = true;
				break;
			}
	
			cmdLine += strspn(cmdLine, " \t");
		}
	}
	else 
	{
		// It may be a bmfc file, in which case we assume the app is being 
		// opened with the intention to show the GUI and load that file
		cmdLine = getArgValue(cmdLine, configFile_);
		return true;
	}

	// Attach to the console and assign the streams
	AttachConsole(ATTACH_PARENT_PROCESS);
	freopen("CONOUT$","w",stdout);
	freopen("CONOUT$","w",stderr);

	if( hasError || outputFile == "" )
	{
		cerr << "Incorrect arguments. See documentation for instructions." << endl;
		return false;
	}

	for (int i = 0; i < configFiles.size(); i++)
	{
		auto fontGen = new CFontGen();
		fontGen->SetCheckExport(check_export);
		fontGen->LoadConfiguration(configFiles[i].c_str());
		if( textFile != "" )
		{
			cout << "Selecting characters from file." << endl;
			fontGen->SelectCharsFromFile(textFile.c_str());
		}
		fontGen->GeneratePages(false, true);
		fontGens.push_back(fontGen);
	}

	fontGens[0]->PostGeneratePages(fontGens);

	cout << "Saving font." << endl;
	fontGens[0]->SaveFont(outputFile.c_str());

	/*for (int i = 0; i < fontGens.size(); i++)
	{
		delete fontGens[i];
	}*/
	cout << "Finished." << endl;

	return false;
}

int WINAPI WinMain(HINSTANCE hInst, 
				   HINSTANCE hPrevInst, 
				   LPSTR     cmdLine, 
				   int       showFlag)
{
	// Turn on memory leak detection
	// Find the memory leak with _CrtSetBreakAlloc(n) where n is the number reported
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF|_CRTDBG_ALLOC_MEM_DF);
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);

	Init();

	string configFile;
	bool openGui = processCmdLine(cmdLine, configFile);

	if( openGui )
	{
		CCharWin *wnd = new CCharWin;
		wnd->Create(512, 512, configFile);
		while( !acWindow::CWindow::CheckMessages(true) ) {}
		delete wnd;
	}

	Uninit();

	return 0;
}
