inline
GaugeInfo* get_personality_gauge_info(std::string const& name)
{
	for(u32 i = 0; i < global_app->data->personalities.size(); ++i)
	{
		GaugeInfo &gauge = global_app->data->personalities[i];
		if(gauge.name == name)
		{
			return &gauge;
		}
	}
	return nullptr;
}

inline
GaugeInfo* get_interest_gauge_info(std::string const& name)
{
	for(u32 i = 0; i < global_app->data->interests.size(); ++i)
	{
		GaugeInfo &gauge = global_app->data->interests[i];
		if(gauge.name == name)
		{
			return &gauge;
		}
	}
	return nullptr;
}


inline
UserGauge* get_personality_gauge(User *user, u32 gauge_id)
{
	for(auto& gauge : user->identity.personalities)
	{
		if(gauge.id == gauge_id)
			return &gauge;
	}

	return nullptr;
}

inline
UserGauge* get_interest_gauge(User *user, u32 gauge_id)
{
	for(auto& gauge : user->identity.interests)
	{
		if(gauge.id == gauge_id)
			return &gauge;
	}

	return nullptr;
}

inline
r32 *get_relation_value(User *user, u32 other_user_id)
{
	return &user->identity.relations[other_user_id];
}
