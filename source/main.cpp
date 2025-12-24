#include <arriba.h>
#include <AmiigoUI.h>
#include <sys/stat.h>
#include <Networking.h>
#include <utils.h>
#include <emuiibo.hpp>
#include <AmiigoSettings.h>
#include <NFCDumper.h>

int main(int argc, char *argv[]) {
	// Init
	socketInitializeDefault();
	nxlinkStdio();
	romfsInit();
	nifmInitialize(NifmServiceType_User);
	// Create folder for caching API data
	mkdir("sdmc:/config/amiigo", 0);
	// Create folder for Amiibos
	mkdir("sdmc:/emuiibo/amiibo", 0);
	Arriba::init();
	// Save current path
	strcpy(Amiigo::Settings::amiigoPath, argv[0]);
	printf("%s\n", Amiigo::Settings::amiigoPath);
	// Load bg
	Arriba::Primitives::Quad* bg = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
	// If a custom background shader exists use it, otherwise load the default from romfs
	if(checkIfFileExists("sdmc:/config/amiigo/bgFragment.glsl"))
		bg->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "sdmc:/config/amiigo/bgFragment.glsl");
	else
		bg->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/bgFragment.glsl");
	bg->name = "AmiigoBG";
	// Init NFC dumper
	Amiigo::NFC::Dumper::init();
	// Init UI
	Amiigo::UI::initUI();

	// Main loop
	while (appletMainLoop()) {
		// Handle UI input and rendering
		Amiigo::UI::handleInput();
		if(Arriba::Input::buttonUp(Arriba::Input::PlusButtonSwitch) || !Amiigo::UI::isRunning) break;
		Arriba::drawFrame();
		bg->renderer->thisShader.setFloat1("iTime", Arriba::time);
		
		// Scan for Physical amiibos to dump
		//NfpDeviceState nfpState;
		//bool hasDumped = false;
		//nfpGetDeviceState(&Amiigo::NFC::Dumper::readerHandle, &nfpState);
		//if (nfpState == NfpDeviceState_TagFound) hasDumped = Amiigo::NFC::Dumper::dumpNFC();
		//if (hasDumped) Amiigo::UI::updateSelectorStrings();
	}
	
	// Deinit
	socketExit();
	romfsExit();
	nifmExit();
	Amiigo::NFC::Dumper::exit();
	Arriba::exit();
	if(emu::IsAvailable()) emu::Exit();
	return 0;
}
