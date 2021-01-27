
void process_inputs_and_events(Application *app)
{
	app->inputs.mouse_pressed = false;
	
	sf::Event event;
	while(app->window->pollEvent(event))
	{
#ifdef DEBUG
		ImGui::SFML::ProcessEvent(event);
#endif
		if(event.type == sf::Event::Closed)
			app->window->close();
		if(event.type == sf::Event::MouseButtonPressed)
			app->inputs.mouse_pressed = event.mouseButton.button == sf::Mouse::Button::Left;
	}

	app->inputs.mouse_pos = v2(sf::Mouse::getPosition(*app->window));
}
