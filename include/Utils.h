#include <string>
#include <sys/stat.h>
using namespace std;

int fsize(string fil);
bool CheckFileExists(string);
string GoUpDir(string);
bool copy_me(string origen, string destino);
void DrawJsonColorConfig(SDL_Renderer* renderer, string Head);