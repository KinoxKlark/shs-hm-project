#include <stack>

void render(Renderer *renderer)
{
	v2u render_region = renderer->window->getSize();
	renderer->view = sf::View(rect(0.f, 0.f, render_region.x, render_region.y));
	renderer->window->setView(renderer->view);

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

