#include <arriba.h>
#include <AmiigoUI.h>
#include <sys/stat.h>
#include <Networking.h>
#include <emuiibo.hpp>
#include <AmiigoSettings.h>
#include <NFCDumper.h>

int main(int argc, char *argv[]) {
	socketInitializeDefault();
	nxlinkStdio();
	romfsInit();
	nifmInitialize(NifmServiceType_User);

	mkdir("sdmc:/config/amiigo", 0);
	mkdir("sdmc:/emuiibo/amiibo", 0);

	Arriba::init();

	strcpy(Amiigo::Settings::amiigoPath, argv[0]);

	Amiigo::NFC::Dumper::init();

	Amiigo::UI::initUI();

	while (appletMainLoop()) {
		Amiigo::UI::handleInput();
		if(Arriba::Input::buttonUp(Arriba::Input::PlusButtonSwitch) || !Amiigo::UI::isRunning) break;
		Arriba::drawFrame();
		MainThread::poll();
		
		// TODO: Enabled once feature complete
		//if (Amiigo::NFC::Dumper::poll()) Amiigo::UI::updateSelectorStrings();
	}
	
	workerQueue.shutdown();
	socketExit();
	romfsExit();
	nifmExit();
	Amiigo::NFC::Dumper::exit();
	Arriba::exit();
	if(emu::IsAvailable()) emu::Exit();
	return 0;
}
