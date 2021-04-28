
#include<ctime>



Application* application_init()
{
	random_init(time(0));
	//random_init(0);
	
	Application *app = new Application();
	app->window = new sf::RenderWindow(sf::VideoMode(1200,600), "SHS");
	//app->window = new sf::RenderWindow(sf::VideoMode::getFullscreenModes()[0], "SHS", sf::Style::Fullscreen);
	app->frame_duration = sf::Time::Zero;
	app->frame_target_duration = sf::seconds(1.f/60.f);
	app->data = game_data_init();
	app->should_quit = false;
	global_app = app;

	return app;
}

void application_shutdown(Application *app)
{
	game_data_shutdown(app->data);
	delete app->window;
	delete app;
	global_app = nullptr;
}
