#include <arriba.h>
#include <AmiigoUI.h>
#include <sys/stat.h>
#include <Networking.h>
#include <utils.h>
#include <emuiibo.hpp>
#include <AmiigoSettings.h>

int main(int argc, char *argv[])
{
	//Init
	socketInitializeDefault();
	nxlinkStdio();
	romfsInit();
	nifmInitialize(NifmServiceType_User);
	//Create folder for caching API data
	mkdir("sdmc:/config/amiigo", 0);
	//Create folder for Amiibos
	mkdir("sdmc:/emuiibo/amiibo", 0);
	Arriba::init();
	//Save current path
	strcpy(Amiigo::Settings::amiigoPath, argv[0]);
	printf("%s\n", Amiigo::Settings::amiigoPath);
	//Load bg
	Arriba::Primitives::Quad* bg = new Arriba::Primitives::Quad(0,0,Arriba::Graphics::windowWidth,Arriba::Graphics::windowHeight,Arriba::Graphics::Pivot::topLeft);
	//If a custom background shader exists use it, otherwise load the default from romfs
	if(checkIfFileExists("sdmc:/config/amiigo/bgFragment.glsl")) bg->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "sdmc:/config/amiigo/bgFragment.glsl");
	else bg->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/bgFragment.glsl");
	bg->name = "AmiigoBG";
	//Init UI
	Amiigo::UI::initUI();
	if(emu::IsAvailable()) emu::Initialize();
	//if(!checkIfFileExists("sdmc:/config/amiigo/API.json"))retrieveToFile("https://www.amiiboapi.com/api/amiibo", "sdmc:/config/amiigo/API.json");

	//Main loop
	while (appletMainLoop())
	{
		Amiigo::UI::handleInput();
		if(Arriba::Input::buttonUp(HidNpadButton_Plus) || !Amiigo::UI::isRunning) break;
		Arriba::drawFrame();
		bg->renderer->thisShader.setFloat1("iTime", Arriba::time);
	}
	//Deinit
	socketExit();
	romfsExit();
	nifmExit();
	Arriba::exit();
	if(emu::IsAvailable) emu::Exit();
	return 0;
}