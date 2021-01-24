

void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif

	// TODO(Sam): Update...
	
#ifdef DEBUG
	ImGui::ShowDemoWindow();
	ImGui::Begin("Debug Infos");
	int debug_time = app->debug_clock.restart().asMilliseconds();
	ImGui::Text("Frame duration: %ims (%.2ffps)", debug_time, 1e3/(float)debug_time);
	ImGui::End();
#endif
}
