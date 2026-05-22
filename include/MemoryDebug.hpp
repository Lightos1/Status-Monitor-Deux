#pragma once

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