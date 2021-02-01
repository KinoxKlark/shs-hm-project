
bool drag_drop_accept_payload(void* user_data)
{
	++global_app->data->drop_counter;
	return true;
}

void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif

	gui_update(dt);

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

	GuiObject obj4 = obj1;
	obj4.padding = {1,1,1,1};

	GuiObject obj5;
	obj5.size = {1.f/3.f, .25};
	obj5.margin = {1,1,1,1};
	obj5.padding = {};
	obj5.bg_color = sf::Color(255,150,150);
	
	GuiBeginGrid(2, 3, obj4);
	{
		GuiSelectGridCell(0,0);
		GuiDroppableArea(drag_drop_accept_payload);
		GuiBeginContainer(obj2);
		GuiEndContainer();
		GuiBeginContainer(obj2);
		GuiEndContainer();

		GuiSelectGridCell(0,1);
		GuiDroppableArea(drag_drop_accept_payload);
		GuiBeginContainer(obj3);
		{
			GuiBeginContainer(obj2);
			GuiEndContainer();
			GuiBeginContainer(obj2);
			GuiEndContainer();
		}
		GuiEndContainer();

		GuiSelectGridCell(0,2);
		GuiDroppableArea(drag_drop_accept_payload);

		GuiSelectGridCell(1,0);
		GuiDroppableArea(drag_drop_accept_payload);

		GuiSelectGridCell(1,1);
		GuiDroppableArea(drag_drop_accept_payload);

		GuiSelectGridCell(1,2);
		GuiDroppableArea(drag_drop_accept_payload);
		GuiBeginContainer(obj3);
		{
			GuiBeginContainer(obj2);
			GuiEndContainer();
			GuiBeginContainer(obj2);
			GuiEndContainer();
		}
		GuiEndContainer();
				
	}
	GuiEndGrid();

	GuiBeginTabs(obj1);
	{
		if(GuiTab("Tab1"))
		{
			GuiBeginDraggableContainer(obj5);
			GuiButton("Test");
			GuiEndDraggableContainer();
	
			GuiBeginDraggableContainer(obj5);
			GuiEndDraggableContainer();
			
			GuiBeginDraggableContainer(obj5);
			{
				GuiBeginContainer(obj3);
				GuiEndContainer();
			}
			GuiEndDraggableContainer();
			
			GuiBeginDraggableContainer(obj5);
			GuiEndDraggableContainer();
			
			GuiBeginDraggableContainer(obj5);
			GuiEndDraggableContainer();
			
			GuiBeginDraggableContainer(obj5);
			GuiEndDraggableContainer();
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

			GuiBeginContainer(obj3);
			{
				GuiBeginContainer(obj2);
				GuiEndContainer();
				GuiBeginContainer(obj2);
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
	//ImGui::ShowDemoWindow();
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
	data->drop_counter = 0;
	
	return data;
}

inline
void game_data_shutdown(GameData *data)
{
	delete data;
}
