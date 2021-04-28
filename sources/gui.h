#include <array>
#include <unordered_map>
#include <stack>

#include "HTMLText.h"

const sf::Color BACKGROUND_COLOR = sf::Color({24, 119, 242});
const sf::Color UI_MAIN_BG_COLOR = sf::Color({240, 242, 245});
const sf::Color UI_POST_BG_COLOR = sf::Color({255, 255, 255});
const sf::Color UI_POST_BORDER_COLOR = sf::Color({228, 230, 235});
const sf::Color UI_MAIN_TEXT_COLOR = sf::Color({0,0,0});
const sf::Color UI_SMALL_TEXT_COLOR = sf::Color({96, 103, 112});
const sf::Color UI_TRANSPARENT = sf::Color({0,0,0,0});

constexpr r32 pt2px = 1.33333f;
constexpr r32 invRefWidth = 1.f/1920.f;
constexpr r32 invRefHeight = 1.f/1040.f;

constexpr r32 UI_MAIN_TEXT_FS = 15.f*pt2px*invRefWidth;
constexpr r32 UI_SMALL_TEXT_FS = 13.f*pt2px*invRefWidth;
constexpr r32 UI_BIG_TEXT_FS = 19.f*pt2px*invRefWidth;

constexpr r32 UI_MARGIN_REF = 25.f*invRefHeight;
constexpr r32 UI_OUTTER_MARGIN = 25.f*invRefHeight/UI_MARGIN_REF;
constexpr r32 UI_FEED_INNER_MARGIN = 16.f*invRefHeight/UI_MARGIN_REF;
constexpr r32 UI_POST_INTER_MARGIN = 9.f*invRefHeight/UI_MARGIN_REF;
constexpr r32 UI_POST_INNER_MARGIN_SIDES = 14.f*invRefHeight/UI_MARGIN_REF;
constexpr r32 UI_POST_INNER_MARGIN_TOP = 10.f*invRefHeight/UI_MARGIN_REF; 
constexpr r32 UI_BORDER_ROUNDING_RADIUS = 4.5f*invRefWidth;

constexpr r32 UI_FEED_RATIO = 284.f/473.f;

#define ID_BANDWIDTH 1000
std::stack<u32> gui_global_container_id;
inline u32 create_id()
{
	local_persist u32 id = 1;
	return id++;
}

// HASHING

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define FILE_AND_LINE __FILE__ " " STRINGIFY(__LINE__)
#define GET_UNIQUE_ID_OLD() hash(FILE_AND_LINE)
#define GET_UNIQUE_ID() ((gui_global_container_id.size() > 0 ? gui_global_container_id.top()*ID_BANDWIDTH : 0) + __COUNTER__+1)

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

struct GuiObject {
	v2 size;
	r32 keep_ratio;
	v4 margin;
	v4 padding;
	sf::Color bg_color;

	// TODO(Sam): Proper text management
	sf::Text text;
	sf::HTMLText htmltext;
};

enum class GuiElementAlignment {
	NONE,
	HORIZONTAL,
	VERTICAL
};

struct GuiElement {

	bool set_viewport;
	bool always_show;
	u32 container_one_past_last;
	
	u32 id;
	u32 container_id;
	bool render;
	GuiElement *parent;
	rect inner_bounds;
	rect bounds;
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

	bool draggable;
};

typedef bool (*DropCallback)(void *payload, void* user_data);
struct GuiElementPayloadTarget {
	GuiElement *element;
	DropCallback callback;
	void *user_data;
};

struct GuiElementProperties {
	bool touched;
	r32 timer;
	bool timer_active;

	u32 selected_tab_id;

	bool dragged, dropped;
	v2 drag_grab_offset;
	v2 drag_pos;
	v2 drag_target_pos;
	v2 drag_source_pos;
	GuiElementPayloadTarget drag_target;
	void *payload;
};

struct GuiManager {
	// Note: We need an array for pointer consistency
	std::array<GuiElement,1024> elements;
	u32 elements_count;
	GuiElement *most_recent_container;
	r32 margin_unit;

	bool push_to_dragging_payload;
	u32 dragging_payload_id;
	u32 dragging_payload;
	u32 dragging_payload_size;
	std::vector<GuiElementPayloadTarget> payload_targets;
	
	std::unordered_map<u32, GuiElementProperties> properties;
	
	sf::Font font;
	sf::Font font_bold;

	// Debug:
	u32 intersect_count;
};

GuiManager *global_gui_manager = nullptr;

inline GuiManager* gui_init()
{
	GuiManager* gui = new GuiManager();
	gui->most_recent_container = nullptr;
	gui->elements_count = 0;

	// TODO(Sam): Window independent code!
	TCHAR windir[1024];
	GetWindowsDirectory(windir, ArraySize(windir));
	sf::String font_file = sf::String(windir);
	font_file.insert(font_file.getSize(), "\\fonts\\Segoeui.ttf");

	sf::String font_file_bold = sf::String(windir);
	font_file_bold.insert(font_file_bold.getSize(), "\\fonts\\Seguisb.ttf");

	// TODO(Sam): Font fallback from assets if windows font fail
	if(!gui->font.loadFromFile(font_file))
	{
		// TODO(Sam): Proper error management
		assert(("Problem with font loading!", false));
	}

	// TODO(Sam): Font fallback from assets if windows font fail
	if(!gui->font_bold.loadFromFile(font_file_bold))
	{
		// TODO(Sam): Proper error management
		assert(("Problem with bold font loading!", false));
	}


	gui->push_to_dragging_payload = false;
	gui->dragging_payload = -1;
	gui->dragging_payload_size = 0;

	gui->intersect_count = 0;
	
	global_gui_manager = gui;
	return gui;
}

inline void gui_shutdown()
{
	GuiManager *gui = global_gui_manager;
	
	global_gui_manager = nullptr;
	delete gui;
}

inline void gui_update(sf::Time dt);
inline void gui_post_treatment();

inline
void GuiReset()
{
	GuiManager *gui = global_gui_manager;
	
	gui->elements_count = 0;
	gui->most_recent_container = nullptr;

	gui->push_to_dragging_payload = false;
	gui->dragging_payload = -1;
	gui->dragging_payload_size = 0;

	gui->payload_targets.clear();
	
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

	gui->intersect_count = 0;
}

#define EXPAND(x) x
#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4

// Container
inline void GuiBeginContainer(u32 container_id, GuiObject obj, GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
inline void GuiBeginContainer(GuiObject obj, GuiElementAlignment alignment = GuiElementAlignment::VERTICAL)
{
	GuiBeginContainer(0, obj, alignment);
}
inline void GuiEndContainer();

// Tabs
inline void _GuiBeginTabs(u32 id, GuiObject obj, GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
#define _GuiBeginTabs_1ARGS(obj) _GuiBeginTabs(GET_UNIQUE_ID(), obj)
#define _GuiBeginTabs_2ARGS(obj, alignment) _GuiBeginTabs(GET_UNIQUE_ID(), obj, (GuiElementAlignment)(alignment))
#define _GuiBeginTabs_MACRO(...) EXPAND(GET_3TH_ARG(__VA_ARGS__, _GuiBeginTabs_2ARGS, _GuiBeginTabs_1ARGS))
#define GuiBeginTabs(...) EXPAND(_GuiBeginTabs_MACRO(__VA_ARGS__)(__VA_ARGS__))
inline void GuiEndTabs();
inline bool _GuiTab(u32 id, sf::String label, GuiElementAlignment alignment = GuiElementAlignment::VERTICAL);
#define _GuiTab_1ARGS(label) _GuiTab(GET_UNIQUE_ID(), label)
#define _GuiTab_2ARGS(label, alignment) _GuiTab(GET_UNIQUE_ID(), label, (GuiElementAlignment)(alignment))
#define _GuiTab_MACRO(...) EXPAND(GET_3TH_ARG(__VA_ARGS__, _GuiTab_2ARGS, _GuiTab_1ARGS))
#define GuiTab(...) EXPAND(_GuiTab_MACRO(__VA_ARGS__)(__VA_ARGS__))

// Grids
inline void GuiBeginGrid(u32 n_rows, u32 n_cols, GuiObject obj);
inline void GuiEndGrid();
inline void GuiSelectGridCell(u32 row, u32 col);

// Drag & Drop
inline void _GuiDefineContainerAsDraggable(u32 id, void *payload);
#define GuiDefineContainerAsDraggable(payload) _GuiDefineContainerAsDraggable(GET_UNIQUE_ID(), payload)
inline void GuiDroppableArea(DropCallback callback, void *user_data = nullptr);

// Button
inline bool _GuiButton(u32 id, sf::String label);
#define GuiButton(label) _GuiButton(GET_UNIQUE_ID(), label)

// Texts
inline void GuiTitle(sf::String title);
inline void GuiText(sf::String text);

// Debug
void GuiDebug();
