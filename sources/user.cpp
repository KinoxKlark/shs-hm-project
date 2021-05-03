#include <fstream>

std::vector<std::string> importNames(char *filename)
{
	std::ifstream file;
	std::vector<std::string> names;
	
	file.open(filename, std::ios::in);
	assert(("Identities file can't be open!", file.is_open()));

	std::string line;
	while(std::getline(file, line))
	{
		names.push_back(line);
	}

	file.close();

	return names;
}

std::vector<std::string> importFirstNames(bool men)
{
	if(men) return importNames("data/firstnames_men.txt");
	else return importNames("data/firstnames_women.txt");
}

std::vector<std::string> importLastNames()
{
	return importNames("data/lastnames.txt");
}

std::vector<GaugeInfo> importGauges(char *filename)
{
	std::ifstream file;
	std::vector<GaugeInfo> gauges;
	
	file.open(filename, std::ios::in);
	assert(("Identities file can't be open!", file.is_open()));

	std::string line;
	while(std::getline(file, line))
	{
		gauges.push_back({ (u32)gauges.size(), line });
	}

	file.close();

	return gauges;
}

UserIdentity createUserIdentity(GameData *data)
{
	std::vector<GaugeInfo> *personalities = &data->personalities;
	std::vector<GaugeInfo> *interests = &data->interests;
	
	UserIdentity identity = {};
	identity.personalities.resize(personalities->size());
	for(u32 idx = 0; idx < personalities->size(); ++idx)
	{
		identity.personalities[idx].id = (*personalities)[idx].id;
		identity.personalities[idx].amount = get_random_number_between(0.f, 1.f);
	}

	u8 interest_count = get_random_number_between(3, 5);
	for(u8 idx = 0; idx < interest_count; ++idx)
	{
		u32 interest_id = get_random_number_between(0, (u32)interests->size()-1);
		GaugeInfo *interest = &((*interests)[interest_id]);

		for(u8 idx_interests = 0; idx_interests < identity.interests.size(); ++idx_interests)
		{
			if(identity.interests[idx_interests].id == interest->id)
			{
				identity.interests[idx_interests].id = interest->id;
				identity.interests[idx_interests].amount =
					get_random_number_between(identity.interests[idx_interests].amount, 1.f);

				if(identity.interests[idx_interests].amount > 1.f)
					identity.interests[idx_interests].amount = 1.f;
				goto next_interest;
			}
		}

		identity.interests.push_back({interest->id, get_random_number_between(.3f, 1.f)});

	next_interest:
		interest_id; // Dummy line
	}

	// NOTE(Sam): Relations are selected after for every pnj at once
	identity.relations = std::vector<r32>(6, 0.f);

	return identity;
	
}

r32 user_modif_nudge_up(r32 value)
{
	return std::sqrt(value);
}

r32 user_modif_nudge_down(r32 value)
{
	return 1.f - std::sqrt(1.f - value);
}

void user_react_to_modifs(User *user, std::vector<Modif> *modifs, bool modify_score)
{
	i8 nudge_alignment = 0;
	
	for(auto const& modif : *modifs)
	{
		r32 *value;
		switch(modif.type)
		{
		case ModifType::PERSONALITY:
		{
			UserGauge* gauge = get_personality_gauge(user, modif.gauge_id);
			if(!gauge) continue;
			value = &gauge->amount;
		} break;
		case ModifType::INTEREST:
		{
			UserGauge* gauge = get_interest_gauge(user, modif.gauge_id);
			if(!gauge) continue;
			value = &gauge->amount;
		} break;
		case ModifType::RELATION:
		{
			value = get_relation_value(user, modif.gauge_id);
			if(!value) continue;
		}break;
		InvalidDefaultCase;
		}

		u8 nudge_amount = std::abs(modif.nudge_amount);
		bool up = modif.nudge_amount > 0;
		for(u8 i = 0; i < nudge_amount; ++i)
		{
			if(up)
			{
				nudge_alignment += *value > .5f ? +1 : -1;
				*value = user_modif_nudge_up(*value);
			}
			else
			{
				nudge_alignment += *value < .5f ? +1 : -1;
				*value = user_modif_nudge_down(*value);
			}
		}

		if(modif.type == ModifType::RELATION)
		{
			r32 *sym_val = get_relation_value(&global_app->data->users[modif.gauge_id], user->id);
			*sym_val = *value;
		}
	}
	
	if(modify_score)
	{
		user_score_up(user, 0.005f);
		if(nudge_alignment > 0)
		{
			user_score_up(user, 0.05f*nudge_alignment);
		}
		else
		{
			user_score_down(user, 0.05f*std::abs(nudge_alignment));
		}
	}
}

void user_react_to_modifs(std::vector<Modifs> *users_modifs, bool modify_score)
{
	for(u32 i = 0; i < users_modifs->size(); ++i)
	{
		Modifs *modifs = &((*users_modifs)[i]);
		user_react_to_modifs(&global_app->data->users[modifs->user_id], &modifs->modifs, modify_score);	
	}
}
	

void user_see_post(EventSystem *event_system, SocialFeed *feed, SocialPost *post)
{
	// (SOCIAL_POST_SEEN, post id, ?viewer, ?a, ?b, ?c, ... )
		
	Pattern p0 = {};
	p0.symbole = false;
	p0.next = nullptr;
	{
		Pattern *trans = &p0;
		Pattern *tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::SOCIAL_POST_SEEN;
		tmp->data = 0;
		p0.first = tmp;
		trans = tmp;

		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::SOCIAL_POST;
		tmp->data = (u64)(post->social_post_id);
		trans->next = tmp;
		trans = tmp;

		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(feed->user_id);
		trans->next = tmp;
		trans = tmp;
		
		for(auto const& user_id : post->major_user_ids)
		{
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::USER;
			tmp->data = (u64)(user_id);
			trans->next = tmp;
			trans = tmp;
		}
	}

	Fact f0 = {};
	f0.id = event_system->fact_next_id++;
	f0.pattern = new Pattern(p0);

	event_system->facts.insert({f0.id,f0});
	event_system->facts[f0.id].pattern_proprio = true;

	std::vector<u32> modifs_ids;
	
	// We set correctly the viewer
	for(u32 idx = 0; idx < post->users_modifs.size(); ++idx)
	{
		auto& modifs = post->users_modifs[idx];
		if(modifs.user_id == feed->user_id)
			modifs_ids.push_back(idx);
		if(modifs.user_id == -1)
			modifs.user_id = feed->user_id;
		for(auto& modif : modifs.modifs)
		{
			if(modif.type == ModifType::RELATION && modif.gauge_id == -1)
				modif.gauge_id = feed->user_id;
		}
	}

	// TODO(Sam): If post type is a publication on another wall, multiple viewers should be considered ?
	if(modifs_ids.size() > 0)
	{
		for(auto const& idx : modifs_ids)
		{
			auto& modifs = post->users_modifs[idx];
			user_react_to_modifs(&global_app->data->users[modifs.user_id],&modifs.modifs, true);
		}
	}
	else
	for(auto& modifs : post->users_modifs)
	{
		if(modifs.user_id == feed->user_id)
			user_react_to_modifs(&global_app->data->users[feed->user_id],&modifs.modifs, true);
		
	}
}

void user_score_up(User *user, r32 dx)
{
	if(user->interaction_score > 0.f)
	{
		user->interaction_score += dx;
		if(user->interaction_score > 1.f)
			user->interaction_score = 1.f;
	}
}

void user_score_down(User *user, r32 dx)
{
	user->interaction_score -= dx;
	if(user->interaction_score <= 0.f)
		user->interaction_score = 0.f;
}
