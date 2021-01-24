
#include "imgui_includes.cpp"

#include <SFML/Graphics.hpp>

#include "application.h"
#include "renderer.h"

Application *global_app = nullptr;
Renderer *global_renderer = nullptr;

#include "renderer.cpp"
#include "inputs.cpp"
#include "game.cpp"
#include "application.cpp"


int main()
{
	Application *app = application_init();
	Renderer *renderer = renderer_init(app);
	
	while(app->window->isOpen())
	{

		app->frame_duration += app->update_clock.restart();
		if(app->frame_duration > app->frame_target_duration)
		{
			app->frame_duration -= app->frame_target_duration; 
		
			process_inputs_and_events(app);

			update(app,app->update_clock.restart());
			
			render(renderer);
		}
		
	}

	renderer_shutdown(renderer);
	application_shutdown(app);
	
    return 0;
}
