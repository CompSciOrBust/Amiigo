#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
using namespace std;

int fsize(string fil) {
 std::streampos fsize = 0;

    std::ifstream myfile (fil, ios::in);  // File is of type const char*

    fsize = myfile.tellg();         // The file pointer is currently at the beginning
    myfile.seekg(0, ios::end);      // Place the file pointer at the end of file

    fsize = myfile.tellg() - fsize;
    myfile.close();

    static_assert(sizeof(fsize) >= sizeof(long long), "Oops.");
    return fsize;
}

bool CheckFileExists(string Path)
{
	struct stat Buffer;   
	return (stat (Path.c_str(), &Buffer) == 0); 
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