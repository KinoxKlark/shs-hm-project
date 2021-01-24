void process_inputs_and_events(Application *app)
{
	sf::Event event;
	while(app->window->pollEvent(event))
	{
#ifdef DEBUG
		ImGui::SFML::ProcessEvent(event);
#endif
		if(event.type == sf::Event::Closed)
			app->window->close();
	}
}
