
bool merge_environement(Environement *environement, EnvironementAssociation association)
{
	for(size_t idx = 0; idx < environement->associations.size(); ++idx)
	{
		if(environement->associations[idx].variable_name == association.variable_name)
			return *(environement->associations[idx].datum) == *(association.datum);
	}

	environement->associations.push_back(association);
	return true;
}

void replace_with_environments(Pattern *pattern, Environement *environement)
{
	while(pattern != nullptr)
	{
		if(pattern->symbole && pattern->variable)
		{
			for(auto& association : environement->associations)
			{
				if(association.variable_name == pattern->name)
				{
					if(association.datum->symbole)
					{
						assert(!association.datum->variable);
						pattern->id = association.datum->id;
					}
					else
					{
						// TODO(Sam): Est-ce qu'on a besoins d'une deep copie ?
						pattern->symbole = false;
						pattern->first = association.datum->fist;
					}

					break;
				}
			}
		}
		pattern = pattern->next;
	}
}

bool filter(Pattern *pattern, Pattern *datum, Environement *environement_to_set)
{
	
	if(pattern->symbole)
	{
		if(*pattern == *datum) return true;
		if(pattern->variable) {
			return merge_environement(environement_to_set, { pattern->name : datum });
		}
		return false;
	}

	if(datum->symbole) return false;

	// Si les deux sont vide ok, si seulement un sur deux pas ok
	if(pattern->first == nullptr && datum->first == nullptr)
		return true;
	if(pattern->first == nullptr || datum->first == nullptr)
		return false;

	
	if(!filter(pattern->first, datum->first, environement_to_set))
		return false;

	Pattern rest1;
	rest1.symbole = false;
	rest1.first = pattern->first->next;
	rest1.next = nullptr;

	Pattern rest2;
	rest2.symbole = false;
	rest2.first = datum->first->next;
	rest2.next = nullptr;
	
	replace_with_environments(rest1, environement_to_set);

	return filter(&rest1, &rest2, environement_to_set);
}

bool filter(Condition *condition, Fact fact, Environement *environement_to_set)
{
	// TODO(Sam): mmmh délicieux ...
	// NOTE(Sam): environement_to_set peut déjà contenir des trucs et faut que ça reste compatible !!
	return false;
}

std::vector<ConditionWithEnvironement> rule_has_condition(Fact fact, Rule *rule)
{
	std::vector<ConditionWithEnvironement> result;
	
	for(u32 idx = 0; idx < rule->conditions.size(); ++idx)
	{
		if(rule->conditions[idx] == fact)
		{
			Environement environement;
			if(filter(&rule->condition[idx], fact, &environement))
			{
				result.push_back({ idx, environement });
			}

		}
	}
	
	return result;
}


std::vector<Environement> rule_is_valid(std::unordered_set<Fact> *facts, Rule *rule,
										u32 condition_id, Envrionement *environement)
{
	std::vector<Environement> result({*environement});
	
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
		if(cond_id == condition_id) continue;

		Condition *condition = &rule->conditions[cond_id];
		std::vector<Environement> environements;

		for(auto& fact : facts)
		{
			for(u32 env_id = 0; env_id < result.size(); ++env_id)
			{
				Environement environement = result[env_id];
				if(fitler(conditions, fact, &environement))
				{
					environements.push_back(environement);
				}
			}
		}

		// NOTE(Sam): If environements is empty, it means that this
		// condition is not satisfied, thus no environements can solve the rule
		if(environements.size() == 0)
		{
			return std::vector<Environement>();
		}

		result.swap(environements);
	}
	
	return result;
}

Fact instanciate_conclusion(Rule *rule, Environement *environement)
{
	Fact result;

	// TODO(Sam): Va falloir organiser qui a la propriété de quoi dans cette histoire
	Pattern *pattern = DEEP_COPY_OF_PATTERNS(rule->conclusion);
	replace_with_environments(pattern, environement);

	// TODO(Sam): Comment on passe de Pattern à Fact ?
	
	return result;
}


void inference(Application *app)
{
	GameData *data = app->data;
	EventSystem *event_system = &data->event_system;
	
	std::queue<Fact> working_queue{};
	std::unordered_set<Fact> deduced_facts{};
	
	for(auto it = event_system->facts.begin(); it != event_system->facts.end(); ++it)
	{
		working_queue.push(*it);
	}
	
	while(!working_queue.empty())
	{
		Fact fact = working_queue.front();
		working_queue.pop();

		if(deduced_facts.count(fact) == 0)
		{
			deduced_facts.insert(fact);

			// TODO(Sam): On extrait si besoins...
			std::cout << fact.description << std::endl;

			for(u32 rule_id = 0; rule_id < event_system->rules.size(); ++ rule_id)
			{
				Rule *rule = &event_system->rules[rule_id];
				std::vector<ConditionWithEnvironement> conds_envs = rule_has_condition(fact, rule);
				
				for(u32 condenv_id = 0; condenv_id < conds_envs.size(); ++condenv_id)
				{
					u32 *condition = conds_envs[condenv_id].condition;
					Environement *environement = &(conds_envs[condenv_id].environement);

					std::vector<Environement> envs = rule_is_valid(&deduced_facts, rule, condition, environement);
					for(u32 envs_id = 0; envs_id < envs.size(); ++envs_id)
					{
						Fact conclusion = instanciate_conclusion(rule, &envs[env_id]);
						working_queue.push(conclusion);
					}
				}

			}

		}
			
	}
	
}

void init_event_system(EventSystem *event_system)
{
	u32 fact_id = 0;
	Fact fact1 = {fact_id++, "vin"}; 
	Fact fact2 = {fact_id++, "< 2 litres"}; 
	Fact fact3 = {fact_id++, "< 100 chf"}; 
	Fact fact4 = {fact_id++, "adulte"}; 
	Fact fact5 = {fact_id++, "hors-taxe"};
	Fact fact6 = {fact_id++, "cognac"};
	Fact fact7 = {fact_id++, "< 1 litre"};	
	Fact fact8 = {fact_id++, "petite quantite"};
	
	event_system->facts.insert(fact6);
	event_system->facts.insert(fact2);
	event_system->facts.insert(fact7);
	//event_system->facts.insert(fact3);
	event_system->facts.insert(fact4);
	
	event_system->rules.push_back({{fact1, fact2}, fact8});
	event_system->rules.push_back({{fact6, fact7}, fact8});
	event_system->rules.push_back({{fact3}, fact8});
	event_system->rules.push_back({{fact8, fact4}, fact5});

	std::cout << "Hello Event System !" << std::endl;
	
}
