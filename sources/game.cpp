

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
		for(u32 idx(0); idx < random_between(1,10); ++idx)
		{
			data->next_user_duration = random_between(2000, 5000);
			data->users.push_back(random_between(0,100));
			data->users_duration.push_back(random_between(2000, 5000));
		}
	}

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
			random_between(2000, 5000),
			random_between(2000, 5000),
			random_between(2000, 5000)});

	data->next_user_duration = random_between(2000, 5000);
	
	return data;
}

inline
void game_data_shutdown(GameData *data)
{
	delete data;
}
