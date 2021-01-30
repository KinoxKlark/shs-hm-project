
#include "imgui_includes.cpp"

#include <SFML/Graphics.hpp>

struct Application;

#include "internals.h"
#include "math.h"
#include "rng.h"
#include "gui.h"
#include "inputs.h"
#include "game.h"
#include "application.h"
#include "renderer.h"

Application *global_app = nullptr;
Renderer *global_renderer = nullptr;
GuiManager *global_gui_manager = nullptr;

#include "gui.cpp"
#include "renderer.cpp"
#include "inputs.cpp"
#include "game.cpp"
#include "application.cpp"


int main()
{
	Application *app = application_init();
	Renderer *renderer = renderer_init(app);
	GuiManager *gui = gui_init();
	
	while(app->window->isOpen())
	{
		app->frame_duration += app->update_clock.restart();
		if(app->frame_duration > app->frame_target_duration)
		{
			app->frame_duration -= app->frame_target_duration; 
		
			process_inputs_and_events(app);

			// TODO(Sam): Put this at the right place...
			//gui->margin_unit = .02*global_renderer->window->getSize().x;
			gui->margin_unit = .02*Min(global_renderer->window->getSize().x,global_renderer->window->getSize().y); 
			
			update(app,app->frame_target_duration);
			
			render(renderer);
			GuiReset(gui);
		}
		
	}

	gui_shutdown(gui);
	renderer_shutdown(renderer);
	application_shutdown(app);
	
    return 0;
}
