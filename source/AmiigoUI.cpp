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
#include <AmiigoLang.h>

namespace {
    std::vector<std::string> seriesList;
    std::vector<AmiiboCreatorData> creatorData;
    bool makerIsInCategory = false;
    std::vector<AmiiboEntry> selectorAmiibos;
    std::string selectorPath = "sdmc:/emuiibo/amiibo";
    enum class SettingsView { TopLevel, AmiiboSettings, Updates, ThemeHub, ThemeStatusBar, ThemeList, ThemeStore, ThemeSection };
    SettingsView settingsView = SettingsView::TopLevel;
    bool settingsUpdateAvailable = false;

    struct ThemeSwatch { colour* colourPtr; Arriba::Primitives::Quad* quad; };
    std::vector<ThemeSwatch> themeSwatches;

    std::vector<Arriba::Primitives::Quad*> settingsPaneRegistry;

    struct ThemeSubPane {
        SettingsView view;
        SettingsView parentView;
        const char* paneName;
        std::vector<Arriba::UIObject*> buttons;
    };
    std::vector<ThemeSubPane> themeSubPanes;
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
		if (checkForUpdates()) {
			Arriba::findObjectByName<Arriba::Elements::Button>("SettingsButton")->setText(Amiigo::Lang::get("nav_update").c_str());
			settingsUpdateAvailable = true;
		}
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
			Arriba::findObjectByName("AmiigoBG")->renderer->thisShader.setFloat1("iTime", Arriba::time);
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
			return Amiigo::Lang::get("category_save_to_root").c_str();

			case Amiigo::Settings::categoryModes::saveByGameName:
			return Amiigo::Lang::get("category_save_to_game_name").c_str();

			case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
			return Amiigo::Lang::get("category_save_to_amiibo_series").c_str();

			case Amiigo::Settings::categoryModes::saveByCurrentFolder:
			return Amiigo::Lang::get("category_save_to_current_folder").c_str();

			default:
			return Amiigo::Lang::get("error_generic").c_str();
		}
	}

	void disableAllSettingsElements() {
		for (auto* btn : Arriba::findObjectsByTag("SettingsButton")) btn->enabled = false;
		for (auto* pane : settingsPaneRegistry) pane->enabled = false;
		Arriba::findObjectByName("SettingsCredits")->enabled = false;
	}

	void resetSettingsToTopLevel() {
		disableAllSettingsElements();
		settingsView = SettingsView::TopLevel;
		Arriba::findObjectByName("SettingsTopAmiiboButton")->enabled = true;
		Arriba::findObjectByName("SettingsTopUpdateButton")->enabled = true;
		Arriba::findObjectByName("SettingsTopThemeButton")->enabled = true;
		Arriba::findObjectByName("SettingsCredits")->enabled = true;
	}

	void goToSettingsTopLevel() {
		resetSettingsToTopLevel();
		Arriba::highlightedObject = Arriba::findObjectByName("SettingsTopAmiiboButton");
	}

	void goToAmiiboSubmenu() {
		disableAllSettingsElements();
		settingsView = SettingsView::AmiiboSettings;
		Arriba::findObjectByName("SettingsAmiiboPane")->enabled = true;
		Arriba::findObjectByName("CategorySettingsButton")->enabled = true;
		Arriba::findObjectByName("RandomUUIDCheckBox")->enabled = true;
		Arriba::findObjectByName("SaveAmiiboImagesCheckBox")->enabled = true;
		Arriba::findObjectByName("SettingsAmiiboBackButton")->enabled = true;
		Arriba::highlightedObject = Arriba::findObjectByName("SettingsAmiiboPane");
	}

	void goToUpdatesSubmenu() {
		disableAllSettingsElements();
		settingsView = SettingsView::Updates;
		Arriba::findObjectByName("SettingsUpdatePane")->enabled = true;
		Arriba::findObjectByName("CacheUpdateButton")->enabled = hasNetworkConnection();
		Arriba::findObjectByName("UpdaterButton")->enabled = settingsUpdateAvailable;
		Arriba::findObjectByName("ReinstallEmuiiboButton")->enabled = hasNetworkConnection();
		Arriba::findObjectByName("SettingsUpdateBackButton")->enabled = true;
		Arriba::highlightedObject = Arriba::findObjectByName("SettingsUpdatePane");
	}

	void activateThemePane(SettingsView view) {
		disableAllSettingsElements();
		settingsView = view;
		for (auto& subPane : themeSubPanes) {
			if (subPane.view != view) continue;
			Arriba::findObjectByName(subPane.paneName)->enabled = true;
			for (auto* btn : subPane.buttons) btn->enabled = true;
			for (auto& swatch : themeSwatches) swatch.quad->setColour(*swatch.colourPtr);
			Arriba::highlightedObject = Arriba::findObjectByName(subPane.paneName);
			return;
		}
	}

	void goToThemeHub() { activateThemePane(SettingsView::ThemeHub); }

	void initSettings() {
		const int buttonHeight = 100;
		const int topButtonWidth = 550;
		const int subButtonWidth = 700;
		settingsPaneRegistry.clear();
		themeSubPanes.clear();
		themeSwatches.clear();
		Arriba::Primitives::Quad* settingsScene = new Arriba::Primitives::Quad(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		settingsScene->setName("SettingsScene");
		settingsScene->setTag("List");
		settingsScene->enabled = false;
		settingsScene->setColour({0, 0, 0, 0});

		Arriba::Primitives::Quad* creditsQuad = new Arriba::Primitives::Quad(0, 0, 330, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		creditsQuad->setParent(settingsScene);
		creditsQuad->setColour({0, 0, 0, 0.9});
		creditsQuad->setName("SettingsCredits");
		int yOffset = 0;

		char emuVer[12];
		emu::Version emuiiboVersion = emu::GetVersion();
		sprintf(emuVer, "%u.%u.%u", emuiiboVersion.major, emuiiboVersion.minor, emuiiboVersion.micro);

		Arriba::Primitives::Text* creditsTitleText = new Arriba::Primitives::Text(Amiigo::Lang::get("credits_title").c_str(), 64);
		creditsTitleText->setColour({0, 0.7, 1, 1});
		creditsTitleText->setParent(creditsQuad);
		creditsTitleText->transform.position = {creditsQuad->width/2, yOffset += creditsTitleText->height + 30, 0};
		
		struct Credit { std::u32string title; std::u32string name; };
		const std::u32string emuiiboTitle = U"Emuiibo " + Arriba::Text::ASCIIToUnicode(emuVer);
		const Credit credits[] = {
			{Amiigo::Lang::get("credits_developer"),   U"CompSciOrBust"},
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

		int topCenterX = settingsScene->width / 2 + 165;

		Arriba::Elements::Button* topAmiiboButton = new Arriba::Elements::Button();
		topAmiiboButton->setParent(settingsScene);
		topAmiiboButton->setDimensions(topButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		topAmiiboButton->transform.position = {topCenterX, settingsScene->height * 1/4, 0};
		topAmiiboButton->setText(Amiigo::Lang::get("settings_amiibo_settings").c_str());
		topAmiiboButton->setName("SettingsTopAmiiboButton");
		topAmiiboButton->setTag("SettingsButton");
		topAmiiboButton->registerCallback(goToAmiiboSubmenu);

		Arriba::Elements::Button* topThemeButton = new Arriba::Elements::Button();
		topThemeButton->setParent(settingsScene);
		topThemeButton->setDimensions(topButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		topThemeButton->transform.position = {topCenterX, settingsScene->height * 2/4, 0};
		topThemeButton->setText(Amiigo::Lang::get("settings_theme").c_str());
		topThemeButton->setName("SettingsTopThemeButton");
		topThemeButton->setTag("SettingsButton");
		topThemeButton->registerCallback(goToThemeHub);

		Arriba::Elements::Button* topUpdateButton = new Arriba::Elements::Button();
		topUpdateButton->setParent(settingsScene);
		topUpdateButton->setDimensions(topButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		topUpdateButton->transform.position = {topCenterX, settingsScene->height * 3/4, 0};
		topUpdateButton->setText(Amiigo::Lang::get("settings_updates").c_str());
		topUpdateButton->setName("SettingsTopUpdateButton");
		topUpdateButton->setTag("SettingsButton");
		topUpdateButton->registerCallback(goToUpdatesSubmenu);

		Arriba::Primitives::Quad* amiiboPane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		amiiboPane->setParent(settingsScene);
		amiiboPane->setColour({0, 0, 0, 0.7});
		amiiboPane->setName("SettingsAmiiboPane");
		amiiboPane->enabled = false;
		settingsPaneRegistry.push_back(amiiboPane);

		Arriba::Elements::Button* categoryButton = new Arriba::Elements::Button();
		categoryButton->setParent(amiiboPane);
		categoryButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		categoryButton->transform.position = {amiiboPane->width / 2, amiiboPane->height * 1/5, 0};
		categoryButton->setText(getCategoryModeLabel());
		categoryButton->setName("CategorySettingsButton");
		categoryButton->setTag("SettingsButton");
		categoryButton->enabled = false;
		categoryButton->registerCallback([](){
			using Option = Amiigo::Elements::DropdownMenu::Option;
			const struct { unsigned char mode; const char* labelKey; const char* statusKey; } modes[] = {
				{ Amiigo::Settings::categoryModes::saveToRoot,         "category_save_to_root",         "status_category_root" },
				{ Amiigo::Settings::categoryModes::saveByGameName,      "category_save_to_game_name",      "status_category_game_name" },
				{ Amiigo::Settings::categoryModes::saveByAmiiboSeries,  "category_save_to_amiibo_series",  "status_category_amiibo_series" },
				{ Amiigo::Settings::categoryModes::saveByCurrentFolder, "category_save_to_current_folder", "status_category_current_folder" },
			};
			std::vector<Option> opts;
			for (auto& m : modes) {
				opts.push_back({ Amiigo::Lang::get(m.labelKey), [mode = m.mode, statusKey = m.statusKey](){
					Amiigo::Settings::categoryMode = mode;
					Amiigo::Settings::saveSettings();
					Arriba::findObjectByName<Arriba::Elements::Button>("CategorySettingsButton")->setText(getCategoryModeLabel());
					updateStatus(Amiigo::Lang::get(statusKey).c_str(), StatusLevel::Info);
				}});
			}
			auto* btn = Arriba::findObjectByName<Arriba::Primitives::Quad>("CategorySettingsButton");
			auto* scene = Arriba::findObjectByName("SettingsScene");
			int dropX = (int)(scene->transform.position.x + btn->transform.position.x) - btn->width / 2;
			int dropY = (int)(scene->transform.position.y + btn->transform.position.y) + btn->height / 2;
			new Amiigo::Elements::DropdownMenu(dropX, dropY, btn->width, opts, Amiigo::Settings::categoryMode);
		});

		Amiigo::Elements::CheckBox* randomUUIDCheckBox = new Amiigo::Elements::CheckBox(Amiigo::Settings::useRandomisedUUID, Amiigo::Lang::get("settings_enable_random_uuid").c_str());
		randomUUIDCheckBox->setParent(amiiboPane);
		randomUUIDCheckBox->transform.position = {(amiiboPane->width - subButtonWidth) / 2, amiiboPane->height * 2/5 - buttonHeight / 2, 0};
		randomUUIDCheckBox->setName("RandomUUIDCheckBox");
		randomUUIDCheckBox->setTag("SettingsButton");
		randomUUIDCheckBox->enabled = false;
		randomUUIDCheckBox->registerCallback([](bool checked){
			Amiigo::Settings::useRandomisedUUID = checked;
			Amiigo::Settings::saveSettings();
			if (checked) updateStatus(Amiigo::Lang::get("status_uuid_random_enabled").c_str(), StatusLevel::Info);
			else updateStatus(Amiigo::Lang::get("status_uuid_random_disabled").c_str(), StatusLevel::Info);
		});

		Amiigo::Elements::CheckBox* saveImagesCheckBox = new Amiigo::Elements::CheckBox(Amiigo::Settings::saveAmiiboImages, Amiigo::Lang::get("settings_save_amiibo_images").c_str());
		saveImagesCheckBox->setParent(amiiboPane);
		saveImagesCheckBox->transform.position = {(amiiboPane->width - subButtonWidth) / 2, amiiboPane->height * 3/5 - buttonHeight / 2, 0};
		saveImagesCheckBox->setName("SaveAmiiboImagesCheckBox");
		saveImagesCheckBox->setTag("SettingsButton");
		saveImagesCheckBox->enabled = false;
		saveImagesCheckBox->registerCallback([](bool checked){
			Amiigo::Settings::saveAmiiboImages = checked;
			Amiigo::Settings::saveSettings();
			if (checked) updateStatus(Amiigo::Lang::get("status_save_amiibo_images_enabled").c_str(), StatusLevel::Info);
			else updateStatus(Amiigo::Lang::get("status_save_amiibo_images_disabled").c_str(), StatusLevel::Info);
		});

		Arriba::Elements::Button* amiiboBackButton = new Arriba::Elements::Button();
		amiiboBackButton->setParent(amiiboPane);
		amiiboBackButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		amiiboBackButton->transform.position = {amiiboPane->width / 2, amiiboPane->height * 4/5, 0};
		amiiboBackButton->setText(Amiigo::Lang::get("settings_back").c_str());
		amiiboBackButton->setName("SettingsAmiiboBackButton");
		amiiboBackButton->setTag("SettingsButton");
		amiiboBackButton->enabled = false;
		amiiboBackButton->registerCallback(goToSettingsTopLevel);

		Arriba::Primitives::Quad* updatePane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		updatePane->setParent(settingsScene);
		updatePane->setColour({0, 0, 0, 0.7});
		updatePane->setName("SettingsUpdatePane");
		updatePane->enabled = false;
		settingsPaneRegistry.push_back(updatePane);

		Arriba::Elements::Button* cacheUpdateButton = new Arriba::Elements::Button();
		cacheUpdateButton->setParent(updatePane);
		cacheUpdateButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		cacheUpdateButton->transform.position = {updatePane->width / 2, updatePane->height * 1/5, 0};
		cacheUpdateButton->setText(Amiigo::Lang::get("settings_update_api_cache").c_str());
		cacheUpdateButton->setName("CacheUpdateButton");
		cacheUpdateButton->setTag("SettingsButton");
		cacheUpdateButton->enabled = false;
		cacheUpdateButton->registerCallback([]() {
			remove("sdmc:/config/amiigo/API.json");
			initSplash();
		});

		Arriba::Elements::Button* updaterButton = new Arriba::Elements::Button();
		updaterButton->setParent(updatePane);
		updaterButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		updaterButton->transform.position = {updatePane->width / 2, updatePane->height * 2/5, 0};
		updaterButton->setText(Amiigo::Lang::get("settings_update_amiigo").c_str());
		updaterButton->setName("UpdaterButton");
		updaterButton->setTag("SettingsButton");
		updaterButton->enabled = false;
		updaterButton->registerCallback([](){
			if (!hasNetworkConnection()) {
				updateStatus(Amiigo::Lang::get("error_no_network").c_str(), StatusLevel::Error);
			} else {
				std::ofstream fileStream("sdmc:/config/amiigo/update.flag");
				fileStream.close();
				initSplash();
			}
		});

		Arriba::Elements::Button* reinstallEmuiiboButton = new Arriba::Elements::Button();
		reinstallEmuiiboButton->setParent(updatePane);
		reinstallEmuiiboButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		reinstallEmuiiboButton->transform.position = {updatePane->width / 2, updatePane->height * 3/5, 0};
		reinstallEmuiiboButton->setText(Amiigo::Lang::get("settings_reinstall_emuiibo").c_str());
		reinstallEmuiiboButton->setName("ReinstallEmuiiboButton");
		reinstallEmuiiboButton->setTag("SettingsButton");
		reinstallEmuiiboButton->enabled = false;
		reinstallEmuiiboButton->registerCallback([](){
			if (!hasNetworkConnection()) {
				updateStatus(Amiigo::Lang::get("error_no_network").c_str(), StatusLevel::Error);
			} else {
				pmshellInitialize();
				pmshellTerminateProgram(0x0100000000000352);
				pmshellExit();
				remove("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp");
				initSplash();
			}
		});

		Arriba::Elements::Button* updateBackButton = new Arriba::Elements::Button();
		updateBackButton->setParent(updatePane);
		updateBackButton->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
		updateBackButton->transform.position = {updatePane->width / 2, updatePane->height * 4/5, 0};
		updateBackButton->setText(Amiigo::Lang::get("settings_back").c_str());
		updateBackButton->setName("SettingsUpdateBackButton");
		updateBackButton->setTag("SettingsButton");
		updateBackButton->enabled = false;
		updateBackButton->registerCallback(goToSettingsTopLevel);

		{
			struct Entry { const char* name; bool show; };
			const Entry entries[] = {
				{ "CacheUpdateButton",        hasNetworkConnection()  },
				{ "UpdaterButton",            settingsUpdateAvailable },
				{ "ReinstallEmuiiboButton",   hasNetworkConnection()  },
				{ "SettingsUpdateBackButton", true                    },
			};
			std::vector<Arriba::UIObject*> visible;
			for (const auto& entry : entries) {
				if (entry.show) visible.push_back(Arriba::findObjectByName(entry.name));
			}
			const float cx = updatePane->width / 2.0f;
			const float spacing = (float)updatePane->height / (float)(visible.size() + 1);
			for (size_t i = 0; i < visible.size(); i++) {
				visible[i]->transform.position = {cx, (float)(i + 1) * spacing, 0};
			}
		}

		Arriba::Primitives::Quad* themePane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		themePane->setParent(settingsScene);
		themePane->setColour({0, 0, 0, 0.7f});
		themePane->setName("SettingsThemePane");
		themePane->enabled = false;
		settingsPaneRegistry.push_back(themePane);
		themeSubPanes.push_back({SettingsView::ThemeHub, SettingsView::TopLevel, "SettingsThemePane", {}});

		{
			const struct { const char* nameKey; SettingsView target; } hubEntries[] = {
				{ "settings_theme_status_bar",       SettingsView::ThemeStatusBar },
				{ "settings_theme_section_list",     SettingsView::ThemeList      },
				{ "settings_theme_section_store",    SettingsView::ThemeStore     },
				{ "settings_theme_section_settings", SettingsView::ThemeSection   },
			};
			const int numHub = sizeof(hubEntries) / sizeof(hubEntries[0]);
			const int hubSlots = numHub + 2;
			for (int i = 0; i < numHub; i++) {
				Arriba::Elements::Button* btn = new Arriba::Elements::Button();
				btn->setParent(themePane);
				btn->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
				btn->transform.position = {(float)themePane->width / 2, (float)themePane->height * (i + 1) / hubSlots, 0};
				btn->setText(Amiigo::Lang::get(hubEntries[i].nameKey).c_str());
				btn->setTag("SettingsButton");
				btn->enabled = false;
				SettingsView target = hubEntries[i].target;
				btn->registerCallback([target](){ activateThemePane(target); });
				themeSubPanes.back().buttons.push_back(btn);
			}
			Arriba::Elements::Button* hubBackBtn = new Arriba::Elements::Button();
			hubBackBtn->setParent(themePane);
			hubBackBtn->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
			hubBackBtn->transform.position = {(float)themePane->width / 2, (float)themePane->height * (numHub + 1) / hubSlots, 0};
			hubBackBtn->setText(Amiigo::Lang::get("settings_back").c_str());
			hubBackBtn->setTag("SettingsButton");
			hubBackBtn->enabled = false;
			hubBackBtn->registerCallback(goToSettingsTopLevel);
			themeSubPanes.back().buttons.push_back(hubBackBtn);
		}

		auto makeColourBtn = [&](Arriba::Primitives::Quad* parentPane, int slot, int slots, colour* targetColour, const char* nameKey) -> Arriba::Elements::Button* {
			Arriba::Elements::Button* btn = new Arriba::Elements::Button();
			btn->setParent(parentPane);
			btn->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
			btn->transform.position = {(float)parentPane->width / 2, (float)parentPane->height * slot / slots, 0};
			btn->setText(Amiigo::Lang::get(nameKey).c_str());
			btn->setTag("SettingsButton");
			btn->enabled = false;
			Arriba::Primitives::Quad* swatch = new Arriba::Primitives::Quad(0, 0, 36, 36, Arriba::Graphics::Pivot::centre);
			swatch->setParent(btn);
			swatch->transform.position = {-(float)(subButtonWidth / 2) + 28, 0, 0};
			swatch->setColour(*targetColour);
			themeSwatches.push_back({targetColour, swatch});
			const char32_t* colourName = Amiigo::Lang::get(nameKey).c_str();
			btn->registerCallback([targetColour, colourName]() {
				const int dialogW = Amiigo::Elements::ColourPickerDialog::DIALOG_W;
				const int dialogH = Amiigo::Elements::ColourPickerDialog::DIALOG_H;
				const int settingsW = Arriba::Graphics::windowWidth - switcherWidth - 1;
				const int dialogX = (settingsW - dialogW) / 2;
				const int dialogY = statusHeight + (Arriba::Graphics::windowHeight - statusHeight - dialogH) / 2;
				new Amiigo::Elements::ColourPickerDialog(dialogX, dialogY, targetColour, colourName, []() {
					for (auto& swatch : themeSwatches) swatch.quad->setColour(*swatch.colourPtr);
					Arriba::Colour::neutral    = Amiigo::Settings::Colour::settingsNeutral;
					Arriba::Colour::highlightA = Amiigo::Settings::Colour::settingsHighlightA;
					Arriba::Colour::highlightB = Amiigo::Settings::Colour::settingsHighlightB;
				}, []() {
					Amiigo::Settings::saveTheme();
				});
			});
			return btn;
		};

		auto makeThemeSubPane = [&](SettingsView view, const char* paneName) -> Arriba::Primitives::Quad* {
			Arriba::Primitives::Quad* pane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
			pane->setParent(settingsScene);
			pane->setColour({0, 0, 0, 0.7f});
			pane->setName(paneName);
			pane->enabled = false;
			settingsPaneRegistry.push_back(pane);
			themeSubPanes.push_back({view, SettingsView::ThemeHub, paneName, {}});
			return pane;
		};

		auto addResetDefaultsBtn = [&](Arriba::Primitives::Quad* pane, int slot, int slots, std::function<void()> resetFn) {
			Arriba::Elements::Button* btn = new Arriba::Elements::Button();
			btn->setParent(pane);
			btn->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
			btn->transform.position = {(float)pane->width / 2, (float)pane->height * slot / slots, 0};
			btn->setText(Amiigo::Lang::get("settings_theme_reset_defaults").c_str());
			btn->setTag("SettingsButton");
			btn->enabled = false;
			btn->registerCallback([resetFn]() {
				resetFn();
				for (auto& swatch : themeSwatches) swatch.quad->setColour(*swatch.colourPtr);
				Arriba::Colour::neutral    = Amiigo::Settings::Colour::settingsNeutral;
				Arriba::Colour::highlightA = Amiigo::Settings::Colour::settingsHighlightA;
				Arriba::Colour::highlightB = Amiigo::Settings::Colour::settingsHighlightB;
				Amiigo::Settings::saveTheme();
			});
			themeSubPanes.back().buttons.push_back(btn);
		};

		auto addHubBackBtn = [&](Arriba::Primitives::Quad* pane, int slot, int slots) {
			Arriba::Elements::Button* btn = new Arriba::Elements::Button();
			btn->setParent(pane);
			btn->setDimensions(subButtonWidth, buttonHeight, Arriba::Graphics::Pivot::centre);
			btn->transform.position = {(float)pane->width / 2, (float)pane->height * slot / slots, 0};
			btn->setText(Amiigo::Lang::get("settings_back").c_str());
			btn->setTag("SettingsButton");
			btn->enabled = false;
			btn->registerCallback([]() { activateThemePane(SettingsView::ThemeHub); });
			themeSubPanes.back().buttons.push_back(btn);
		};

		{
			auto* pane = makeThemeSubPane(SettingsView::ThemeStatusBar, "ThemeStatusBarPane");
			themeSubPanes.back().buttons.push_back(makeColourBtn(pane, 1, 4, &Amiigo::Settings::Colour::statusBar, "settings_theme_status_bar"));
			addResetDefaultsBtn(pane, 2, 4, []() {
				using namespace Amiigo::Settings::Colour;
				statusBar = Defaults::statusBar;
			});
			addHubBackBtn(pane, 3, 4);
		}

		{
			auto* pane = makeThemeSubPane(SettingsView::ThemeList, "ThemeListPane");
			const struct { colour* c; const char* k; } entries[] = {
				{ &Amiigo::Settings::Colour::listNeutral,    "settings_theme_list_neutral"     },
				{ &Amiigo::Settings::Colour::listHighlightA, "settings_theme_list_highlight_a" },
				{ &Amiigo::Settings::Colour::listHighlightB, "settings_theme_list_highlight_b" },
			};
			for (int i = 0; i < 3; i++) themeSubPanes.back().buttons.push_back(makeColourBtn(pane, i + 1, 6, entries[i].c, entries[i].k));
			addResetDefaultsBtn(pane, 4, 6, []() {
				using namespace Amiigo::Settings::Colour;
				listNeutral    = Defaults::listNeutral;
				listHighlightA = Defaults::listHighlightA;
				listHighlightB = Defaults::listHighlightB;
			});
			addHubBackBtn(pane, 5, 6);
		}

		{
			auto* pane = makeThemeSubPane(SettingsView::ThemeStore, "ThemeStorePane");
			const struct { colour* c; const char* k; } entries[] = {
				{ &Amiigo::Settings::Colour::makerNeutral,    "settings_theme_store_neutral"     },
				{ &Amiigo::Settings::Colour::makerHighlightA, "settings_theme_store_highlight_a" },
				{ &Amiigo::Settings::Colour::makerHighlightB, "settings_theme_store_highlight_b" },
			};
			for (int i = 0; i < 3; i++) themeSubPanes.back().buttons.push_back(makeColourBtn(pane, i + 1, 6, entries[i].c, entries[i].k));
			addResetDefaultsBtn(pane, 4, 6, []() {
				using namespace Amiigo::Settings::Colour;
				makerNeutral    = Defaults::makerNeutral;
				makerHighlightA = Defaults::makerHighlightA;
				makerHighlightB = Defaults::makerHighlightB;
			});
			addHubBackBtn(pane, 5, 6);
		}

		{
			auto* pane = makeThemeSubPane(SettingsView::ThemeSection, "ThemeSectionPane");
			const struct { colour* c; const char* k; } entries[] = {
				{ &Amiigo::Settings::Colour::settingsNeutral,    "settings_theme_settings_neutral"     },
				{ &Amiigo::Settings::Colour::settingsHighlightA, "settings_theme_settings_highlight_a" },
				{ &Amiigo::Settings::Colour::settingsHighlightB, "settings_theme_settings_highlight_b" },
			};
			for (int i = 0; i < 3; i++) themeSubPanes.back().buttons.push_back(makeColourBtn(pane, i + 1, 6, entries[i].c, entries[i].k));
			addResetDefaultsBtn(pane, 4, 6, []() {
				using namespace Amiigo::Settings::Colour;
				settingsNeutral    = Defaults::settingsNeutral;
				settingsHighlightA = Defaults::settingsHighlightA;
				settingsHighlightB = Defaults::settingsHighlightB;
			});
			addHubBackBtn(pane, 5, 6);
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
					updateStatus(Amiigo::Lang::get("status_emuiibo_disabled").c_str(), StatusLevel::Info);
				break;
				case emu::EmulationStatus::Off:
					emu::SetEmulationStatus(emu::EmulationStatus::On);
					updateStatus(Amiigo::Lang::get("status_emuiibo_enabled").c_str(), StatusLevel::Info);
				break;
				default:
					updateStatus(Amiigo::Lang::get("error_unknown_emulation_status").c_str(), StatusLevel::Info);
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
		if (Arriba::highlightedObject == Arriba::findObjectByName("SettingsScene")) Arriba::highlightedObject = Arriba::findObjectByName("SettingsTopAmiiboButton");

		if (Arriba::Input::buttonDown(Arriba::Input::DPadRight) && Arriba::highlightedObject->getTag() != "SwitcherButton") {
			Arriba::highlightedObject = Arriba::findObjectByName("SelectorButton");
			return;
		}

		if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && settingsView != SettingsView::TopLevel) {
			for (auto& subPane : themeSubPanes) {
				if (subPane.view != settingsView) continue;
				if (subPane.parentView == SettingsView::TopLevel) goToSettingsTopLevel();
				else activateThemePane(subPane.parentView);
				return;
			}
			goToSettingsTopLevel();
			return;
		}

		for (auto* pane : settingsPaneRegistry) {
			if (Arriba::highlightedObject != pane) continue;
			for (auto* btn : Arriba::findObjectsByTag("SettingsButton")) {
				if (btn->enabled) {
					Arriba::highlightedObject = btn;
					return;
				}
			}
			return;
		}

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
			updateStatus(U"Amiigo + Arriba", StatusLevel::Silent);
		} else if (Arriba::highlightedObject == Arriba::findObjectByName("MakerButton")) {
			Arriba::Elements::InertialList* makerList = Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList");
			makerList->enabled = true;
			makerIsInCategory = false;
			makerList->updateStrings(seriesList);
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

	void selectorInput(int index) {
		if (selectorAmiibos[index].isCategory) {
			if (checkIfFileExists(selectorAmiibos[index].path.c_str()) || selectorAmiibos[index].path == "Favorites") {
				selectorPath = selectorAmiibos[index].path;
				updateSelectorStrings();
			} else {
				updateStatus(Amiigo::Lang::get("error_folder_not_exist").c_str(), StatusLevel::Error);
			}
		} else {
			std::string path = selectorAmiibos[index].path;
			emu::SetEmulationStatus(emu::EmulationStatus::On);
			Result res = emu::SetActiveVirtualAmiibo(path.c_str(), path.size());
			if R_FAILED(res) {
				updateStatus(Amiigo::Lang::get("error_failed_set_amiibo").c_str(), StatusLevel::Error);
				return;
			}
			
			updateStatus(Arriba::Text::ASCIIToUnicode(path.c_str()).c_str(), StatusLevel::Info);
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
				updateStatus((U"Created " + creatorData[index-1].name).c_str(), StatusLevel::Info);
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

	void updateStatus(const char32_t* text, StatusLevel level) {
		Arriba::findObjectByName<Arriba::Primitives::Text>("StatusBarText")->setText(text);
		switch (level) {
			case StatusLevel::Info:   Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,1,1,1}); break;
			case StatusLevel::Error:  Arriba::findObjectByName<Arriba::Primitives::Quad>("StatusBar")->setColour({1,0,0,1}); break;
			case StatusLevel::Silent: break;
		}
	}

	const std::string& getSelectorPath() {
		return selectorPath;
	}
}  // namespace Amiigo::UI
