#include <string>
#include <sys/stat.h>
using namespace std;

size_t fsize(string fil);
bool CheckFileExists(string);
string GoUpDir(string);
bool copy_me(string origen, string destino);