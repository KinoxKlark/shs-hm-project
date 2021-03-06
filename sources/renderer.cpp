#include <stack>

void render(Renderer *renderer)
{
	v2u render_region = renderer->window->getSize();
	renderer->view = sf::View(rect(0.f, 0.f, render_region.x, render_region.y));
	renderer->window->setView(renderer->view);

	renderer->window->clear();

	GuiManager *gui = global_gui_manager;

	struct viewport_info {
		u32 last_elem_id;
		sf::View view;
	};

	std::stack<viewport_info> viewport_infos;
	viewport_infos.push({ gui->elements_count, renderer->view });

	sf::RectangleShape rect_shape;
	for(u32 idx(0); idx < gui->elements_count; ++idx)
	{
		GuiElement *element = &gui->elements[idx];

		while(idx == viewport_infos.top().last_elem_id)
		{
			viewport_infos.pop();
			renderer->window->setView(viewport_infos.top().view);
		}
		
		if(!element->render) continue;

		if(element->set_viewport)
		{
			sf::View view;
			rect viewport;
			viewport_infos.top().view.getViewport().intersects({
					element->bounds.left / render_region.x,
					element->bounds.top / render_region.y,
					element->bounds.width / render_region.x,
					element->bounds.height / render_region.y }, viewport);

			view.reset(element->bounds);
			view.setViewport(viewport);
			renderer->window->setView(view);

			viewport_infos.push({element->container_one_past_last, view});
		}

		if(element->always_show)
			renderer->window->setView(renderer->view);

		rect_shape.setSize(rect_size(element->bounds));
		rect_shape.setPosition(rect_pos(element->bounds));
		rect_shape.setFillColor(element->obj.bg_color);
		renderer->window->draw(rect_shape);

		if(element->obj.text.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.text.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.text.setPosition(rect_pos(element->inner_bounds) + offset);
			renderer->window->draw(element->obj.text);
		}

		if(element->obj.htmltext.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.htmltext.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.htmltext.setPosition(rect_pos(element->inner_bounds) + offset);
			renderer->window->draw(element->obj.htmltext);
		}
		
		if(element->always_show)
			renderer->window->setView(viewport_infos.top().view);
	}

	while(viewport_infos.size() > 0 && gui->elements_count == viewport_infos.top().last_elem_id)
	{
		viewport_infos.pop();
		if(viewport_infos.size() > 0)
			renderer->window->setView(viewport_infos.top().view);
	}
	
	assert(viewport_infos.size() == 0);

	viewport_infos.push({ gui->elements_count, renderer->view });
	
	// Drag and Drop Payload
	for(u32 idx = 0; idx < gui->dragging_payload_size; ++idx)
	{
		GuiElement *element = &gui->elements[gui->dragging_payload + idx];

		while(idx == viewport_infos.top().last_elem_id)
		{
			viewport_infos.pop();
			renderer->window->setView(viewport_infos.top().view);
		}
		
		if(element->set_viewport)
		{
			sf::View view;
			rect viewport;
			viewport_infos.top().view.getViewport().intersects({
					element->bounds.left / render_region.x,
					element->bounds.top / render_region.y,
					element->bounds.width / render_region.x,
					element->bounds.height / render_region.y }, viewport);

			view.reset(element->bounds);
			view.setViewport(viewport);
			renderer->window->setView(view);

			viewport_infos.push({element->container_one_past_last, view});
		}

		if(element->always_show)
			renderer->window->setView(renderer->view);

		rect_shape.setSize(rect_size(element->bounds));
		rect_shape.setPosition(rect_pos(element->bounds));
		rect_shape.setFillColor(element->obj.bg_color);
		renderer->window->draw(rect_shape);

		if(element->obj.text.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.text.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.text.setPosition(rect_pos(element->inner_bounds) + offset);
			renderer->window->draw(element->obj.text);
		}

		if(element->obj.htmltext.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.htmltext.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.htmltext.setPosition(rect_pos(element->inner_bounds) + offset);
			renderer->window->draw(element->obj.htmltext);
		}
		
		if(element->always_show)
			renderer->window->setView(viewport_infos.top().view);
	}


	// Debug
#ifdef DEBUG
	for(u32 idx = 0; idx < renderer->debug_rects.size(); ++idx)
	{
		DebugRect *rect = &renderer->debug_rects[idx];
		rect_shape.setSize(rect_size(rect->bounds));
		rect_shape.setPosition(rect_pos(rect->bounds));
		rect_shape.setFillColor(rect->color);
		renderer->window->draw(rect_shape);
	}
	renderer->debug_rects.clear();
#endif

#ifdef DEBUG
	ImGui::SFML::Render(*renderer->window);
#endif
	
	renderer->window->display();
}

Renderer* renderer_init(Application *app)
{
	Renderer *renderer = new Renderer();
	renderer->window = app->window;
	renderer->view = sf::View(sf::FloatRect(0.f, 0.f, app->window->getSize().x, app->window->getSize().y));
	global_renderer = renderer;
	
#ifdef DEBUG
	ImGui::SFML::Init(*app->window);
#endif

	return renderer;
}

void renderer_shutdown(Renderer *renderer)
{
#ifdef DEBUG
	ImGui::SFML::Shutdown();
#endif

	delete renderer;
	global_renderer = nullptr;
}

