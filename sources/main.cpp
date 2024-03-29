
#include "imgui_includes.cpp"

#include <SFML/Graphics.hpp>

struct Application;

#include "internals.h"
#include "math.h"
#include "rng.h"
#include "rounded_rectangle.h"
#include "gui.h"
#include "inputs.h"
#include "events.h"
#include "social_posts.h"
#include "user.h"
#include "game.h"
#include "application.h"
#include "renderer.h"

Application *global_app = nullptr;
Renderer *global_renderer = nullptr;

#include "game_utilities.h"

#include "events.cpp"
#include "social_posts.cpp"
#include "events_parser.cpp"
#include "gui.cpp"
#include "renderer.cpp"
#include "inputs.cpp"
#include "user.cpp"
#include "game.cpp"
#include "application.cpp"


int main()
{
	Application *app = application_init();
	Renderer *renderer = renderer_init(app);
	GuiManager *gui = gui_init();

	init_event_system(&(app->data->event_system));

	importEvents(app->data);
	starter_events(app->data);
	
	while(app->window->isOpen())
	{
		app->frame_duration += app->update_clock.restart();
		if(app->frame_duration > app->frame_target_duration)
		{
			app->frame_duration -= app->frame_target_duration; 
		
			process_inputs_and_events(app);

			// TODO(Sam): Put this at the right place...
			//gui->margin_unit = .02*global_renderer->window->getSize().x;
			//gui->margin_unit = UI_MARGIN_REF*Min(global_renderer->window->getSize().x,global_renderer->window->getSize().y);
			gui->current_size = global_renderer->window->getSize();
			gui->margin_unit = UI_MARGIN_REF*global_renderer->window->getSize().y; 

			update(app,app->frame_target_duration);
			
			render(renderer);
			GuiReset();

			if(app->should_quit) app->window->close();
		}
		
	}

	gui_shutdown();
	destroy_event_system(&(app->data->event_system));
	renderer_shutdown(renderer);
	application_shutdown(app);
	
    return 0;
}
