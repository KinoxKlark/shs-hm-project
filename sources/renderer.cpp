void render(Renderer *renderer)
{
	renderer->window->clear();

	
	

#ifdef DEBUG
	ImGui::SFML::Render(*renderer->window);
#endif
	renderer->window->display();
}

Renderer* renderer_init(Application *app)
{
	Renderer *renderer = new Renderer();
	renderer->window = app->window;
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

