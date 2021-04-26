
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

		switch(post.type)
		{
		case PostType::PUBLICATION:
			post.color = sf::Color({153,204,255});
			break;
		case PostType::PARTAGE:
			post.color = sf::Color({102,102,255});
			break;
		case PostType::PHOTO:
			post.color = sf::Color({128,255,212});
			break;
		case PostType::ARTICLE:
			post.color = sf::Color({255,204,153});
			break;
		case PostType::PUB:
			post.color = sf::Color({255,179,242});
			break;
		case PostType::LOCALISATION:
			post.color = sf::Color({179,255,179});
			break;
			InvalidDefaultCase;
		};
		
		for(auto const& pair : event->users)
		{
			User *user = &app->data->users[pair.second];
			str_replace(post.text,
						std::string("?")+pair.first,
						user->fullname);
			str_replace(post.article_origin,
						std::string("?")+pair.first,
						user->fullname);
			str_replace(post.article_title,
						std::string("?")+pair.first,
						user->fullname);
			str_replace(post.localisation,
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

		if(post.author_id != -1)
			post.author_id = event->users[(char)post.author_id];

		if(post.receiver_id != -1)
			post.receiver_id = event->users[(char)post.receiver_id];

		
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
