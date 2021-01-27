
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
void GuiBeginContainer(GuiManager *gui, GuiObject obj)
{
	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;

	GuiElement *parent_container = gui->most_recent_container;
	rect container_region;
	v2 pos_offset;
	if(parent_container)
	{
		container_region = parent_container->inner_bounds;
		pos_offset = parent_container->next_valid_pos_offset;
	}
	else
	{
		// TODO(Sam): Find a better way to deal with root element
		container_region = {0., 0.,
							(r32)global_renderer->window->getSize().x,
							(r32)global_renderer->window->getSize().y};
		pos_offset = {0,0};
	}
	
	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(container_region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	v2 outer_pos = pos_offset + rect_pos(container_region);
	v2 inner_pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);

	gui->elements[idx].parent = parent_container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_pos_offset = {0,0};
	gui->elements[idx].obj = obj;

	if(parent_container)
		parent_container->next_valid_pos_offset.y += outer_size.y;
	
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
	obj.bg_color = sf::Color(50,50,50);

	assert(gui->elements_count < gui->elements.size());
	u32 idx = gui->elements_count++;

	GuiElement *parent_container = gui->most_recent_container;
	assert(("GuiBeginContainer() must be call before adding a Button", parent_container));
	
	rect container_region = parent_container->inner_bounds;
	v2 pos_offset = parent_container->next_valid_pos_offset;
	
	v2 relative_size = {
		obj.size.x < 0 ? 1.f : obj.size.x,
		obj.size.y < 0 ? 1.f : obj.size.y
	};
	v2 outer_size = hadamar(rect_size(container_region), relative_size);
	v2 inner_size = { outer_size.x - (obj.margin.left+obj.margin.right)*gui->margin_unit,
					  outer_size.y - (obj.margin.top+obj.margin.bottom)*gui->margin_unit };

	v2 outer_pos = pos_offset + rect_pos(container_region);
	v2 inner_pos = outer_pos + v2(obj.margin.left*gui->margin_unit, obj.margin.top*gui->margin_unit);

	gui->elements[idx].parent = parent_container;
	gui->elements[idx].inner_bounds = rect(inner_pos, inner_size);
	gui->elements[idx].outer_bounds = rect(outer_pos, outer_size);
	gui->elements[idx].next_valid_pos_offset = {0,0};
	gui->elements[idx].obj = obj;

	parent_container->next_valid_pos_offset.y += outer_size.y;
			
	return false;
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
