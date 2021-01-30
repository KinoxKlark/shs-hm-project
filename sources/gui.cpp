#include <sysinfoapi.h>

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

	// TODO(Sam): Font fallback from assets if windows font fail
	if(!gui->font.loadFromFile(font_file))
	{
		// TODO(Sam): Proper error management
		assert(("Problem with font loading!", false));
	}
	
	global_gui_manager = gui;
	return gui;
}

inline void gui_update(GuiManager *gui, sf::Time dt)
{
	for(auto it = gui->properties.begin(); it != gui->properties.end(); ++it)
	{
		GuiElementProperties *props = &(it->second);
		props->timer -= dt.asSeconds();
		if(props->timer < 0.f) props->timer = 0.f;
	}
}

inline void gui_shutdown(GuiManager* gui)
{
	global_gui_manager = nullptr;
	delete gui;
}

inline
u32 GuiAddElementToContainer(GuiManager *gui, GuiObject obj, GuiElementAlignment alignment)
{
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;

	GuiElement *parent_container = gui->most_recent_container;
	assert(("GuiAddElementToContainer() must be called with a valid container", parent_container));

	rect container_region = parent_container->inner_bounds;
	v2 next_valid_block_pos = parent_container->next_valid_block_pos;
	GuiElementAlignment parent_alignment = parent_container->alignment;
	r32 parent_line_width = parent_container->line_width;
	
	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(container_region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	if(inner_size.x < gui->margin_unit)
	{
		inner_size.x = gui->margin_unit;
		outer_size.x = (1+obj.margin.left+obj.margin.right)*gui->margin_unit;
	}
	if(inner_size.y < gui->margin_unit)
	{
		inner_size.y = gui->margin_unit;
		outer_size.y = (1+obj.margin.top+obj.margin.bottom)*gui->margin_unit;
	}

	v2 offset_pos = next_valid_block_pos;
	switch(parent_alignment)
	{
	case GuiElementAlignment::HORIZONTAL:
	{
		if(offset_pos.x + outer_size.x > container_region.width)
		{
			offset_pos = v2(0, offset_pos.y + parent_line_width);
			parent_line_width = 0.f;
			if(parent_container)
				parent_container->next_valid_block_pos = offset_pos;
		}
	} break;
	case GuiElementAlignment::VERTICAL:
	{
		if(offset_pos.y + outer_size.y > container_region.height)
		{
			offset_pos = v2(offset_pos.x + parent_line_width,0);
			parent_line_width = 0.f;
			if(parent_container)
				parent_container->next_valid_block_pos = offset_pos;
		}
	} break;
	case GuiElementAlignment::NONE: break;
	InvalidDefaultCase;
	}
	
	v2 outer_pos = offset_pos + rect_pos(container_region);
	v2 inner_pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);

	gui->elements[idx].parent = parent_container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_block_pos = {0,0};
	gui->elements[idx].line_width = 0.f;
	gui->elements[idx].alignment = alignment;
	gui->elements[idx].obj = obj;

	if(parent_container)
	{
		switch(parent_alignment)
		{
		case GuiElementAlignment::HORIZONTAL:
		{
			parent_container->next_valid_block_pos.x += outer_size.x;
			parent_container->line_width = Max(parent_line_width, outer_size.y);
		} break;
		case GuiElementAlignment::VERTICAL:
		{
			parent_container->next_valid_block_pos.y += outer_size.y;
			parent_container->line_width = Max(parent_line_width, outer_size.x);
		} break;
		case GuiElementAlignment::NONE:
		{
			parent_container->next_valid_block_pos = {0,0};
		} break;
		InvalidDefaultCase;
		}
	}

	return idx;
}

inline
void GuiCreateRootContainer(GuiManager *gui)
{
	GuiElement *parent_container = gui->most_recent_container;
	
	assert(("Root container already exist", !parent_container));
	assert(("Problem with root container", gui->elements_count == 0));
	++gui->elements_count;
	gui->elements[0].parent = nullptr;
	gui->elements[0].inner_bounds = {0., 0.,
									 (r32)global_renderer->window->getSize().x,
									 (r32)global_renderer->window->getSize().y};
	gui->elements[0].outer_bounds = gui->elements[0].inner_bounds;
	gui->elements[0].next_valid_block_pos = {0,0};
	gui->elements[0].line_width = 0.f;
	gui->elements[0].alignment = GuiElementAlignment::HORIZONTAL;

	gui->elements[0].obj.bg_color = sf::Color::White;

	gui->most_recent_container = &gui->elements[0];
		
}

inline
void GuiBeginContainer(GuiManager *gui, GuiObject obj, GuiElementAlignment alignment)
{
	GuiElement *parent_container = gui->most_recent_container;
	
	if(!parent_container)
		GuiCreateRootContainer(gui);
	
	u32 idx = GuiAddElementToContainer(gui, obj, alignment);

	gui->most_recent_container = &gui->elements[idx];
}

inline
void GuiEndContainer(GuiManager *gui)
{
	assert(("GuiEndContainer() must be called once per GuiBeginContainer() call!", gui->most_recent_container));
	gui->most_recent_container = gui->most_recent_container->parent;
}

inline
void _GuiBeginTabs(GuiManager *gui, u32 id, GuiObject obj)
{
	GuiElementProperties *props;
	if(gui->properties.find(id) == gui->properties.end())
	{
		props = &gui->properties[id];
		props->selected_tab_id = 0;
	}
	else
	{
		props = &gui->properties[id];
	}
	props->touched = true;

	// TODO(Sam): Magic value
	r32 tab_bar_height = 2;
	obj.margin.top += tab_bar_height;
	GuiBeginContainer(gui, obj);

	GuiElement *container = gui->most_recent_container;
	container->selected_tab_id = props->selected_tab_id;
	container->next_tab_pos = 0.f;
	container->properties_id = id;
}

inline
void GuiEndTabs(GuiManager *gui)
{
	GuiEndContainer(gui);
}

inline
bool _GuiTab(GuiManager *gui, u32 id, sf::String label)
{
	GuiElementProperties *props = &gui->properties[id];
	props->touched = true;

	GuiElement *container = gui->most_recent_container;

	if(container->selected_tab_id == 0)
		container->selected_tab_id = id;

	// TODO(Sam): Magic value
	r32 tab_bar_height = 2*gui->margin_unit;
	
	GuiObject obj;
	obj.margin = {.5,.5,0,0};
	obj.padding = {.5,.5,.5,.5};
	obj.bg_color = container->selected_tab_id == id ? sf::Color(50, 100, 100) : sf::Color(100, 150, 150);
	obj.text.setString(label);
	obj.text.setFont(gui->font);
	obj.text.setCharacterSize(18);
	obj.text.setFillColor(sf::Color::White);

	rect region = container->inner_bounds;
	region.top -= tab_bar_height;
	region.height = tab_bar_height;
	
	v2 text_size = rect_size(obj.text.getLocalBounds());

	obj.size = {
		(text_size.x + (obj.margin.left + obj.margin.right
						+ obj.padding.left + obj.padding.right)*gui->margin_unit)/region.width,
		1
	};

	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	v2 offset_pos = v2(container->next_tab_pos,0);
	
	v2 outer_pos = offset_pos + rect_pos(region);
	v2 inner_pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);
	
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;
	gui->elements[idx].parent = container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_block_pos = {0,0};
	gui->elements[idx].line_width = 0.f;
	gui->elements[idx].alignment = GuiElementAlignment::NONE;
	gui->elements[idx].obj = obj;

	container->next_tab_pos += outer_size.x;

	bool selected = container->selected_tab_id == id;
	if(gui->elements[idx].inner_bounds.contains(global_app->inputs.mouse_pos))
	{
		if(global_app->inputs.mouse_pressed)
		{
			gui->properties[container->properties_id].selected_tab_id = id;
			container->selected_tab_id = id;
		}
	}
	
	return selected;
}

inline
bool _GuiButton(GuiManager *gui, u32 id, sf::String label)
{
	GuiElementProperties *props = &gui->properties[id];
	props->touched = true;

	GuiObject obj;
	
	obj.margin = {1,1,1,1};
	obj.padding = {.5,.5,.5,.5};
	obj.bg_color = sf::Color(
		(1.f-props->timer)*100 + props->timer*255,
		(1.f-props->timer)*50 + props->timer*255,
		(1.f-props->timer)*100 + props->timer*255);
	obj.text.setString(label);
	obj.text.setFont(gui->font);
	obj.text.setCharacterSize(18);
	obj.text.setFillColor(sf::Color::White);

	assert(("Buttons can only be puts in container", gui->most_recent_container));

	v2 region = rect_size(gui->most_recent_container->inner_bounds);
	v2 text_size = rect_size(obj.text.getLocalBounds());

	obj.size = {
		(text_size.x + (obj.margin.left + obj.margin.right
						+ obj.padding.left + obj.padding.right)*gui->margin_unit)/region.x,
		(text_size.y + (obj.margin.top + obj.margin.bottom
						+ obj.padding.top + obj.padding.bottom)*gui->margin_unit)/region.y
	};

	u32 idx = GuiAddElementToContainer(gui, obj, GuiElementAlignment::NONE);

    // Button Behavior
	bool pressed = false;
	if(gui->elements[idx].inner_bounds.contains(global_app->inputs.mouse_pos))
	{
		if(props->timer <= 0.f)
			gui->elements[idx].obj.bg_color = sf::Color(150,100,150);
		pressed = global_app->inputs.mouse_pressed;
		if(pressed)
			props->timer = 1.f;
	}
	
	
	return pressed;
}

#ifdef DEBUG
void GuiDebug(GuiManager *gui)
{
	ImGui::Begin("GUI Debug");

	if(ImGui::BeginTable("Properties", 2))
	{
		ImGui::TableSetupColumn("id");
		ImGui::TableSetupColumn("timer");

		ImGui::TableHeadersRow();
		for(auto it = gui->properties.begin(); it != gui->properties.end(); ++it)
		{
			GuiElementProperties *props = &(it->second);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%u", it->first);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f", props->timer);
		}

		ImGui::EndTable();
	}

	ImGui::Text("%u elements", gui->elements_count);

	if(ImGui::BeginTable("Elements", 2))
	{
		ImGui::TableSetupColumn("Pos");
		ImGui::TableSetupColumn("Size");

		ImGui::TableHeadersRow();
		for(size_t idx(0); idx < gui->elements_count; ++idx)
		{
			GuiElement *element = &gui->elements[idx];
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%.2f, %.2f", rect_pos(element->outer_bounds).x, rect_pos(element->outer_bounds).y);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2f, %.2f", rect_size(element->outer_bounds).x, rect_size(element->outer_bounds).y);
			
		}

		ImGui::EndTable();
	}
	
	ImGui::End();
}
#endif
