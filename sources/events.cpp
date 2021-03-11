
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

// NOTE(Sam): pattern is modified here !
void replace_with_environments(Pattern *pattern, Environement *environement)
{
	while(pattern != nullptr)
	{
		if(!pattern->symbole)
		{
			replace_with_environments(pattern->first, environement);
		}
		else if(pattern->variable)
		{
			for(auto& association : environement->associations)
			{
				if(association.variable_name == pattern->name)
				{
					if(association.datum->symbole)
					{
						assert(!association.datum->variable);
						pattern->id = association.datum->id;
						pattern->variable = false;
					}
					else
					{
						pattern->symbole = false;
						pattern->first = new Pattern(*(association.datum->first));
					}

					break;
				}
			}
		}
		
		pattern = pattern->next;
	}
}

// NOTE(Sam): pattern WILL be modified !
bool filter_recurse(Pattern *pattern, Pattern *datum, Environement *environement_to_set)
{
	
	if(pattern->symbole)
	{
		if(*pattern == *datum) return true;
		if(pattern->variable) {
			return merge_environement(environement_to_set, { pattern->name , datum });
		}
		return false;
	}

	if(datum->symbole) return false;

	// Si les deux sont vide ok, si seulement un sur deux pas ok
	if(pattern->first == nullptr && datum->first == nullptr)
		return true;
	if(pattern->first == nullptr || datum->first == nullptr)
		return false;

	if(!filter_recurse(pattern->first, datum->first, environement_to_set))
		return false;

	Pattern rest1 = {};
	rest1.symbole = false;
	rest1.first = pattern->first->next;
	rest1.next = nullptr;

	Pattern rest2 = {};
	rest2.symbole = false;
	rest2.first = datum->first->next;
	rest2.next = nullptr;

	// NOTE(Sam): Modifies the pattern, thus we need to pass a copy
	replace_with_environments(&rest1, environement_to_set);

	bool result = filter_recurse(&rest1, &rest2, environement_to_set);

	// NOTE(Sam): We need to remove temporal property of rest1 and rest2 in order
	// to not delete the entire list at the end of the function!
	rest1.first = nullptr;
	rest2.first = nullptr;

	return result;
}

bool filter(Pattern *condition, Fact const& datum, Environement *environement_to_set)
{
	Pattern modified_condition(*condition);
	replace_with_environments( &modified_condition, environement_to_set);
	return filter_recurse( &modified_condition, datum.pattern, environement_to_set);
}


std::vector<ConditionWithEnvironement> rule_has_condition(Fact const& fact, Rule *rule)
{
	std::vector<ConditionWithEnvironement> result;
	
	for(u32 idx = 0; idx < rule->conditions.size(); ++idx)
	{
		Environement environement = {};
		if(filter(&rule->conditions[idx], fact, &environement))
		{
			result.push_back({ idx, environement });
		}
	}
	
	return result;
}


std::vector<Environement> rule_is_valid(std::unordered_map<Fact, Fact> *facts, Rule *rule,
										u32 condition_id, Environement *environement)
{
	std::vector<Environement> result({*environement});
	
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
		if(cond_id == condition_id) continue;

		Pattern *condition = &rule->conditions[cond_id];
		std::vector<Environement> environements;

		for(auto const& fact : *facts)
		{
			for(u32 env_id = 0; env_id < result.size(); ++env_id)
			{
				Environement environement = result[env_id];
				if(filter(condition, fact.second, &environement))
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

// TODO(Sam): Est ce qu'on peut pas faire ça avec des rvalue ref ?
// NOTE(Sam): Returned Fact MUST take property of pattern !
Fact instanciate_conclusion(Rule *rule, Environement *environement, u32 fact_id)
{
	Fact result;
	result.id = fact_id;
	result.pattern = new Pattern(rule->conclusion);
    // NOTE(Sam): Ici le result est le proprio mais on ne veut pas qu'il y aie
	// destruction du pattern a la fin de la fonction donc on prétend
	// qu'il n'est pas proprio et on attend du caller qu'il prenne volontairement
	// la propriété !
	result.pattern_proprio = false;

	replace_with_environments(result.pattern, environement);

	return result;
}


void inference(Application *app)
{
	GameData *data = app->data;
	EventSystem *event_system = &data->event_system;

	u32 fact_id = event_system->fact_next_id;
	
	std::queue<Fact> working_queue{};
	std::unordered_map<Fact, Fact> deduced_facts{};
	
	for(auto it = event_system->facts.begin(); it != event_system->facts.end(); ++it)
	{
		working_queue.push(it->first);
	}
	
	while(!working_queue.empty())
	{
		Fact fact = working_queue.front();
		fact_swap_property(fact, working_queue.front());
		working_queue.pop();

		if(deduced_facts.count(fact) == 0)
		{
			deduced_facts.insert({fact,fact});
			fact_swap_property(deduced_facts[fact],fact);

			// TODO(Sam): On extrait si besoins...
			std::cout << convert_pattern_to_string(fact.pattern) << std::endl;

			for(u32 rule_id = 0; rule_id < event_system->rules.size(); ++rule_id)
			{
				Rule *rule = &event_system->rules[rule_id];
				std::vector<ConditionWithEnvironement> conds_envs = rule_has_condition(fact, rule);

				for(u32 condenv_id = 0; condenv_id < conds_envs.size(); ++condenv_id)
				{
					u32 condition_id = conds_envs[condenv_id].condition;
					Environement *environement = &(conds_envs[condenv_id].environement);

					std::vector<Environement> envs = rule_is_valid(&deduced_facts, rule, condition_id, environement);
					for(u32 env_id = 0; env_id < envs.size(); ++env_id)
					{
						// NOTE(Sam): Il doit y avoir transfert de propriété du pattern
						Fact conclusion = instanciate_conclusion(rule, &envs[env_id], fact_id++);
						working_queue.push(conclusion);
						working_queue.back().pattern_proprio = true;
					}
				}

			}

		}
			
	}
	
}

enum class SymboleType {
	NONE = 0, // Must not be used
	PERE,
	FRERE,
	ONCLE,
	Jacques,
	Charles,
	Francis,
	Pierre
};

std::string convert_pattern_to_string(Pattern *pattern)
{
	std::string result = "";

	if(!pattern)
		return result;
	
	if(pattern->symbole)
	{
		if(pattern->variable)
			result += std::string({'?',pattern->name});
		else
		switch(pattern->id)
		{
		case SymboleType::PERE:
			result += "PERE";
			break;
		case SymboleType::FRERE:
			result += "FRERE";
			break;
		case SymboleType::ONCLE:
			result += "ONCLE";
			break;
		case SymboleType::Jacques:
			result += "Jacques";
			break;
		case SymboleType::Charles:
			result += "Charles";
			break;
		case SymboleType::Francis:
			result += "Francis";
			break;
		case SymboleType::Pierre:
			result += "Pierre";
			break;
		default:
			result += "-";
			break;
		}
	}
	else
	{
		result += "(" + convert_pattern_to_string(pattern->first) + ")";
	}

	if(pattern->next)
		result += ", " + convert_pattern_to_string(pattern->next);
	
	return result;
}

void init_event_system(EventSystem *event_system)
{
	u32 fact_id = 0;
	/*
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
	*/

	Pattern p1 = {};
	{
		p1.symbole = false;
		p1.next = nullptr;

		Pattern *trans = &p1;
		Pattern *tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::PERE;
		p1.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Jacques;
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Charles;
		trans->next = tmp;
		trans = tmp;
		
	}
	Fact f1 = {};
	f1.id = fact_id++;
	f1.pattern = new Pattern(p1);
	
	Pattern p2 = {};
	{
		p2.symbole = false;
		p2.next = nullptr;

		Pattern *trans = &p2;
		Pattern *tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::FRERE;
		p2.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Charles;
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Francis;
		trans->next = tmp;
		trans = tmp;
		
	}
	Fact f2 = {};
	f2.id = fact_id++;
	f2.pattern = new Pattern(p2);

	Pattern p3 = {};
	{
		p3.symbole = false;
		p3.next = nullptr;

		Pattern *trans = &p3;
		Pattern *tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::FRERE;
		p3.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Jacques;
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->id = (u32)SymboleType::Pierre;
		trans->next = tmp;
		trans = tmp;
		
	}
	Fact f3 = {};
	f3.id = fact_id++;
	f3.pattern = new Pattern(p3);

	event_system->facts.insert({f1,f1});
	event_system->facts.insert({f2,f2});
	event_system->facts.insert({f3,f3});

	event_system->facts[f1].pattern_proprio = true;
	event_system->facts[f2].pattern_proprio = true;
	event_system->facts[f3].pattern_proprio = true;

	event_system->fact_next_id = fact_id;


	{
		Pattern pr11 = {};
		{
			pr11.symbole = false;
			pr11.next = nullptr;

			Pattern *trans = &pr11;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::PERE;
			pr11.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'y';
			trans->next = tmp;
			trans = tmp;
		
		}
		Pattern pr12 = {};
		{
			pr12.symbole = false;
			pr12.next = nullptr;

			Pattern *trans = &pr12;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::FRERE;
			pr12.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'y';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'z';
			trans->next = tmp;
			trans = tmp;
		
		}Pattern pr13 = {};
		{
			pr13.symbole = false;
			pr13.next = nullptr;

			Pattern *trans = &pr13;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::PERE;
			pr13.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'z';
			trans->next = tmp;
			trans = tmp;
		
		}

		event_system->rules.push_back({{pr11, pr12}, pr13});
	}
	
	{
		Pattern pr11 = {};
		{
			pr11.symbole = false;
			pr11.next = nullptr;

			Pattern *trans = &pr11;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::PERE;
			pr11.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'y';
			trans->next = tmp;
			trans = tmp;
		
		}
		Pattern pr12 = {};
		{
			pr12.symbole = false;
			pr12.next = nullptr;

			Pattern *trans = &pr12;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::FRERE;
			pr12.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'z';
			trans->next = tmp;
			trans = tmp;
		
		}Pattern pr13 = {};
		{
			pr13.symbole = false;
			pr13.next = nullptr;

			Pattern *trans = &pr13;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->id = (u32)SymboleType::ONCLE;
			pr13.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'z';
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'y';
			trans->next = tmp;
			trans = tmp;
		
		}

		event_system->rules.push_back({{pr11, pr12}, pr13});
	}

	std::cout << "Hello Event System !" << std::endl;
	
}
