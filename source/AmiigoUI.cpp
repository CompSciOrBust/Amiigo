#include <AmiigoUI.h>
#include <arriba.h>
#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <utils.h>
#include <emuiibo.hpp>
#include <math.h>
#include <thread>
#include <Networking.h>
#include <AmiigoSettings.h>
#include <fstream>
#include <AmiigoElements.h>

namespace Amiigo::UI
{
	void initUI()
	{
		Arriba::Colour::neutral = {0.22,0.47,0.93,0.97};
		Arriba::Colour::highlightA = {0.1,0.95,0.98,0.97};
		Arriba::Colour::highlightB = {0.5,0.85,1,0.97};
	    if(!checkIfFileExists("sdmc:/config/amiigo/API.json") || !checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) initSplash();
		Amiigo::Settings::loadSettings();
		initSceneSwitcher();
		initSelector();
	    initMaker();
		initSettings();
		Arriba::highlightedObject = selectorButton;
		//Cache these now for performance gains later
		lists = Arriba::findObjectsByTag("List");
		buttons = Arriba::findObjectsByTag("SwitcherButton");
		//Check if a new Amiigo version is available
		if(checkForUpdates())
		{
			settingsButton->setText("Update");
			Arriba::findObjectByName("UpdaterButton")->enabled = true;
		}
	}

	void initSplash()
	{
		Arriba::activeLayer++;
		splashScene = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, Arriba::Graphics::windowHeight, Arriba::Graphics::Pivot::topLeft);
		splashScene->setColour({0.25,0.25,0.25,0.95});
		//Title text
		Arriba::Primitives::Text* titleText = new Arriba::Primitives::Text("Amiigo", 128);
		titleText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 140, 0};
		titleText->setColour({1,1,1,1});
		titleText->setParent(splashScene);
		//By text
		Arriba::Primitives::Text* byText = new Arriba::Primitives::Text("by CompSciOrBust", 64);
		byText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 - 15, 0};
		byText->setColour({1,1,1,1});
		byText->setParent(splashScene);
		//Doing text
		Arriba::Primitives::Text* doingText = new Arriba::Primitives::Text("", 48);
		doingText->transform.position = {Arriba::Graphics::windowWidth/2, Arriba::Graphics::windowHeight/2 + 160, 0};
		doingText->setColour({1,1,1,1});
		doingText->setParent(splashScene);
		std::thread initThread(firstTimeSetup);
		//Hide other elements
		if(settingsScene) settingsScene->enabled = false;
		if(sceneSwitcher) sceneSwitcher->enabled = false;
		while (!checkIfFileExists("sdmc:/config/amiigo/API.json") || !checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp") || checkIfFileExists("sdmc:/config/amiigo/update.flag"))
		{
			splashScene->setColour({(sin(Arriba::time)+1)/4,(cos(Arriba::time)+1)/4,0.5,0.95});
			Arriba::findObjectByName("AmiigoBG")->renderer->thisShader.setFloat1("iTime", Arriba::time);
			if(!hasNetworkConnection()) doingText->setText("Waiting for internet connection...");
			else if(!checkIfFileExists("sdmc:/config/amiigo/API.json")) doingText->setText("Caching API data...");
			else if(!checkIfFileExists("sdmc:/atmosphere/contents/0100000000000352/exefs.nsp")) doingText->setText("Installing Emuiibo...");
			else if(checkIfFileExists("sdmc:/config/amiigo/update.flag")) doingText->setText("Updating Amiigo...");
			Arriba::drawFrame();
		}
		initThread.join();
		splashScene->destroy();
		if(settingsScene) settingsScene->enabled = true;
		if(sceneSwitcher) sceneSwitcher->enabled = true;
		Arriba::activeLayer--;
	}

	void initSceneSwitcher()
	{
		//Set up divider quads
		Arriba::Primitives::Quad* div1 = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth - switcherWidth - 1, statusHeight, 5, switcherHeight, Arriba::Graphics::Pivot::topLeft);
		div1->setColour({0,0,0,1});
		Arriba::Primitives::Quad* div2 = new Arriba::Primitives::Quad(0, statusHeight - 1, Arriba::Graphics::windowWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div2->setColour({0,0,0,1});
		int buttomDivX = Arriba::Graphics::windowWidth - switcherWidth;
		Arriba::Primitives::Quad* div3 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4) - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div3->setColour({0,0,0,1});
		Arriba::Primitives::Quad* div4 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4)*2 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div4->setColour({0,0,0,1});
		Arriba::Primitives::Quad* div5 = new Arriba::Primitives::Quad(buttomDivX, statusHeight + (switcherHeight/4)*3 - 1, switcherWidth, 5, Arriba::Graphics::Pivot::topLeft);
		div5->setColour({0,0,0,1});
		//Set up status bar
		Arriba::Primitives::Quad* statusBar = new Arriba::Primitives::Quad(0, 0, Arriba::Graphics::windowWidth, statusHeight - 1, Arriba::Graphics::Pivot::topLeft);
		statusBar->setColour({0.5,0.7,0.7,0.9});
		Arriba::Primitives::Text* statusText = new Arriba::Primitives::Text("Amiigo + Arriba", 34);
		statusText->name = "StatusBarText";
		statusText->setDimensions(statusText->width, statusText->height, Arriba::Graphics::centre);
		statusText->transform.position = {statusBar->width/2, statusBar->height/2, 0};
		statusText->setParent(statusBar);
		//Set up switcher quad
		sceneSwitcher = new Arriba::Primitives::Quad(Arriba::Graphics::windowWidth, statusHeight, switcherWidth, switcherHeight, Arriba::Graphics::Pivot::topRight);
		sceneSwitcher->setColour({0,0,0,0});
		//Set up Amiibo list button
		selectorButton = new Arriba::Elements::Button();
		selectorButton->setText("My Amiibo");
		selectorButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		selectorButton->setParent(sceneSwitcher);
		selectorButton->name = "SelectorButton";
		selectorButton->tag = "SwitcherButton";
		selectorButton->registerCallback(switcherPressed);
		//Set up Amiigo maker button
		makerButton = new Arriba::Elements::Button();
		makerButton->setText("Amiigo Store");
		makerButton->transform.position.y = selectorButton->height + 1;
		makerButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		makerButton->setParent(sceneSwitcher);
		makerButton->name = "MakerButton";
		makerButton->tag = "SwitcherButton";
		makerButton->registerCallback(switcherPressed);
		//Set up settings button
		settingsButton = new Arriba::Elements::Button();
		settingsButton->setText("Settings");
		settingsButton->transform.position.y = makerButton->height + makerButton->transform.position.y + 1;
		settingsButton->setDimensions(switcherWidth, switcherHeight/4 - 1, Arriba::Graphics::Pivot::topRight);
		settingsButton->setParent(sceneSwitcher);
		settingsButton->name = "SettingsButton";
		settingsButton->tag = "SwitcherButton";
		settingsButton->registerCallback(switcherPressed);
		//Set up exit button
		exitButton = new Arriba::Elements::Button();
		exitButton->setText("Exit");
		exitButton->transform.position.y = settingsButton->height + settingsButton->transform.position.y + 1;
		exitButton->setDimensions(switcherWidth, switcherHeight/4, Arriba::Graphics::Pivot::topRight);
		exitButton->setParent(sceneSwitcher);
		exitButton->name = "ExitButton";
		exitButton->tag = "SwitcherButton";
		exitButton->registerCallback([](){isRunning = 0;});
	}

	void initSelector()
	{
	    //Set up the list
	    selectorList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, {});
		updateSelectorStrings();
		selectorList->name = "SelectorList";
		selectorList->tag = "List";
		selectorList->registerCallback(selectorInput);
		selectorList->registerAltCallback(selectorContextMenuSpawner);
	}

	void initMaker()
	{
		seriesList = getListOfSeries();
	    makerList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, seriesList);
		makerList->registerCallback(makerInput);
		makerList->name = "MakerList";
		makerList->tag = "List";
		makerList->enabled = false;
	}

	void initSettings()
	{
		settingsScene = new Arriba::Primitives::Quad(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		//Not a list but pretending makes scene switching easier
		settingsScene->name = "SettingsScene";
		settingsScene->tag = "List";
		settingsScene->enabled = false;
		settingsScene->setColour({0,0,0,0});
		//Toggle category save button
		Arriba::Elements::Button* categoryButton = new Arriba::Elements::Button();
		categoryButton->setParent(settingsScene);
		categoryButton->setDimensions(550, 125, Arriba::Graphics::Pivot::centre);
		categoryButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 1/4,0};
		switch (Amiigo::Settings::categoryMode)
		{
			case Amiigo::Settings::categoryModes::saveToRoot:
			categoryButton->setText("Save to game name");
			break;

			case Amiigo::Settings::categoryModes::saveByGameName:
			categoryButton->setText("Save to Amiibo series");
			break;

			case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
			categoryButton->setText("Save to current folder");
			break;

			case Amiigo::Settings::categoryModes::saveByCurrentFolder:
			categoryButton->setText("Save to root");
			break;
		
			default:
			categoryButton->setText("Error");
			break;
		}
		categoryButton->name = "CategorySettingsButton";
		//Callback to save setting
		categoryButton->registerCallback([](){
			Amiigo::Settings::categoryMode = (Amiigo::Settings::categoryMode+1) % Amiigo::Settings::categoryModes::categoryCount;
			printf("%d", Amiigo::Settings::categoryMode);
			Amiigo::Settings::saveSettings();
			switch (Amiigo::Settings::categoryMode)
			{
				case Amiigo::Settings::categoryModes::saveToRoot:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText("Save to game name");
				static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiibos will save to sdmc:/emuiibo/amiibo");
				break;
			
				case Amiigo::Settings::categoryModes::saveByGameName:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText("Save to Amiibo series");
				static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiibos will save to sdmc:/emuiibo/game name");
				break;

				case Amiigo::Settings::categoryModes::saveByAmiiboSeries:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText("Save to current folder");
				static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiibos will save to sdmc:/emuiibo/amiibo series");
				break;

				case Amiigo::Settings::categoryModes::saveByCurrentFolder:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText("Save to root");
				static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiibos will save to the current location");
				break;

				default:
				static_cast<Arriba::Elements::Button*>(Arriba::findObjectByName("CategorySettingsButton"))->setText("Error");
				static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Error, uknown category mode");
				break;
			}
		});
		//Update API cache button
		Arriba::Elements::Button* apiUpdateButton = new Arriba::Elements::Button();
		apiUpdateButton->setParent(settingsScene);
		apiUpdateButton->setDimensions(550, 125, Arriba::Graphics::Pivot::centre);
		apiUpdateButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 2/4,0};
		apiUpdateButton->setText("Update API cache");
		apiUpdateButton->name = "UpdateAPIButton";
		//Callback for updating API cache
		apiUpdateButton->registerCallback([](){
			if(!hasNetworkConnection()) static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("No network connection!");
			else
			{
				if(checkIfFileExists("sdmc:/config/amiigo/API.json")) remove("sdmc:/config/amiigo/API.json");
				//This is kinda jank but I'm lazy and this won't be used often
				initSplash();
				seriesList = getListOfSeries();
			}
		});
		//Check for updates button
		Arriba::Elements::Button* updaterButton = new Arriba::Elements::Button();
		updaterButton->setParent(settingsScene);
		updaterButton->setDimensions(550, 125, Arriba::Graphics::Pivot::centre);
		updaterButton->transform.position = {settingsScene->width / 2 + 165, settingsScene->height * 3/4,0};
		updaterButton->setText("Update Amiigo");
		updaterButton->name = "UpdaterButton";
		updaterButton->enabled = false;
		//Callback for updating Amiigo
		updaterButton->registerCallback([](){
			if(!hasNetworkConnection()) static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("No network connection!");
			else
			{
				std::ofstream fileStream("sdmc:/config/amiigo/update.flag");
				fileStream.close();
				initSplash();
			}
		});

		//Credits Quad
		Arriba::Primitives::Quad* creditsQuad = new Arriba::Primitives::Quad(0,0,330, Arriba::Graphics::windowHeight - statusHeight, Arriba::Graphics::Pivot::topLeft);
		creditsQuad->setParent(settingsScene);
		creditsQuad->setColour({0,0,0,0.9});
		int yOffset = 0;
		//Credits text
		Arriba::Primitives::Text* creditsTitleText = new Arriba::Primitives::Text("Credits", 64);
		creditsTitleText->setColour({0,0.7,1,1});
		creditsTitleText->setParent(creditsQuad);
		creditsTitleText->transform.position = {creditsQuad->width/2, yOffset += creditsTitleText->height + 30,0};
		for (int i = 0; i < 5; i++)
		{
			std::string titleText = "Place holder";
			std::string nameText = "Place holder";
			switch (i)
			{
				case 0:
				titleText = "Developer";
				nameText = "CompSciOrBust";
				break;
				case 1:
				titleText = "Emuiibo dev";
				nameText = "XorTroll";
				break;
				case 2:
				titleText = "Contribuyente";
				nameText = "Kronos2308";
				break;
				case 3:
				titleText = "The Pizza Guy";
				nameText = "Za";
				break;
				case 4:
				titleText = "Amiibo API";
				nameText = "N3evin";
				break;
				case 5:
				titleText = "Beta testers";
				nameText = "Too many to name <3";
				break;
			}
			Arriba::Primitives::Text* titleTextObject = new Arriba::Primitives::Text(titleText.c_str(),38);
			titleTextObject->setParent(creditsQuad);
			titleTextObject->transform.position = {creditsQuad->width/2, yOffset += titleTextObject->height + 20,0};
			Arriba::Primitives::Text* nameTextObject = new Arriba::Primitives::Text(nameText.c_str(), 28);
			nameTextObject->setParent(creditsQuad);
			nameTextObject->transform.position = {creditsQuad->width/2, yOffset += nameTextObject->height + 10,0};
			titleTextObject->setColour({0,0.7,1,1});
			nameTextObject->setColour({0,0.7,1,1});
		}
	}

	void handleInput()
	{
		//We only want to handle input for the base layer of the UI
		if(Arriba::activeLayer != 0) return;
		//If no object is selected
		if(Arriba::highlightedObject == nullptr)
		{
			if(Arriba::Input::buttonDown(Arriba::Input::controllerButton(Arriba::Input::DPadRight | Arriba::Input::DPadLeft | Arriba::Input::DPadUp | Arriba::Input::DPadDown))) Arriba::highlightedObject = selectorButton;
		}
		//If one of the lists are selected
		for (size_t i = 0; i < lists.size(); i++)
		{
			//Right pressed
			if(Arriba::highlightedObject == lists[i] && Arriba::Input::buttonDown(Arriba::Input::DPadRight))
			{
				Arriba::highlightedObject = selectorButton;
				return;
			}
			//Selector list is selected
			if(lists[i]->name == "SelectorList" && lists[i]->enabled)
			{
				if(Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && selectorPath != "sdmc:/emuiibo/amiibo")
				{
					selectorPath = selectorPath.substr(0, selectorPath.find_last_of("/"));
					if(selectorPath.length() < sizeof("sdmc:/emuiibo/amiibo")) selectorPath = "sdmc:/emuiibo/amiibo";
					updateSelectorStrings();
				}
				if(Arriba::Input::buttonDown(Arriba::Input::XButtonSwitch))
				{
					switch (emu::GetEmulationStatus())
					{
						case emu::EmulationStatus::On:
							emu::ResetActiveVirtualAmiibo();
							emu::SetEmulationStatus(emu::EmulationStatus::Off);
							static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Emuiibo disabled");
						break;
						case emu::EmulationStatus::Off:
							emu::SetEmulationStatus(emu::EmulationStatus::On);
							static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Emuiibo enabled");
						break;
						default:
							static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Error: Unkown emulation status!");
						break;
					}
				}
			}
			//Maker list is selected
			else if(lists[i]->name == "MakerList" && lists[i]->enabled)
			{
				if(Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && makerIsInCategory)
				{
					static_cast<Arriba::Elements::InertialList*>(lists[i])->updateStrings(seriesList);
					makerIsInCategory = false;
				}
			}
			//Settings is selected
			else if(lists[i]->name == "SettingsScene" && lists[i]->enabled)
			{
				//Left / right pressed
				if(Arriba::Input::buttonDown(Arriba::Input::DPadRight) && Arriba::highlightedObject->tag != "SwitcherButton") Arriba::highlightedObject = selectorButton;
				if(Arriba::Input::buttonDown(Arriba::Input::DPadLeft) && Arriba::highlightedObject->tag == "SwitcherButton") Arriba::highlightedObject = Arriba::findObjectByName("CategorySettingsButton");
				//Up pressed
				if(Arriba::Input::buttonDown(Arriba::Input::DPadUp))
				{
					if(Arriba::highlightedObject == Arriba::findObjectByName("UpdateAPIButton")) Arriba::highlightedObject = Arriba::findObjectByName("CategorySettingsButton");
					else if(Arriba::highlightedObject == Arriba::findObjectByName("UpdaterButton")) Arriba::highlightedObject = Arriba::findObjectByName("UpdateAPIButton");
				}
				//Down pressed
				if(Arriba::Input::buttonDown(Arriba::Input::DPadDown))
				{
					if(Arriba::highlightedObject == Arriba::findObjectByName("CategorySettingsButton")) Arriba::highlightedObject = Arriba::findObjectByName("UpdateAPIButton");
					else if(Arriba::highlightedObject == Arriba::findObjectByName("UpdateAPIButton") && Arriba::findObjectByName("UpdaterButton")->enabled) Arriba::highlightedObject = Arriba::findObjectByName("UpdaterButton");
				}
			}
		}
		//If one of the switcher buttons are selected
		for (size_t i = 0; i < buttons.size(); i++)
		{
			if(Arriba::highlightedObject == buttons[i])
			{
				//If left is pressed switch to whichever list is enabled
				if(Arriba::Input::buttonDown(Arriba::Input::DPadLeft))
				{
					for (size_t j = 0; j < lists.size(); j++) if(lists[j]->enabled) Arriba::highlightedObject = lists[j];
				}
				//If up is pressed go to whichever button is above in the switcher
				else if(Arriba::Input::buttonDown(Arriba::Input::DPadUp))
				{
					if(buttons[i]->name == "MakerButton") Arriba::highlightedObject = selectorButton;
					else if(buttons[i]->name == "SettingsButton") Arriba::highlightedObject = makerButton;
					else if(buttons[i]->name == "ExitButton") Arriba::highlightedObject = settingsButton;
				}
				//If down button is pressed go to whichever button is below in the switcher
				else if(Arriba::Input::buttonDown(Arriba::Input::DPadDown))
				{
					if(buttons[i]->name == "SelectorButton") Arriba::highlightedObject = makerButton;
					else if(buttons[i]->name == "MakerButton") Arriba::highlightedObject = settingsButton;
					else if(buttons[i]->name == "SettingsButton") Arriba::highlightedObject = exitButton;
				}
				break;
			}
		}
	}

	void switcherPressed()
	{
		//Disable all the lists
		for (size_t i = 0; i < lists.size(); i++) lists[i]->enabled = false;
		//Enable a list depending on which button was pressed
		if(Arriba::highlightedObject == selectorButton)
		{
			selectorList->enabled = true;
			selectorPath = "sdmc:/emuiibo/amiibo";
			updateSelectorStrings();
			Arriba::Colour::neutral = {0.22,0.47,0.93,0.97};
	    	Arriba::Colour::highlightA = {0.1,0.95,0.98,0.97};
	    	Arriba::Colour::highlightB = {0.5,0.85,1,0.97};
			static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiigo + Arriba");
		}
		else if(Arriba::highlightedObject == makerButton)
		{
			makerList->enabled = true;
			if(makerIsInCategory)
			{
				makerIsInCategory = false;
			}
			makerList->updateStrings(seriesList);
			Arriba::Colour::neutral = {0.20,0.76,0.45,0.97};
	    	Arriba::Colour::highlightA = {0.6,0.95,0.98,0.97};
	    	Arriba::Colour::highlightB = {0.1,0.98,0.55,0.97};
			static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiigo Store");
		}
		else if(Arriba::highlightedObject == settingsButton)
		{
			settingsScene->enabled = true;
			Arriba::Colour::neutral = {0.57,0.21,0.93,0.97};
	    	Arriba::Colour::highlightA = {0.9,0.95,0.94,0.97};
	    	Arriba::Colour::highlightB = {1,0.85,1,0.97};
			static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Settings");
		}
	}

	void selectorInput(int index)
	{
		if(selectorAmiibos[index].isCategory)
		{
			if(checkIfFileExists(selectorAmiibos[index].path.c_str()) || selectorAmiibos[index].path == "Favorites")
			{
				selectorPath = selectorAmiibos[index].path;
				updateSelectorStrings();
			}
			else static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Folder does not exist");
		}
		else
		{
			char path[FS_MAX_PATH];
			strcpy(path, selectorAmiibos[index].path.c_str());
			emu::SetEmulationStatus(emu::EmulationStatus::On);
			emu::SetActiveVirtualAmiibo(path, FS_MAX_PATH);
			static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText(path);
		}
	}

	void makerInput(int index)
	{
		if(makerIsInCategory)
		{
			if(index == 0)
			{
				makerList->updateStrings(seriesList);
				makerIsInCategory = false;
			}
			else createVirtualAmiibo(creatorData[index-1]);
		}
		else
		{
			creatorData = getAmiibosFromSeries(seriesList[index]);
			std::vector<std::string> amiiboNames = {"¤ Back"};
			for (size_t i = 0; i < creatorData.size(); i++)
			{
				amiiboNames.push_back(creatorData[i].name);
			}
			makerList->updateStrings(amiiboNames);
			selectedSeries = seriesList[index];
			makerIsInCategory = true;
		}
	}

	void updateSelectorStrings()
	{
		selectorAmiibos = scanForAmiibo(selectorPath.c_str());
		std::vector<std::string> amiiboNames;
		for (size_t i = 0; i < selectorAmiibos.size(); i++)
		{
			amiiboNames.push_back(selectorAmiibos[i].name);
		}
		selectorList->updateStrings(amiiboNames);
	}

	void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos)
	{
		if(index != -1) Arriba::Primitives::Quad* contextMenu = new Amiigo::Elements::selectorContextMenu((int)pos.x, (int)pos.y, selectorAmiibos[index]);
	}
}