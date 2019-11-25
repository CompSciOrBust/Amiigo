#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cstring>
using namespace std;

bool CheckFileExists(string Path)
{
	struct stat Buffer;   
	return !(stat (Path.c_str(), &Buffer) == 0); 
}

string GoUpDir(string Path)
{
	char CurrentPath[Path.length()] = "";
	strcat(CurrentPath, Path.c_str());
	CurrentPath[Path.length()-1] = 0x00;
	Path = CurrentPath;
	int LastSlash = Path.find_last_of('/');
	CurrentPath[LastSlash] = 0x00;
	if(strlen(CurrentPath) < 21)
	{
		return "sdmc:/emuiibo/amiibo/";
	}
	return CurrentPath;
}