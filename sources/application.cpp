
Application* application_init()
{
	Application *app = new Application();
	app->window = new sf::RenderWindow(sf::VideoMode(800,600), "SHS");
	//app->window = new sf::RenderWindow(sf::VideoMode::getFullscreenModes()[0], "SHS", sf::Style::Fullscreen);
	app->frame_duration = sf::Time::Zero;
	app->frame_target_duration = sf::seconds(1.f/60.f);
	app->data = game_data_init();
	app->should_quit = false;
	global_app = app;

	random_init(0);
	
	return app;
}

void application_shutdown(Application *app)
{
	game_data_shutdown(app->data);
	delete app->window;
	delete app;
	global_app = nullptr;
}
