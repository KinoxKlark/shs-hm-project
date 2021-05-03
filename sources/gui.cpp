#include <sysinfoapi.h>

inline void gui_update(sf::Time dt)
{
	GuiManager *gui = global_gui_manager;
	
	for(auto it = gui->properties.begin(); it != gui->properties.end(); ++it)
	{
		GuiElementProperties *props = &(it->second);
		if(props->timer_active)
			props->timer -= dt.asSeconds();
		if(props->timer < 0.f) props->timer = 0.f;

		// TODO(Sam): We will want a flag test to know which props to update
		if(props->drag_target_pos != props->drag_pos)
		{
			v2 delta_before = props->drag_target_pos - props->drag_pos;
			v2 speed = 3e3f*safe_normalize(delta_before);
			props->drag_pos += speed*dt.asSeconds();
			v2 delta_after = props->drag_target_pos - props->drag_pos;
			if(dot(delta_before, delta_after) <= 0.f)
			{
				props->drag_pos = props->drag_target_pos;

				if(!props->dragged && !props->dropped && props->drag_target.callback)
				{
					bool accepted = props->drag_target.callback(props->payload, props->drag_target.user_data);
					if(accepted) props->dropped = true;
					else
					{
						props->drag_target_pos = props->drag_source_pos;
					}
				}
			}
		}
	}
}

inline void gui_post_treatment()
{
	GuiManager *gui = global_gui_manager;
	
	if(gui->dragging_payload != -1)
	{
		GuiElementProperties *payload_props = &gui->properties[gui->dragging_payload_id];

		if(payload_props->dragged)
		{
			GuiElement *payload = &gui->elements[gui->dragging_payload];
			rect payload_rect = payload->bounds;

			bool target_any = false;
			u32 drop_id = -1;
			r32 max_cover = 0;

			//global_renderer->debug_rects.push_back({payload_rect, sf::Color::Blue});

			for(u32 id = 0; id < gui->payload_targets.size(); ++id)
			{
				GuiElementPayloadTarget *target = &gui->payload_targets[id];
				rect target_rect = target->element->bounds;

				//global_renderer->debug_rects.push_back({target_rect, sf::Color::Green});

				rect intersection;
				if(payload_rect.intersects(target_rect, intersection))
				{
					++gui->intersect_count;
				
					r32 cover = intersection.width*intersection.height;
					if(cover > max_cover)
					{
						max_cover = cover;
						drop_id = id;
						target_any = true;
					}

					//global_renderer->debug_rects.push_back({intersection, sf::Color::Red});
				}
			}

			if(target_any)
			{
				//payload_props->drag_target_pos = rect_pos(gui->payload_targets[drop_id].element->bounds);
				payload_props->drag_target_pos = rect_pos(gui->payload_targets[drop_id].element->bounds)+gui->payload_targets[drop_id].element->next_valid_block_pos;
				payload_props->drag_target = gui->payload_targets[drop_id];
			}
			else
			{
				GuiElement *origin = &gui->elements[gui->dragging_payload-1];
				payload_props->drag_target_pos = rect_pos(origin->bounds);
				payload_props->drag_target = {};
			}
		}
	}
}

inline
u32 GuiAddElementToContainer(GuiObject obj, GuiElementAlignment alignment)
{
	GuiManager *gui = global_gui_manager;
	
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
	v2 size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
				outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	v2 offset_ratio_change = {0.f, 0.f};
	if(obj.keep_ratio > 0.f && size.y != 0.f)
	{
		r32 ratio = size.x/size.y;
		
		if(ratio > obj.keep_ratio)
		{
			r32 old_size = size.x;
			size.x = size.y*obj.keep_ratio;
			offset_ratio_change.x = .5f*(old_size - size.x);
		}
		else
		{
			r32 old_size = size.y;
			size.y = size.x/obj.keep_ratio;
			offset_ratio_change.y = .5f*(old_size - size.y);
		}
	}
	
	v2 inner_size = { size.x - (obj.padding.left+obj.padding.right)*gui->margin_unit,
					  size.y - (obj.padding.top+obj.padding.bottom)*gui->margin_unit };


	if(inner_size.x < gui->margin_unit)
	{
		inner_size.x = gui->margin_unit;
		size.x = (1+obj.padding.left+obj.padding.right)*gui->margin_unit;
		outer_size.x = (1+obj.margin.left+obj.margin.right+obj.padding.left+obj.padding.right)*gui->margin_unit;
	}
	if(inner_size.y < gui->margin_unit)
	{
		inner_size.y = gui->margin_unit;
		size.y = (1+obj.padding.top+obj.padding.bottom)*gui->margin_unit;
		outer_size.y = (1+obj.margin.top+obj.margin.bottom+obj.padding.top+obj.padding.bottom)*gui->margin_unit;
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
	v2 pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit) + offset_ratio_change;
	v2 inner_pos = pos + v2(obj.padding.left*gui->margin_unit, obj.padding.top*gui->margin_unit);

	gui->elements[idx].set_viewport = false;
	gui->elements[idx].always_show = true;
	gui->elements[idx].container_one_past_last = idx+1;
	gui->elements[idx].id = idx;
	gui->elements[idx].container_id = 0;
	gui->elements[idx].render = parent_container->render;
	gui->elements[idx].parent = parent_container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].bounds = rect(pos, size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_block_pos = {0,0};
	gui->elements[idx].line_width = 0.f;
	gui->elements[idx].alignment = alignment;
	gui->elements[idx].obj = obj;
	gui->elements[idx].draggable = false;

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

	if(gui->push_to_dragging_payload)
		++gui->dragging_payload_size;

	return idx;
}

inline
void GuiCreateRootContainer()
{
	GuiManager *gui = global_gui_manager;
	
	GuiElement *parent_container = gui->most_recent_container;
	
	assert(("Root container already exist", !parent_container));
	assert(("Problem with root container", gui->elements_count == 0));
	++gui->elements_count;
	gui->elements[0].set_viewport = false;
	gui->elements[0].always_show = false;
	gui->elements[0].container_one_past_last = 1;
	gui->elements[0].render = true;
	gui->elements[0].parent = nullptr;
	gui->elements[0].inner_bounds = {0., 0.,
									 (r32)global_renderer->window->getSize().x,
									 (r32)global_renderer->window->getSize().y};
	gui->elements[0].bounds = gui->elements[0].inner_bounds;
	gui->elements[0].outer_bounds = gui->elements[0].inner_bounds;
	gui->elements[0].next_valid_block_pos = {0,0};
	gui->elements[0].line_width = 0.f;
	gui->elements[0].alignment = GuiElementAlignment::HORIZONTAL;

	gui->elements[0].obj.bg_color = BACKGROUND_COLOR;
	gui->elements[0].draggable = false;
		

	gui->most_recent_container = &gui->elements[0];
		
}

inline
void GuiBeginContainer(u32 container_id, GuiObject obj, GuiElementAlignment alignment)
{
	GuiManager *gui = global_gui_manager;
	
	GuiElement *parent_container = gui->most_recent_container;
	
	if(!parent_container)
		GuiCreateRootContainer();
	
	u32 idx = GuiAddElementToContainer(obj, alignment);
	gui->elements[idx].container_id = container_id;
	
	gui->elements[idx].set_viewport = true;
	gui->elements[idx].container_one_past_last = idx+1;

	if(container_id != 0)
		gui_global_container_id.push(container_id);
	
	gui->most_recent_container = &gui->elements[idx];
}

inline
void GuiEndContainer()
{
	GuiManager *gui = global_gui_manager;

	if(gui->most_recent_container->container_id != 0)
		gui_global_container_id.pop();

	gui->most_recent_container->container_one_past_last = gui->elements_count;
	
	if(gui->most_recent_container->draggable)
	{
		gui->push_to_dragging_payload = false;
	}

	assert(("GuiEndContainer() must be called once per GuiBeginContainer() call!", gui->most_recent_container));
	gui->most_recent_container = gui->most_recent_container->parent;
}

inline
void _GuiBeginTabs(u32 id, GuiObject obj, GuiElementAlignment alignment)
{
	GuiManager *gui = global_gui_manager;
	
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
	GuiBeginContainer(obj, alignment);

	GuiElement *container = gui->most_recent_container;
	container->selected_tab_id = props->selected_tab_id;
	container->next_tab_pos = 0.f;
	container->properties_id = id;
}

inline
void GuiEndTabs()
{
	GuiManager *gui = global_gui_manager;
	
	GuiEndContainer();
}

inline
bool _GuiTab(u32 id, sf::String label, GuiElementAlignment alignment)
{
	GuiManager *gui = global_gui_manager;
	
	//GuiElementProperties *props = &gui->properties[id];
	//props->touched = true;

	GuiElement *container = gui->most_recent_container;
	container->alignment = alignment;

	if(container->selected_tab_id == 0)
		container->selected_tab_id = id;

	// TODO(Sam): Magic value
	r32 tab_bar_height = 2*gui->margin_unit;
	
	GuiObject obj = {};
	obj.margin = {.2,.2,0,0};
	obj.padding = {.5,.5,.75,0};
	obj.bg_color = container->selected_tab_id == id ? UI_MAIN_BG_COLOR : UI_POST_BG_COLOR;
	obj.text.setString(label);
	obj.text.setFont(gui->font);
	obj.text.setCharacterSize(UI_BIG_TEXT_FS*gui->current_size.y);
	obj.text.setFillColor(UI_MAIN_TEXT_COLOR);
	
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
	v2 size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
				outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };
	v2 inner_size = { size.x - (obj.padding.left+obj.padding.right)*gui->margin_unit,
					  size.y - (obj.padding.top+obj.padding.bottom)*gui->margin_unit };

	v2 offset_pos = v2(container->next_tab_pos,0);
	
	v2 outer_pos = offset_pos + rect_pos(region);
	v2 pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);
	v2 inner_pos = pos + v2(obj.padding.left*gui->margin_unit, obj.padding.top*gui->margin_unit);
	
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;
	gui->elements[idx].set_viewport = false;
	gui->elements[idx].always_show = true;
	gui->elements[idx].container_one_past_last = idx+1;
	gui->elements[idx].render = container->render;
	gui->elements[idx].parent = container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].bounds = rect(pos, size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_block_pos = {0,0};
	gui->elements[idx].line_width = 0.f;
	gui->elements[idx].alignment = GuiElementAlignment::NONE;
	gui->elements[idx].obj = obj;
	gui->elements[idx].draggable = false;

	container->next_tab_pos += outer_size.x;

	bool selected = container->selected_tab_id == id;
	if(gui->elements[idx].bounds.contains(global_app->inputs.mouse_pos))
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
void GuiBeginGrid(u32 n_rows, u32 n_cols, GuiObject obj)
{
	GuiManager *gui = global_gui_manager;
	
	assert(("Grid must contain at least one cell", n_rows*n_cols > 0));

	GuiObject grid_obj = obj;
	grid_obj.padding = .5f*obj.padding;
	
	GuiBeginContainer(grid_obj, GuiElementAlignment::HORIZONTAL);
	u32 idx = gui->most_recent_container->id;
	gui->most_recent_container->grid_n_rows = n_rows;
	gui->most_recent_container->grid_n_cols = n_cols;

	GuiObject cell_obj = {};
	r32 one_before_epsilon = 1.f - 1e-4;
	cell_obj.size = { one_before_epsilon*1.f/(r32)n_cols, one_before_epsilon*1.f/(r32)n_rows };
	cell_obj.margin = .5f*obj.padding;
	cell_obj.padding = {};
	cell_obj.bg_color = sf::Color(0,0,0,0);

	//cell_obj.bg_color = sf::Color(100, 100, 150);

	u32 last_id = idx;
	for(u32 y = 0; y < n_rows; ++y)
	for(u32 x = 0; x < n_cols; ++x)
	{		
		GuiBeginContainer(cell_obj);
		u32 idx = gui->most_recent_container->id;
		assert(idx == last_id + 1);
		last_id = idx;
		GuiEndContainer();
	}

	gui->most_recent_container = &gui->elements[idx+1];
}

inline
void GuiEndGrid()
{
	GuiManager *gui = global_gui_manager;
	
	gui->most_recent_container = gui->most_recent_container->parent;
	GuiEndContainer();
}

inline
void GuiSelectGridCell(u32 row, u32 col)
{
	GuiManager *gui = global_gui_manager;
	
	GuiElement *container = gui->most_recent_container->parent;

	assert(("Out of bound grid cell", row < container->grid_n_rows && col < container->grid_n_cols));
	
	gui->most_recent_container = &gui->elements[container->id + 1 + row * container->grid_n_cols + col];
}

inline
u32 _GuiDefineContainerAsDraggable(u32 id, void* payload)
{
	GuiManager *gui = global_gui_manager;
	
	GuiElementProperties *props;
	if(gui->properties.find(id) == gui->properties.end())
	{
		props = &gui->properties[id];
		props->dragged = false;
		props->drag_grab_offset = {0,0};
		props->drag_target = {};
		props->dropped = false;
		props->last_frame_bounds = gui->most_recent_container->inner_bounds;
	}
	else
	{
		props = &gui->properties[id];
	}
	props->payload = payload;
	props->touched = true;

	GuiElement *drag_source = gui->most_recent_container;

	v2 delta = rect_size(props->last_frame_bounds) - rect_size(drag_source->inner_bounds);
	//drag_source->inner_bounds.width += delta.x;
	drag_source->inner_bounds.height += delta.y;
	//drag_source->bounds.width += delta.x;
	drag_source->bounds.height += delta.y;
	//drag_source->outer_bounds.width += delta.x;
	drag_source->outer_bounds.height += delta.y;
	drag_source->parent->next_valid_block_pos.y += delta.y;
	
	GuiObject obj = drag_source->obj;
	
	GuiObject drag_source_obj = obj;
	if(props->dragged || props->drag_pos != props->drag_target_pos)
	{
		drag_source_obj.bg_color = sf::Color(255,255,255,50);
	}
	drag_source->obj = drag_source_obj;
	drag_source->draggable = true;

	props->drag_source_pos = rect_pos(drag_source->bounds);

	if(gui->dragging_payload == -1)
	{
		if(!props->dragged)
		{
			if(drag_source->inner_bounds.contains(global_app->inputs.mouse_pos))
			{
				if(global_app->inputs.mouse_pressed)
				{
					props->dragged = true;
					props->dropped = false;
					props->drag_grab_offset = global_app->inputs.mouse_pos - rect_pos(drag_source->bounds);
				}
			}
		}
 		else
		{
			if(global_app->inputs.mouse_released)
			{
				props->dragged = false;
			}
		}
	}

	if(props->dragged || props->drag_pos != props->drag_target_pos)
	{
		assert(gui->elements_count < gui->elements.size());
		u32 idx = gui->elements_count++;

		GuiElement *payload = &gui->elements[idx];

		*payload = *drag_source;
		payload->id = id;
		payload->render = false;
		payload->obj = obj;

		drag_source->container_one_past_last = idx;
		

		if(props->dragged)
		{
			payload->bounds.left = global_app->inputs.mouse_pos.x - props->drag_grab_offset.x;
			payload->bounds.top = global_app->inputs.mouse_pos.y - props->drag_grab_offset.y;

			props->drag_pos = rect_pos(payload->bounds);
		}
		else
		{
			payload->bounds.left = props->drag_pos.x;
			payload->bounds.top = props->drag_pos.y;
		}

		payload->inner_bounds.left = payload->bounds.left + payload->obj.padding.left*gui->margin_unit;
		payload->inner_bounds.top = payload->bounds.top + payload->obj.padding.top*gui->margin_unit;
		
		gui->push_to_dragging_payload = true;
		gui->dragging_payload_id = id;
		gui->dragging_payload = idx;
		gui->dragging_payload_size = 1;

		gui->most_recent_container = payload;
	}

	return id;
}

inline
void GuiDroppableArea(DropCallback callback, void *user_data)
{
	GuiManager *gui = global_gui_manager;

	GuiElement *container = gui->most_recent_container;

	gui->payload_targets.push_back({ container, callback, user_data });
}

inline
bool _GuiButton(u32 id, sf::String label)
{
	GuiManager *gui = global_gui_manager;
	
	GuiElementProperties *props = &gui->properties[id];
	props->touched = true;

	GuiObject obj = {};
	
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

	u32 idx = GuiAddElementToContainer(obj, GuiElementAlignment::NONE);

    // Button Behavior
	bool pressed = false;
	if(gui->elements[idx].bounds.contains(global_app->inputs.mouse_pos))
	{
		if(props->timer <= 0.f)
			gui->elements[idx].obj.bg_color = sf::Color(150,100,150);
		pressed = global_app->inputs.mouse_pressed;
		if(pressed)
		{
			props->timer = 1.f;
			props->timer_active = false;
		}
	}
	if(global_app->inputs.mouse_released)
		props->timer_active = true;
	
	
	return pressed;
}


inline
void GuiTitle(sf::String title, r32 font_size)
{
	GuiManager *gui = global_gui_manager;

	GuiObject obj = {};
	
	obj.margin = {0,0,0,.5};
	obj.padding = {};
	obj.bg_color = sf::Color(0,0,0,0);
	obj.text.setString(title);
	obj.text.setFont(gui->font);
	obj.text.setCharacterSize(font_size*gui->current_size.y);
	obj.text.setFillColor(sf::Color(50,50,50));
	
	assert(("Title can only be puts in container", gui->most_recent_container));

	v2 region = rect_size(gui->most_recent_container->inner_bounds);
	v2 text_size = rect_size(obj.text.getLocalBounds());

	obj.size = {
		(text_size.x + (obj.margin.left + obj.margin.right
						+ obj.padding.left + obj.padding.right)*gui->margin_unit)/region.x,
		(text_size.y + (obj.margin.top + obj.margin.bottom
						+ obj.padding.top + obj.padding.bottom)*gui->margin_unit)/region.y
	};

	u32 idx = GuiAddElementToContainer(obj, GuiElementAlignment::NONE);
	
}

inline
void GuiText(sf::String text, r32 font_size)
{
	GuiManager *gui = global_gui_manager;

	assert(("Text can only be puts in container", gui->most_recent_container));
	
	GuiObject obj = {};
	
	//obj.margin = {1,1,1,1};
	//obj.padding = {.5,.5,.5,.5};

	obj.margin = {};
	obj.padding = {};

	obj.bg_color = sf::Color(0,0,0,0);
	obj.htmltext.setString(text);
	obj.htmltext.setFont(gui->font);
	obj.htmltext.setCharacterSize(font_size*gui->current_size.y);
	obj.htmltext.setFillColor(sf::Color(50,50,50));
	obj.htmltext.setTextWidth(rect_size(gui->most_recent_container->inner_bounds).x);

	v2 region = rect_size(gui->most_recent_container->inner_bounds);
	v2 text_size = rect_size(obj.htmltext.getLocalBounds());

	obj.size = {
		1.f,
		(text_size.y + (obj.margin.top + obj.margin.bottom
						+ obj.padding.top + obj.padding.bottom)*gui->margin_unit)/region.y
	};

	u32 idx = GuiAddElementToContainer(obj, GuiElementAlignment::NONE);

}

inline
void GuiFeedName(sf::String name)
{
	GuiManager *gui = global_gui_manager;
	GuiTitle(name, UI_BIG_TEXT_FS);
}

#if DEBUG
void GuiDebug()
{
	GuiManager *gui = global_gui_manager;
	
	ImGui::Begin("GUI Debug");

	ImGui::Text("Payload: %u (size: %u)", gui->dragging_payload, gui->dragging_payload_size);
	ImGui::Text(" - dropped: %s", gui->properties[gui->dragging_payload_id].dropped ? "True" : "False");
	ImGui::Text(" - intersections: %u", gui->intersect_count);

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
