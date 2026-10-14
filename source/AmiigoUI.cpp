#include <AmiigoUI.h>
#include <AmiigoUISelector.h>
#include <AmiigoUIMaker.h>
#include <AmiigoUISettings.h>
#include <arriba.h>
#include <utils.h>

#include <cmath>
#include <thread>

#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <emuiibo.hpp>
#include <Networking.h>
#include <AmiigoSettings.h>
#include <AmiigoElements.h>
#include <AmiigoBehaviours.h>
#include <AmiigoLang.h>

namespace Amiigo::UI {
	void initBG() {
		Arriba::Primitives::Quad* bg = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
		bg->setName("AmiigoBG");
		Amiigo::AmiigoBehaviours::BGBehaviour* bgBehaviour = new Amiigo::AmiigoBehaviours::BGBehaviour();
		bg->addBehaviour(bgBehaviour);
		bgBehaviour->init();
	}

	void initUI() {
		Amiigo::Lang::init();
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
			Arriba::findObjectByName("AmiigoBG")->renderer->shader.setFloat1("iTime", Arriba::time);
			if (!hasNetworkConnection()) doingText->setText(Amiigo::Lang::get("splash_waiting_internet").c_str());
			else if (!checkIfFileExists("sdmc:/config/amiigo/API.json")) doingText->setText(Amiigo::Lang::get("splash_caching_api").c_str());
			else if (!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) doingText->setText(Amiigo::Lang::get("splash_installing_emuiibo").c_str());
			else if (checkIfFileExists("sdmc:/config/amiigo/update.flag")) doingText->setText(Amiigo::Lang::get("splash_updating_amiigo").c_str());
			Arriba::drawFrame();
			svcSleepThread(1'000'000'000 / 60);
		}
		initThread.join();
		splashScene->destroy();
		if (settingsScene) settingsScene->enabled = true;
		if (sceneSwitcher) sceneSwitcher->enabled = true;
		if (statusBar) statusBar->enabled = true;
		Arriba::activeLayer--;

		if (emuiiboJustInstalled) {
			new Amiigo::Elements::ConfirmDialog(
				(Arriba::Graphics::windowWidth - Amiigo::Elements::ConfirmDialog::DIALOG_W) / 2,
				(Arriba::Graphics::windowHeight - Amiigo::Elements::ConfirmDialog::DIALOG_H) / 2,
				Amiigo::Lang::get("emuiibo_installed_message").c_str(),
				[]() { bpcInitialize(); bpcRebootSystem(); bpcExit(); },
				[]() { Amiigo::UI::isRunning = 0; },
				Amiigo::Lang::get("emuiibo_installed_reboot").c_str(),
				Amiigo::Lang::get("emuiibo_installed_exit").c_str()
			);
		}
	}

	void applySettingsQuadStyle(Arriba::Primitives::Quad* quad) {
		quad->renderer->shader.setProgID(Arriba::Graphics::createSharedShader("dialog", "romfs:/VertexDefault.glsl", "romfs:/dialogFragment.glsl"));
		quad->renderer->shader.setFloat1("aspectRatio", quad->width / (float)quad->height);
		quad->renderer->shader.setFloat1("radius", 20.0f / quad->height);
		quad->renderer->shader.setFloat1("outlineWidth", 2.0f / quad->height);
	}

	void initSceneSwitcher() {
		int buttonDivX = Arriba::Graphics::windowWidth - switcherWidth;
		auto makeDivider = [](int x, int y, int w, int h) {
			auto* div = new Arriba::Primitives::Quad(x, y, w, h, Arriba::Graphics::Pivot::topLeft);
			div->setColour({0, 0, 0, 1});
		};
		makeDivider(Arriba::Graphics::windowWidth - switcherWidth - 1, statusHeight, 5, switcherHeight);
		makeDivider(0, statusHeight - 1, Arriba::Graphics::windowWidth, 5);
		makeDivider(buttonDivX, statusHeight + (switcherHeight/4) - 1, switcherWidth, 5);
		makeDivider(buttonDivX, statusHeight + (switcherHeight/4)*2 - 1, switcherWidth, 5);
		makeDivider(buttonDivX, statusHeight + (switcherHeight/4)*3 - 1, switcherWidth, 5);

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
		selectorButton->setText(Amiigo::Lang::get("nav_my_amiibo").c_str());
		selectorButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		selectorButton->setParent(sceneSwitcher);
		selectorButton->setName("SelectorButton");
		selectorButton->setTag("SwitcherButton");
		selectorButton->registerCallback(switcherPressed);

		// Amiigo store button
		Arriba::Elements::Button* storeButton = new Arriba::Elements::Button();
		storeButton->setText(Amiigo::Lang::get("nav_amiigo_store").c_str());
		storeButton->transform.position.y = selectorButton->height + 1;
		storeButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		storeButton->setParent(sceneSwitcher);
		storeButton->setName("MakerButton");
		storeButton->setTag("SwitcherButton");
		storeButton->registerCallback(switcherPressed);

		// Settings button
		Arriba::Elements::Button* settingsButton = new Arriba::Elements::Button();
		settingsButton->setText(Amiigo::Lang::get("nav_settings").c_str());
		settingsButton->transform.position.y = storeButton->height + storeButton->transform.position.y + 1;
		settingsButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		settingsButton->setParent(sceneSwitcher);
		settingsButton->setName("SettingsButton");
		settingsButton->setTag("SwitcherButton");
		settingsButton->registerCallback(switcherPressed);

		// Exit button
		Arriba::Elements::Button* exitButton = new Arriba::Elements::Button();
		exitButton->setText(Amiigo::Lang::get("nav_exit").c_str());
		exitButton->transform.position.y = settingsButton->height + settingsButton->transform.position.y + 1;
		exitButton->setDimensions(switcherWidth, switcherHeight/4, Arriba::Graphics::Pivot::topRight);
		exitButton->setParent(sceneSwitcher);
		exitButton->setName("ExitButton");
		exitButton->setTag("SwitcherButton");
		exitButton->registerCallback([](){isRunning = 0;});
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
			resetSelectorToRoot();
			Arriba::Colour::neutral = Amiigo::Settings::Colour::listNeutral;
			Arriba::Colour::highlightA = Amiigo::Settings::Colour::listHighlightA;
			Arriba::Colour::highlightB = Amiigo::Settings::Colour::listHighlightB;
			updateStatus(U"Amiigo + Arriba", StatusLevel::Silent);
		} else if (Arriba::highlightedObject == Arriba::findObjectByName("MakerButton")) {
			Arriba::findObjectByName("MakerList")->enabled = true;
			resetMakerToRoot();
			Arriba::Colour::neutral = Amiigo::Settings::Colour::makerNeutral;
			Arriba::Colour::highlightA = Amiigo::Settings::Colour::makerHighlightA;
			Arriba::Colour::highlightB = Amiigo::Settings::Colour::makerHighlightB;
			updateStatus(Amiigo::Lang::get("nav_amiigo_store").c_str(), StatusLevel::Silent);
		} else if (Arriba::highlightedObject == Arriba::findObjectByName("SettingsButton")) {
			Arriba::findObjectByName("SettingsScene")->enabled = true;
			resetSettingsToTopLevel();
			Arriba::Colour::neutral = Amiigo::Settings::Colour::settingsNeutral;
			Arriba::Colour::highlightA = Amiigo::Settings::Colour::settingsHighlightA;
			Arriba::Colour::highlightB = Amiigo::Settings::Colour::settingsHighlightB;
			updateStatus(Amiigo::Lang::get("nav_settings").c_str(), StatusLevel::Silent);
		}
	}

	void updateStatus(const char32_t* text, StatusLevel level) {
		Arriba::findObjectByName<Arriba::Primitives::Text>("StatusBarText")->setText(text);
		switch (level) {
			case StatusLevel::Info:   Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,1,1,1}); break;
			case StatusLevel::Error:  Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,0,0,1}); break;
			case StatusLevel::Silent: break;
		}
	}
}  // namespace Amiigo::UI
