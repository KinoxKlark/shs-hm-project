#include <stack>

void render(Renderer *renderer)
{
	v2u render_region = renderer->window->getSize();
	//renderer->view = sf::View(rect(0.f, 0.f, render_region.x, render_region.y));
	//renderer->window->setView(renderer->view);

	renderer->window->clear();

	GuiManager *gui = global_gui_manager;
	
	sf::RectangleShape rect_shape;
	for(u32 idx(0); idx < gui->elements_count; ++idx)
	{
		GuiElement *element = &gui->elements[idx];
		rect_shape.setSize(rect_size(element->inner_bounds));
		rect_shape.setPosition(rect_pos(element->inner_bounds));
		rect_shape.setFillColor(element->obj.bg_color);
		renderer->window->draw(rect_shape);
	}

	
	/*
	std::stack<rect> container_region, outer_container_region;
	container_region.push({0., 0., renderer->view.getSize().x, renderer->view.getSize().y});
	outer_container_region.push(container_region.top());
	i32 container_depth = 0;

	sf::RectangleShape rect_shape;

	r32 margin_unit = .02*outer_container_region.top().width;

	for(u32 idx(0); idx < renderer->gui_cmd.size(); ++idx)
	{
		GuiCmd *cmd = &renderer->gui_cmd[idx];
		switch(cmd->type)
		{
		case GuiCmdType::BEGIN_CONTAINER:
		{
			GuiObject *obj = &cmd->obj;
			v2 relative_size = {
				obj->size.x < 0 ? 1.f : obj->size.x,
				obj->size.y < 0 ? 1.f : obj->size.y
			};
			v2 outer_size = hadamar(rect_size(container_region.top()), relative_size);
			v2 inner_size = { outer_size.x - (obj->margin.left+obj->margin.right)*margin_unit,
							  outer_size.y - (obj->margin.top+obj->margin.bottom)*margin_unit };

			v2 outer_pos = rect_pos(container_region.top());
			v2 inner_pos = outer_pos + v2(obj->margin.left*margin_unit, obj->margin.top*margin_unit);
			
			rect_shape.setSize(inner_size);
			rect_shape.setPosition(inner_pos);
			rect_shape.setFillColor(obj->bg_color);
			renderer->window->draw(rect_shape);

			container_region.push({
					inner_pos.x + obj->padding.left * margin_unit,
					inner_pos.y + obj->padding.top * margin_unit,
					inner_size.x - (obj->padding.right + obj->padding.left)*margin_unit,
					inner_size.y - (obj->padding.bottom + obj->padding.top)*margin_unit
				});

			outer_container_region.push({
					outer_pos.x,
					outer_pos.y,
					outer_size.x,
					outer_size.y
				});
			

			++container_depth;
		} break;

		case GuiCmdType::END_CONTAINER:
		{
			assert(("END_CONTAINER must be used once per BEGIN_CONTAINER",container_depth > 0));
			rect previous_region = outer_container_region.top();
			container_region.pop();
			outer_container_region.pop();
			container_region.top().top += previous_region.height;
			--container_depth;
		} break;

		case GuiCmdType::BUTTON:
		{
			GuiObject *obj = &cmd->obj;
			v2 relative_size = {
				obj->size.x < 0 ? 1.f : obj->size.x,
				obj->size.y < 0 ? 1.f : obj->size.y
			};
			v2 outer_size = hadamar(rect_size(container_region.top()), relative_size);
			v2 inner_size = { outer_size.x - (obj->margin.left+obj->margin.right)*margin_unit,
							  outer_size.y - (obj->margin.top+obj->margin.bottom)*margin_unit };

			v2 outer_pos = rect_pos(container_region.top());
			v2 inner_pos = outer_pos + v2(obj->margin.left*margin_unit, obj->margin.top*margin_unit);
			
			rect_shape.setSize(inner_size);
			rect_shape.setPosition(inner_pos);
			rect_shape.setFillColor(obj->bg_color);
			renderer->window->draw(rect_shape);

			container_region.top().top += outer_size.y;

			
		} break;
		
		InvalidDefaultCase;
		};
	}

	assert(("BEGIN_CONTAINER/END_CONTAINER must always be en pair",container_depth == 0));
	GuiClearCmd(renderer);
	*/


#ifdef DEBUG
	ImGui::SFML::Render(*renderer->window);
#endif
	
	renderer->window->display();
}

Renderer* renderer_init(Application *app)
{
	Renderer *renderer = new Renderer();
	renderer->window = app->window;
	//renderer->view = sf::View(sf::FloatRect(0.f, 0.f, app->window->getSize().x, app->window->getSize().y));
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

