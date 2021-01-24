void render(Renderer *renderer)
{
	renderer->window->setView(renderer->view);
	renderer->window->clear();

	sf::RectangleShape rect(sf::Vector2f(.1f,.1f));
	renderer->window->draw(rect);

	rect.setPosition(.9f,0.f);
	renderer->window->draw(rect);
	

#ifdef DEBUG
	ImGui::SFML::Render(*renderer->window);
#endif
	renderer->window->display();
}

Renderer* renderer_init(Application *app)
{
	Renderer *renderer = new Renderer();
	renderer->window = app->window;
	renderer->view = sf::View(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
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

