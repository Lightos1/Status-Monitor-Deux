#pragma once

class ConfigurationSubMenu : public tsl::Gui {
private:
	std::string m_type;
	std::string showType;
	std::string m_name;
public:
	ConfigurationSubMenu(std::string type, std::string name) {
		m_type = type;
		if (m_type.compare("bool") == 0) {
			showType = "\uE142\uE14B\uE14C";
		}
		else if (m_type.compare("int") == 0) {
			showType = "1\uE08C60";
		}
		else if (m_type.compare("color") == 0) {
			showType = "\uE135";
		}
		else if (m_type.compare("list") == 0) {
			showType = "\uE047\uE048";
		}
		m_name = name + "\n" + showType;
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_name);
		auto list = new tsl::elm::List();

		for (const auto& [key, data] : configs) {
			if (data.type.compare(m_type) == 0) {
				if (m_type.compare("int") == 0) {
					int64_t temp;
					if (isNumeric(data.rangeMin, &temp) == true && isNumeric(data.rangeMax, &temp) == true) {
						auto Item = new tsl::elm::ListItem(key, data.value);
						Item->setClickListener([this, key, data, Item](uint64_t keys) {
							if (keys & KEY_A) {
								tsl::hlp::requestForeground(true);
								tsl::changeTo<EditConfigInt>(key, configs.at(key).value, data.rangeMin, data.rangeMax, data.defaultValue, Item);
								return true;
							}
							return false;
						});
						list->addItem(Item);
					}
					else {
						auto Item = new tsl::elm::ListItem(key, data.type.c_str(), true);
						list->addItem(Item);
					}
				}
				else if (m_type.compare("list") == 0) {
					auto Item = new tsl::elm::ListItem(key);
					Item->setClickListener([this, key, data, Item](uint64_t keys) {
						if (keys & KEY_A) {
							std::string value = configs[key].value;
							std::string defaultValue = data.defaultValue;
							if (value.starts_with("LIST")) value = listToFlatList(value);
							if (defaultValue.starts_with("LIST")) defaultValue = listToFlatList(defaultValue);
							tsl::hlp::requestForeground(true);
							tsl::changeTo<EditConfigOrdering>(key, value, defaultValue, Item);
							return true;
						}
						return false;
					});
					list->addItem(Item);
				}
				else if (data.type.compare("bool") == 0) {
					bool isTrue = data.value.compare("true") == 0;
					auto Item = new tsl::elm::ToggleListItem(key, isTrue);
					Item->setClickListener([this, key, Item](uint64_t keys) {
						if (keys & KEY_A) {
							configs.at(key).value = Item->getState() ? "true" : "false";
							return true;
						}
						return false;
					});
					list->addItem(Item);
				}
				else if (data.type.compare("color") == 0) {
					bool isValid = false;
					u16 color;
					std::string error;
					if (data.value.length() == 13) {
						std::string hexColor = data.value.substr(6);
						hexColor = hexColor.substr(0, 6);
						isValid = isValid4444HexColor(hexColor);
						if (isValid) {
							std::string r = hexColor.substr(2, 1);
							std::string g = hexColor.substr(3, 1);
							std::string b = hexColor.substr(4, 1);
							std::string a = hexColor.substr(5, 1);
							color = tsl::gfx::Color((u8)std::stoi(r, nullptr, 16), (u8)std::stoi(g, nullptr, 16), (u8)std::stoi(b, nullptr, 16), (u8)std::stoi(a, nullptr, 16)).rgba;
						}
						else error = "invalid color";
					}
					if (isValid) {
						auto Item = new tsl::elm::ColorListItem(key, color);
						Item->setClickListener([this, key, Item, color](uint64_t keys) {
							if (keys & KEY_A) {
								tsl::hlp::requestForeground(true);
								tsl::changeTo<EditConfigColor>(key, Item);
								return true;
							}
							return false;
						});
						list->addItem(Item);
					}
					else {
						auto Item = new tsl::elm::ListItem(error, data.type.c_str(), true);
						list->addItem(Item);
					}
				}
				else {
					auto Item = new tsl::elm::ListItem(key, "\uE150");
					list->addItem(Item);
				}
			}
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
