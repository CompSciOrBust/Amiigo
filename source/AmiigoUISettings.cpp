#include <AmiigoUI.h>
#include <AmiigoUISelector.h>
#include <AmiigoUISettings.h>
#include <arriba.h>
#include <utils.h>

#include <atomic>
#include <cstdio>
#include <fstream>
#include <memory>

#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <emuiibo.hpp>
#include <Networking.h>
#include <AmiigoSettings.h>
#include <AmiigoElements.h>
#include <AmiigoLang.h>
#include <arribaText.h>

namespace {
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

    void onGenerateAllConfirmed() {
        auto allAmiibos = getAllAmiibos();
        int total = (int)allAmiibos.size();
        if (total == 0) {
			Amiigo::UI::updateStatus(Amiigo::Lang::get("error_no_api_cache").c_str(), Amiigo::UI::StatusLevel::Error);
			return;
		}

        auto progress = std::make_shared<std::atomic<int>>(0);

        const int settingsW = Arriba::Graphics::windowWidth - Amiigo::UI::switcherWidth - 1;
        new Amiigo::Elements::ProgressDialog(
            (settingsW - Amiigo::Elements::ProgressDialog::DIALOG_W) / 2,
            Amiigo::UI::statusHeight + (Arriba::Graphics::windowHeight - Amiigo::UI::statusHeight - Amiigo::Elements::ProgressDialog::DIALOG_H) / 2,
            Amiigo::Lang::get("settings_generate_all_progress").c_str(),
            progress, total,
            Arriba::findObjectByName("GenerateAllAmiiboButton")
        );

        bool downloadImages = Amiigo::Settings::saveAmiiboImages;
        workerQueue.enqueue([amiibos = std::move(allAmiibos), progress, downloadImages]() {
            for (const auto& amiibo : amiibos) {
                std::string pathBase = createVirtualAmiibo(amiibo, false);
                if (downloadImages) saveAmiiboImage(pathBase, amiibo);
                progress->fetch_add(1, std::memory_order_relaxed);
            }
            MainThread::dispatch([]() {
                Amiigo::UI::updateSelectorStrings();
                Amiigo::UI::updateStatus(Amiigo::Lang::get("status_generate_all_complete").c_str(), Amiigo::UI::StatusLevel::Info);
            });
        });
    }

    void showGenerateAllConfirm() {
        const int settingsW = Arriba::Graphics::windowWidth - Amiigo::UI::switcherWidth - 1;
        new Amiigo::Elements::ConfirmDialog(
            (settingsW - Amiigo::Elements::ConfirmDialog::DIALOG_W) / 2,
            Amiigo::UI::statusHeight + (Arriba::Graphics::windowHeight - Amiigo::UI::statusHeight - Amiigo::Elements::ConfirmDialog::DIALOG_H) / 2,
            Amiigo::Lang::get("settings_generate_all_confirm").c_str(),
            onGenerateAllConfirmed,
            nullptr,
            Amiigo::Lang::get("settings_generate_all_yes").c_str(),
            Amiigo::Lang::get("settings_generate_all_no").c_str()
        );
    }
}

namespace Amiigo::UI {
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
		Arriba::findObjectByName("SettingsTopLevelPane")->enabled = true;
		Arriba::findObjectByName("SettingsTopAmiiboButton")->enabled = true;
		Arriba::findObjectByName("SettingsTopUpdateButton")->enabled = true;
		Arriba::findObjectByName("SettingsTopThemeButton")->enabled = true;
		Arriba::findObjectByName("SettingsCredits")->enabled = true;
	}

	void goToSettingsTopLevel() {
		resetSettingsToTopLevel();
		Arriba::highlightedObject = Arriba::findObjectByName("SettingsTopAmiiboButton");
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

	void goToAmiiboSubmenu() {
		disableAllSettingsElements();
		settingsView = SettingsView::AmiiboSettings;
		Arriba::findObjectByName("SettingsAmiiboPane")->enabled = true;
		Arriba::findObjectByName("CategorySettingsButton")->enabled = true;
		Arriba::findObjectByName("RandomUUIDCheckBox")->enabled = true;
		Arriba::findObjectByName("SaveAmiiboImagesCheckBox")->enabled = true;
		Arriba::findObjectByName("GenerateAllAmiiboButton")->enabled = true;
		Arriba::findObjectByName("SettingsAmiiboBackButton")->enabled = true;
		Arriba::highlightedObject = Arriba::findObjectByName("SettingsAmiiboPane");
	}

	void goToUpdatesSubmenu() {
		disableAllSettingsElements();
		settingsView = SettingsView::Updates;
		auto* updatePane = Arriba::findObjectByName<Arriba::Primitives::Quad>("SettingsUpdatePane");
		updatePane->enabled = true;
		bool hasNet = hasNetworkConnection();
		Arriba::findObjectByName("CacheUpdateButton")->enabled = hasNet;
		Arriba::findObjectByName("UpdaterButton")->enabled = settingsUpdateAvailable;
		Arriba::findObjectByName("ReinstallEmuiiboButton")->enabled = hasNet;
		Arriba::findObjectByName("SettingsUpdateBackButton")->enabled = true;
		const char* const names[] = { "CacheUpdateButton", "UpdaterButton", "ReinstallEmuiiboButton", "SettingsUpdateBackButton" };
		std::vector<Arriba::UIObject*> visible;
		for (const auto* name : names) {
			auto* btn = Arriba::findObjectByName(name);
			if (btn->enabled) visible.push_back(btn);
		}
		const float cx = updatePane->width / 2.0f;
		const float spacing = (float)updatePane->height / (float)(visible.size() + 1);
		for (size_t i = 0; i < visible.size(); i++) visible[i]->transform.position = {cx, (float)(i + 1) * spacing, 0};
		Arriba::highlightedObject = updatePane;
	}

	void initSettings() {
		if (checkForUpdates()) {
			Arriba::findObjectByName<Arriba::Elements::Button>("SettingsButton")->setText(Amiigo::Lang::get("nav_update").c_str());
			settingsUpdateAvailable = true;
		}

		const int buttonHeight = 107;
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

		Arriba::Primitives::Quad* topLevelPane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		topLevelPane->setParent(settingsScene);
		topLevelPane->setColour({0, 0, 0, 0.7f});
		topLevelPane->setName("SettingsTopLevelPane");
		topLevelPane->enabled = false;
		settingsPaneRegistry.push_back(topLevelPane);

		Arriba::Primitives::Quad* creditsQuad = new Arriba::Primitives::Quad(0, 0, 330, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		creditsQuad->setParent(settingsScene);
		creditsQuad->setColour({0, 0, 0, 0.77});
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

		auto makeBtn = [&](Arriba::Primitives::Quad* parent, int width, float x, float y,
		                   const char32_t* text, const char* name,
		                   std::function<void()> callback) -> Arriba::Elements::Button* {
			Arriba::Elements::Button* btn = new Arriba::Elements::Button();
			btn->setParent(parent);
			btn->setDimensions(width, buttonHeight, Arriba::Graphics::Pivot::centre);
			btn->transform.position = {x, y, 0};
			btn->setText(text);
			btn->setName(name);
			btn->setTag("SettingsButton");
			btn->enabled = false;
			btn->registerCallback(callback);
			applySettingsQuadStyle(btn);
			return btn;
		};

		makeBtn(settingsScene, topButtonWidth, topCenterX, settingsScene->height * 1/4,
		        Amiigo::Lang::get("settings_amiibo_settings").c_str(), "SettingsTopAmiiboButton", goToAmiiboSubmenu);
		makeBtn(settingsScene, topButtonWidth, topCenterX, settingsScene->height * 2/4,
		        Amiigo::Lang::get("settings_theme").c_str(), "SettingsTopThemeButton", goToThemeHub);
		makeBtn(settingsScene, topButtonWidth, topCenterX, settingsScene->height * 3/4,
		        Amiigo::Lang::get("settings_updates").c_str(), "SettingsTopUpdateButton", goToUpdatesSubmenu);

		Arriba::Primitives::Quad* amiiboPane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		amiiboPane->setParent(settingsScene);
		amiiboPane->setColour({0, 0, 0, 0.7});
		amiiboPane->setName("SettingsAmiiboPane");
		amiiboPane->enabled = false;
		settingsPaneRegistry.push_back(amiiboPane);

		makeBtn(amiiboPane, subButtonWidth, amiiboPane->width / 2, amiiboPane->height * 1/6,
		        getCategoryModeLabel(), "CategorySettingsButton", [](){
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
		randomUUIDCheckBox->transform.position = {(amiiboPane->width - subButtonWidth) / 2, amiiboPane->height * 2/6 - buttonHeight / 2, 0};
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
		saveImagesCheckBox->transform.position = {(amiiboPane->width - subButtonWidth) / 2, amiiboPane->height * 3/6 - buttonHeight / 2, 0};
		saveImagesCheckBox->setName("SaveAmiiboImagesCheckBox");
		saveImagesCheckBox->setTag("SettingsButton");
		saveImagesCheckBox->enabled = false;
		saveImagesCheckBox->registerCallback([](bool checked){
			Amiigo::Settings::saveAmiiboImages = checked;
			Amiigo::Settings::saveSettings();
			if (checked) updateStatus(Amiigo::Lang::get("status_save_amiibo_images_enabled").c_str(), StatusLevel::Info);
			else updateStatus(Amiigo::Lang::get("status_save_amiibo_images_disabled").c_str(), StatusLevel::Info);
		});

		makeBtn(amiiboPane, subButtonWidth, amiiboPane->width / 2, amiiboPane->height * 4/6,
		        Amiigo::Lang::get("settings_generate_all").c_str(), "GenerateAllAmiiboButton", showGenerateAllConfirm);
		makeBtn(amiiboPane, subButtonWidth, amiiboPane->width / 2, amiiboPane->height * 5/6,
		        Amiigo::Lang::get("settings_back").c_str(), "SettingsAmiiboBackButton", goToSettingsTopLevel);

		Arriba::Primitives::Quad* updatePane = new Arriba::Primitives::Quad(0, 0, settingsScene->width, settingsScene->height, Arriba::Graphics::Pivot::topLeft);
		updatePane->setParent(settingsScene);
		updatePane->setColour({0, 0, 0, 0.7});
		updatePane->setName("SettingsUpdatePane");
		updatePane->enabled = false;
		settingsPaneRegistry.push_back(updatePane);

		makeBtn(updatePane, subButtonWidth, updatePane->width / 2, updatePane->height * 1/5,
		        Amiigo::Lang::get("settings_update_api_cache").c_str(), "CacheUpdateButton", []() {
			remove("sdmc:/config/amiigo/API.json");
			initSplash();
		});
		makeBtn(updatePane, subButtonWidth, updatePane->width / 2, updatePane->height * 2/5,
		        Amiigo::Lang::get("settings_update_amiigo").c_str(), "UpdaterButton", [](){
			if (!hasNetworkConnection()) {
				updateStatus(Amiigo::Lang::get("error_no_network").c_str(), StatusLevel::Error);
			} else {
				std::ofstream fileStream("sdmc:/config/amiigo/update.flag");
				fileStream.close();
				initSplash();
			}
		});
		makeBtn(updatePane, subButtonWidth, updatePane->width / 2, updatePane->height * 3/5,
		        Amiigo::Lang::get("settings_reinstall_emuiibo").c_str(), "ReinstallEmuiiboButton", [](){
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
		makeBtn(updatePane, subButtonWidth, updatePane->width / 2, updatePane->height * 4/5,
		        Amiigo::Lang::get("settings_back").c_str(), "SettingsUpdateBackButton", goToSettingsTopLevel);

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
				applySettingsQuadStyle(btn);
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
			applySettingsQuadStyle(hubBackBtn);
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
			applySettingsQuadStyle(btn);
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
			applySettingsQuadStyle(btn);
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
			applySettingsQuadStyle(btn);
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

	void handleSettingsInput() {
		if (Arriba::highlightedObject == Arriba::findObjectByName("SettingsScene")) {
			for (int i = 0; i < settingsPaneRegistry.size(); i++) {
				if (settingsPaneRegistry.at(i)->enabled) {
					Arriba::highlightedObject = settingsPaneRegistry.at(i);
					break;
				}
			}
		}

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
}
