Application* application_init()
{
	Application *app = new Application();
	app->window = new sf::RenderWindow(sf::VideoMode(800,600), "SHS");
	app->frame_duration = sf::Time::Zero;
	app->frame_target_duration = sf::seconds(1.f/60.f);
	global_app = app;
	return app;
}

void application_shutdown(Application *app)
{
	delete app->window;
	delete app;
	global_app = nullptr;
}
