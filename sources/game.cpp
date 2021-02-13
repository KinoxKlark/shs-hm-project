
struct post_payload {
	u32 id;
};

bool drag_drop_accept_payload(void *payload, void* user_data)
{
	GameData *data = global_app->data;
	SocialPost *post = (SocialPost*)payload;
	SocialFeed *feed = (SocialFeed*)user_data;

	feed->posts.push_back(*post);

	for(auto it = data->available_posts.begin(); it != data->available_posts.end(); ++it)
	{
		if(it->id == post->id)
		{
			data->available_posts.erase(it);
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
	GuiTitle("Test");
	GuiText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec id arcu at diam interdum fringilla. Quisque euismod in augue imperdiet aliquet. In ornare fermentum nisl, ut cursus orci commodo eget. In hac habitasse platea dictumst. Ut finibus venenatis tincidunt. Nulla commodo aliquam tellus vel gravida. Aliquam semper elementum lacus, vitae bibendum ante volutpat sagittis. Ut libero velit, vulputate eget suscipit et, hendrerit vehicula tellus. Aliquam erat volutpat. Ut mattis et odio in fringilla. Phasellus pretium aliquet eros, mollis tempor odio. Aliquam auctor ante in turpis lacinia lobortis. Quisque mauris nunc, pulvinar sed euismod et, lacinia sit amet sapien. Fusce tristique mi sed volutpat molestie. Aliquam congue sagittis tellus, vitae pretium tellus rhoncus posuere.");
	GuiEndContainer();
}

void update(Application *app, sf::Time dt)
{
#ifdef DEBUG
	ImGui::SFML::Update(*app->window, dt);
#endif

	gui_update(dt);

	GameData *data = app->data;

	// TODO(Sam): Update...

	
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
		for(u32 idx = 0; idx < data->social_feeds.size(); ++idx)
		{
			GuiSelectGridCell(idx/3, idx % 3);
			GuiBeginContainer(obj_full);
			GuiDroppableArea(drag_drop_accept_payload, &data->social_feeds[idx]);

			for(u32 post_idx = 0; post_idx < data->social_feeds[idx].posts.size(); ++post_idx)
			{
				social_post_gui(&(data->social_feeds[idx].posts[post_idx]));
			}
			GuiEndContainer();
		}
				
	}
	GuiEndGrid();

	GuiBeginTabs(obj1);
	{
		if(GuiTab("Tab1"))
		{
			for(u32 idx = 0; idx < data->available_posts.size(); ++idx)
			{
				social_post_gui(&(data->available_posts[idx]), true);
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

		ImGui::Text("User %u", user->id);

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

	data->personalities = importGauges("data/identities.txt");
	data->interests = importGauges("data/interests.txt");

	data->users.resize(6);
	for(u32 idx = 0; idx < data->users.size(); ++idx)
	{
		data->users[idx].id = idx;
		data->users[idx].identity = createUserIdentity(data);
	}
	
	data->click_counter = 0;
	data->drop_counter = 0;

	for(u32 idx = 0; idx < 6; ++idx)
		data->social_feeds.push_back({});

	for(u32 idx = 0; idx < 12; ++idx)
	{
		sf::Color color( 0, 0, 0, 255);
		color.r = get_random_number_between(100,200);
		color.g = get_random_number_between(100,200);
		color.b = get_random_number_between(100,200);
		data->available_posts.push_back({create_id(), color});
	}
	
	return data;
}

inline
void game_data_shutdown(GameData *data)
{
	delete data;
}
