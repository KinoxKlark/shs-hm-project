#include <array>

#define GUI_STRETCH -1

struct GuiObject
{
	v2 size;
	v4 margin;
	v4 padding;
	sf::Color bg_color;
};

struct GuiElement {
	GuiElement *parent;
	rect inner_bounds;
	rect outer_bounds;
	v2 next_valid_pos_offset;
	GuiObject obj;
};

struct GuiManager {
	std::array<GuiElement,1024> elements;
	u32 elements_count;
	GuiElement *most_recent_container;
	r32 margin_unit;
};

inline GuiManager* gui_init();
inline void gui_shutdown(GuiManager* gui);

inline
void GuiReset(GuiManager *gui)
{
	gui->elements_count = 0;
	gui->most_recent_container = nullptr;
}

inline void GuiBeginContainer(GuiManager *gui, GuiObject obj);
inline void GuiEndContainer(GuiManager *gui);
inline bool GuiButton(GuiManager *gui, sf::String label);

void GuiDebug(GuiManager *gui);
