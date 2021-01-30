#include <array>
#include <unordered_map>

// HASHING

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define FILE_AND_LINE __FILE__ " " STRINGIFY(__LINE__)
#define GET_UNIQUE_ID() hash(FILE_AND_LINE)


#include <iostream>
template<size_t N, size_t I=0>
struct hash_calc {
    static constexpr size_t apply (const char (&s)[N]) {
       return  (hash_calc<N, I+1>::apply(s) ^ s[I]) * 16777619u;
    };
};

template<size_t N>
struct hash_calc<N,N> {
    static constexpr size_t apply (const char (&s)[N]) {
       return  2166136261u;
    };
};

template<size_t N>
constexpr size_t hash ( const char (&s)[N] ) {
    return hash_calc<N>::apply(s);
}

// ---


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

struct GuiElementProperties {
	bool touched;
	r32 timer;
};

struct GuiManager {
	std::array<GuiElement,1024> elements;
	u32 elements_count;
	GuiElement *most_recent_container;
	r32 margin_unit;

	// TODO(Sam): Manage untouched elements that should be removed
	std::unordered_map<u32, GuiElementProperties> properties;

	sf::Font font;
};

inline GuiManager* gui_init();
inline void gui_update(GuiManager *gui, sf::Time dt);
inline void gui_shutdown(GuiManager* gui);

inline
void GuiReset(GuiManager *gui)
{
	gui->elements_count = 0;
	gui->most_recent_container = nullptr;

	for(auto it = gui->properties.begin(); it != gui->properties.end();)
	{
		GuiElementProperties *props = &(it->second);
		if(props->touched)
		{
			props->touched = false;
			++it;
		}
		else
		{
			it = gui->properties.erase(it);
		}	
	}
}

inline void GuiBeginContainer(GuiManager *gui, GuiObject obj,
							  GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
inline void GuiEndContainer(GuiManager *gui);
inline bool _GuiButton(GuiManager *gui, u32 id, sf::String label);
#define GuiButton(gui, label) _GuiButton(gui, GET_UNIQUE_ID(), label)

void GuiDebug(GuiManager *gui);
