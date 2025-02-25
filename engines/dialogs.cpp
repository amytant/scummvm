/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/about.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/scrollcontainer.h"

#include "graphics/font.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/metaengine.h"

MainMenuDialog::MainMenuDialog(Engine *engine)
	: GUI::Dialog("GlobalMenu"), _engine(engine) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->setGfxFromTheme(GUI::ThemeEngine::kImageLogoSmall);
	} else {
		GUI::StaticTextWidget *title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
		title->setAlign(Graphics::kTextAlignCenter);
	}
#else
	GUI::StaticTextWidget *title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
	title->setAlign(Graphics::kTextAlignCenter);
#endif

	GUI::StaticTextWidget *version = new GUI::StaticTextWidget(this, "GlobalMenu.Version", Common::U32String(gScummVMVersionDate));
	version->setAlign(Graphics::kTextAlignCenter);

	new GUI::ButtonWidget(this, "GlobalMenu.Resume", _("~R~esume"), Common::U32String(), kPlayCmd, 'P');

	new GUI::ButtonWidget(this, "GlobalMenu.Load", _("~L~oad"), Common::U32String(), kLoadCmd);
	new GUI::ButtonWidget(this, "GlobalMenu.Save", _("~S~ave"), Common::U32String(), kSaveCmd);

	new GUI::ButtonWidget(this, "GlobalMenu.Options", _("~O~ptions"), Common::U32String(), kOptionsCmd);

	// The help button is disabled by default.
	// To enable "Help", an engine needs to use a subclass of MainMenuDialog
	// (at least for now, we might change how this works in the future).
	_helpButton = new GUI::ButtonWidget(this, "GlobalMenu.Help", _("~H~elp"), Common::U32String(), kHelpCmd);
	_helpButton->setVisible(_engine->hasFeature(Engine::kSupportsHelp));
	_helpButton->setEnabled(_engine->hasFeature(Engine::kSupportsHelp));

	new GUI::ButtonWidget(this, "GlobalMenu.About", _("~A~bout"), Common::U32String(), kAboutCmd);

	if (g_gui.getGUIWidth() > 320)
		_returnToLauncherButton = new GUI::ButtonWidget(this, "GlobalMenu.ReturnToLauncher", _("~R~eturn to Launcher"), Common::U32String(), kLauncherCmd);
	else
		_returnToLauncherButton = new GUI::ButtonWidget(this, "GlobalMenu.ReturnToLauncher", _c("~R~eturn to Launcher", "lowres"), Common::U32String(), kLauncherCmd);
	_returnToLauncherButton->setEnabled(_engine->hasFeature(Engine::kSupportsReturnToLauncher));

	if (!g_system->hasFeature(OSystem::kFeatureNoQuit) && (!(ConfMan.getBool("gui_return_to_launcher_at_exit")) || !_engine->hasFeature(Engine::kSupportsReturnToLauncher)))
		new GUI::ButtonWidget(this, "GlobalMenu.Quit", _("~Q~uit"), Common::U32String(), kQuitCmd);

	_aboutDialog = new GUI::AboutDialog();
	_loadDialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	_saveDialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _loadDialog;
	delete _saveDialog;
}

void MainMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayCmd:
		close();
		break;
	case kLoadCmd:
		load();
		break;
	case kSaveCmd:
		save();
		break;
	case kOptionsCmd: {
		GUI::ConfigDialog configDialog;
		configDialog.runModal();
		break;
	}
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
	case kHelpCmd: {
		GUI::MessageDialog dialog(
					_("Sorry, this engine does not currently provide in-game help. "
					"Please consult the README for basic information, and for "
					"instructions on how to obtain further assistance."));
		dialog.runModal();
		}
		break;
	case kLauncherCmd: {
		Common::Event eventReturnToLauncher;
		eventReturnToLauncher.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(eventReturnToLauncher);
		close();
		}
		break;
	case kQuitCmd: {
		Common::Event eventQ;
		eventQ.type = Common::EVENT_QUIT;
		g_system->getEventManager()->pushEvent(eventQ);
		close();
		}
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::reflowLayout() {
	// Overlay size might have changed since the construction of the dialog.
	// Update labels when it might be needed
	// FIXME: it might be better to declare GUI::StaticTextWidget::setLabel() virtual
	// and to reimplement it in GUI::ButtonWidget to handle the hotkey.
	if (g_gui.getGUIWidth() > 320)
		_returnToLauncherButton->setLabel(_returnToLauncherButton->cleanupHotkey(_("~R~eturn to Launcher")));
	else
		_returnToLauncherButton->setLabel(_returnToLauncherButton->cleanupHotkey(_c("~R~eturn to Launcher", "lowres")));

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		if (!_logo)
			_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->setGfxFromTheme(GUI::ThemeEngine::kImageLogoSmall);

		GUI::StaticTextWidget *title = (GUI::StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (title) {
			removeWidget(title);
			title->setNext(0);
			delete title;
		}
	} else {
		GUI::StaticTextWidget *title = (GUI::StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (!title) {
			title = new GUI::StaticTextWidget(this, "GlobalMenu.Title", Common::U32String("ScummVM"));
			title->setAlign(Graphics::kTextAlignCenter);
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}
#endif

	Dialog::reflowLayout();
}

void MainMenuDialog::save() {
	if (!_engine->hasFeature(Engine::kSupportsSavingDuringRuntime)) {
		GUI::MessageDialog dialog(_("This game does not support saving from the menu. Use in-game interface"));
		dialog.runModal();

		return;
	}

	Common::U32String msg;
	if (!_engine->canSaveGameStateCurrently(&msg)) {
		if (msg.empty())
			msg = _("This game cannot be saved at this time. Please try again later");

		GUI::MessageDialog dialog(msg);
		dialog.runModal();

		return;
	}

	int slot = _saveDialog->runModalWithCurrentTarget();

	if (slot >= 0) {
		Common::String result(_saveDialog->getResultString());
		if (result.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			result = _saveDialog->createDefaultSaveDescription(slot);
		}

		Common::Error status = _engine->saveGameState(slot, result);
		if (status.getCode() != Common::kNoError) {
			Common::U32String failMessage = Common::U32String::format(_("Failed to save game (%s)! "
				  "Please consult the README for basic information, and for "
				  "instructions on how to obtain further assistance."), status.getDesc().c_str());
			GUI::MessageDialog dialog(failMessage);
			dialog.runModal();
		}

		close();
	}
}

void MainMenuDialog::load() {
	if (!_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime)) {
		GUI::MessageDialog dialog(_("This game does not support loading from the menu. Use in-game interface"));
		dialog.runModal();

		return;
	}

	Common::U32String msg;
	if (!_engine->canLoadGameStateCurrently(&msg)) {
		if (msg.empty())
			msg = _("This game cannot be loaded at this time. Please try again later");

		GUI::MessageDialog dialog(msg);
		dialog.runModal();

		return;
	}

	int slot = _loadDialog->runModalWithCurrentTarget();

	_engine->setGameToLoadSlot(slot);

	if (slot >= 0)
		close();
}

namespace GUI {

// FIXME: We use the empty string as domain name here. This tells the
// ConfigManager to use the 'default' domain for all its actions. We do that
// to get as close as possible to editing the 'active' settings.
//
// However, that requires bad & evil hacks in the ConfigManager code,
// and even then still doesn't work quite correctly.
// For example, if the transient domain contains 'false' for the 'fullscreen'
// flag, but the user used a hotkey to switch to windowed mode, then the dialog
// will display the wrong value anyway.
//
// Proposed solution consisting of multiple steps:
// 1) Add special code to the open() code that reads out everything stored
//    in the transient domain that is controlled by this dialog, and updates
//    the dialog accordingly.
// 2) Even more code is added to query the backend for current settings, like
//    the fullscreen mode flag etc., and also updates the dialog accordingly.
// 3) The domain being edited is set to the active game domain.
// 4) If the dialog is closed with the "OK" button, then we remove everything
//    stored in the transient domain (or at least everything corresponding to
//    switches in this dialog.
//    If OTOH the dialog is closed with "Cancel" we do no such thing.
//
// These changes will achieve two things at once: Allow us to get rid of using
//  "" as value for the domain, and in fact provide a somewhat better user
// experience at the same time.
ConfigDialog::ConfigDialog() :
		GUI::OptionsDialog("", "GlobalConfig"),
		_engineOptions(nullptr) {
	assert(g_engine);

	const Common::String &gameDomain = ConfMan.getActiveDomainName();
	const MetaEngine *metaEngine = g_engine->getMetaEngine();

	// GUI:  Add tab widget
	GUI::TabWidget *tab = new GUI::TabWidget(this, "GlobalConfig.TabWidget");

	//
	// The game specific options tab
	//

	int tabId = tab->addTab(_("Game"), "GlobalConfig_Engine");

	if (g_engine->hasFeature(Engine::kSupportsChangingOptionsDuringRuntime)) {
		ScrollContainerWidget *engineContainer = new ScrollContainerWidget(tab, "GlobalConfig_Engine.Container", "GlobalConfig_Engine_Container");
		engineContainer->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);
		engineContainer->setTarget(this);

		_engineOptions = metaEngine->buildEngineOptionsWidget(engineContainer, "GlobalConfig_Engine_Container.Container", gameDomain);
	}

	if (_engineOptions) {
		_engineOptions->setParentDialog(this);
	} else {
		tab->removeTab(tabId);
	}

	//
	// The Audio / Subtitles tab
	//

	tab->addTab(_("Audio"), "GlobalConfig_Audio");

	//
	// Sound controllers
	//

	addVolumeControls(tab, "GlobalConfig_Audio.");
	setVolumeSettingsState(true); // could disable controls by GUI options

	//
	// Subtitle speed and toggle controllers
	//

	if (g_engine->hasFeature(Engine::kSupportsSubtitleOptions)) {
		// Global talkspeed range of 0-255
		addSubtitleControls(tab, "GlobalConfig_Audio.", 255);
		setSubtitleSettingsState(true); // could disable controls by GUI options
	}

	//
	// The Keymap tab
	//

	Common::KeymapArray keymaps = metaEngine->initKeymaps(gameDomain.c_str());
	if (!keymaps.empty()) {
		tab->addTab(_("Keymaps"), "GlobalConfig_KeyMapper");

		ScrollContainerWidget *keymapContainer = new ScrollContainerWidget(tab, "GlobalConfig_KeyMapper.Container", "GlobalConfig_KeyMapper_Container");
		keymapContainer->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);
		keymapContainer->setTarget(this);

		addKeyMapperControls(keymapContainer, "GlobalConfig_KeyMapper_Container.", keymaps, gameDomain);
	}

	//
	// The backend tab (shown only if the backend implements one)
	//
	int backendTabId = tab->addTab(_("Backend"), "GlobalConfig_Backend");

	ScrollContainerWidget *backendContainer = new ScrollContainerWidget(tab, "GlobalConfig_Backend.Container", "GlobalConfig_Backend_Container");
	backendContainer->setBackgroundType(ThemeEngine::kWidgetBackgroundNo);
	backendContainer->setTarget(this);

	_backendOptions = g_system->buildBackendOptionsWidget(backendContainer, "GlobalConfig_Backend_Container.Container", gameDomain);

	if (_backendOptions) {
		_backendOptions->setParentDialog(this);
	} else {
		tab->removeTab(backendTabId);
	}

	//
	// The Achievements & The Statistics tabs
	//
	AchMan.setActiveDomain(metaEngine->getAchievementsInfo(gameDomain));
	if (AchMan.getAchievementCount()) {
		tab->addTab(_("Achievements"), "GlobalConfig_Achievements");
		addAchievementsControls(tab, "GlobalConfig_Achievements.");
	}
	if (AchMan.getStatCount()) {
		tab->addTab(_("Statistics"), "GlobalConfig_Achievements");
		addStatisticsControls(tab, "GlobalConfig_Achievements.");
	}

	// Activate the first tab
	tab->setActiveTab(0);

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("~O~K"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("~C~ancel"), Common::U32String(), GUI::kCloseCmd);
}

ConfigDialog::~ConfigDialog() {
}

void ConfigDialog::build() {
	OptionsDialog::build();

	// Engine options
	if (_engineOptions) {
		_engineOptions->load();
	}
}

void ConfigDialog::apply() {
	if (_engineOptions) {
		_engineOptions->save();
	}

	OptionsDialog::apply();
}

ExtraGuiOptionsWidget::ExtraGuiOptionsWidget(GuiObject *containerBoss, const Common::String &name, const Common::String &domain, const ExtraGuiOptions &options) :
		OptionsContainerWidget(containerBoss, name, "ExtraGuiOptionsDialog", domain),
		_options(options) {

	for (uint i = 0; i < _options.size(); i++) {
		Common::String id = Common::String::format("%d", i + 1);
		uint32 cmd = _options[i].groupLeaderId ? kClickGroupLeaderCmd : 0;
		_checkboxes.push_back(new CheckboxWidget(widgetsBoss(),
			_dialogLayout + ".customOption" + id + "Checkbox", _(_options[i].label), _(_options[i].tooltip), cmd));
	}
}

ExtraGuiOptionsWidget::~ExtraGuiOptionsWidget() {
}

void ExtraGuiOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kClickGroupLeaderCmd: {
		byte groupLeaderId = 0;

		for (uint i = 0; i < _checkboxes.size(); i++) {
			if (_checkboxes[i] == (CheckboxWidget *)sender) {
				groupLeaderId = _options[i].groupLeaderId;
				break;
			}
		}

		if (!groupLeaderId)
			break;

		// We have found the "group leader" checkbox. Enable or disable
		// all checkboxes in the group. Theoretically, this could mean
		// that we disable another group leader, so its group should
		// also be disabled. But that seems overkill for now.

		for (uint i = 0; i < _options.size(); i++) {
			if (_options[i].groupId == groupLeaderId) {
				_checkboxes[i]->setEnabled(data != 0);
			}
		}
		break;
	}
	default:
		OptionsContainerWidget::handleCommand(sender, cmd, data);
		break;
	}
}

void ExtraGuiOptionsWidget::load() {
	// Set the state of engine-specific checkboxes
	for (uint j = 0; j < _options.size() && j < _checkboxes.size(); ++j) {
		// The default values for engine-specific checkboxes are not set when
		// ScummVM starts, as this would require us to load and poll all of the
		// engine plugins on startup. Thus, we set the state of each custom
		// option checkbox to what is specified by the engine plugin, and
		// update it only if a value has been set in the configuration of the
		// currently selected game.
		bool isChecked = _options[j].defaultState;
		if (ConfMan.hasKey(_options[j].configOption, _domain))
			isChecked = ConfMan.getBool(_options[j].configOption, _domain);
		_checkboxes[j]->setState(isChecked);
	}
}

bool ExtraGuiOptionsWidget::save() {
	// Set the state of engine-specific checkboxes
	for (uint i = 0; i < _options.size() && i < _checkboxes.size(); i++) {
		ConfMan.setBool(_options[i].configOption, _checkboxes[i]->isEnabled() && _checkboxes[i]->getState(), _domain);
	}

	return true;
}

void ExtraGuiOptionsWidget::defineLayout(ThemeEval& layouts, const Common::String& layoutName, const Common::String& overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 0, 0);

	for (uint i = 0; i < _options.size(); i++) {
		Common::String id = Common::String::format("%d", i + 1);
		layouts.addWidget("customOption" + id + "Checkbox", "Checkbox");
	}

	layouts.closeLayout().closeDialog();
}

} // End of namespace GUI
