#include <AmiigoUI.h>
#include <arriba.h>
#include <utils.h>

#include <cmath>
#include <cstdio>

#include <fstream>
#include <thread>

#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <arribaText.h>
#include <emuiibo.hpp>
#include <Networking.h>
#include <AmiigoSettings.h>
#include <AmiigoElements.h>
#include <AmiigoBehaviours.h>

namespace Amiigo::UI {
	void initUI() {
		Amiigo::Settings::loadSettings();
		Arriba::Colour::neutral = Amiigo::Settings::Colour::listNeutral;
		Arriba::Colour::highlightA = Amiigo::Settings::Colour::listHighlightA;
		Arriba::Colour::highlightB = Amiigo::Settings::Colour::listHighlightB;
	    if (!checkIfFileExists("sdmc:/config/amiigo/API.json") || !checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) initSplash();
		if (emu::IsAvailable()) emu::Initialize();
		initSceneSwitcher();
		initSelector();
	    initMaker();
		initSettings();
		Arriba::highlightedObject = Arriba::findObjectByName("SelectorList");
		// Cache these now for performance gains later
		lists = Arriba::findObjectsByTag("List");
		buttons = Arriba::findObjectsByTag("SwitcherButton");
		// Check if a new Amiigo version is available
		if (checkForUpdates()) {
			settingsButton->setText(U"Update");
			Arriba::findObjectByName("UpdaterButton")->enabled = true;
		}
	}

	void initSplash() {
		Arriba::activeLayer++;
		splashScene = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
		splashScene->setColour({0.25, 0.25, 0.25, 0.95});
		// Title text
		Arriba::Primitives::Text* titleText = new Arriba::Primitives::Text(U"Amiigo", 128);
		titleText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 140, 0};
		titleText->setColour({1, 1, 1, 1});
		titleText->setParent(splashScene);
		// By text
		Arriba::Primitives::Text* byText = new Arriba::Primitives::Text(U"by CompSciOrBust", 64);
		byText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 15, 0};
		byText->setColour({1, 1, 1, 1});
		byText->setParent(splashScene);
		// Doing text
		Arriba::Primitives::Text* doingText = new Arriba::Primitives::Text("", 48);
		doingText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 + 160, 0};
		doingText->setColour({1, 1, 1, 1});
		doingText->setParent(splashScene);
		std::thread initThread(firstTimeSetup);
		// Hide other elements
		if (settingsScene) settingsScene->enabled = false;
		if (sceneSwitcher) sceneSwitcher->enabled = false;
		while (!checkIfFileExists("sdmc:/config/amiigo/API.json") || !checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp") || checkIfFileExists("sdmc:/config/amiigo/update.flag")) {
			splashScene->setColour({(sin(Arriba::time)+1)/4, (cos(Arriba::time)+1)/4, 0.5, 0.95});
			Arriba::findObjectByName("AmiigoBG")->renderer->thisShader.setFloat1("iTime", Arriba::time);
			if (!hasNetworkConnection()) doingText->setText(U"Waiting for internet connection...");
			else if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) doingText->setText(U"Caching API data...");
			else if (!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) doingText->setText(U"Installing Emuiibo...");
			else if (checkIfFileExists("sdmc:/config/amiigo/update.flag")) doingText->setText(U"Updating Amiigo...");
			Arriba::drawFrame();
		}
		initThread.join();
		splashScene->destroy();
		if (settingsScene) settingsScene->enabled = true;
		if (sceneSwitcher) sceneSwitcher->enabled = true;
		Arriba::activeLayer--;
	}

	void initSceneSwitcher() {
		// Set up divider quads
		Arriba::Primitives::Quad* div1 = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth - switcherWidth - 1, statusHeight, 5, switcherHeight, Arriba::Graphics::Pivot::topLeft);
		div1->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div2 = new Arriba::Primitives::Quad(0, statusHeight - 1, Arriba::Graphics::windowWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div2->setColour({0, 0, 0, 1});
		int buttomDivX = Arriba::Graphics::windowWidth - switcherWidth;
		Arriba::Primitives::Quad* div3 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4) - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div3->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div4 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4)*2 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div4->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div5 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4)*3 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div5->setColour({0, 0, 0, 1});
		// Set up status bar
		Arriba::Primitives::Quad* statusBar = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, statusHeight - 1, Arriba::Graphics::Pivot::topLeft);
		Amiigo::AmiigoBehaviours::StatusBarColourBehaviour* statusBarColourBehaviour = new Amiigo::AmiigoBehaviours::StatusBarColourBehaviour();
		statusBar->setColour(Amiigo::Settings::Colour::statusBar);
		statusBar->addBehaviour(statusBarColourBehaviour);
		statusBar->name = "StatusBar";
		Arriba::Primitives::Text* statusText = new Arriba::Primitives::Text(U"Amiigo + Arriba", 34);
		statusText->name = "StatusBarText";
		statusText->setDimensions(statusText->width, statusText->height, Arriba::Graphics::centre);
		statusText->transform.position = {statusBar->width/2, statusBar->height/2, 0};
		statusText->setParent(statusBar);
		// Set up switcher quad
		sceneSwitcher = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth, statusHeight, switcherWidth, switcherHeight, Arriba::Graphics::Pivot::topRight);
		sceneSwitcher->setColour({0, 0, 0, 0});
		// Set up Amiibo list button
		selectorButton = new Arriba::Elements::Button();
		selectorButton->setText(U"My Amiibo");
		selectorButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		selectorButton->setParent(sceneSwitcher);
		selectorButton->name = "SelectorButton";
		selectorButton->tag = "SwitcherButton";
		selectorButton->registerCallback(switcherPressed);
		// Set up Amiigo maker button
		makerButton = new Arriba::Elements::Button();
		makerButton->setText(U"Amiigo Store");
		makerButton->transform.position.y = selectorButton->height + 1;
		makerButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		makerButton->setParent(sceneSwitcher);
		makerButton->name = "MakerButton";
		makerButton->tag = "SwitcherButton";
		makerButton->registerCallback(switcherPressed);
		// Set up settings button
		settingsButton = new Arriba::Elements::Button();
		settingsButton->setText(U"Settings");
		settingsButton->transform.position.y = makerButton->height + makerButton->transform.position.y + 1;
		settingsButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		settingsButton->setParent(sceneSwitcher);
		settingsButton->name = "SettingsButton";
		settingsButton->tag = "SwitcherButton";
		settingsButton->registerCallback(switcherPressed);
		// Set up exit button
		exitButton = new Arriba::Elements::Button();
		exitButton->setText(U"Exit");
		exitButton->transform.position.y = settingsButton->height + settingsButton->transform.position.y + 1;
		exitButton->setDimensions(switcherWidth, switcherHeight/4, Arriba::Graphics::Pivot::topRight);
		exitButton->setParent(sceneSwitcher);
		exitButton->name = "ExitButton";
		exitButton->tag = "SwitcherButton";
		exitButton->registerCallback([](){isRunning = 0;});
	}

	void initSelector() {
	    // Set up the list
	    selectorList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, std::vector<std::string>{});
		updateSelectorStrings();
		selectorList->name = "SelectorList";
		selectorList->tag = "List";
		selectorList->registerCallback(selectorInput);
		selectorList->registerAltCallback(selectorContextMenuSpawner);
	}

	void initMaker() {
		seriesList = getListOfSeries();
	    makerList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, seriesList);
		makerList->registerCallback(makerInput);
		makerList->name = "MakerList";
		makerList->tag = "List";
		makerList->enabled = false;
	}

	void initSettings() {
		const int buttonHeight = 100;
		const int buttonCount = 3 + 1;
		settingsScene = new Arriba::Primitives::Quad(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		// Not a list but pretending makes scene switching easier
		settingsScene->name = "SettingsScene";
		settingsScene->tag = "List";
		settingsScene->enabled = false;
		settingsScene->setColour({0, 0, 0, 0});
		// Toggle category save button
		Arriba::Elements::Button* categoryButton = new Arriba::Elements::Button();
		categoryButton->setParent(settingsScene);
		categoryButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		categoryButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 1/buttonCount, 0};
		switch (Amiigo::Settings::categoryMode) {
			case Amiigo::Settings::categoryModes::saveToRoot:
			categoryButton->setText(U"Save to game name");
			break;

			case Amiigo::Settings::categoryModes::saveByGameName:
			categoryButton->setText(U"Save to Amiibo series");
			break;

			case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
			categoryButton->setText(U"Save to current folder");
			break;

			case Amiigo::Settings::categoryModes::saveByCurrentFolder:
			categoryButton->setText(U"Save to root");
			break;
		
			default:
			categoryButton->setText(U"Error");
			break;
		}
		categoryButton->name = "CategorySettingsButton";
		// Callback to save setting
		categoryButton->registerCallback([](){
			Amiigo::Settings::categoryMode = (Amiigo::Settings::categoryMode+1) % Amiigo::Settings::categoryModes::categoryCount;
			printf("%d", Amiigo::Settings::categoryMode);
			Amiigo::Settings::saveSettings();
			switch (Amiigo::Settings::categoryMode) {
				case Amiigo::Settings::categoryModes::saveToRoot:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText(U"Save to game name");
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/amiibo");
				break;
			
				case Amiigo::Settings::categoryModes::saveByGameName:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText(U"Save to Amiibo series");
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/game name");
				break;

				case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText(U"Save to current folder");
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/amiibo series");
				break;

				case Amiigo::Settings::categoryModes::saveByCurrentFolder:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText(U"Save to root");
				updateStatusInfo(U"Amiibos will save to the current location");
				break;

				default:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText(U"Error");
				updateStatusError(U"Error, uknown category mode");
				break;
			}
		});
		// Toggle random UUIDs button
		Arriba::Elements::Button* randomUUIDButton = new Arriba::Elements::Button();
		randomUUIDButton->setParent(settingsScene);
		randomUUIDButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		randomUUIDButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 2/buttonCount, 0};
		switch (Amiigo::Settings::useRandomisedUUID) {
			case true:
			randomUUIDButton->setText(U"Disable random UUID");
			break;

			case false:
			randomUUIDButton->setText(U"Enable random UUID");
			break;
		
			default:
			randomUUIDButton->setText(U"UUID Status error");
			break;
		}
		randomUUIDButton->name = "ToggleRandomUUIDButton";
		randomUUIDButton->registerCallback([](){
			Amiigo::Settings::useRandomisedUUID = !Amiigo::Settings::useRandomisedUUID;
			Amiigo::Settings::saveSettings();
			switch (Amiigo::Settings::useRandomisedUUID) {
				case true:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("ToggleRandomUUIDButton"))->setText(U"Disable random UUID");
				updateStatusInfo(U"Amiibos will now generate with random UUIDs");
				break;

				case false:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("ToggleRandomUUIDButton"))->setText(U"Enable random UUID");
				updateStatusInfo(U"Amiibos will now generate with static UUIDs");
				break;

				default:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("ToggleRandomUUIDButton"))->setText(U"UUID Status error");
				break;
			}
		});
		// Check for updates button
		Arriba::Elements::Button* updaterButton = new Arriba::Elements::Button();
		updaterButton->setParent(settingsScene);
		updaterButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		updaterButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 3/buttonCount, 0};
		updaterButton->setText(U"Update Amiigo");
		updaterButton->name = "UpdaterButton";
		updaterButton->enabled = false;
		// Callback for updating Amiigo
		updaterButton->registerCallback([](){
			if (!hasNetworkConnection()) {
				updateStatusError(U"No network connection!");
			} else {
				std::ofstream fileStream("sdmc:/config/amiigo/update.flag");
				fileStream.close();
				initSplash();
			}
		});

		// Credits Quad
		Arriba::Primitives::Quad* creditsQuad = new Arriba::Primitives::Quad(0, 0, 330, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		creditsQuad->setParent(settingsScene);
		creditsQuad->setColour({0, 0, 0, 0.9});
		int yOffset = 0;
		// et Emuiibo version
		char emuVer[12];
		emu::Version emuiiboVersion = emu::GetVersion();
		sprintf(emuVer, "%u.%u.%u", emuiiboVersion.major, emuiiboVersion.minor, emuiiboVersion.micro);
		Amiigo::Settings::emuiiboVersionText = emuVer;
		// Credits text
		Arriba::Primitives::Text* creditsTitleText = new Arriba::Primitives::Text(U"Credits", 64);
		creditsTitleText->setColour({0, 0.7, 1, 1});
		creditsTitleText->setParent(creditsQuad);
		creditsTitleText->transform.position = {creditsQuad->width/2, yOffset += creditsTitleText->height + 30, 0};
		for (int i = 0; i < 5; i++) {
			std::u32string titleText = U"Place holder";
			std::u32string nameText = U"Place holder";
			switch (i) {
				case 0:
				titleText = U"Developer";
				nameText = U"CompSciOrBust";
				break;
				case 1:
				titleText = U"Emuiibo " + std::u32string(Arriba::Text::ASCIIToUnicode(Amiigo::Settings::emuiiboVersionText.c_str()));
				nameText = U"XorTroll";
				break;
				case 2:
				titleText = U"Contribuyente";
				nameText = U"Kronos2308";
				break;
				case 3:
				titleText = U"The Pizza Guy";
				nameText = U"Za";
				break;
				case 4:
				titleText = U"Amiibo API";
				nameText = U"N3evin";
				break;
			}
			Arriba::Primitives::Text* titleTextObject = new Arriba::Primitives::Text(titleText.c_str(), 38);
			titleTextObject->setParent(creditsQuad);
			titleTextObject->transform.position = {creditsQuad->width/2, yOffset += titleTextObject->height + 20, 0};
			Arriba::Primitives::Text* nameTextObject = new Arriba::Primitives::Text(nameText.c_str(), 28);
			nameTextObject->setParent(creditsQuad);
			nameTextObject->transform.position = {creditsQuad->width/2, yOffset += nameTextObject->height + 10, 0};
			titleTextObject->setColour({0, 0.7, 1, 1});
			nameTextObject->setColour({0, 0.7, 1, 1});
		}
	}

	void handleInput() {
		// We only want to handle input for the base layer of the UI
		if (Arriba::activeLayer != 0) return;
		// If no object is selected
		if (Arriba::highlightedObject == nullptr) {
			if (Arriba::Input::buttonDown(Arriba::Input::controllerButton(Arriba::Input::DPadRight | Arriba::Input::DPadLeft | Arriba::Input::DPadUp | Arriba::Input::DPadDown))) Arriba::highlightedObject = selectorButton;
		}
		// If one of the lists are selected
		for (size_t i = 0; i < lists.size(); i++) {
			// Right pressed
			if (Arriba::highlightedObject == lists[i] && Arriba::Input::buttonDown(Arriba::Input::DPadRight)) {
				Arriba::highlightedObject = selectorButton;
				return;
			}
			// Selector list is selected
			if (lists[i]->name == "SelectorList" && lists[i]->enabled) {
				if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && selectorPath != "sdmc:/emuiibo/amiibo") {
					selectorPath = selectorPath.substr(0, selectorPath.find_last_of("/"));
					if (selectorPath.length() < sizeof("sdmc:/emuiibo/amiibo")) selectorPath = "sdmc:/emuiibo/amiibo";
					updateSelectorStrings();
				}
				if (Arriba::Input::buttonDown(Arriba::Input::XButtonSwitch)) {
					switch (emu::GetEmulationStatus()) {
						case emu::EmulationStatus::On:
							emu::ResetActiveVirtualAmiibo();
							emu::SetEmulationStatus(emu::EmulationStatus::Off);
							updateStatusInfo(U"Emuiibo disabled");
						break;
						case emu::EmulationStatus::Off:
							emu::SetEmulationStatus(emu::EmulationStatus::On);
							updateStatusInfo(U"Emuiibo enabled");
						break;
						default:
							updateStatusInfo(U"Error: Unkown emulation status!");
						break;
					}
				}
			// Maker list is selected
			} else if (lists[i]->name == "MakerList" && lists[i]->enabled) {
				if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && makerIsInCategory) {
					static_cast<Arriba::Elements::InertialList*>(lists[i])->updateStrings(seriesList);
					makerIsInCategory = false;
				}
			// Settings is selected
			} else if (lists[i]->name == "SettingsScene" && lists[i]->enabled) {
				// Left / right pressed
				if (Arriba::Input::buttonDown(Arriba::Input::DPadRight) && Arriba::highlightedObject->tag != "SwitcherButton") Arriba::highlightedObject = selectorButton;
				if (Arriba::Input::buttonDown(Arriba::Input::DPadLeft) && Arriba::highlightedObject->tag == "SwitcherButton") Arriba::highlightedObject = Arriba::findObjectByName("CategorySettingsButton");
				// Up pressed
				if (Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {
					if (Arriba::highlightedObject == Arriba::findObjectByName("ToggleRandomUUIDButton")) Arriba::highlightedObject = Arriba::findObjectByName("CategorySettingsButton");
					else if (Arriba::highlightedObject == Arriba::findObjectByName("UpdaterButton")) Arriba::highlightedObject = Arriba::findObjectByName("ToggleRandomUUIDButton");
				}
				// Down pressed
				if (Arriba::Input::buttonDown(Arriba::Input::DPadDown)) {
					if (Arriba::highlightedObject == Arriba::findObjectByName("CategorySettingsButton")) Arriba::highlightedObject = Arriba::findObjectByName("ToggleRandomUUIDButton");
					else if (Arriba::highlightedObject == Arriba::findObjectByName("ToggleRandomUUIDButton") && Arriba::findObjectByName("UpdaterButton")->enabled) Arriba::highlightedObject = Arriba::findObjectByName("UpdaterButton");
				}
			}
		}
		// If one of the switcher buttons are selected
		for (size_t i = 0; i < buttons.size(); i++) {
			if (Arriba::highlightedObject == buttons[i]) {
				// If left is pressed switch to whichever list is enabled
				if (Arriba::Input::buttonDown(Arriba::Input::DPadLeft)) {
					for (size_t j = 0; j < lists.size(); j++) {
						if (lists[j]->enabled) Arriba::highlightedObject = lists[j];
					}
				} else if (Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {  // If up is pressed go to whichever button is above in the switcher
					if (buttons[i]->name == "MakerButton") Arriba::highlightedObject = selectorButton;
					else if (buttons[i]->name == "SettingsButton") Arriba::highlightedObject = makerButton;
					else if (buttons[i]->name == "ExitButton") Arriba::highlightedObject = settingsButton;
				} else if (Arriba::Input::buttonDown(Arriba::Input::DPadDown)) {  // If down button is pressed go to whichever button is below in the switcher
					if (buttons[i]->name == "SelectorButton") Arriba::highlightedObject = makerButton;
					else if (buttons[i]->name == "MakerButton") Arriba::highlightedObject = settingsButton;
					else if (buttons[i]->name == "SettingsButton") Arriba::highlightedObject = exitButton;
				}
				break;
			}
		}
	}

	void switcherPressed() {
		// Disable all the lists
		for (size_t i = 0; i < lists.size(); i++) lists[i]->enabled = false;
		// Enable a list depending on which button was pressed
		if (Arriba::highlightedObject == selectorButton) {
			selectorList->enabled = true;
			selectorPath = "sdmc:/emuiibo/amiibo";
			updateSelectorStrings();
			Arriba::Colour::neutral = Amiigo::Settings::Colour::listNeutral;
			Arriba::Colour::highlightA = Amiigo::Settings::Colour::listHighlightA;
			Arriba::Colour::highlightB = Amiigo::Settings::Colour::listHighlightB;
			updateStatusSilent(U"Amiigo + Arriba");
		} else if (Arriba::highlightedObject == makerButton) {
			makerList->enabled = true;
			if (makerIsInCategory) {
				makerIsInCategory = false;
			}
			makerList->updateStrings(seriesList);
			Arriba::Colour::neutral = Amiigo::Settings::Colour::makerNeutral;
	    	Arriba::Colour::highlightA = Amiigo::Settings::Colour::makerHighlightA;
	    	Arriba::Colour::highlightB = Amiigo::Settings::Colour::makerHighlightB;
			updateStatusSilent(U"Amiigo Store");
		} else if (Arriba::highlightedObject == settingsButton) {
			settingsScene->enabled = true;
			Arriba::Colour::neutral = Amiigo::Settings::Colour::settingsNeutral;
	    	Arriba::Colour::highlightA = Amiigo::Settings::Colour::settingsHighlightA;
	    	Arriba::Colour::highlightB = Amiigo::Settings::Colour::settingsHighlightB;
			updateStatusSilent(U"Settings");
		}
	}

	void selectorInput(int index) {
		if (selectorAmiibos[index].isCategory) {
			if (checkIfFileExists(selectorAmiibos[index].path.c_str()) || selectorAmiibos[index].path == "Favorites") {
				selectorPath = selectorAmiibos[index].path;
				updateSelectorStrings();
			} else {
				updateStatusError(U"Folder does not exist");
			}
		} else {
			std::string path;
			path += selectorAmiibos[index].path;
			emu::SetEmulationStatus(emu::EmulationStatus::On);
			Result res = emu::SetActiveVirtualAmiibo(path.c_str(), path.size());
			if R_FAILED(res) {
				updateStatusError(U"Failed to set active Amiibo");
				return;
			}
			updateStatusInfo(Arriba::Text::ASCIIToUnicode(path.c_str()));
		}
	}

	void makerInput(int index) {
		if (makerIsInCategory) {
			if (index == 0) {
				makerList->updateStrings(seriesList);
				makerIsInCategory = false;
			} else {
				createVirtualAmiibo(creatorData[index-1]);
				updateStatusInfo((U"Created " + creatorData[index-1].name).c_str());
			}
		} else {
			creatorData = getAmiibosFromSeries(seriesList[index]);
			std::vector<std::u32string> amiiboNames = {U"← Back"};
			for (size_t i = 0; i < creatorData.size(); i++) {
				amiiboNames.push_back(creatorData[i].name);
			}
			makerList->updateStrings(amiiboNames);
			selectedSeries = seriesList[index];
			makerIsInCategory = true;
		}
	}

	void updateSelectorStrings() {
		selectorAmiibos = scanForAmiibo(selectorPath.c_str());
		std::vector<std::u32string> amiiboNames;
		for (size_t i = 0; i < selectorAmiibos.size(); i++) {
			amiiboNames.push_back(selectorAmiibos[i].name);
		}
		selectorList->updateStrings(amiiboNames);
	}

	void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos) {
		if (index != -1) Arriba::Primitives::Quad* contextMenu = new Amiigo::Elements::selectorContextMenu(static_cast<int>(pos.x), static_cast<int>(pos.y), selectorAmiibos[index]);
	}

	
	void updateStatusInfo(const char32_t* text) {
		static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText(text);
		static_cast<Arriba::Primitives::Quad*>(Arriba::findObjectByName("StatusBar"))->setColour({1,1,1,1});
	}

	void updateStatusError(const char32_t* text) {
		static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText(text);
		static_cast<Arriba::Primitives::Quad*>(Arriba::findObjectByName("StatusBar"))->setColour({1,0,0,1});
	}

	void updateStatusSilent(const char32_t* text) {
		static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText(text);
	}
}  // namespace Amiigo::UI
