
void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif

	GameData *data = app->data;

	// TODO(Sam): Update...

	for(u32 idx(0); idx < data->users_duration.size(); ++idx)
	{
		data->users_duration[idx] -= dt.asMilliseconds();
		if(data->users_duration[idx] <= 0)
		{
			data->users.erase(data->users.begin() + idx);
			data->users_duration.erase(data->users_duration.begin() + idx);
			--idx;
		}
	}

	data->next_user_duration -= dt.asMilliseconds();
	if(data->next_user_duration <= 0)
	{
		for(u32 idx(0); idx < get_random_number_between(1,10); ++idx)
		{
			data->next_user_duration = get_random_number_between(2000, 5000);
			data->users.push_back(get_random_number_between(0,100));
			data->users_duration.push_back(get_random_number_between(2000, 5000));
		}
	}

	
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
	
	GuiBeginContainer(global_gui_manager, obj1, GuiElementAlignment::HORIZONTAL);
	{
		GuiBeginContainer(global_gui_manager, obj3);
		{
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
		}
		GuiEndContainer(global_gui_manager);

		GuiBeginContainer(global_gui_manager, obj3);
		{
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
		}
		GuiEndContainer(global_gui_manager);

		GuiBeginContainer(global_gui_manager, obj2, GuiElementAlignment::HORIZONTAL);
		if(GuiButton(global_gui_manager, "Click Me"))
		{
			data->click_counter++;
		}
		GuiButton(global_gui_manager, "Click Me");
		GuiButton(global_gui_manager, "Click Me");
		GuiButton(global_gui_manager, "Click Me");
		GuiButton(global_gui_manager, "Click Me");
		GuiEndContainer(global_gui_manager);

		
		GuiBeginContainer(global_gui_manager, obj3);
		{
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
			GuiBeginContainer(global_gui_manager, obj2);
			GuiEndContainer(global_gui_manager);
		}
		GuiEndContainer(global_gui_manager);
	}
	GuiEndContainer(global_gui_manager);


#ifdef DEBUG
	GuiDebug(global_gui_manager);
#endif

	ImGui::Begin("Click");
	ImGui::Text("Clicked %u times!", data->click_counter);
	ImGui::End();
	
	ImGui::Begin("Users");
	if(ImGui::BeginTable("Users", 2))
	{
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("Duration");
		ImGui::TableHeadersRow();
		for(size_t idx(0); idx < data->users.size(); ++idx)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%i", data->users[idx]);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.2fs", (float)data->users_duration[idx]/1e3);
		}

		ImGui::EndTable();
	}
	ImGui::End();
	
#ifdef DEBUG
	ImGui::ShowDemoWindow();
	ImGui::Begin("Debug Infos");
	int debug_time = app->debug_clock.restart().asMilliseconds();
	ImGui::Text("Frame duration: %ims (%.2ffps)", debug_time, 1e3/(float)debug_time);
	ImGui::End();
#endif
}

inline
GameData* game_data_init()
{
	GameData* data = new GameData();
	data->users = std::vector<int>({42, 21, 13});
	data->users_duration = std::vector<int>({
			get_random_number_between(2000, 5000),
			get_random_number_between(2000, 5000),
			get_random_number_between(2000, 5000)});

	data->next_user_duration = get_random_number_between(2000, 5000);

	data->click_counter = 0;
	
	return data;
}

inline
void game_data_shutdown(GameData *data)
{
	delete data;
}
