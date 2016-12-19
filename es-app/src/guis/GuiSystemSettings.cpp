#include "EmulationStation.h"
#include "guis/GuiWifi.h"
#include "guis/GuiStorageInfo.h"
#include "guis/GuiEmulatorList.h"
#include "guis/GuiSystemSettings.h"
#include "guis/GuiWifiConnect.h"
#include "guis/GuiKeyboard.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiDetectDevice.h"
#include "views/ViewController.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/ProgressBarComponent.h"
#include "components/MenuComponent.h"
#include "VolumeControl.h"
#include "scrapers/GamesDBScraper.h"
#include "scrapers/TheArchiveScraper.h"
#include "guis/GuiTextEditPopup.h"

GuiSystemSettings::GuiSystemSettings(Window* window) : GuiComponent(window), mMenu(window, "SYSTEM SETTINGS"), mVersion(window)
{
	// SYSTEM SETTINGS

	// UPDATES >
	// NETWORK SETTINGS >
	// STORAGE >

	/// Change network settings
	addEntry("NETWORK SETTINGS", 0x777777FF, true, [this, window] {
		mWindow->pushGui(new GuiWifi(mWindow));
	});

	addEntry("EMULATORS", 0x777777FF, true, [this, window] {
		mWindow->pushGui(new GuiEmulatorList(window));
	});

	addEntry("OTHER SETTINGS", 0x777777FF, true,
		[this] {
		auto s = new GuiSettings(mWindow, "OTHER SETTINGS");
		
		// gamelists
		auto save_gamelists = std::make_shared<SwitchComponent>(mWindow);
		save_gamelists->setState(Settings::getInstance()->getBool("SaveGamelistsOnExit"));
		s->addWithLabel("SAVE METADATA ON EXIT", save_gamelists);
		s->addSaveFunc([save_gamelists] { Settings::getInstance()->setBool("SaveGamelistsOnExit", save_gamelists->getState()); });

		auto parse_gamelists = std::make_shared<SwitchComponent>(mWindow);
		parse_gamelists->setState(Settings::getInstance()->getBool("ParseGamelistOnly"));
		s->addWithLabel("PARSE GAMESLISTS ONLY", parse_gamelists);
		s->addSaveFunc([parse_gamelists] { Settings::getInstance()->setBool("ParseGamelistOnly", parse_gamelists->getState()); });
		
		mWindow->pushGui(s);
	});
			
	auto openScrapeNow = [this] { mWindow->pushGui(new GuiScraperStart(mWindow)); };
	addEntry("SCRAPER", 0x777777FF, true, 
		[this, openScrapeNow] { 
			auto s = new GuiSettings(mWindow, "SCRAPER");

			// scrape from
			auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, "SCRAPE FROM", false);
			std::vector<std::string> scrapers = getScraperList();
			for(auto it = scrapers.begin(); it != scrapers.end(); it++)
				scraper_list->add(*it, *it, *it == Settings::getInstance()->getString("Scraper"));

			s->addWithLabel("SCRAPE FROM", scraper_list);
			s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

			// scrape ratings
			auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
			scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
			s->addWithLabel("SCRAPE RATINGS", scrape_ratings);
			s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

			// scrape now
			ComponentListRow row;
			std::function<void()> openAndSave = openScrapeNow;
			openAndSave = [s, openAndSave] { s->save(); openAndSave(); };
			row.makeAcceptInputHandler(openAndSave);

			auto scrape_now = std::make_shared<TextComponent>(mWindow, "SCRAPE NOW", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
			auto bracket = makeArrow(mWindow);
			row.addElement(scrape_now, true);
			row.addElement(bracket, false);
			s->addRow(row);

			mWindow->pushGui(s);
	});

	/// See storage on internal memory card.
	addEntry("STORAGE", 0x777777FF, true, [this] {
		mWindow->pushGui(new GuiStorageInfo(mWindow));
	});

	mVersion.setFont(Font::get(FONT_SIZE_SMALL));
	mVersion.setColor(0xAAAAFFFF);
	mVersion.setText("BUILD " + strToUpper(PROGRAM_VERSION_STRING));
	mVersion.setAlignment(ALIGN_CENTER);

	addChild(&mMenu);
	addChild(&mVersion);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiSystemSettings::onSizeChanged()
{
	mVersion.setSize(mSize.x(), 0);
	mVersion.setPosition(0, mSize.y() - mVersion.getSize().y());
}

void GuiSystemSettings::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}



bool GuiSystemSettings::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiSystemSettings::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
