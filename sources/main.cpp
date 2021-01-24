
#include "imgui_includes.cpp"

#include <SFML/Graphics.hpp>

struct Application
{
	sf::RenderWindow *window;
	sf::Clock update_clock;
	sf::Time frame_duration;
	sf::Time frame_target_duration;

	sf::Clock debug_clock;
};

Application *global_app;

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

void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif


	
#ifdef DEBUG
	ImGui::ShowDemoWindow();
	ImGui::Begin("Debug Infos");
	int debug_time = app->debug_clock.restart().asMilliseconds();
	ImGui::Text("Frame duration: %ims (%.2ffps)", debug_time, 1e3/(float)debug_time);
	ImGui::End();
#endif
}

void render(Application *app)
{
	app->window->clear();


#ifdef DEBUG
	ImGui::SFML::Render(*app->window);
#endif
	app->window->display();
}

int main()
{
	Application app;
	app.window = new sf::RenderWindow(sf::VideoMode(800,600), "SHS");
	app.frame_duration = sf::Time::Zero;
	app.frame_target_duration = sf::seconds(1.f/60.f);
	global_app = &app;
	
#ifdef DEBUG
	ImGui::SFML::Init(*app.window);
#endif
	
	while(app.window->isOpen())
	{

		app.frame_duration += app.update_clock.restart();
		if(app.frame_duration > app.frame_target_duration)
		{
			app.frame_duration -= app.frame_target_duration; 
		
			process_inputs_and_events(&app);

			update(&app,app.update_clock.restart());
			
			render(&app);
		}
		
	}

#ifdef DEBUG
	ImGui::SFML::Shutdown();
#endif


	delete app.window;
	
    return 0;
}
