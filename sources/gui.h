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
	u32 id;
	GuiElement *parent;
	rect inner_bounds;
	rect outer_bounds;
	v2 next_valid_block_pos;
	r32 line_width;
	GuiElementAlignment alignment;
	GuiObject obj;

	// TODO(Sam): Clean GuiElement
	u32 selected_tab_id;
	r32 next_tab_pos;
	u32 properties_id;

	u32 grid_n_rows;
	u32 grid_n_cols;
};

struct GuiElementProperties {
	bool touched;
	r32 timer;
	u32 selected_tab_id;
};

struct GuiManager {
	std::array<GuiElement,1024> elements;
	u32 elements_count;
	GuiElement *most_recent_container;
	r32 margin_unit;

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

#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4

// Container
inline void GuiBeginContainer(GuiManager *gui, GuiObject obj,
							  GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
inline void GuiEndContainer(GuiManager *gui);

// Tabs
inline void _GuiBeginTabs(GuiManager *gui, u32 id, GuiObject obj,
							  GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
#define _GuiBeginTabs_2ARGS(gui, obj) _GuiBeginTabs(gui, GET_UNIQUE_ID(), obj)
#define _GuiBeginTabs_3ARGS(gui, obj, alignment) _GuiBeginTabs(gui, GET_UNIQUE_ID(), obj, alignment)
#define _GuiBeginTabs_MACRO(...) GET_4TH_ARG(__VA_ARGS__, _GuiBeginTabs_3ARGS, _GuiBeginTabs_2ARGS)
#define GuiBeginTabs(...) _GuiBeginTabs_MACRO(__VA_ARGS__)(__VA_ARGS__)
inline void GuiEndTabs(GuiManager *gui);
inline bool _GuiTab(GuiManager *gui, u32 id, sf::String label,
					GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
#define _GuiTab_2ARGS(gui, label) _GuiTab(gui, GET_UNIQUE_ID(), label)
#define _GuiTab_3ARGS(gui, label, alignment) _GuiTab(gui, GET_UNIQUE_ID(), label, alignment)
#define _GuiTab_MACRO(...) GET_4TH_ARG(__VA_ARGS__, _GuiTab_3ARGS, _GuiTab_2ARGS)
#define GuiTab(...) _GuiTab_MACRO(__VA_ARGS__)(__VA_ARGS__)

// Grids
inline void GuiBeginGrid(GuiManager *gui, u32 n_rows, u32 n_cols, GuiObject obj);
inline void GuiEndGrid(GuiManager *gui);
inline void GuiSelectGridCell(GuiManager *gui, u32 row, u32 col);

// Button
inline bool _GuiButton(GuiManager *gui, u32 id, sf::String label);
#define GuiButton(gui, label) _GuiButton(gui, GET_UNIQUE_ID(), label)

// Debug
void GuiDebug(GuiManager *gui);
