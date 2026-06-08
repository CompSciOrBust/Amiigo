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

namespace {
    std::vector<std::string> seriesList;
    std::vector<AmiiboCreatorData> creatorData;
    bool makerIsInCategory = false;
    std::vector<AmiiboEntry> selectorAmiibos;
    std::string selectorPath = "sdmc:/emuiibo/amiibo";
}

namespace Amiigo::UI {
	void initBG() {
		Arriba::Primitives::Quad* bg = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
		bg->setName("AmiigoBG");
		Amiigo::AmiigoBehaviours::BGBehaviour* bgBehaviour = new Amiigo::AmiigoBehaviours::BGBehaviour();
		bg->addBehaviour(bgBehaviour);
		bgBehaviour->init();
	}

	void initUI() {
		initBG();
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
		if (checkForUpdates()) {
			Arriba::findObjectByName<Arriba::Elements::Button>("SettingsButton")->setText(U"Update");
			Arriba::findObjectByName("UpdaterButton")->enabled = true;
		}
	}

	void initSplash() {
		Arriba::activeLayer++;
		Arriba::Primitives::Quad* splashScene = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
		splashScene->setColour({0.25, 0.25, 0.25, 0.95});
		
		Arriba::Primitives::Text* titleText = new Arriba::Primitives::Text(U"Amiigo", 128);
		titleText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 140, 0};
		titleText->setColour({1, 1, 1, 1});
		titleText->setParent(splashScene);
		
		Arriba::Primitives::Text* byText = new Arriba::Primitives::Text(U"by CompSciOrBust", 64);
		byText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 15, 0};
		byText->setColour({1, 1, 1, 1});
		byText->setParent(splashScene);
		
		Arriba::Primitives::Text* doingText = new Arriba::Primitives::Text("", 48);
		doingText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 + 160, 0};
		doingText->setColour({1, 1, 1, 1});
		doingText->setParent(splashScene);
		
		std::thread initThread(firstTimeSetup);

		Arriba::UIObject* sceneSwitcher = Arriba::findObjectByName("SceneSwitcher");
		Arriba::UIObject* settingsScene = Arriba::findObjectByName("SettingsScene");
		Arriba::UIObject* statusBar = Arriba::findObjectByName("StatusBar");
		if (settingsScene) settingsScene->enabled = false;
		if (sceneSwitcher) sceneSwitcher->enabled = false;
		if (statusBar) statusBar->enabled = false;
		while (!checkIfFileExists("sdmc:/config/amiigo/API.json") || !checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp") || checkIfFileExists("sdmc:/config/amiigo/update.flag")) {
			splashScene->setColour({(sin(Arriba::time)+1)/4, (cos(Arriba::time)+1)/4, 0.5, 0.95});
			Arriba::findObjectByName("AmiigoBG")->renderer->thisShader.setFloat1("iTime", Arriba::time);
			if (!hasNetworkConnection()) doingText->setText(U"Waiting for internet connection...");
			else if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) doingText->setText(U"Caching API data...");
			else if (!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) doingText->setText(U"Installing Emuiibo...");
			else if (checkIfFileExists("sdmc:/config/amiigo/update.flag")) doingText->setText(U"Updating Amiigo...");
			Arriba::drawFrame();
			svcSleepThread(1'000'000'000 / 60);
		}
		initThread.join();
		splashScene->destroy();
		if (settingsScene) settingsScene->enabled = true;
		if (sceneSwitcher) sceneSwitcher->enabled = true;
		if (statusBar) statusBar->enabled = true;
		Arriba::activeLayer--;
	}

	void initSceneSwitcher() {
		// Divider quads
		int buttonDivX = Arriba::Graphics::windowWidth - switcherWidth;
		Arriba::Primitives::Quad* div1 = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth - switcherWidth - 1, statusHeight, 5, switcherHeight, Arriba::Graphics::Pivot::topLeft);
		div1->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div2 = new Arriba::Primitives::Quad(0, statusHeight - 1, Arriba::Graphics::windowWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div2->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div3 = new Arriba::Primitives::Quad(buttonDivX, statusHeight + (switcherHeight/4) - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div3->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div4 = new Arriba::Primitives::Quad(buttonDivX, statusHeight + (switcherHeight/4)*2 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div4->setColour({0, 0, 0, 1});
		Arriba::Primitives::Quad* div5 = new Arriba::Primitives::Quad(buttonDivX, statusHeight + (switcherHeight/4)*3 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div5->setColour({0, 0, 0, 1});

		// Status bar
		Arriba::Primitives::Quad* statusBar = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, statusHeight - 1, Arriba::Graphics::Pivot::topLeft);
		Amiigo::AmiigoBehaviours::StatusBarColourBehaviour* statusBarColourBehaviour = new Amiigo::AmiigoBehaviours::StatusBarColourBehaviour();
		statusBar->setColour(Amiigo::Settings::Colour::statusBar);
		statusBar->addBehaviour(statusBarColourBehaviour);
		statusBar->setName("StatusBar");
		Arriba::Primitives::Text* statusText = new Arriba::Primitives::Text(U"Amiigo + Arriba", 34);
		statusText->setName("StatusBarText");
		statusText->setDimensions(statusText->width, statusText->height, Arriba::Graphics::centre);
		statusText->transform.position = {statusBar->width/2, statusBar->height/2, 0};
		statusText->setParent(statusBar);

		// Switcher holder quad
		Arriba::Primitives::Quad* sceneSwitcher = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth, statusHeight, switcherWidth, switcherHeight, Arriba::Graphics::Pivot::topRight);
		sceneSwitcher->setName("SceneSwitcher");
		sceneSwitcher->setColour({0, 0, 0, 0});

		// Amiibo list button
		Arriba::Elements::Button* selectorButton = new Arriba::Elements::Button();
		selectorButton->setText(U"My Amiibo");
		selectorButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		selectorButton->setParent(sceneSwitcher);
		selectorButton->setName("SelectorButton");
		selectorButton->setTag("SwitcherButton");
		selectorButton->registerCallback(switcherPressed);

		// Amiigo store button
		Arriba::Elements::Button* storeButton = new Arriba::Elements::Button();
		storeButton->setText(U"Amiigo Store");
		storeButton->transform.position.y = selectorButton->height + 1;
		storeButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		storeButton->setParent(sceneSwitcher);
		storeButton->setName("MakerButton");
		storeButton->setTag("SwitcherButton");
		storeButton->registerCallback(switcherPressed);

		// Settings button
		Arriba::Elements::Button* settingsButton = new Arriba::Elements::Button();
		settingsButton->setText(U"Settings");
		settingsButton->transform.position.y = storeButton->height + storeButton->transform.position.y + 1;
		settingsButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		settingsButton->setParent(sceneSwitcher);
		settingsButton->setName("SettingsButton");
		settingsButton->setTag("SwitcherButton");
		settingsButton->registerCallback(switcherPressed);

		// Exit button
		Arriba::Elements::Button* exitButton = new Arriba::Elements::Button();
		exitButton->setText(U"Exit");
		exitButton->transform.position.y = settingsButton->height + settingsButton->transform.position.y + 1;
		exitButton->setDimensions(switcherWidth, switcherHeight/4, Arriba::Graphics::Pivot::topRight);
		exitButton->setParent(sceneSwitcher);
		exitButton->setName("ExitButton");
		exitButton->setTag("SwitcherButton");
		exitButton->registerCallback([](){isRunning = 0;});
	}

	void initSelector() {
	    Arriba::Elements::InertialList* selectorList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, std::vector<std::string>{});
		selectorList->setName("SelectorList");
		selectorList->setTag("List");
		selectorList->registerCallback(selectorInput);
		selectorList->registerAltCallback(selectorContextMenuSpawner);
		updateSelectorStrings();
	}

	void initMaker() {
		seriesList = getListOfSeries();
	    Arriba::Elements::InertialList* makerList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, seriesList);
		makerList->registerCallback(makerInput);
		makerList->setName("MakerList");
		makerList->setTag("List");
		makerList->enabled = false;
	}

	const char32_t* getCategoryModeLabel() {
		switch (Amiigo::Settings::categoryMode) {
			case Amiigo::Settings::categoryModes::saveToRoot:
			return U"Save to Game Name";

			case Amiigo::Settings::categoryModes::saveByGameName:
			return U"Save to Amiibo Series";

			case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
			return U"Save to Current Folder";

			case Amiigo::Settings::categoryModes::saveByCurrentFolder:
			return U"Save to Root";

			default:
			return U"Error";
		}
	}

	void initSettings() {
		const int buttonHeight = 100;
		const int buttonOffsets = 5;
		Arriba::Primitives::Quad* settingsScene = new Arriba::Primitives::Quad(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		settingsScene->setName("SettingsScene");
		settingsScene->setTag("List"); // Not a list but pretending makes scene switching easier
		settingsScene->enabled = false;
		settingsScene->setColour({0, 0, 0, 0});

		Arriba::Elements::Button* categoryButton = new Arriba::Elements::Button();
		categoryButton->setParent(settingsScene);
		categoryButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		categoryButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 1/buttonOffsets, 0};
		categoryButton->setText(getCategoryModeLabel());
		categoryButton->setName("CategorySettingsButton");
		categoryButton->setTag("SettingsButton");

		categoryButton->registerCallback([](){
			Amiigo::Settings::categoryMode = (Amiigo::Settings::categoryMode+1) % Amiigo::Settings::categoryModes::categoryCount;
			Amiigo::Settings::saveSettings();
			Arriba::findObjectByName<Arriba::Elements::Button>("CategorySettingsButton")->setText(getCategoryModeLabel());
			switch (Amiigo::Settings::categoryMode) {
				case Amiigo::Settings::categoryModes::saveToRoot:
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/amiibo");
				break;
			
				case Amiigo::Settings::categoryModes::saveByGameName:
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/game name");
				break;

				case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
				updateStatusInfo(U"Amiibos will save to sdmc:/emuiibo/amiibo series");
				break;

				case Amiigo::Settings::categoryModes::saveByCurrentFolder:
				updateStatusInfo(U"Amiibos will save to the current location");
				break;

				default:
				Arriba::findObjectByName<Arriba::Elements::Button>("CategorySettingsButton")->setText(U"Error");
				updateStatusError(U"Error, uknown category mode");
				break;
			}
		});

		Arriba::Elements::Button* randomUUIDButton = new Arriba::Elements::Button();
		randomUUIDButton->setParent(settingsScene);
		randomUUIDButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		randomUUIDButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 2/buttonOffsets, 0};
		if (Amiigo::Settings::useRandomisedUUID) randomUUIDButton->setText(U"Disable Random UUID");
		else randomUUIDButton->setText(U"Enable Random UUID");
		randomUUIDButton->setName("ToggleRandomUUIDButton");
		randomUUIDButton->setTag("SettingsButton");

		randomUUIDButton->registerCallback([](){
			Amiigo::Settings::useRandomisedUUID = !Amiigo::Settings::useRandomisedUUID;
			Amiigo::Settings::saveSettings();
			auto* uuidBtn = Arriba::findObjectByName<Arriba::Elements::Button>("ToggleRandomUUIDButton");
			if (Amiigo::Settings::useRandomisedUUID) {
				uuidBtn->setText(U"Disable random UUID");
				updateStatusInfo(U"Amiibos will now generate with random UUIDs");
			} else {
				uuidBtn->setText(U"Enable random UUID");
				updateStatusInfo(U"Amiibos will now generate with static UUIDs");
			}
		});

		Arriba::Elements::Button* cacheUpdateButton = new Arriba::Elements::Button();
		cacheUpdateButton->setParent(settingsScene);
		cacheUpdateButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		cacheUpdateButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 3/buttonOffsets, 0};
		cacheUpdateButton->setText(U"Update API Cache");
		cacheUpdateButton->setName("CacheUpdateButton");
		cacheUpdateButton->setTag("SettingsButton");
		cacheUpdateButton->enabled = hasNetworkConnection();

		cacheUpdateButton->registerCallback([]() {
			remove("sdmc:/config/amiigo/API.json");
			initSplash();
		});

		Arriba::Elements::Button* updaterButton = new Arriba::Elements::Button();
		updaterButton->setParent(settingsScene);
		updaterButton->setDimensions(550, buttonHeight, Arriba::Graphics::Pivot::centre);
		updaterButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 4/buttonOffsets, 0};
		updaterButton->setText(U"Update Amiigo");
		updaterButton->setName("UpdaterButton");
		updaterButton->setTag("SettingsButton");
		updaterButton->enabled = false;

		updaterButton->registerCallback([](){
			if (!hasNetworkConnection()) {
				updateStatusError(U"No network connection!");
			} else {
				std::ofstream fileStream("sdmc:/config/amiigo/update.flag");
				fileStream.close();
				initSplash();
			}
		});

		Arriba::Primitives::Quad* creditsQuad = new Arriba::Primitives::Quad(0, 0, 330, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		creditsQuad->setParent(settingsScene);
		creditsQuad->setColour({0, 0, 0, 0.9});
		int yOffset = 0;

		char emuVer[12];
		emu::Version emuiiboVersion = emu::GetVersion();
		sprintf(emuVer, "%u.%u.%u", emuiiboVersion.major, emuiiboVersion.minor, emuiiboVersion.micro);

		Arriba::Primitives::Text* creditsTitleText = new Arriba::Primitives::Text(U"Credits", 64);
		creditsTitleText->setColour({0, 0.7, 1, 1});
		creditsTitleText->setParent(creditsQuad);
		creditsTitleText->transform.position = {creditsQuad->width/2, yOffset += creditsTitleText->height + 30, 0};
		struct Credit { std::u32string title; std::u32string name; };
		const std::u32string emuiiboTitle = U"Emuiibo " + Arriba::Text::ASCIIToUnicode(emuVer);
		const Credit credits[] = {
			{U"Developer",   U"CompSciOrBust"},
			{emuiiboTitle,   U"XorTroll"},
			{U"Contribuyente", U"Kronos2308"},
			{U"The Pizza Guy", U"Za"},
			{U"Amiibo API",  U"N3evin"},
		};
		for (const auto& credit : credits) {
			Arriba::Primitives::Text* titleTextObject = new Arriba::Primitives::Text(credit.title.c_str(), 38);
			titleTextObject->setParent(creditsQuad);
			titleTextObject->transform.position = {creditsQuad->width/2, yOffset += titleTextObject->height + 20, 0};
			Arriba::Primitives::Text* nameTextObject = new Arriba::Primitives::Text(credit.name.c_str(), 28);
			nameTextObject->setParent(creditsQuad);
			nameTextObject->transform.position = {creditsQuad->width/2, yOffset += nameTextObject->height + 10, 0};
			titleTextObject->setColour({0, 0.7, 1, 1});
			nameTextObject->setColour({0, 0.7, 1, 1});
		}
	}

	void handleSelectorInput() {
		if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && selectorPath != "sdmc:/emuiibo/amiibo") {
			selectorPath = selectorPath.substr(0, selectorPath.find_last_of("/"));
			if (selectorPath.length() < strlen("sdmc:/emuiibo/amiibo")) selectorPath = "sdmc:/emuiibo/amiibo";
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
	}

	void handleMakerInput() {
		if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && makerIsInCategory) {
			Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(seriesList);
			makerIsInCategory = false;
		}
	}

	void handleSettingsInput() {
		if (Arriba::highlightedObject == Arriba::findObjectByName("SettingsScene")) Arriba::highlightedObject = Arriba::findObjectByName("CategorySettingsButton");
		if (Arriba::Input::buttonDown(Arriba::Input::DPadRight) && Arriba::highlightedObject->getTag() != "SwitcherButton") Arriba::highlightedObject = Arriba::findObjectByName("SelectorButton");
		int direction = 0;
		if (Arriba::Input::buttonDown(Arriba::Input::DPadUp)) direction -= 1;
		if (Arriba::Input::buttonDown(Arriba::Input::DPadDown)) direction += 1;
		if (direction == 0) return;
		std::vector<Arriba::UIObject*> settingsButtons = Arriba::findObjectsByTag("SettingsButton");
		int buttonCount = (int)settingsButtons.size();
		for (size_t i = 0; i < settingsButtons.size(); i++) {
			if (settingsButtons[i] == Arriba::highlightedObject) {
				int buttonIndex = i;
				do {
					buttonIndex = (buttonIndex + direction + buttonCount) % buttonCount;
				} while (!settingsButtons[buttonIndex]->enabled);
				Arriba::highlightedObject = settingsButtons[buttonIndex];
				break;
			}
		}
	}

	void handleSwitcherInput(const std::vector<Arriba::UIObject*>& lists) {
		std::vector<Arriba::UIObject*> buttons = Arriba::findObjectsByTag("SwitcherButton");
		for (size_t i = 0; i < buttons.size(); i++) {
			if (Arriba::highlightedObject != buttons[i]) continue;
			if (Arriba::Input::buttonDown(Arriba::Input::DPadLeft)) {
				for (size_t j = 0; j < lists.size(); j++) {
					if (lists[j]->enabled) Arriba::highlightedObject = lists[j];
				}
			} else if (Arriba::Input::buttonDown(Arriba::Input::DPadUp) && i > 0) {
				Arriba::highlightedObject = buttons[i-1];
			} else if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i < buttons.size()-1) {
				Arriba::highlightedObject = buttons[i+1];
			}
			break;
		}
	}

	void handleInput() {
		if (Arriba::activeLayer != 0) return;
		if (Arriba::highlightedObject == nullptr) {
			if (Arriba::Input::buttonDown(Arriba::Input::controllerButton(Arriba::Input::DPadRight | Arriba::Input::DPadLeft | Arriba::Input::DPadUp | Arriba::Input::DPadDown))) Arriba::highlightedObject = Arriba::findObjectByName("SelectorButton");
		}
		std::vector<Arriba::UIObject*> lists = Arriba::findObjectsByTag("List");
		for (size_t i = 0; i < lists.size(); i++) {
			if (Arriba::highlightedObject == lists[i] && Arriba::Input::buttonDown(Arriba::Input::DPadRight)) {
				Arriba::highlightedObject = Arriba::findObjectByName("SelectorButton");
				return;
			}
			if      (lists[i]->getName() == "SelectorList"  && lists[i]->enabled) handleSelectorInput();
			else if (lists[i]->getName() == "MakerList"     && lists[i]->enabled) handleMakerInput();
			else if (lists[i]->getName() == "SettingsScene" && lists[i]->enabled) handleSettingsInput();
		}
		handleSwitcherInput(lists);
	}

	void switcherPressed() {
		std::vector<Arriba::UIObject*> lists = Arriba::findObjectsByTag("List");
		for (size_t i = 0; i < lists.size(); i++) lists[i]->enabled = false;

		if (Arriba::highlightedObject == Arriba::findObjectByName("SelectorButton")) {
			Arriba::findObjectByName("SelectorList")->enabled = true;
			selectorPath = "sdmc:/emuiibo/amiibo";
			updateSelectorStrings();
			Arriba::Colour::neutral = Amiigo::Settings::Colour::listNeutral;
			Arriba::Colour::highlightA = Amiigo::Settings::Colour::listHighlightA;
			Arriba::Colour::highlightB = Amiigo::Settings::Colour::listHighlightB;
			updateStatusSilent(U"Amiigo + Arriba");
		} else if (Arriba::highlightedObject == Arriba::findObjectByName("MakerButton")) {
			Arriba::Elements::InertialList* makerList = Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList");
			makerList->enabled = true;
			makerIsInCategory = false;
			makerList->updateStrings(seriesList);
			Arriba::Colour::neutral = Amiigo::Settings::Colour::makerNeutral;
	    	Arriba::Colour::highlightA = Amiigo::Settings::Colour::makerHighlightA;
	    	Arriba::Colour::highlightB = Amiigo::Settings::Colour::makerHighlightB;
			updateStatusSilent(U"Amiigo Store");
		} else if (Arriba::highlightedObject == Arriba::findObjectByName("SettingsButton")) {
			Arriba::findObjectByName("SettingsScene")->enabled = true;
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
			std::string path = selectorAmiibos[index].path;
			emu::SetEmulationStatus(emu::EmulationStatus::On);
			Result res = emu::SetActiveVirtualAmiibo(path.c_str(), path.size());
			if R_FAILED(res) {
				updateStatusError(U"Failed to set active Amiibo");
				return;
			}
			
			updateStatusInfo(Arriba::Text::ASCIIToUnicode(path.c_str()).c_str());
			Arriba::UIObject* amiiboPreview = Arriba::findObjectByName("AmiiboPreview");
			if (amiiboPreview) amiiboPreview->destroy();
			amiiboPreview = new Amiigo::Elements::AmiiboPreview(path);
			amiiboPreview->setParent(Arriba::findObjectByName("SelectorList"));
			amiiboPreview->setName("AmiiboPreview");
		}
	}

	void makerInput(int index) {
		if (makerIsInCategory) {
			if (index == 0) {
				Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(seriesList);
				makerIsInCategory = false;
			} else {
				createVirtualAmiibo(creatorData[index-1]);
				updateStatusInfo((U"Created " + creatorData[index-1].name).c_str());
			}
		} else {
			creatorData = getAmiibosFromSeries(seriesList[index]);
			std::vector<std::u32string> amiiboNames = {U"← Back"};
			for (const auto& data : creatorData) amiiboNames.push_back(data.name);
			Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(amiiboNames);
			makerIsInCategory = true;
		}
	}

	void updateSelectorStrings() {
		selectorAmiibos = scanForAmiibo(selectorPath.c_str());
		std::vector<std::u32string> amiiboNames;
		for (const auto& amiibo : selectorAmiibos) amiiboNames.push_back(amiibo.name);
		Arriba::findObjectByName<Arriba::Elements::InertialList>("SelectorList")->updateStrings(amiiboNames);
	}

	void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos) {
		if (index != -1) new Amiigo::Elements::SelectorContextMenu(static_cast<int>(pos.x), static_cast<int>(pos.y), selectorAmiibos[index]);
	}

	void updateStatusInfo(const char32_t* text) {
		Arriba::findObjectByName<Arriba::Primitives::Text>("StatusBarText")->setText(text);
		Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,1,1,1});
	}

	void updateStatusError(const char32_t* text) {
		Arriba::findObjectByName<Arriba::Primitives::Text>("StatusBarText")->setText(text);
		Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,0,0,1});
	}

	void updateStatusSilent(const char32_t* text) {
		Arriba::findObjectByName<Arriba::Primitives::Text>("StatusBarText")->setText(text);
	}

	const std::string& getSelectorPath() {
		return selectorPath;
	}
}  // namespace Amiigo::UI
