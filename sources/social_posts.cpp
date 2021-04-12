
void instanciate_social_post_for_event(Application *app, Event *event)
{
	EventSystem *event_system = &app->data->event_system;
	SocialPostSystem *social_post_system = &app->data->social_post_system;

	std::vector<u32> post_compatibles;
	for(u32 idx = 0; idx < social_post_system->all_posts.size(); ++idx)
	{
		SocialPost *post = &social_post_system->all_posts[idx];
		if(post->event_id == event->id)
			post_compatibles.push_back(idx);

	}

	// TODO(Sam) Random selection of one or more compatible posts
	for(auto const& idx : post_compatibles)
	{
		SocialPost post = social_post_system->all_posts[idx];
		post.id = create_id();
		
		post.color = sf::Color({200,100,190});
		for(auto const& pair : event->users)
		{
			User *user = &app->data->users[pair.second];
			str_replace(post.text,
						std::string("?")+pair.first,
						user->fullname);
			// TODO(Sam): tmp
			str_replace(post.type,
						std::string("?")+pair.first,
						user->fullname);
		}

		post.major_user_ids.reserve(event->major_variables.size());
		for(auto const& major_variable : event->major_variables)
		{
			post.major_user_ids.push_back(event->users[major_variable]);
		}
		
		social_post_system->available_posts.push_back(post);
	}
}
