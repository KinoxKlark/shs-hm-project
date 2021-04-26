
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

	for(auto const& idx : post_compatibles)
	{
		// TODO(Sam): For now we have in average 2 post per event, maybe we want more?
		// problem is if there is only one post. How can we setup the probability correctly?
		r32 r = get_random_number_between(0.f, 1.f);
		if(r > 2.f/(r32)post_compatibles.size())
			continue;
		
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

			//TODO(Sam): If publication or partage -> score up
			//TODO(Sam): If partage receiver -> rect to post
			//TODO(Sam): If publication or partage and score == 0 -> skip
		}

		post.major_user_ids.reserve(event->major_variables.size());
		for(auto const& major_variable : event->major_variables)
		{
			post.major_user_ids.push_back(event->users[major_variable]);
		}

		for(u32 idx = 0; idx < post.users_modifs.size(); ++idx)
		{
			if(post.users_modifs[idx].user_id != -1)
				post.users_modifs[idx].user_id = event->users[(char)post.users_modifs[idx].user_id];

			for(u32 idx_modif = 0; idx_modif < post.users_modifs[idx].modifs.size(); ++idx_modif)
			{
				Modif *modif = &post.users_modifs[idx].modifs[idx_modif];
				if(modif->type == ModifType::RELATION)
				{
					if(modif->gauge_id == -1) modif->gauge_id = event->users[(char)post.users_modifs[idx].user_id];
					else
						modif->gauge_id = event->users[modif->gauge_id];
				}
			}
		}
		
		social_post_system->available_posts.push_back(post);
	}
}
