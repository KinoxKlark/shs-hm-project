
inline GuiManager* gui_init()
{
	GuiManager* gui = new GuiManager();
	gui->most_recent_container = nullptr;
	gui->elements_count = 0;
	global_gui_manager = gui;
	return gui;
}

inline void gui_shutdown(GuiManager* gui)
{
	global_gui_manager = nullptr;
	delete gui;
}

inline
void GuiBeginContainer(GuiManager *gui, GuiObject obj, GuiElementAlignment alignment)
{
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;

	GuiElement *parent_container = gui->most_recent_container;
	rect container_region;
	v2 next_valid_block_pos;
	GuiElementAlignment parent_alignment;
	r32 parent_line_width;
	if(parent_container)
	{
		container_region = parent_container->inner_bounds;
		next_valid_block_pos = parent_container->next_valid_block_pos;
		parent_alignment = parent_container->alignment;
		parent_line_width = parent_container->line_width;
	}
	else
	{
		// TODO(Sam): Find a better way to deal with root element
		container_region = {0., 0.,
							(r32)global_renderer->window->getSize().x,
							(r32)global_renderer->window->getSize().y};
		next_valid_block_pos = {0,0};
		parent_alignment = GuiElementAlignment::HORIZONTAL;
		parent_line_width = 0.f;
	}
	
	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(container_region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

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
	
	gui->most_recent_container = &gui->elements[idx];
}

inline
void GuiEndContainer(GuiManager *gui)
{
	assert(("GuiEndContainer() must be called once per GuiBeginContainer() call!", gui->most_recent_container));
	gui->most_recent_container = gui->most_recent_container->parent;
}

inline
bool GuiButton(GuiManager *gui, sf::String label)
{
	GuiObject obj;
	obj.size = v2(.3, .2);
	obj.margin = {1,1,1,1};
	obj.padding = {};
	obj.bg_color = sf::Color(100,50,100);

	// TODO(Sam): This code is duplicated in every element, we should abstract it
	
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;

	GuiElement *parent_container = gui->most_recent_container;
	assert(("GuiBeginContainer() must be call before adding a Button", parent_container));
	
	rect container_region = parent_container->inner_bounds;
	
	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(container_region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	v2 offset_pos = parent_container->next_valid_block_pos;
	switch(parent_container->alignment)
	{
	case GuiElementAlignment::HORIZONTAL:
	{
		if(offset_pos.x + outer_size.x > parent_container->inner_bounds.width)
		{
			offset_pos = v2(0, offset_pos.y + parent_container->line_width);
			parent_container->line_width = 0.f;
			parent_container->next_valid_block_pos = offset_pos;
		}
	} break;
	case GuiElementAlignment::VERTICAL:
	{
		if(offset_pos.y + outer_size.y > parent_container->inner_bounds.height)
		{
			offset_pos = v2(offset_pos.x + parent_container->line_width,0);
			parent_container->line_width = 0.f;
			parent_container->next_valid_block_pos = offset_pos;
		}
	} break;
	case GuiElementAlignment::NONE: break;
	InvalidDefaultCase;
	}
	
	v2 outer_pos = offset_pos + rect_pos(container_region);
	v2 inner_pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);

	rect inner_bounds = rect(inner_pos, inner_size);
	rect outer_bounds = rect(outer_pos, outer_size);

	// Button Behavior
	bool pressed = false;
	if(inner_bounds.contains(global_app->inputs.mouse_pos))
	{
		obj.bg_color = sf::Color(150,100,150);
		pressed = global_app->inputs.mouse_pressed;
	}
	
	gui->elements[idx].parent = parent_container;
	gui->elements[idx].inner_bounds = inner_bounds;
	gui->elements[idx].outer_bounds = outer_bounds;
	gui->elements[idx].next_valid_block_pos = {0,0};
	gui->elements[idx].line_width = 0.f;
	gui->elements[idx].alignment = GuiElementAlignment::NONE;
	gui->elements[idx].obj = obj;

	switch(parent_container->alignment)
	{
	case GuiElementAlignment::HORIZONTAL:
	{
		parent_container->next_valid_block_pos.x += outer_size.x;
		parent_container->line_width = Max(parent_container->line_width, outer_size.y);
	} break;
	case GuiElementAlignment::VERTICAL:
	{
		parent_container->next_valid_block_pos.y += outer_size.y;
		parent_container->line_width = Max(parent_container->line_width, outer_size.x);
	} break;
	case GuiElementAlignment::NONE:
	{
		parent_container->next_valid_block_pos = {0,0};
	} break;
	InvalidDefaultCase;
	}
	
	return pressed;
}

#ifdef DEBUG
void GuiDebug(GuiManager *gui)
{
	ImGui::Begin("GUI Debug");

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
