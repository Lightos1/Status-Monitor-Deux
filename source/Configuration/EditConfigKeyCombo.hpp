#pragma once

class EditConfigKeyCombo : public tsl::Gui {
private:
	std::string m_key;
	std::string m_localName;
	std::string m_value;
	std::string m_footerValue;
	u64 keyMapping;
	bool m_mainCombo;
public:
	EditConfigKeyCombo(bool isMainCombo, std::string key, std::string value, std::string localName) {
		m_mainCombo = isMainCombo;
		keyMapping = MapButtons(value);
		m_localName = localName;
		m_key = key;
		if (m_mainCombo == true) {
			if (value.length() > 0) m_value = value;
			else {
				if (ultrahandCombo == true) {
					m_value = keyCombo;
					m_footerValue = "(Ultrahand combo)";
				}
				else if (teslaCombo == true) {
					m_value = keyCombo;
					m_footerValue = "(Tesla menu combo)";
				}
				else m_value = keyCombo;
			}
		}
		else {
			m_value = value;
		}
	}

	virtual tsl::elm::Element* createUI() override {
		rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, m_localName);
		auto Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			const size_t fontsize2 = 40;
			const size_t m_height = (360+fontsize2) - (fontsize2 / 2);
			auto [width, height] = renderer->drawString(m_value.c_str(), false, 0, fontsize2, fontsize2, renderer->a(0x0000), true);
			renderer->drawString(m_value.c_str(), false, (tsl::cfg::FramebufferWidth - width) / 2, m_height, fontsize2, renderer->a(0xFFFF), true);
			if (m_mainCombo == true && (m_footerValue.length() > 0)) {
				const size_t fontsize1 = 30;
				auto [width2, height2] = renderer->drawString(m_footerValue.c_str(), false, 0, fontsize1, fontsize1, renderer->a(0x0000), true);
				if (m_footerValue.length() > 0) renderer->drawString(m_footerValue.c_str(), false, (tsl::cfg::FramebufferWidth - width2) / 2, m_height+fontsize2, fontsize1, renderer->a(0xFFFF), true);
			}
        });
		rootFrame->setContent(Status);
		return rootFrame;
	}

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & KEY_B) {
			tsl::hlp::requestForeground(true);
			tsl::goBack();
			return true;
		}
		if (keysDown & KEY_A) {
			configs[m_key].value = m_value;
			tsl::hlp::requestForeground(true);
			tsl::goBack();
			return true;
		}
		return false;
	}
};
