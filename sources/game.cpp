
struct post_payload {
	u32 id;
};

bool drag_drop_accept_payload(void *payload, void* user_data)
{
	GameData *data = global_app->data;
	EventSystem *event_system = &data->event_system;
	SocialPost *post = (SocialPost*)payload;
	SocialFeed *feed = (SocialFeed*)user_data;

	user_see_post(event_system, feed, post);
	
	feed->posts.push_back(*post);

	for(auto it = data->social_post_system.available_posts.begin();
		it != data->social_post_system.available_posts.end(); ++it)
	{
		if(it->id == post->id)
		{
			data->social_post_system.available_posts.erase(it);
			break;
		}
	}

	++global_app->data->drop_counter;
	
	return true;
}

void social_post_gui(SocialPost *post, bool draggable = false)
{
	GuiObject obj;
	obj.size = { draggable ? 1.f/3.f : -1, .25 };
	obj.margin = {1,1,1,1};
	obj.padding = {1,1,1,1};
	obj.bg_color = post->color;

	GuiBeginContainer(post->id, obj, GuiElementAlignment::HORIZONTAL);
	if(draggable) GuiDefineContainerAsDraggable(post);
	GuiTitle(post->type);
	GuiText(post->text);
	GuiEndContainer();
}

inline
void main_simulation_update(Application *app, sf::Time dt)
{
	EventSystem *event_system = &app->data->event_system;


	const sf::Time EVENT_SYSTEM_INFERENCE_DT(sf::seconds(3.f));
	event_system->time_since_last_inference += dt;
	if(event_system->time_since_last_inference > EVENT_SYSTEM_INFERENCE_DT)
	{
	
		bool do_the_event_selection = false;

		event_system->thread_mutex.lock();
		do_the_event_selection = event_system->event_selection_done;
		event_system->thread_mutex.unlock();

		if(do_the_event_selection)
		{
			event_system->time_since_last_inference -= EVENT_SYSTEM_INFERENCE_DT;

			event_system->event_selection_done = false;

			event_system->thread->launch();
		}
	}
}

void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif

	gui_update(dt);

	GameData *data = app->data;
	Inputs *inputs = &app->inputs;
	EventSystem *event_system = &data->event_system;

	

	if(inputs->quit)
		app->should_quit = true;
	
	main_simulation_update(app, dt);
	
	
	// TODO(Sam): Put this in the right place
	GuiObject obj1;
	obj1.size = v2(.5, GUI_STRETCH);
	obj1.margin = {1, 1, 1, 1};
	obj1.padding = {};
	obj1.bg_color = sf::Color(150, 150, 150);

	GuiObject obj2;
	obj2.size = v2(GUI_STRETCH, .25);
	obj2.margin = {1,1,1,1};
	obj2.padding = {};
	obj2.bg_color = sf::Color(255, 150, 150);

	GuiObject obj3;
	obj3.size = v2(.33, .5);
	obj3.margin = {};
	obj3.padding = {};
	obj3.bg_color = sf::Color(150, 255, 150);

	GuiObject obj4 = obj1;
	obj4.padding = {1,1,1,1};

	GuiObject obj5;
	obj5.size = {1.f/3.f, .25};
	obj5.margin = {1,1,1,1};
	obj5.padding = {};
	obj5.bg_color = sf::Color(255,150,150);

	GuiObject obj6;
	obj6.size = {.33,.5};
	obj6.margin = {1,1,1,1};
	obj6.padding = {1,1,1,1};
	obj6.bg_color = sf::Color(150,150,255);

	GuiObject obj_full;
	obj_full.size = {1,1};
	obj_full.margin = {};
	obj_full.padding = {};
	obj_full.bg_color = sf::Color(0,0,0,0);
	
	GuiBeginGrid(2, 3, obj4);
	{
		for(u32 idx = 0; idx < data->social_post_system.social_feeds.size(); ++idx)
		{
			SocialFeed *feed = &data->social_post_system.social_feeds[idx];
			GuiSelectGridCell(idx/3, idx % 3);
			GuiBeginContainer(obj_full);
			GuiTitle(data->users[feed->user_id].fullname);
			GuiDroppableArea(drag_drop_accept_payload, feed);

			for(u32 post_idx = 0; post_idx < data->social_post_system.social_feeds[idx].posts.size(); ++post_idx)
			{
				social_post_gui(&(data->social_post_system.social_feeds[idx].posts[post_idx]));
			}
			GuiEndContainer();
		}
				
	}
	GuiEndGrid();

	GuiBeginTabs(obj1);
	{
		if(GuiTab("Tab1"))
		{
			for(u32 idx = 0; idx < data->social_post_system.available_posts.size(); ++idx)
			{
				social_post_gui(&(data->social_post_system.available_posts[idx]), true);
			}			
		}

		if(GuiTab("Tab2"))
		{
			GuiBeginContainer(obj2, GuiElementAlignment::HORIZONTAL);
			if(GuiButton("Click Me"))
			{
				data->click_counter++;
			}
			GuiEndContainer();
		}

		if(GuiTab("Tab3", GuiElementAlignment::HORIZONTAL))
		{
			GuiBeginContainer(obj3);
			{
				GuiBeginContainer(obj2);
				GuiEndContainer();
				GuiBeginContainer(obj2);
				GuiEndContainer();
			}
			GuiEndContainer();

			GuiBeginContainer(obj6, GuiElementAlignment::HORIZONTAL);
			{
				GuiButton("Click");
				GuiTitle("Test");
				GuiText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. \n\nDonec id arcu at diam interdum fringilla. Quisque euismod in augue imperdiet aliquet. In ornare fermentum nisl, ut cursus orci commodo eget. In hac habitasse platea dictumst. Ut finibus venenatis tincidunt. Nulla commodo aliquam tellus vel gravida. Aliquam semper elementum lacus, vitae bibendum ante volutpat sagittis. Ut libero velit, vulputate eget suscipit et, hendrerit vehicula tellus. Aliquam erat volutpat. Ut mattis et odio in fringilla. Phasellus pretium aliquet eros, mollis tempor odio. Aliquam auctor ante in turpis lacinia lobortis. Quisque mauris nunc, pulvinar sed euismod et, lacinia sit amet sapien. Fusce tristique mi sed volutpat molestie. Aliquam congue sagittis tellus, vitae pretium tellus rhoncus posuere.");
				GuiBeginContainer(obj3);
				GuiEndContainer();
			}
			GuiEndContainer();

			GuiBeginContainer(obj6, GuiElementAlignment::HORIZONTAL);
			{
				GuiButton("Click");
				GuiTitle("Test");
				GuiText("test");
				GuiBeginContainer(obj3);
				GuiEndContainer();
			}
			GuiEndContainer();
			
			GuiBeginContainer(obj2, GuiElementAlignment::HORIZONTAL);
			if(GuiButton("Click Me"))
			{
				data->click_counter++;
			}
			if(GuiButton("Hello"))
			{
				//GuiButton("YEP");
			}
			GuiButton("I am a button");
			GuiButton("Click Here");
			GuiButton("CLICK!");
			GuiEndContainer();
		
			GuiBeginContainer(obj3);
			{
				GuiBeginContainer(obj2);
				GuiEndContainer();
				GuiBeginContainer(obj2);
				GuiEndContainer();
			}
			GuiEndContainer();
		}
	}
	GuiEndTabs();

	gui_post_treatment();

#ifdef DEBUG
	GuiDebug();
#endif

	ImGui::Begin("Click");
	ImGui::Text("Clicked %u times!", data->click_counter);
	ImGui::Text("Dropped %u times!", data->drop_counter);
	ImGui::End();
	
	ImGui::Begin("Users");

	if(ImGui::BeginTable("Personalities", 2))
	{
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("Label");
		ImGui::TableHeadersRow();
		for(size_t idx(0); idx < data->personalities.size(); ++idx)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%i", data->personalities[idx].id);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", data->personalities[idx].name.c_str());
		}

		ImGui::EndTable();
	}

	if(ImGui::BeginTable("Interests", 2))
	{
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("Label");
		ImGui::TableHeadersRow();
		for(size_t idx(0); idx < data->interests.size(); ++idx)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%i", data->interests[idx].id);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", data->interests[idx].name.c_str());
		}

		ImGui::EndTable();
	}
	
	for(u32 idx_user = 0; idx_user < data->users.size(); ++idx_user)
	{
		User *user = &data->users[idx_user];

		ImGui::Text("[%u] %s", user->id, user->fullname.c_str());

		if(ImGui::BeginTable("User Personality", 3))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Label");
			ImGui::TableSetupColumn("Amount");
			ImGui::TableHeadersRow();
			for(size_t idx(0); idx < user->identity.personalities.size(); ++idx)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%i", user->identity.personalities[idx].id);
				
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", data->personalities[user->identity.personalities[idx].id].name.c_str());
				
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%.2f", user->identity.personalities[idx].amount);
			}

			ImGui::EndTable();
		}

		if(ImGui::BeginTable("User Interests", 3))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Label");
			ImGui::TableSetupColumn("Amount");
			ImGui::TableHeadersRow();
			for(size_t idx(0); idx < user->identity.interests.size(); ++idx)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%i", user->identity.interests[idx].id);
				
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", data->interests[user->identity.interests[idx].id].name.c_str());

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%.2f", user->identity.interests[idx].amount);
			}

			ImGui::EndTable();
		}

		if(ImGui::BeginTable("Relations", 3))
		{
			ImGui::TableSetupColumn("User ID");
			ImGui::TableSetupColumn("Fullname");
			ImGui::TableSetupColumn("Amount");
			ImGui::TableHeadersRow();
			for(size_t idx(0); idx < user->identity.relations.size(); ++idx)
			{
				if(idx == idx_user) continue;
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%i", idx);
				
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", data->users[idx].fullname.c_str());

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%.2f", user->identity.relations[idx]);
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

	
	event_system->thread_mutex.lock();

	ImGui::Begin("Known Facts");
	for(auto& pair : event_system->facts)
	{
		ImGui::Text("- %s", convert_pattern_to_string(pair.second.pattern).c_str());

		if(!pair.second.pattern->symbole && pair.second.pattern->first->symbole &&
		   pair.second.pattern->first->type == SymboleType::EVENT_OCCURED)
		{
			Event *event = &event_system->all_events[pair.second.pattern->first->next->data];
			if(event->description.size() > 0)
				ImGui::Text("    %s", event->description.c_str());
		}

	}
	ImGui::End();
	
	ImGui::Begin("Event System");
	if(ImGui::CollapsingHeader("All Events"))
	{
		for(auto& event : event_system->all_events)
		{
			ImGui::Text("[%i] %s", event.id, event.description.c_str());
			
			for(Modifs& modifs : event.users_modifs)
			{
				for(Modif& modif : modifs.modifs)
				{
					switch(modif.type)
					{
					case ModifType::PERSONALITY:
						ImGui::Text(" * %c %s: %+i", (char)modifs.user_id, data->personalities[modif.gauge_id].name.c_str(), (int)modif.nudge_amount );
						break;
					case ModifType::INTEREST:
						ImGui::Text(" * %c %s: %+i", (char)modifs.user_id, data->interests[modif.gauge_id].name.c_str(), (int)modif.nudge_amount );
						break;
					case ModifType::RELATION:
						ImGui::Text(" * Relation %c et %c: %+i", (char)modifs.user_id, (char)modif.gauge_id, modif.nudge_amount );
						break;
					}
				} 
			}
								
		}
	}
	if(ImGui::CollapsingHeader("Selected Events"))
	{
		ImGui::Text("%i events", event_system->selected_events.size());
		for(auto& event : event_system->selected_events)
		{
			ImGui::Text("[%i] %s", event.id, event.description.c_str());
			for(auto& pair : event.users)
			{
				ImGui::Text("  - ?%c = %s", pair.first, data->users[pair.second].fullname.c_str());
			}
		}
	}
	if(ImGui::CollapsingHeader("Rules"))
	{
		u32 i = 0;
		for(auto& rule : event_system->rules)
		{
			ImGui::Text("Rule [%i]:", i);
			for(auto & condition : rule.conditions)
			{
				ImGui::Text("  + %s", convert_pattern_to_string(&condition).c_str());
			}
			ImGui::Text(" => %s", convert_pattern_to_string(&rule.conclusion).c_str());
			++i;
		}
	}
	if(ImGui::CollapsingHeader("Instancied events"))
	{
		ImGui::Text("%i events", event_system->debut_instancied_events.size());
		for(i32 i = event_system->debut_instancied_events.size() - 1;
			i >= 0 && i >= (i32)(event_system->debut_instancied_events.size()) - 10;
			--i)
		{
			Event& event = event_system->debut_instancied_events[i];
			ImGui::Text("[%i] %s", event.id, event.description.c_str());
			for(auto& pair : event.users)
			{
				ImGui::Text("  - ?%c = %s", pair.first, data->users[pair.second].fullname.c_str());
			}
		}
	}
	ImGui::End();
	
#ifdef DEBUG
	//ImGui::ShowDemoWindow();
	ImGui::Begin("Debug Infos");
	int debug_time = app->debug_clock.restart().asMilliseconds();
	ImGui::Text("Frame duration: %ims (%.2ffps)", debug_time, 1e3/(float)debug_time);
	ImGui::Text("Skipped %i rules over %i", global_app->data->event_counters.nb_discarded_facts,
				global_app->data->event_counters.nb_total_facts);
	ImGui::Text("Events instanciation: %ums:", global_app->data->event_chrono.getElapsedTime().asMilliseconds());
	ImGui::Text("- %u facts", global_app->data->event_counters.facts);
	ImGui::Text("- %u rules", global_app->data->event_counters.rules);
	ImGui::Text("- %u conditions", global_app->data->event_counters.conditions);
	ImGui::Text("- %u filter calls", global_app->data->event_counters.filter_calls);
	ImGui::Text("- %u compile bool", global_app->data->event_counters.compile_bool);
	ImGui::Text("- %u compile number", global_app->data->event_counters.compile_number);
	ImGui::Text("- %u compile user", global_app->data->event_counters.compile_user);
	ImGui::Text("First event events instanciation:"); //, global_app->data->event_counters_first.duration.asMilliseconds());
	ImGui::Text("- %u facts", global_app->data->event_counters_first.facts);
	ImGui::Text("- %u rules", global_app->data->event_counters_first.rules);
	ImGui::Text("- %u conditions", global_app->data->event_counters_first.conditions);
	ImGui::Text("- %u filter calls", global_app->data->event_counters_first.filter_calls);
	ImGui::Text("- %u compile bool", global_app->data->event_counters_first.compile_bool);
	ImGui::Text("- %u compile number", global_app->data->event_counters_first.compile_number);
	ImGui::Text("- %u compile user", global_app->data->event_counters_first.compile_user);
	ImGui::End();
#endif

	
	event_system->thread_mutex.unlock();

}

inline
GameData* game_data_init()
{
	GameData* data = new GameData();

#if DEBUG
	data->event_counters = {};
#endif

	std::vector<std::string> firstnames_men = importFirstNames(true);
	std::vector<std::string> firstnames_women = importFirstNames(false);
	std::vector<std::string> lastnames = importLastNames();

	data->personalities = importGauges("data/identities.txt");
	data->interests = importGauges("data/interests.txt");
	
	data->users.resize(6);
	for(u32 idx = 0; idx < data->users.size(); ++idx)
	{
		data->users[idx].id = idx;
		data->users[idx].identity = createUserIdentity(data);
		data->users[idx].isMan = get_random_number_between(0,1) == 0;

		if(data->users[idx].isMan)
		{
			data->users[idx].fullname = get_random_element(firstnames_men) + " " + get_random_element(lastnames);
		}
		else
		{
			data->users[idx].fullname = get_random_element(firstnames_women) + " " + get_random_element(lastnames);		
		}
	
	}

	for(u32 idx1 = 0; idx1 < data->users.size(); ++idx1)
		for(u32 idx2 = 0; idx2 < idx1; ++idx2)
		{
			r32 relation = get_random_number_between(0.f, 1.f);
			data->users[idx1].identity.relations[idx2] = relation;
			data->users[idx2].identity.relations[idx1] = relation;
		}
	
	data->click_counter = 0;
	data->drop_counter = 0;

	for(u32 idx = 0; idx < 6; ++idx)
		data->social_post_system.social_feeds.push_back({idx, {}});

#if 0
	for(u32 idx = 0; idx < 3; ++idx)
	{
		sf::Color color( 0, 0, 0, 255);
		color.r = get_random_number_between(100,200);
		color.g = get_random_number_between(100,200);
		color.b = get_random_number_between(100,200);
		data->social_post_system.available_posts.push_back({create_id(), (u32)(-1), (u32)(-1), "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec id arcu at diam interdum fringilla. Quisque euismod in augue imperdiet aliquet. In ornare fermentum nisl, ut cursus orci commodo eget. In hac habitasse platea dictumst. Ut finibus venenatis tincidunt. Nulla commodo aliquam tellus vel gravida. Aliquam semper elementum lacus, vitae bibendum ante volutpat sagittis. Ut libero velit, vulputate eget suscipit et, hendrerit vehicula tellus. Aliquam erat volutpat. Ut mattis et odio in fringilla. Phasellus pretium aliquet eros, mollis tempor odio. Aliquam auctor ante in turpis lacinia lobortis. Quisque mauris nunc, pulvinar sed euismod et, lacinia sit amet sapien. Fusce tristique mi sed volutpat molestie. Aliquam congue sagittis tellus, vitae pretium tellus rhoncus posuere.", {}, {}, "DEFAULT", color});
	}
#endif
	
	return data;
}



inline
void game_data_shutdown(GameData *data)
{
	delete data;
}
