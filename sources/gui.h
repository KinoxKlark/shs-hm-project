#include <array>

#define GUI_STRETCH -1

struct GuiObject
{
	v2 size;
	v4 margin;
	v4 padding;
	sf::Color bg_color;

	// TODO(Sam): Proper text management
	sf::Text text;
};

enum class GuiElementAlignment {
	NONE,
	HORIZONTAL,
	VERTICAL
};

struct GuiElement {
	GuiElement *parent;
	rect inner_bounds;
	rect outer_bounds;
	v2 next_valid_block_pos;
	r32 line_width;
	GuiElementAlignment alignment;
	GuiObject obj;
};

struct GuiManager {
	std::array<GuiElement,1024> elements;
	u32 elements_count;
	GuiElement *most_recent_container;
	r32 margin_unit;

	sf::Font font;
};

inline GuiManager* gui_init();
inline void gui_shutdown(GuiManager* gui);

inline
void GuiReset(GuiManager *gui)
{
	gui->elements_count = 0;
	gui->most_recent_container = nullptr;
}

inline void GuiBeginContainer(GuiManager *gui, GuiObject obj,
							  GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
inline void GuiEndContainer(GuiManager *gui);
inline bool GuiButton(GuiManager *gui, sf::String label);

void GuiDebug(GuiManager *gui);
