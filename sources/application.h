struct Application
{
	sf::RenderWindow *window;
	sf::Clock update_clock;
	sf::Time frame_duration;
	sf::Time frame_target_duration;

	sf::Clock debug_clock;
};

Application* application_init();
void application_shutdown(Application *app);

