
bool merge_environement(Environement *environement, EnvironementAssociation association)
{
	for(size_t idx = 0; idx < environement->associations.size(); ++idx)
	{
		if(environement->associations[idx].variable_name == association.variable_name)
			return *(environement->associations[idx].datum) == *(association.datum);
		if(environement->associations[idx].datum->symbole
		   && environement->associations[idx].datum->type == SymboleType::USER
		   && environement->associations[idx].datum->data == association.datum->data)
			return false;
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
						pattern->type = association.datum->type;
						pattern->data = association.datum->data;
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


// NOTE(Sam): Given pattern MUST be compilable to User*, no checking, only assert in debug mode!
User* compile_pattern_to_user(Pattern *pattern)
{
	if(pattern->symbole)
	{
		assert(!pattern->variable);
		assert(pattern->type == SymboleType::USER);
		return &global_app->data->users[pattern->data];
	}

	Pattern *first = pattern->first;

	switch(first->type)
	{
		// NOTE(Sam): Nothing for now
	default:
		assert(false);
	}

	return nullptr;
}

// NOTE(Sam): Given pattern MUST be compilable to number, no checking, only assert in debug mode!
r32 compile_pattern_to_number(Pattern *pattern)
{
	if(pattern->symbole)
	{
		assert(!pattern->variable);
		assert(pattern->type == SymboleType::NUMBER);
		return *((r32*)(&pattern->data));
	}

	Pattern *first = pattern->first;

	switch(first->type)
	{
	case SymboleType::PERSONALITY_GAUGE:
	{
		Pattern *second = first->next;

		User * user = compile_pattern_to_user(second);
		UserGauge* gauge =  get_personality_gauge(user, (u32)first->data);

		assert(gauge);
		return gauge->amount;
	}
	case SymboleType::INTEREST_GAUGE:
	{
		Pattern *second = first->next;

		User * user = compile_pattern_to_user(second);
		UserGauge* gauge =  get_interest_gauge(user, (u32)first->data);

		return gauge ? gauge->amount : 0.f;	
	} break;
	default:
		assert(false);
	}
	return 0.f;
}

// NOTE(Sam): Given pattern MUST be compilable to boolean, no checking, only assert in debug mode!
bool compile_pattern_to_boolean(Pattern *pattern)
{
	assert(!pattern->symbole);
	
	Pattern *first = pattern->first;

	bool result = false;

	switch(first->type)
	{
	case SymboleType::CMP_GREATER:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value > rhs_value;
	} break;
	default:
		result =  false;
		break;
	}

	return result;
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

std::unordered_set<char> get_variables_in_pattern(Pattern *pattern)
{
	std::unordered_set<char> result;

	while(pattern != nullptr)
	{
		if(!pattern->symbole)
		{
			std::unordered_set<char> subresult = get_variables_in_pattern(pattern->first);
			for(auto& variable : subresult)
			{
				result.insert(variable);
			}
			
		}
		else if(pattern->variable)
		{
			result.insert(pattern->name);
		}
		
		pattern = pattern->next;
	}
	
	return result;
}

std::vector<ConditionWithEnvironement> rule_has_condition(Fact const& fact, Rule *rule)
{
	std::vector<ConditionWithEnvironement> result;
	
	for(u32 idx = 0; idx < rule->conditions.size(); ++idx)
	{
		if(fact.pattern->symbole
		   && !fact.pattern->variable
		   && fact.pattern->type == SymboleType::USER)
		{
			std::unordered_set<char> variables = get_variables_in_pattern(&rule->conditions[idx]);
			for(auto& variable : variables)
			{
				Environement environement = {};
				environement.conditions_to_compile.insert(idx);
				environement.associations.push_back({ variable, fact.pattern });
				result.push_back({ idx, environement });
			}
		}
		else
		{
			Environement environement = {};
			if(filter(&rule->conditions[idx], fact, &environement))
			{
				result.push_back({ idx, environement });
			}
		}
	}
	
	return result;
}

std::vector<Environement> rule_may_be_valid(std::unordered_map<Fact, Fact> *facts, Rule *rule,
											u32 condition_id, Environement *environement)
{
	std::vector<Environement> result({*environement});
	
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
		if(cond_id == condition_id && environement->conditions_to_compile.count(cond_id) == 0)
			continue;

		Pattern *condition = &rule->conditions[cond_id];
		std::vector<Environement> environements;

		for(auto const& fact : *facts)
		{
			for(u32 env_id = 0; env_id < result.size(); ++env_id)
			{
				Environement environement = result[env_id];

				if(fact.second.pattern->symbole
				   && !fact.second.pattern->variable
				   && fact.second.pattern->type == SymboleType::USER)
				{
					Pattern modified_condition(*condition);
					replace_with_environments( &modified_condition, &environement);

					std::unordered_set<char> variables = get_variables_in_pattern(condition);
					for(auto& variable : variables)
					{
						Environement environement_fusion = environement;
						if(merge_environement(&environement_fusion, { variable , fact.second.pattern }))
						{
							environement_fusion.conditions_to_compile.insert(cond_id);
							environements.push_back(environement_fusion);
						}
					}
				}
				else
				{
					if(filter(condition, fact.second, &environement))
					{
						environements.push_back(environement);
					}
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

// NOTE(Sam): Rule is supposed to be potentialy valide given knowed facts
// this function only check if each conditions compiles to TRUE for a given
// environement.
bool rule_is_valid(std::unordered_map<Fact, Fact> *facts, Rule *rule, Environement *environement)
{
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
		Pattern *condition = &rule->conditions[cond_id];
		bool condition_should_compile = environement->conditions_to_compile.count(cond_id) > 0;
		if(condition_should_compile)
		{
			Pattern modified_condition(*condition);
			replace_with_environments(&modified_condition, environement);
			u32 nb_variables = get_variables_in_pattern(&modified_condition).size();
			if(nb_variables > 0) return false;
			bool value = compile_pattern_to_boolean(&modified_condition);
			if(!value) return false;
		}
	}

	return true;
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

void apply_conclusion(Rule *rule, Environement *environement, std::queue<Fact>* queue, u32 *fact_id)
{
	if(rule->conclusion.symbole
	   && !rule->conclusion.variable
	   && rule->conclusion.type == SymboleType::_SELECT_EVENT)
	{
		// TODO(Sam): Gestion des events
		std::cout << "New Event: [" << rule->conclusion.data << "] ";

		if(environement->associations.size() > 0)
		{
			std::cout << "with: ";

			for(u32 idx = 0; idx < environement->associations.size(); ++idx)
			{
				if(environement->associations[idx].datum->type == SymboleType::USER)
				{
					std::cout << global_app->data->users[environement->associations[idx].datum->data].fullname;
					if( idx+1 < environement->associations.size())
						std::cout << ", ";
				}
			}
		}

		std::cout << std::endl;
	}
	else
	{
		// NOTE(Sam): Il doit y avoir transfert de propriété du pattern
		Fact conclusion = instanciate_conclusion(rule, environement, (*fact_id)++);
		queue->push(conclusion);
		queue->back().pattern_proprio = true;
	}
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

					std::vector<Environement> envs = rule_may_be_valid(&deduced_facts, rule, condition_id, environement);
					for(u32 env_id = 0; env_id < envs.size(); ++env_id)
					{
						if(!rule_is_valid(&deduced_facts, rule, &envs[env_id])) continue;

						apply_conclusion(rule, &envs[env_id], &working_queue, &fact_id);
						
					}
				}

			}

		}
			
	}
	
}

#include <sstream>

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
		switch(pattern->type)
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
		case SymboleType::NUMBER:
		{
			std::stringstream ss;
			ss << *((r32*)(&pattern->data));
			result += ss.str();
		} break;
		case SymboleType::CMP_GREATER:
			result += ">";
			break;
		case SymboleType::USER:
			result += global_app->data->users[pattern->data].fullname;
			break;
		case SymboleType::PERSONALITY_GAUGE:
			result += "PERSONALITY " + global_app->data->personalities[pattern->data].name;
			if(pattern->next && pattern->next->symbole && pattern->next->type == SymboleType::USER)
			{
				std::stringstream ss;
				User *user = &global_app->data->users[pattern->next->data];
				UserGauge *gauge = get_personality_gauge(user, pattern->data);
				ss << " [" << gauge->amount << "]";
				result += ss.str();
			}
			break;
		case SymboleType::INTEREST_GAUGE:
			result += "INTEREST " + global_app->data->personalities[pattern->data].name;
			if(pattern->next && pattern->next->symbole && pattern->next->type == SymboleType::USER)
			{
				std::stringstream ss;
				User *user = &global_app->data->users[pattern->next->data];
				UserGauge *gauge = get_interest_gauge(user, pattern->data);
				if(gauge)
					ss << " [" << gauge->amount << "]";
				else
					ss << " [-]";
				result += ss.str();
			}
			break;
		case SymboleType::_SELECT_EVENT:
		{
			std::stringstream ss;
			ss << "SELECT EVENT " << pattern->data;
			result += ss.str();
		} break;
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
	GameData *data = global_app->data;
	
	u32 fact_id = 0;

	for(u32 i = 0; i < data->users.size(); ++i)
	{
		Pattern p0 = {};
		p0.symbole = true;
		p0.variable = false;
		p0.type = SymboleType::USER;
		p0.data = (u64)(data->users[i].id);
		Fact f0 = {};
		f0.id = fact_id++;
		f0.pattern = new Pattern(p0);

		event_system->facts.insert({f0,f0});
		event_system->facts[f0].pattern_proprio = true;
	
	}
	
	Pattern p1 = {};
	{
		p1.symbole = false;
		p1.next = nullptr;

		Pattern *trans = &p1;
		Pattern *tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::PERE;
		p1.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[0].id);
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[1].id);
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
		tmp->type = SymboleType::FRERE;
		p2.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[1].id);
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[3].id);
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
		tmp->type = SymboleType::FRERE;
		p3.first = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[0].id);
		trans->next = tmp;
		trans = tmp;
		
		tmp = new Pattern();
		tmp->symbole = true;
		tmp->variable = false;
		tmp->type = SymboleType::USER;
		tmp->data = (u64)(data->users[2].id);
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
			tmp->type = SymboleType::PERE;
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
			tmp->type = SymboleType::FRERE;
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
			tmp->type = SymboleType::PERE;
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
			tmp->type = SymboleType::PERE;
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
			tmp->type = SymboleType::FRERE;
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
			tmp->type = SymboleType::ONCLE;
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
	
	{
		Pattern pr11 = {};
		{
			pr11.symbole = false;
			pr11.next = nullptr;

			Pattern *trans = &pr11;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::CMP_GREATER;
			pr11.first = tmp;
			trans = tmp;

			tmp = new Pattern();
			tmp->symbole = false;
			trans->next = tmp;
			trans = tmp;
			{
				Pattern *trans2 = tmp;
				Pattern* tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = false;
				tmp2->type = SymboleType::PERSONALITY_GAUGE;
				tmp2->data = 0;
				trans2->first = tmp2;
				trans2 = tmp2;
			
				tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = true;
				tmp2->name = 'x';
				trans2->next = tmp2;
				trans2 = tmp2;
			}
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::NUMBER;
			r32 tmpdata = .3f;
			tmp->data = *((u64*)(&tmpdata));
			trans->next = tmp;
			trans = tmp;
		
		}
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = 01;

			/*
			
			pr12.symbole = false;
			pr12.next = nullptr;

			Pattern *trans = &pr12;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::PERSONALITY_GAUGE;
			tmp->data = 0;
			pr12.first = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
			*/
		}

		std::cout << convert_pattern_to_string(&pr11) << " => "
				  << convert_pattern_to_string(&pr12) << std::endl;
		
		event_system->rules.push_back({{pr11}, pr12});
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
			tmp->type = SymboleType::CMP_GREATER;
			pr11.first = tmp;
			trans = tmp;

			tmp = new Pattern();
			tmp->symbole = false;
			trans->next = tmp;
			trans = tmp;
			{
				Pattern *trans2 = tmp;
				Pattern* tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = false;
				tmp2->type = SymboleType::PERSONALITY_GAUGE;
				tmp2->data = 0;
				trans2->first = tmp2;
				trans2 = tmp2;
			
				tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = true;
				tmp2->name = 'x';
				trans2->next = tmp2;
				trans2 = tmp2;
			}
		
			tmp = new Pattern();
			tmp->symbole = false;
			trans->next = tmp;
			trans = tmp;
			{
				Pattern *trans2 = tmp;
				Pattern* tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = false;
				tmp2->type = SymboleType::PERSONALITY_GAUGE;
				tmp2->data = 0;
				trans2->first = tmp2;
				trans2 = tmp2;
			
				tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = true;
				tmp2->name = 'y';
				trans2->next = tmp2;
				trans2 = tmp2;
			}
		}
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = 42;

			/*
			
			pr12.symbole = false;
			pr12.next = nullptr;

			Pattern *trans = &pr12;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::PERSONALITY_GAUGE;
			tmp->data = 0;
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
			tmp->variable = false;
			tmp->type = SymboleType::PERSONALITY_GAUGE;
			tmp->data = 0;
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'y';
			trans->next = tmp;
			trans = tmp;
			*/
		}

		std::cout << convert_pattern_to_string(&pr11) << " => "
				  << convert_pattern_to_string(&pr12) << std::endl;
		
		event_system->rules.push_back({{pr11}, pr12});
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
			tmp->type = SymboleType::CMP_GREATER;
			pr11.first = tmp;
			trans = tmp;

			tmp = new Pattern();
			tmp->symbole = false;
			trans->next = tmp;
			trans = tmp;
			{
				Pattern *trans2 = tmp;
				Pattern* tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = false;
				tmp2->type = SymboleType::PERSONALITY_GAUGE;
				tmp2->data = 1;
				trans2->first = tmp2;
				trans2 = tmp2;
			
				tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = true;
				tmp2->name = 'x';
				trans2->next = tmp2;
				trans2 = tmp2;
			}
		
			tmp = new Pattern();
			tmp->symbole = false;
			trans->next = tmp;
			trans = tmp;
			{
				Pattern *trans2 = tmp;
				Pattern* tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = false;
				tmp2->type = SymboleType::PERSONALITY_GAUGE;
				tmp2->data = 0;
				trans2->first = tmp2;
				trans2 = tmp2;
			
				tmp2 = new Pattern();
				tmp2->symbole = true;
				tmp2->variable = true;
				tmp2->name = 'x';
				trans2->next = tmp2;
				trans2 = tmp2;
			}
		}
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = 12;

			/*
			pr12.symbole = false;
			pr12.next = nullptr;
			
            Pattern *trans = &pr12;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::PERSONALITY_GAUGE;
			tmp->data = 1;
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
			tmp->variable = false;
			tmp->type = SymboleType::PERSONALITY_GAUGE;
			tmp->data = 0;
			trans->next = tmp;
			trans = tmp;
		
			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = true;
			tmp->name = 'x';
			trans->next = tmp;
			trans = tmp;
			*/
		}

		std::cout << convert_pattern_to_string(&pr11) << " => "
				  << convert_pattern_to_string(&pr12) << std::endl;
		
		event_system->rules.push_back({{pr11}, pr12});
	}

}
