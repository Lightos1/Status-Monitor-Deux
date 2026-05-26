#pragma once

class ConfigurationMainMenu : public tsl::Gui {
private:
	std::unordered_map<std::string, std::string> configs;
	std::string buttonBackup;
public:
	ConfigurationMainMenu() {
		buttonBackup = defaultButtonView;
		defaultButtonView = locale["Footer"];
		auto it = config.find("status-monitor-deux");
		if (it != config.end()) configs = std::unordered_map<std::string, std::string>(it->second.begin(), it->second.end());

		if (configs.find("battery_avg_iir_filter") == configs.end()) configs["battery_avg_iir_filter"] = "false";
		if (configs.find("battery_time_left_refreshrate") == configs.end()) configs["battery_avg_iir_filter"] = "10";
		if (configs.find("touch_screen") == configs.end()) configs["battery_avg_iir_filter"] = "false";
		if (configs.find("motion_control") == configs.end()) configs["motion_control"] = "true";
		if (configs.find("left_joycon_motion_key_combo") == configs.end()) configs["left_joycon_motion_key_combo"] = "ZL+L+LSTICK";
		if (configs.find("right_joycon_motion_key_combo") == configs.end()) configs["right_joycon_motion_key_combo"] = "ZR+R+RSTICK";
		if (configs.find("pro_controller_motion_key_combo") == configs.end()) configs["pro_controller_motion_key_combo"] = "ZR+R+RSTICK";
		if (configs.find("jump_immediately_to_single_smd") == configs.end()) configs["jump_immediately_to_single_smd"] = "true";
		if (configs.find("save_and_load_movable_overlay_position") == configs.end()) configs["save_and_load_movable_overlay_position"] = "true";
		if (configs.find("override_language") == configs.end()) configs["override_language"] = "false";
		if (configs.find("override_language_ietf_code") == configs.end()) configs["override_language_ietf_code"] = "EN-US";
	}

	~ConfigurationMainMenu() {
		defaultButtonView = buttonBackup;
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, locale["Settings"]);
		auto list = new tsl::elm::List();

		{
			auto Item = new tsl::elm::ListItem(locale["key_combo"]);
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					//tsl::changeTo<EditConfigCombo>(true);
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ToggleListItem(locale["battery_avg_iir_filter"], configs["battery_avg_iir_filter"] == "false" ? false : true);
			Item->setClickListener([this, Item](uint64_t keys) {
				if (keys & KEY_A) {
					configs["battery_avg_iir_filter"] = Item->getState() ? "true" : "false";
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ListItem(locale["battery_time_left_refreshrate"], configs["battery_time_left_refreshrate"]);
			Item->setClickListener([this, Item](uint64_t keys) {
				if (keys & KEY_A) {
					tsl::changeTo<EditConfigInt>("battery_time_left_refreshrate", configs["battery_time_left_refreshrate"], "1", "60", "10", Item, locale["battery_time_left_refreshrate"], "int");
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ToggleListItem(locale["touch_screen"], configs["touch_screen"] == "false" ? false : true);
			Item->setClickListener([this, Item](uint64_t keys) {
				if (keys & KEY_A) {
					configs["touch_screen"] = Item->getState() ? "true" : "false";
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ToggleListItem(locale["motion_control"], configs["motion_control"] == "false" ? false : true);
			Item->setClickListener([this, Item](uint64_t keys) {
				if (keys & KEY_A) {
					configs["motion_control"] = Item->getState() ? "true" : "false";
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ListItem(locale["left_joycon_motion_key_combo"]);
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					//tsl::changeTo<EditConfigCombo>(false);
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ListItem(locale["right_joycon_motion_key_combo"]);
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					//tsl::changeTo<EditConfigCombo>(false);
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ListItem(locale["pro_controller_motion_key_combo"]);
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					//tsl::changeTo<EditConfigCombo>(false);
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ToggleListItem(locale["jump_immediately_to_single_smd"], configs["jump_immediately_to_single_smd"] == "false" ? false : true);
			Item->setClickListener([this, Item](uint64_t keys) {
				if (keys & KEY_A) {
					configs["jump_immediately_to_single_smd"] = Item->getState() ? "true" : "false";
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		{
			auto Item = new tsl::elm::ListItem(locale["override_language"]);
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					//tsl::changeTo<EditConfigLanguage>();
					return true;
				}
				return false;
			});		
			list->addItem(Item);
		}

		rootFrame->setContent(list);
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & KEY_B) {
			tsl::hlp::requestForeground(true);
			tsl::goBack();
			return true;
		}
		return false;
	}
};
