#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include "Utils.hpp"
#include <malloc.h>
#include <set>

#include <cstdlib>

tsl::elm::OverlayFrame* rootFrame = nullptr;

std::string file_to_load = "";
HidSixAxisSensorHandle sixaxisHandles[Controller_Max];

#include "rendering_pipeline.hpp"
struct Data {
	std::string value;
	std::string type;
	std::string rangeMin;
	std::string rangeMax;
	std::string defaultValue;
};

class EditConfigColor : public tsl::Gui {
private:
	std::string m_key;
	tsl::elm::ColorListItem* m_item;
	u16 m_color;
	std::map<std::string, Data>* configs;
public:
	uint64_t start_tick;
	int32_t row = 0;
	uint32_t selected_r;
	uint32_t selected_b;
	uint32_t selected_g;
	uint32_t selected_a;
	int32_t selected_predefined;
	std::array<u16, 16> predefinedColors = {0xFF7A, 0xFFF0, 0xFF80, 0xFF8E, 0xF053, 0xF0FF, 0xFFDC, 0xF808, 0xFEEE, 0xF80F, 0xF6CC, 0xF0F8, 0xF744, 0xFAAF, 0xF080, 0xF82E};

	EditConfigColor(std::string key, tsl::elm::ColorListItem* item, std::map<std::string, Data>* m_configs) {
		configs = m_configs;
		tsl::hlp::requestForeground(true);
		m_item = item;
		m_key = key;
		m_color = item->getColor().rgba;
		start_tick = svcGetSystemTick();
		uint32_t r = m_color & 0xF;
		uint32_t g = (m_color >> 4) & 0xF;
		uint32_t b = (m_color >> 8) & 0xF;
		uint32_t a = (m_color >> 12) & 0xF;

		selected_r = 15 - r;
		selected_g = 15 - g;
		selected_b = 15 - b;
		selected_a = a;
		selected_predefined = -1;
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_key);
		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			const size_t position_y = 120;
			const size_t position_x = 60;
			const size_t radius = 18;
			const size_t border_size = 10;
			renderer->drawCircle(position_x, position_y, radius + border_size, true, 0xFFFF);
			renderer->drawCircle(position_x, position_y, radius, true, (m_color & 0xFFF) + 0xF000);
			renderer->drawCircle(position_x+80, position_y, radius + border_size, true, 0xF000);
			renderer->drawCircle(position_x+80, position_y, radius, true, (m_color & 0xFFF) + 0xF000);
			renderer->drawCircle(position_x+160, position_y, radius + border_size, true, 0xF00F);
			renderer->drawCircle(position_x+160, position_y, radius, true, (m_color & 0xFFF) + 0xF000);
			renderer->drawCircle(position_x+240, position_y, radius + border_size, true, 0xF0F0);
			renderer->drawCircle(position_x+240, position_y, radius, true, (m_color & 0xFFF) + 0xF000);
			renderer->drawCircle(position_x+320, position_y, radius + border_size, true, 0xFF00);
			renderer->drawCircle(position_x+320, position_y, radius, true, (m_color & 0xFFF) + 0xF000);

			const size_t square_position_y = 160;
			u16 colorRed = 0xF00F;
			u16 colorGreen = 0xF0F0;
			u16 colorBlue = 0xFF00;
			u16 transparency = 0x0000;
			const size_t square_size = 27;
			for(size_t i = 0; i < 16; i++) {
				renderer->drawRect(position_x+147, square_position_y+(i * (square_size+2)), square_size, square_size, colorRed);
				colorRed--;
				renderer->drawRect(position_x+227, square_position_y+(i * (square_size+2)), square_size, square_size, colorGreen);
				colorGreen -= 0x10;
				renderer->drawRect(position_x+308, square_position_y+(i * (square_size+2)), square_size, square_size, colorBlue);
				colorBlue -= 0x100;
				renderer->drawRect(position_x+67, square_position_y+(i * (square_size+2)), square_size, square_size, transparency);
				transparency += 0x1000;
				renderer->drawRect(position_x-13, square_position_y+(i * (square_size+2)), square_size, square_size, predefinedColors[i]);
			}
			
			const double cycleDuration = 1.0;  // 1 second for one full sine wave
			u64 delta_tick = svcGetSystemTick() - start_tick;
			double seconds = (double)delta_tick / (double)systemtickfrequency;
			double timeCounter = fmod(seconds, cycleDuration);
			float progress = (std::sin(2 * M_PI * timeCounter / cycleDuration) + 1) / 2;

			tsl::gfx::Color highlightColor1 = tsl::gfx::Color(0xFC82);
			tsl::gfx::Color highlightColor2 = tsl::gfx::Color(0xFFF8);

			tsl::gfx::Color highlightColor = {
                static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r),
                static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g),
                static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b),
                0xF
            };

			renderer->drawEmptyRect(position_x+146, square_position_y+(selected_r * (square_size+2))-1, square_size + 2, square_size + 2, highlightColor);
			renderer->drawEmptyRect(position_x+145, square_position_y+(selected_r * (square_size+2))-2, square_size + 4, square_size + 4, highlightColor);
			renderer->drawEmptyRect(position_x+226, square_position_y+(selected_g * (square_size+2))-1, square_size + 2, square_size + 2, highlightColor);
			renderer->drawEmptyRect(position_x+225, square_position_y+(selected_g * (square_size+2))-2, square_size + 4, square_size + 4, highlightColor);
			renderer->drawEmptyRect(position_x+307, square_position_y+(selected_b * (square_size+2))-1, square_size + 2, square_size + 2, highlightColor);
			renderer->drawEmptyRect(position_x+306, square_position_y+(selected_b * (square_size+2))-2, square_size + 4, square_size + 4, highlightColor);
			renderer->drawEmptyRect(position_x+66, square_position_y+(selected_a * (square_size+2))-1, square_size + 2, square_size + 2, highlightColor);
			renderer->drawEmptyRect(position_x+65, square_position_y+(selected_a * (square_size+2))-2, square_size + 4, square_size + 4, highlightColor);

			if (selected_predefined >= 0) {
				renderer->drawEmptyRect(position_x-14, square_position_y+(selected_predefined * (square_size+2))-1, square_size + 2, square_size + 2, highlightColor);
				renderer->drawEmptyRect(position_x-15, square_position_y+(selected_predefined * (square_size+2))-2, square_size + 4, square_size + 4, highlightColor);				
			}

			renderer->drawEmptyRect(position_x+60 + (row * 80), square_position_y-9, 40, (square_size+3) * 16, highlightColor);
        });
		rootFrame->setContent(Status);
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & KEY_B) {
			tsl::goBack();
			return true;
		}
		else if (keysDown & KEY_DRIGHT) {
			if (row == -1) {
				auto it = std::find(predefinedColors.cbegin(), predefinedColors.cend(), m_color);
				if (it == predefinedColors.cend()) selected_predefined = -1;
			}
			if (row < 3) row++;
		}
		else if (keysDown & KEY_DLEFT) {
			if (row == -1) {
				m_color = predefinedColors[selected_predefined];
			}
			if (row > -1) row--;
			if (row == -1) {
				auto it = std::find(predefinedColors.cbegin(), predefinedColors.cend(), m_color);
				if (it == predefinedColors.cend()) selected_predefined = 0;
			}
		}
		else if (keysDown & KEY_DDOWN) {
			switch(row) {
				case -1: 
					if (selected_predefined < 15) {
						selected_predefined++;
						m_color = predefinedColors[selected_predefined];
					}
					break;
				case 0: if (selected_a < 15) selected_a++; break;
				case 1: if (selected_r < 15) selected_r++; break;
				case 2: if (selected_g < 15) selected_g++; break;
				case 3: if (selected_b < 15) selected_b++; break;
			}
			if (row > -1) {
				m_color = (selected_a << 12) + ((15 - selected_b) << 8) + ((15 - selected_g) << 4) + (15 - selected_r);
				auto it = std::find(predefinedColors.cbegin(), predefinedColors.cend(), m_color);
				if (it == predefinedColors.cend()) selected_predefined = -1;
			}
			else {
				uint32_t r = m_color & 0xF;
				uint32_t g = (m_color >> 4) & 0xF;
				uint32_t b = (m_color >> 8) & 0xF;

				selected_r = 15 - r;
				selected_g = 15 - g;
				selected_b = 15 - b;
			}
		}
		else if (keysDown & KEY_DUP) {
			switch(row) {
				case -1: 
					if (selected_predefined > 0) {
						selected_predefined--;
						m_color = predefinedColors[selected_predefined];
					}
					break;
				case 0: if (selected_a > 0) selected_a--; break;
				case 1: if (selected_r > 0) selected_r--; break;
				case 2: if (selected_g > 0) selected_g--; break;
				case 3: if (selected_b > 0) selected_b--; break;
			}
			if (row > -1) {
				m_color = (selected_a << 12) + ((15 - selected_b) << 8) + ((15 - selected_g) << 4) + (15 - selected_r);
				auto it = std::find(predefinedColors.cbegin(), predefinedColors.cend(), m_color);
				if (it == predefinedColors.cend()) selected_predefined = -1;
			}
			else {
				uint32_t r = m_color & 0xF;
				uint32_t g = (m_color >> 4) & 0xF;
				uint32_t b = (m_color >> 8) & 0xF;

				selected_r = 15 - r;
				selected_g = 15 - g;
				selected_b = 15 - b;
			}
		}
		if (keysDown & KEY_A) {
			char buffer[16] = "";
			uint32_t r = m_color & 0xF;
			uint32_t g = (m_color >> 4) & 0xF;
			uint32_t b = (m_color >> 8) & 0xF;
			uint16_t in_color = (r << 12) + (g << 8) + (b << 4) + selected_a;
			snprintf(buffer, sizeof(buffer), "COLOR{0x%04X}", in_color);
			configs->at(m_key).value = buffer;
			m_item->setColor((m_color & 0xFFF) + (selected_a << 12));
			tsl::goBack();
			return true;
		}
		return false;
	}
};

class EditConfigInt : public tsl::Gui {
private:
	std::string m_key;
	std::string min_str;
	std::string max_str;
	std::string defaultvalue_str;
	int64_t min;
	int64_t max;
	int64_t current_value;
	int64_t default_value;
	std::string buttons = "\uE0B1/\uE0C1      \uE0C1/\uE0B2";
	tsl::elm::ListItem* m_item;
	std::map<std::string, Data>* configs;
public:
	EditConfigInt(std::string key, std::string value, std::string rangeMin, std::string rangeMax, std::string defaultValue, tsl::elm::ListItem* item, std::map<std::string, Data>* m_configs) {
		configs = m_configs;
		tsl::hlp::requestForeground(true);
		m_item = item;
		m_key = key;
		isNumeric(value, &current_value);
		isNumeric(rangeMin, &min);
		isNumeric(rangeMax, &max);
		if (defaultValue.length() > 0) {
			isNumeric(defaultValue, &default_value);
			defaultvalue_str = std::to_string(default_value);
		}
		min_str = std::to_string(min);
		max_str = std::to_string(max);
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_key);
		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			const size_t fontsize2 = 60;
			const size_t m_height = (360+fontsize2) - (fontsize2 / 2);
			std::string current = std::to_string(current_value);
			auto [width2, height2] = renderer->drawString(current.c_str(), false, 0, fontsize2, fontsize2, renderer->a(0x0000));
			renderer->drawString(current.c_str(), false, (tsl::cfg::FramebufferWidth - width2) / 2, m_height, fontsize2, renderer->a(0xFFFF));
			const size_t fontsize = 30;
			auto [width3, height3] = renderer->drawString(buttons.c_str(), false, 0, fontsize, fontsize, renderer->a(0x0000));
			renderer->drawString(buttons.c_str(), false, (tsl::cfg::FramebufferWidth - width3) / 2, m_height+60, fontsize, renderer->a(0xFFFF));
			auto [width4, height4] = renderer->drawString("\uE023", false, 0, fontsize, fontsize, renderer->a(0x0000));
			renderer->drawString("\uE023", false, (tsl::cfg::FramebufferWidth - width4) / 2 - 2, m_height+60, fontsize, renderer->a(0xFFFF));
			auto [width5, height5] = renderer->drawString("\uE091      \uE090", false, 0, fontsize, fontsize, renderer->a(0x0000));
			renderer->drawString("\uE091      \uE090", false, (tsl::cfg::FramebufferWidth - width5) / 2, m_height+36, fontsize, renderer->a(0xFFFF));
			const size_t left_offset = (tsl::cfg::FramebufferWidth - width3) / 2;
			const size_t right_offset = tsl::cfg::FramebufferWidth - left_offset;
			auto [width6, height6] = renderer->drawString(min_str.c_str(), false, 0, fontsize, fontsize, renderer->a(0x0000));
			renderer->drawString(min_str.c_str(), false, left_offset - 20 - width6, m_height+60, fontsize, renderer->a(0xFFFF));
			renderer->drawString(max_str.c_str(), false, right_offset + 20, m_height+60, fontsize, renderer->a(0xFFFF));
			std::string reset_str = "\uE0E2 ";
			reset_str += defaultvalue_str;
			auto [width7, height7] = renderer->drawString(reset_str.c_str(), false, 0, fontsize, fontsize, renderer->a(0x0000));
			renderer->drawString(reset_str.c_str(), false, (tsl::cfg::FramebufferWidth - width7) / 2, m_height+120, fontsize, renderer->a(0xFFFF));
        });
		rootFrame->setContent(Status);
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		float joyX = (float)joyStickPosLeft.x / 32768.0;
		float joyY = (float)joyStickPosLeft.y / 32768.0;
		if ((joyX < -0.25 && (joyY > -0.25 && joyY < 0.25)) || (keysDown & KEY_DLEFT) || (keysDown & HidNpadButton_StickLLeft)) {
			if (current_value > min) current_value--;
			return true;
		}
		else if ((joyX > 0.25 && (joyY > -0.25 && joyY < 0.25)) || (keysDown & KEY_DRIGHT) || (keysDown & HidNpadButton_StickLRight)) {
			if (current_value < max) current_value++;
			return true;
		}
		if (keysDown & KEY_X) {
			current_value = default_value;
			return true;
		}
		if (keysDown & KEY_B) {
			tsl::goBack();
			return true;
		}
		if (keysDown & KEY_A) {
			std::string temp = std::to_string(current_value);
			configs->at(m_key).value = temp;
			m_item->setValue(temp, false);
			tsl::goBack();
			return true;
		}
		return false;
	}
};

class ConfigurationSubMenu : public tsl::Gui {
private:
	std::string m_type;
	std::string showType;
	std::string m_name;
	std::map<std::string, Data>* configs;
public:
	ConfigurationSubMenu(std::string type, std::string name, std::map<std::string, Data>* m_configs) {
		configs = m_configs;
		m_type = type;
		if (m_type.compare("bool") == 0) {
			showType = "\uE142\uE14B\uE14C";
		}
		else if (m_type.compare("int") == 0) {
			showType = "\uE047\uE048";
		}
		else if (m_type.compare("color") == 0) {
			showType = "\uE135";
		}
		m_name = name + "\n" + showType;
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_name);
		auto list = new tsl::elm::List();

		for (const auto& [key, data] : *configs) {
			if (data.type.compare(m_type) == 0) {
				if (m_type.compare("int") == 0) {
					int64_t temp;
					if (isNumeric(data.rangeMin, &temp) == true && isNumeric(data.rangeMax, &temp) == true) {
						auto Item = new tsl::elm::ListItem(key, data.value);
						Item->setClickListener([this, key, data, Item](uint64_t keys) {
							if (keys & KEY_A) {
								tsl::changeTo<EditConfigInt>(key, configs->at(key).value, data.rangeMin, data.rangeMax, data.defaultValue, Item, configs);
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
				else if (data.type.compare("bool") == 0) {
					bool isTrue = data.value.compare("true") == 0;
					auto Item = new tsl::elm::ToggleListItem(key, isTrue);
					Item->setClickListener([this, key, Item](uint64_t keys) {
						if (keys & KEY_A) {
							configs->at(key).value = Item->getState() ? "true" : "false";
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
								tsl::changeTo<EditConfigColor>(key, Item, configs);
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
			tsl::goBack();
			return true;
		}
		return false;
	}
};

class Configuration : public tsl::Gui {
private:
	char* buffer = nullptr;
	size_t buffer_size = 0;
	bool isBool = false;
	bool isInt = false;
	bool isColor = false;
	bool isError = false;
	std::map<std::string, Data> configs;
	void FindConfigs(const char* data, size_t size) {
		size_t lineStart = 0;
		for (size_t i = 0; i < size; ++i) {
			if (data[i] != '\n') continue;

			// Slice the current line [lineStart, i), peeling off a trailing \r.
			size_t end = i;
			while (end > lineStart && data[end - 1] == '\r') --end;
			std::string rawLine(data + lineStart, end - lineStart);
			lineStart = i + 1;

			// 2. Standard configuration line processing
			std::string line = StripLineComment(rawLine);
			line = trim(line);
			if (line.empty()) continue;

			if (line == "Start:" || line == "Start: ") break;

			size_t sep = std::string::npos;
			{
				int depth = 0; bool inStr = false;
				for (size_t j = 0; j < line.size(); ++j) {
					char c = line[j];
					if (inStr) {
						if (c == '\\' && j + 1 < line.size()) { ++j; continue; }
						if (c == '"') inStr = false;
						continue;
					}
					if (c == '"') { inStr = true; continue; }
					if (c == '{') ++depth;
					else if (c == '}') --depth;
					else if (depth == 0 && c == '=') { sep = j; break; }
				}
			}
			if (sep == std::string::npos) continue;

			std::string key  = trim(line.substr(0, sep));
			std::string rest = trim(line.substr(sep + 1));

			if (key.starts_with("User_") == false) continue;
			auto sub_key = key.substr(5);

			if (configs[sub_key].value.empty()) {
				configs[sub_key].value = rest;
				
				// Deduce type from value prefix if explicit _Range was not provided
				if (configs[sub_key].type.empty()) {
					if (rest.starts_with("COLOR")) {
						configs[sub_key].type = "color";
					} else if (rest.starts_with("LIST")) {
						configs[sub_key].type = "list";
					}
					else if (rest.compare("true") == 0 || rest.compare("false") == 0) {
						configs[sub_key].type = "bool";
					}
				}
			}
		}

		lineStart = 0;
		for (size_t i = 0; i < size; ++i) {
			if (data[i] != '\n') continue;

			// Slice the current line [lineStart, i), peeling off a trailing \r.
			size_t end = i;
			while (end > lineStart && data[end - 1] == '\r') --end;
			std::string rawLine(data + lineStart, end - lineStart);
			lineStart = i + 1;
			std::string trimmedRaw = trim(rawLine);

			// 1. Intercept metadata lines BEFORE stripping comments
			if (trimmedRaw.starts_with(";;User_")) {
				size_t sep = std::string::npos;
				int depth = 0; bool inStr = false;
				
				for (size_t j = 0; j < trimmedRaw.size(); ++j) {
					char c = trimmedRaw[j];
					if (inStr) {
						if (c == '\\' && j + 1 < trimmedRaw.size()) { ++j; continue; }
						if (c == '"') inStr = false;
						continue;
					}
					if (c == '"') { inStr = true; continue; }
					if (c == '{') ++depth;
					else if (c == '}') --depth;
					else if (depth == 0 && c == '=') { sep = j; break; }
				}

				if (sep != std::string::npos) {
					std::string metaKey = trim(trimmedRaw.substr(2, sep - 2)); 
					std::string rest = trim(trimmedRaw.substr(sep + 1));

					if (metaKey.ends_with("_Range")) {
						std::string sub_key = metaKey.substr(5, metaKey.size() - 5 - 6);
						
						if (!rest.empty() && rest.front() == '{') rest = rest.substr(1);
						if (!rest.empty() && rest.back() == '}') rest.pop_back();
						
						size_t c1 = rest.find(',');
						if (c1 != std::string::npos) {
							size_t c2 = rest.find(',', c1 + 1);
							if (c2 != std::string::npos) {
								if (configs.find(sub_key) != configs.end()) {
									configs[sub_key].type = trim(rest.substr(0, c1));
									configs[sub_key].rangeMin = trim(rest.substr(c1 + 1, c2 - c1 - 1));
									configs[sub_key].rangeMax = trim(rest.substr(c2 + 1));
								}
							}
						}
					} 
					else if (metaKey.ends_with("_DefaultValue")) {
						std::string sub_key = metaKey.substr(5, metaKey.size() - 5 - 13);
						if (configs.find(sub_key) != configs.end())
							configs[sub_key].defaultValue = rest;
					}
				}
				continue; 
			}
		}

		// 3. Fallback check: If type is still completely blank, default to "bool"
		for (auto& [key, data] : configs) {
			if (data.type.empty()) {
				data.type = "error";
			}
		}

		return;
	}

	void setDataToIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::map<std::string, Data>& configs) {
		FILE* configFile = fopen(fileToEdit.c_str(), "r");
		if (!configFile) {
			configFile = fopen(fileToEdit.c_str(), "w");
			if (!configFile) return;
			fprintf(configFile, "[%s]\n", desiredSection.c_str());
			for (const auto& [key, data] : configs)
				fprintf(configFile, "User_%s = %s\n", key.c_str(), trim(data.value).c_str());
			fclose(configFile);
			return;
		}

		std::string updatedContent;
		std::string currentSection;
		std::set<std::string> keysFound;
		char line[131072];

		bool sectionFound = false;
		bool addNewLine   = false;

		// Appends any keys from configs that haven't been written yet.
		auto appendMissingKeys = [&]() {
			for (const auto& [key, data] : configs) {
				if (keysFound.count(key)) continue;
				if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
					updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
					addNewLine = true;
				}
				updatedContent += "User_" + key + " = " + trim(data.value) + "\n";
				if (addNewLine) { updatedContent += "\n"; addNewLine = false; }
			}
		};

		while (fgets(line, sizeof(line), configFile)) {
			std::string trimmedLine = trim(line);

			// Entering a new section header.
			if (trimmedLine.size() >= 2 && trimmedLine[0] == '[' && trimmedLine.back() == ']') {
				// Leaving the desired section — flush any keys not yet written.
				if (sectionFound && trim(currentSection) == trim(desiredSection))
					appendMissingKeys();

				currentSection = removeQuotes(trimmedLine.substr(1, trimmedLine.length() - 2));
			}

			// Inside the desired section — check if this line holds one of our keys.
			if (trim(currentSection) == trim(desiredSection)) {
				sectionFound = true;
				std::string::size_type delimiterPos = trimmedLine.find('=');

				if (delimiterPos != std::string::npos) {
					std::string lineKey = trim(trimmedLine.substr(0, delimiterPos));

					// Strip prefix for map lookup — file stores "User_key", configs is keyed by "key"
					std::string lookupKey = (lineKey.substr(0, 5) == "User_") ? lineKey.substr(5) : lineKey;
					auto it = configs.find(lookupKey);

					if (it != configs.end()) {
						keysFound.insert(lookupKey); // track by unprefixed key, same as configs
						if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
							updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
							addNewLine = true;
						}
						updatedContent += "User_" + lookupKey + " = " + trim(it->second.value) + "\n";
						if (addNewLine) { updatedContent += "\n"; addNewLine = false; }
						continue;
					}
				}
			}

			updatedContent += line;
		}

		fclose(configFile);

		// EOF while still inside the desired section.
		if (sectionFound)
			appendMissingKeys();

		// Section was never encountered — append it at the end.
		if (!sectionFound) {
			updatedContent += "\n[" + desiredSection + "]\n";
			for (const auto& [key, data] : configs)
				updatedContent += "User_" + key + " = " + trim(data.value) + "\n";
		}

		configFile = fopen(fileToEdit.c_str(), "w");
		if (!configFile) return;
		fprintf(configFile, "%s", updatedContent.c_str());
		fclose(configFile);
	}
public:
	std::string filepath;
	std::string m_name;
	std::string m_show;
	std::string section_name;
	std::vector<std::string> list_keys;
	Configuration(std::string path, std::string name) {
		tsl::hlp::requestForeground(true);
		filepath = path;
		FILE* file = fopen(filepath.c_str(), "rb");
		if (file) {
			fseek(file, 0, 2);
			buffer_size = ftell(file);
			fseek(file, 0, 0);
			buffer = (char*)malloc(buffer_size);
			fread(buffer, 1, buffer_size, file);
			fclose(file);
			FindConfigs(buffer, buffer_size);
			free(buffer);
		}
		for (const auto& [key, data] : configs) {
			if (data.type.compare("bool") == 0) {
				isBool = true;
			}
			else if (data.type.compare("int") == 0) {
				isInt = true;
			}
			else if (data.type.compare("color") == 0) {
				isColor = true;
			}
			else isError = true;
		}
		m_name = name;
		m_show = std::string("\uE130 ") + m_name;
		section_name = filepath.substr(strlen("sdmc:/config/status-monitor-deux/modes/"));
		auto it = config.find(section_name);
		if (it == config.end()) return;
		auto settings = it->second; //to co znaleziono w pliku ini
		file = fopen("sdmc:/data.txt", "w");
		for (const auto& [key, data] : configs) { //To co znaleziono w pliku SMD
			auto it2 = settings.find("User_" + key);
			if (it2 == settings.end()) continue;
			std::string temp = it2->second;
			fprintf(file, "[%s] \"%s\" ][ \"%s\"\n", key.c_str(), data.value.c_str(), temp.c_str());
			int64_t value;
			if (data.value.compare("true") == 0 || data.value.compare("false") == 0) {
				if (temp.compare("true") != 0 && temp.compare("false") != 0) continue;
			}
			else if (data.value.starts_with("COLOR{") && data.value.length() == 13 && data.value.substr(6, 2).compare("0x") == 0 && data.value.substr(12).compare("}") == 0) {
				if (temp.starts_with("COLOR{") == false || temp.length() != 13 || temp.substr(6, 2).compare("0x") || temp.substr(12).compare("}")) continue;
			}
			else if (isNumeric(data.value, &value) == true) {
				if (isNumeric(temp, &value) == false) continue;
			}
			else if (data.value.starts_with("LIST{str, {\"") && data.value.ends_with("\"}}")) {
				std::string temp2 = flatListToList(temp);
				temp = temp2;
				list_keys.emplace_back(key);
			}
			else continue;
			configs[key].value = temp;
		}
		fclose(file);
	}

	~Configuration() {
		std::vector<std::string> key_to_reverse;
		for (const auto& [key, data] : configs) {
			if (configs[key].value.starts_with("LIST")) {
				key_to_reverse.emplace_back(key);
				std::string list = listToFlatList(configs[key].value);
				configs[key].value = list;
			}
		}
		setDataToIniFile("sdmc:/config/status-monitor-deux/config.ini", section_name, configs);

		auto it = config.find(section_name);
		if (it == config.end()) return;
		auto settings = &it->second;
		
		for (size_t i = 0; i < key_to_reverse.size(); i++) {
			std::string flatList = flatListToList(configs[key_to_reverse[i]].value);
			configs[key_to_reverse[i]].value = flatList;
		}
		FILE* file = fopen("sdmc:/on_save.txt", "w");
		for (const auto& [key, data] : configs) {
			std::string m_key = "User_" + key;
			fprintf(file, "[%s] %s\n", m_key.c_str(), configs[key].value.c_str());
			settings->at(m_key) = configs[key].value;
		}
		fclose(file);
	}

	virtual tsl::elm::Element* createUI() override {

		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_show);
		auto list = new tsl::elm::List();
		if (isBool == true) {
			auto Item = new tsl::elm::ListItem("Bools", "\uE142\uE14B\uE14C");
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					tsl::changeTo<ConfigurationSubMenu>("bool", m_show, &configs);
					return true;
				}
				return false;
			});
			list->addItem(Item);
		}
		if (isInt == true) {
			auto Item = new tsl::elm::ListItem("Ints", "\uE047\uE048");
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					tsl::changeTo<ConfigurationSubMenu>("int", m_show, &configs);
					return true;
				}
				return false;
			});
			list->addItem(Item);
		}
		if (isColor == true) {
			auto Item = new tsl::elm::ListItem("Colors", "\uE135");
			Item->setClickListener([this](uint64_t keys) {
				if (keys & KEY_A) {
					tsl::changeTo<ConfigurationSubMenu>("color", m_show, &configs);
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
			tsl::goBack();
			return true;
		}
		return false;
	}
};

// Using this dummy because if we go directly to RenderingPipeline, we can't get out of focus. Why? Trivago.
class Dummy : public tsl::Gui {
public:
	std::string filepath;
	char* buffer = 0;
	Dummy(std::string path) {
		filepath = path;

	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame("", "");
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		tsl::changeTo<RenderingPipeline>(filepath, true);
		return true;
	}
};

class MemoryCheck : public tsl::Gui {
public:
	size_t free_space = 0;
	void* buffer = 0;
	MemoryCheck() {
		size_t start_size = 10*1024*1024;
		while(true) {
			buffer = malloc(start_size);
			if (buffer) {
				free(buffer);
				free_space = start_size;
				return;
			}
			start_size -= 1024;
		}
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, "Memory checker");
		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			char test[128] = "";
			snprintf(test, sizeof(test), "Free space: %.03f MB\n\nPointer: %p", (float)free_space / 1024.f / 1024.f, buffer);
			renderer->drawString(test, false, x, y+20, 20, renderer->a(0xFFFF));
		});
		rootFrame->setContent(Status);
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & KEY_B) {
			tsl::goBack();
			return true;
		}
		return false;
	}
};

class MainMenu : public tsl::Gui {
public:
    
    const std::string root_path = "sdmc:/config/status-monitor-deux/modes/";
    std::string standard_path = root_path;
	std::vector<Designs> filesChecked;
	std::string formattedKeyCombo;
	std::string m_folderName;

	bool FindConfigs(const char* data, size_t size) {
		size_t lineStart = 0;
		for (size_t i = 0; i < size; ++i) {
			if (data[i] != '\n') continue;

			// Slice the current line [lineStart, i), peeling off a trailing \r.
			size_t end = i;
			while (end > lineStart && data[end - 1] == '\r') --end;
			std::string rawLine(data + lineStart, end - lineStart);
			lineStart = i + 1;

			std::string trimmedRaw = trim(rawLine);

			// 2. Standard configuration line processing
			std::string line = StripLineComment(rawLine);
			line = trim(line);
			if (line.empty()) continue;

			if (line == "Start:" || line == "Start: ") break;

			size_t sep = std::string::npos;
			{
				int depth = 0; bool inStr = false;
				for (size_t j = 0; j < line.size(); ++j) {
					char c = line[j];
					if (inStr) {
						if (c == '\\' && j + 1 < line.size()) { ++j; continue; }
						if (c == '"') inStr = false;
						continue;
					}
					if (c == '"') { inStr = true; continue; }
					if (c == '{') ++depth;
					else if (c == '}') --depth;
					else if (depth == 0 && c == '=') { sep = j; break; }
				}
			}
			if (sep == std::string::npos) continue;

			std::string key  = trim(line.substr(0, sep));
			std::string rest = trim(line.substr(sep + 1));

			if (key.starts_with("User_") == true) return true;
		}

		return false;
	}

    MainMenu(std::string rel_path, std::string folderName = "") {
		formattedKeyCombo = keyCombo;
		formatButtonCombination(formattedKeyCombo);
        if (!rel_path.empty()) {
            standard_path = rel_path;
        }
        find_smd_files(standard_path, filesChecked);
		if (folderName.length() != 0) m_folderName = folderName;
    }

    virtual tsl::elm::Element* createUI() override {

		if (jumpImmediatelyToSingleSmd == true && filesChecked.size() == 1) {
			if (filesChecked[0].is_directory == false && standard_path.compare(root_path) == 0) {		
				std::string full_path = standard_path + filesChecked[0].name;

				smd::Document::PeekInfo info;

				if (smd::Document::Peek(full_path.c_str(), info, overrideLanguage.c_str())) {
					std::string args = "--file " + filesChecked[0].name;
					tsl::setNextOverlay(filepath, args);
					tsl::Overlay::get()->close();
					backgroundColor = 0x0000;
					rootFrame = new tsl::elm::OverlayFrame("", "");
					return rootFrame;
				}
			}
		}

		std::string version = APP_VERSION;
		version += "\n" + formattedKeyCombo;
		if (m_folderName.length() > 0) version += "\n\n" + m_folderName;

		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, version.c_str());
		auto list = new tsl::elm::List();

        std::string rel_dir = "";
        if (standard_path.length() > root_path.length()) {
            rel_dir = standard_path.substr(root_path.length());
        }

        if (!filesChecked.empty()) {
            for (const auto& item : filesChecked) {
                if (item.is_directory) {
					std::string localPath = standard_path + item.name + "/";
					std::string localName = lookupSMF(localPath);
					std::string name = localName.length() == 0 ? item.name : localName;
                    auto folderItem = new tsl::elm::ListItem(name, "\uE133", true);
                    folderItem->setClickListener([this, localPath, name](uint64_t keys) {
                        if (keys & KEY_A) {
                            tsl::changeTo<MainMenu>(localPath, name);
                            return true;
                        }
						#ifdef DEBUG
						else if (keys & KEY_Y) {
							tsl::changeTo<MemoryCheck>();
							return true;							
						}
						#endif
                        return false;
                    });
                    list->addItem(folderItem);
                } 
                else {
                    std::string full_path = standard_path + item.name;
                    
                    smd::Document::PeekInfo info;
                    smd::Document::Peek(full_path.c_str(), info, overrideLanguage.c_str());
					FILE* file = fopen(full_path.c_str(), "rb");
					bool doesHaveConfig = false;
					if (file) {
						fseek(file, 0, 2);
						size_t size = ftell(file);
						fseek(file, 0, 0);
						char* buffer = 0;
						buffer = (char*)malloc(size);
						if (buffer) {
							fread(buffer, 1, size, file);
							fclose(file);
							doesHaveConfig = FindConfigs(buffer, size);
							free(buffer);
						}
					}
					std::string second = "";
					if (info.name.empty()) second = "\uE098";
					else if (doesHaveConfig) second = "\uE04F";
                    auto fileItem = new tsl::elm::ListItem(info.name.empty() ? item.name : info.name, second.c_str(), info.name.empty() ? true : false);
                    fileItem->setClickListener([this, item, info, full_path, rel_dir, doesHaveConfig](uint64_t keys) {
						if (info.name.empty() == false) {
							if (keys & KEY_A) {
								if (info.layerWidth != 0 && info.layerHeight != 0 && info.layerWidth != 448 && info.layerHeight != 720) {
									smd::Document doc;
									if (doc.LoadFromFile(full_path.c_str()) == false) {
										tsl::changeTo<RenderingPipeline>(full_path);
										return true;
									}
									BindAllPredefined(doc);
									if (doc.Compile() == false) {
										tsl::changeTo<RenderingPipeline>(full_path);
										return true;
									}
									std::string args = "--file " + rel_dir + item.name + " --submenu";
									tsl::setNextOverlay(filepath, args);
									tsl::Overlay::get()->close();
									return true;
								}
								tsl::changeTo<RenderingPipeline>(full_path);
								return true;
							}
							else if (doesHaveConfig == true) {
								if (keys & KEY_Y) {
									tsl::changeTo<Configuration>(full_path, info.name);
									return true;
								}
							}
						}
                        return false;
                    });
                    list->addItem(fileItem);
                }
            }
            rootFrame->setContent(list);
        }
        else {
            auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
                renderer->drawString("No folders or .smd files found!", false, 20, 20, 20, renderer->a(0xFFFF));
            });
            rootFrame->setContent(Status);
        }

        return rootFrame;
    }

	virtual void update() override {

	}

	virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		if (keysDown & KEY_B) {
			tsl::goBack();
			return true;
		}
		return false;
	}
};

class MonitorOverlay : public tsl::Overlay {
public:

	virtual void initServices() override {
		//Initialize services
		tsl::hlp::doWithSmSession([this]{

			if (hosversionAtLeast(8,0,0)) clkrstCheck = clkrstInitialize();
			else pcvCheck = pcvInitialize();

			if (hosversionAtLeast(5,0,0)) tcCheck = tcInitialize();

			if (hosversionAtLeast(6,0,0) && R_SUCCEEDED(pwmInitialize())) {
				pwmCheck = pwmOpenSession2(&g_ICon, 0x3D000001);
			}

			if (R_SUCCEEDED(nvInitialize())) nvCheck = nvOpen(&fd, "/dev/nvhost-ctrl-gpu");

			psmCheck = psmInitialize();
			i2cCheck = i2cInitialize();

			SaltySD = CheckPort();

			if (SaltySD) {
				LoadSharedMemoryAndRefreshRate();
			}
			if (sysclkIpcRunning() && R_SUCCEEDED(sysclkIpcInitialize())) {
				uint32_t sysClkApiVer = 0;
				sysclkIpcGetAPIVersion(&sysClkApiVer);
				if (sysClkApiVer < 4) {
					sysclkIpcExit();
				}
				else sysclkCheck = 0;
			}
			else if (hocclkIpcRunning() && R_SUCCEEDED(hocclkIpcInitialize())) {
				uint32_t hocClkApiVer = 0;
				hocclkIpcGetAPIVersion(&hocClkApiVer);
				if (hocClkApiVer < 2) {
					hocclkIpcExit();
				}
				else hocclkCheck = 0;
			}
			if (overrideLanguage.length() == 0) {
				if (R_SUCCEEDED(setInitialize())) {
					u64 languageCode;
					setGetSystemLanguage(&languageCode);
					SetLanguage language;
					setMakeLanguage(languageCode, &language);
					setExit();
					switch(language) {
						case SetLanguage_JA:     {overrideLanguage = "JA-JP"; break;}
						case SetLanguage_FR:     {overrideLanguage = "FR-FR"; break;}
						case SetLanguage_DE:     {overrideLanguage = "DE-DE"; break;}
						case SetLanguage_IT:     {overrideLanguage = "IT-IT"; break;}
						case SetLanguage_ES:     {overrideLanguage = "ES-ES"; break;}
						case SetLanguage_ZHCN:
						case SetLanguage_ZHHANS: {overrideLanguage = "ZH-CN"; break;}
						case SetLanguage_ZHTW:
						case SetLanguage_ZHHANT: {overrideLanguage = "ZH-TW"; break;}
						case SetLanguage_KO:     {overrideLanguage = "KO-KR"; break;}
						case SetLanguage_NL:     {overrideLanguage = "NL-NL"; break;}
						case SetLanguage_PT:     {overrideLanguage = "PT-PT"; break;}
						case SetLanguage_RU:     {overrideLanguage = "RU-RU"; break;}
						case SetLanguage_ENGB:   {overrideLanguage = "EN-GB"; break;}
						case SetLanguage_FRCA:   {overrideLanguage = "FR-CA"; break;}
						case SetLanguage_ES419:  {overrideLanguage = "ES-419"; break;}
						case SetLanguage_PTBR:   {overrideLanguage = "PT-BR"; break;}
						default:                 {overrideLanguage = "EN-US"; break;}
					}
				}
			}
		});
		Hinted = envIsSyscallHinted(0x6F);
		hidGetSixAxisSensorHandles(&sixaxisHandles[Controller_ProController], 1, HidNpadIdType_No1,      HidNpadStyleTag_NpadFullKey);
		hidGetSixAxisSensorHandles(&sixaxisHandles[Controller_JoyConL], 2, HidNpadIdType_No1,      HidNpadStyleTag_NpadJoyDual);
	}

	virtual void exitServices() override {
		if (R_SUCCEEDED(sysclkCheck)) {
			sysclkIpcExit();
		}
		else if (R_SUCCEEDED(hocclkCheck)) {
			hocclkIpcExit();
		}
		shmemClose(&_sharedmemory);
		//Exit services
		clkrstExit();
		pcvExit();
		tcExit();
		pwmChannelSessionClose(&g_ICon);
		pwmExit();
		nvClose(fd);
		nvExit();
		psmExit();
		i2cExit();
	}

    virtual void onShow() override {}
    virtual void onHide() override {}

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
		remove("sdmc:/dddd.dddd");
		FsFileSystem* filesystem = fsdevGetDeviceFileSystem("sdmc");
		char out_path[FS_MAX_PATH] = "";
		fsdevTranslatePath("sdmc:/dddd.dddd", &filesystem, out_path);
		LocalTime.relative_tick = svcGetSystemTick();
		Result rc = fsFsCreateFile(filesystem, out_path, 0, 0);
		if (R_SUCCEEDED(rc)) {
			struct stat attr;
			stat("sdmc:/dddd.dddd", &attr);
			remove("sdmc:/dddd.dddd");
			LocalTime.timestamp = attr.st_mtime;
		}
		if (file_to_load.length() == 0)
        	return initially<MainMenu>("");
		else {
			return initially<Dummy>(file_to_load);
		}
    }
};

int main(int argc, char **argv) {
	#if !defined(__SWITCH__) && !defined(__OUNCE__)
		systemtickfrequency = armGetSystemTickFreq();
	#endif

	ParseIniFile();
    
	if (argc > 0) {
		filename = argv[0];
		filepath = folderpath + filename;
	}
	int arg = 1;
	while (arg < argc) {
		if (strcasecmp(argv[arg], "--file") == 0) {
			if (arg + 1 < argc) {
				const char* smd_filename = argv[arg+1];
				std::string path = "sdmc:/config/status-monitor-deux/modes/";
				path += smd_filename;
				
				struct stat filedata;
				if (stat(path.c_str(), &filedata) == 0) {
					smd::Document doc;
					if (doc.LoadFromFile(path.c_str()) == true) {
						doc.Free();
						smd::Document::PeekInfo peek;
						smd::Document::Peek(path.c_str(), peek);
						if (peek.layerWidth != 0 && peek.layerHeight != 0) {
							framebufferWidth = peek.layerWidth;
							framebufferHeight = peek.layerHeight;
						}
					}
					file_to_load = path;
				}
			}
		}
		else if (strcasecmp(argv[arg], "--submenu") == 0) {
			tsl::setNextOverlay(filepath, "");
		};
		arg++;
	}
    return tsl::loop<MonitorOverlay>(argc, argv);
}