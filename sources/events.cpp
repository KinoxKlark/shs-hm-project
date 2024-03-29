
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
#if DEBUG
	++global_app->data->event_counters.compile_user;
#endif
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
#if DEBUG
	++global_app->data->event_counters.compile_number;
#endif
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
	case SymboleType::RELATION:
	{
		Pattern *second = first->next;
		Pattern *third = second->next;

		User *user = compile_pattern_to_user(second);
		User *other_user = compile_pattern_to_user(third);

		return user->identity.relations[other_user->id];
	} break;
	default:
		assert(false);
	}
	return 0.f;
}

// NOTE(Sam): Given pattern MUST be compilable to boolean, no checking, only assert in debug mode!
bool compile_pattern_to_boolean(Pattern *pattern)
{
#if DEBUG
	++global_app->data->event_counters.compile_bool;
#endif
			
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
	case SymboleType::CMP_SMALLER:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value < rhs_value;
	} break;
	case SymboleType::CMP_GR_OR_EQ:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value >= rhs_value;
	} break;
	case SymboleType::CMP_SM_OR_EQ:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value <= rhs_value;
	} break;
	case SymboleType::CMP_EQ:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value == rhs_value;
	} break;
	case SymboleType::CMP_NOT_EQ:
	{
		Pattern *lhs = first->next;
		Pattern *rhs = lhs->next;

		r32 lhs_value = compile_pattern_to_number(lhs);
		r32 rhs_value = compile_pattern_to_number(rhs); 
		
		result = lhs_value != rhs_value;
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
#if DEBUG
	++global_app->data->event_counters.filter_calls;
#endif
			
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
#if DEBUG
		++global_app->data->event_counters.conditions;
#endif
			
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

std::vector<Environement> rule_may_be_valid(std::unordered_map<u32, Fact> *facts, Rule *rule,
											u32 condition_id, Environement *environement)
{
	std::vector<Environement> result({*environement});
	
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
		
#if DEBUG
		++global_app->data->event_counters.conditions;
#endif
		
		if(cond_id == condition_id && environement->conditions_to_compile.count(cond_id) == 0)
			continue;

		Pattern *condition = &rule->conditions[cond_id];
		std::vector<Environement> environements;

		for(auto const& fact : *facts)
		{
#if DEBUG
			++global_app->data->event_counters.facts;
#endif
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
bool rule_is_valid(std::unordered_map<u32, Fact> *facts, Rule *rule, Environement *environement)
{
	for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
	{
#if DEBUG
		++global_app->data->event_counters.conditions;
#endif
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

// TODO(Sam): Est ce qu'on peut pas faire �a avec des rvalue ref ?
// NOTE(Sam): Returned Fact MUST take property of pattern !
Fact instanciate_conclusion(Rule *rule, Environement *environement, u32 fact_id)
{
	Fact result;
	result.id = fact_id;
	result.pattern = new Pattern(rule->conclusion);
    // NOTE(Sam): Ici le result est le proprio mais on ne veut pas qu'il y aie
	// destruction du pattern a la fin de la fonction donc on pr�tend
	// qu'il n'est pas proprio et on attend du caller qu'il prenne volontairement
	// la propri�t� !
	result.pattern_proprio = false;

	replace_with_environments(result.pattern, environement);

	return result;
}

void apply_conclusion(Rule *rule, Environement *environement, std::deque<Fact>* queue, u32 *fact_id, EventSystem *event_system)
{
	if(rule->conclusion.symbole
	   && !rule->conclusion.variable
	   && rule->conclusion.type == SymboleType::_SELECT_EVENT)
	{
		Event event = event_system->all_events[rule->conclusion.data];

#if 0 // this will not happens since we unactivate event that already occured for now
		for(auto const& pair : event_system->facts)
		{
#if DEBUG
			++global_app->data->event_counters.facts;
#endif
			
			Pattern *pattern = pair.second.pattern;
			if(pattern->symbole) continue;
			if(!pattern->first->symbole) continue;
			if(pattern->first->type != SymboleType::EVENT_OCCURED) continue;

			// NOTE(Sam): At this stage every pattern should be correclty constructed
			pattern = pattern->first->next;
			if(pattern->data != event.id) continue;

			for(auto const& major_variable : event.major_variables)
			{
				pattern = pattern->next;
				for(auto const& association : environement->associations)
				{
					if(association.variable_name == major_variable
					   && pattern->data != association.datum->data)
						goto event_check_next_iteration;
				}
			}

			return;
			
			event_check_next_iteration:
					int dummy = 0;
		}
#endif

		event.timestamp = 0;
		event.users.reserve(environement->associations.size());

		if(environement->associations.size() > 0)
		{

			for(u32 idx = 0; idx < environement->associations.size(); ++idx)
			{
				EnvironementAssociation *association = &environement->associations[idx];
				Pattern *pattern = environement->associations[idx].datum;
				if(pattern->type == SymboleType::USER)
				{
					event.users[association->variable_name] = global_app->data->users[pattern->data].id;
				}
				else
				{
					User *user = compile_pattern_to_user(pattern);
					event.users[association->variable_name] = user->id;
				}
			}
		}

		for(u32 idx = 0; idx < event.users_modifs.size(); ++idx)
		{
			event.users_modifs[idx].user_id = event.users[(char)event.users_modifs[idx].user_id];

			for(u32 idx_modif = 0; idx_modif < event.users_modifs[idx].modifs.size(); ++idx_modif)
			{
				Modif *modif = &event.users_modifs[idx].modifs[idx_modif];
				if(modif->type == ModifType::RELATION)
				{
					modif->gauge_id = event.users[modif->gauge_id];
				}
			}
		}

		event_system->selected_events.push_back(event);

		#if DEBUG
		global_app->data->event_counters.first_already_computed = true;
		if(global_app->data->event_counters.first_already_computed)
		{
			global_app->data->event_counters_first = global_app->data->event_counters;
			global_app->data->event_duration = global_app->data->event_chrono.getElapsedTime();
		}
		#endif
	}
	else
	{
		// NOTE(Sam): Il doit y avoir transfert de propri�t� du pattern
		Fact conclusion = instanciate_conclusion(rule, environement, (*fact_id)++);
		queue->push_back(conclusion);
		queue->back().pattern_proprio = true;
	}
}

void create_and_push_pattern(std::deque<Fact>& queue, u32& fact_id, SymboleType type,
							 SymboleType gauge_type, u64 gauge_id, User const& user)
{
	Pattern* base = new Pattern();
	base->first = new Pattern(type);
	base->first->next = new Pattern(gauge_type, gauge_id);
	base->first->next->next = new Pattern(SymboleType::USER, (u64)user.id);

	Fact fact = {};
	fact.id = fact_id++;
	fact.pattern = base;

	queue.push_back(fact);
	queue.back().pattern_proprio = true;
}

void create_and_push_pattern(std::deque<Fact>& queue, u32& fact_id, SymboleType type,
							 User const& user, User const& other_user)
{
	Pattern* base = new Pattern();
	base->first = new Pattern(type);
	base->first->next = new Pattern(SymboleType::USER, (u64)user.id);
	base->first->next->next = new Pattern(SymboleType::USER, (u64)other_user.id);
	
	Fact fact = {};
	fact.id = fact_id++;
	fact.pattern = base;

	queue.push_back(fact);
	queue.back().pattern_proprio = true;
}


bool event_condition_inference_function(SymboleType type, r32 amount)
{
	switch(type)
	{
	case SymboleType::HIGH:
		return amount > .5f; 
		break;
	case SymboleType::LOW:
		return amount < .5;
		break;
	case SymboleType::LIKE:
		return amount > .75f;
		break;
	case SymboleType::DISLIKE:
		return amount < .25;
		break;
	case SymboleType::LOVE:
		return amount > .9f;
		break;
	case SymboleType::HATE:
		return amount < .1;
		break;
	case SymboleType::FRIEND:
		return amount > .75f;
		break;
	case SymboleType::NEUTRAL:
		return amount <= .75 && amount >= .25;
		break;
	case SymboleType::ENEMY:
		return amount < .25;
		break;
	}

	return false;
}

void push_precompiled_facts(Application *app, u32& fact_id, std::deque<Fact>& queue)
{
	std::unordered_map<SymboleType, u32>& symbole_count = app->data->event_system.symbole_count;
	symbole_count.clear();

	symbole_count[SymboleType::HIGH] = 0;
	symbole_count[SymboleType::LOW] = 0;
	symbole_count[SymboleType::LIKE] = 0;
	symbole_count[SymboleType::DISLIKE] = 0;
	symbole_count[SymboleType::LOVE] = 0;
	symbole_count[SymboleType::HATE] = 0;
	symbole_count[SymboleType::FRIEND] = 0;
	symbole_count[SymboleType::NEUTRAL] = 0;
	symbole_count[SymboleType::ENEMY] = 0;
	 
	for(auto const& user : app->data->users)
	{		
		for(auto const& gauge : user.identity.personalities)
		{
			if(event_condition_inference_function(SymboleType::HIGH, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::HIGH,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::HIGH];
			}
			else if(event_condition_inference_function(SymboleType::LOW, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LOW,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::LOW];
			}
			
			if(event_condition_inference_function(SymboleType::LIKE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LIKE,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::LIKE];
			}
			else if(event_condition_inference_function(SymboleType::DISLIKE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::DISLIKE,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::DISLIKE];
			}
			
			if(event_condition_inference_function(SymboleType::LOVE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LOVE,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::LOVE];
			}
			else if(event_condition_inference_function(SymboleType::HATE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::HATE,
										SymboleType::PERSONALITY_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::HATE];
			}
		}

		for(auto const& gauge : user.identity.interests)
		{

			if(event_condition_inference_function(SymboleType::HIGH, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::HIGH,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::HIGH];
			}
			else if(event_condition_inference_function(SymboleType::LOW, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LOW,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::LOW];
			}
			
			if(event_condition_inference_function(SymboleType::LIKE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LIKE,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::LIKE];
			}
			else if(event_condition_inference_function(SymboleType::DISLIKE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::DISLIKE,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::DISLIKE];
			}
			
			if(event_condition_inference_function(SymboleType::LOVE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::LOVE,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);
				++symbole_count[SymboleType::LOVE];
			}
			else if(event_condition_inference_function(SymboleType::HATE, gauge.amount))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::HATE,
										SymboleType::INTEREST_GAUGE,
										(u64)gauge.id, user);	
				++symbole_count[SymboleType::HATE];
			}
		}

		for(u32 idx = 0; idx < user.identity.relations.size(); ++idx)
		{
			if(idx == user.id) continue;
			r32 relation = user.identity.relations[idx];

			User const& other_user = app->data->users[idx];

			if(event_condition_inference_function(SymboleType::FRIEND, relation))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::FRIEND, user, other_user);
				create_and_push_pattern(queue, fact_id, SymboleType::FRIEND, other_user, user);
				++symbole_count[SymboleType::FRIEND];
			}
			else if(event_condition_inference_function(SymboleType::ENEMY, relation))
			{
				create_and_push_pattern(queue, fact_id, SymboleType::ENEMY, user, other_user);
				create_and_push_pattern(queue, fact_id, SymboleType::ENEMY, other_user, user);
				++symbole_count[SymboleType::ENEMY];
			}
			else
			{
				create_and_push_pattern(queue, fact_id, SymboleType::NEUTRAL, user, other_user);
				create_and_push_pattern(queue, fact_id, SymboleType::NEUTRAL, other_user, user);
				++symbole_count[SymboleType::NEUTRAL];
			}
		}
	}
}

void deactivate_impossible_rules(Application* app, std::deque<Fact>& queue)
{
	EventSystem* event_system = &app->data->event_system;
	
	std::unordered_set<u32> event_ids;
	for(auto it = queue.begin(); it != queue.end(); ++it)
	{
		Pattern* pattern = it->pattern;
		if(pattern && !pattern->symbole && pattern->first->symbole &&
		   pattern->first->type == SymboleType::EVENT_OCCURED)
		{
			event_ids.insert((u32)pattern->first->next->data);
		}
				
	}
	
	for(u32 rule_id = 0; rule_id < event_system->rules.size(); ++rule_id)
	{
		Rule* rule = &event_system->rules[rule_id];
		rule->active = true;

		// TODO(Sam): For now we will avoid to repeat an event twice, even
		// if it is with different actors because we do not have enough events
		// so that the repetition isnt obvious
		if(rule->conclusion.type == SymboleType::_SELECT_EVENT &&
		   event_ids.count(rule->conclusion.data) > 0)
		{
			rule->active = false;
			goto deactivate_next_rule;
		}
		
		for(u32 cond_id = 0; cond_id < rule->conditions.size(); ++cond_id)
		{
			Pattern* condition = &rule->conditions[cond_id];

			if(!condition->symbole && condition->first->symbole)
			{
				if(condition->first->type == SymboleType::EVENT_OCCURED &&
				   event_ids.count((u32)condition->first->next->data) == 0)
				{
					rule->active = false;
					goto deactivate_next_rule;
				}

				if(condition->first->type == SymboleType::SOCIAL_POST_SEEN &&
				   event_ids.count(
					   app->data->social_post_system.all_posts[(u32)condition->first->next->data].event_id) == 0)
				{
					assert(("diferent post id", app->data->social_post_system.all_posts[(u32)condition->first->next->data].social_post_id == (u32)condition->first->next->data));

					rule->active = false;
					goto deactivate_next_rule;
				}

				if(event_system->symbole_count.count(condition->first->type) &&
				   event_system->symbole_count[condition->first->type] == 0)
				{
					rule->active = false;
					goto deactivate_next_rule;
				}
			
			}

		}

	deactivate_next_rule:

#if DEBUG
		if(!rule->active)
			++app->data->event_counters.nb_discarded_facts;
		++app->data->event_counters.nb_total_facts;
#endif
		
		continue;
	}
}


void inference(Application *app)
{
	GameData *data = app->data;
	EventSystem *event_system = &data->event_system;

	
	event_system->thread_mutex.lock();

	event_system->selected_events.clear();

	u32 fact_id = event_system->fact_next_id;
	
	std::deque<Fact> working_queue{};
	std::unordered_map<u32, Fact> deduced_facts{};
	
	for(auto it = event_system->facts.begin(); it != event_system->facts.end(); ++it)
	{
		working_queue.push_back(it->second);
	}
	
	event_system->thread_mutex.unlock();


	push_precompiled_facts(app, fact_id, working_queue);
	
	deactivate_impossible_rules(app, working_queue);

	while(!working_queue.empty())
	{
		Fact fact = working_queue.front();
		fact_swap_property(fact, working_queue.front());
		working_queue.pop_front();

		if(deduced_facts.count(fact.id) == 0)
		{
			deduced_facts.insert({fact.id,fact});
			fact_swap_property(deduced_facts[fact.id],fact);

#if DEBUG
			++global_app->data->event_counters.facts;
#endif
			// TODO(Sam): On extrait si besoins...
			//std::cout << convert_pattern_to_string(fact.pattern) << std::endl;

			for(u32 rule_id = 0; rule_id < event_system->rules.size(); ++rule_id)
			{
				if(!event_system->rules[rule_id].active) continue;

				
#if DEBUG
				++global_app->data->event_counters.rules;
#endif
			
				Rule *rule = &event_system->rules[rule_id];
				std::vector<ConditionWithEnvironement> conds_envs = rule_has_condition(fact, rule);

				for(u32 condenv_id = 0; condenv_id < conds_envs.size(); ++condenv_id)
				{
#if DEBUG
					++global_app->data->event_counters.conditions;
#endif
			
					u32 condition_id = conds_envs[condenv_id].condition;
					Environement *environement = &(conds_envs[condenv_id].environement);

					std::vector<Environement> envs = rule_may_be_valid(&deduced_facts, rule, condition_id, environement);
					for(u32 env_id = 0; env_id < envs.size(); ++env_id)
					{
						if(!rule_is_valid(&deduced_facts, rule, &envs[env_id])) continue;

						apply_conclusion(rule, &envs[env_id], &working_queue, &fact_id, event_system);
						
					}
				}

			}

		}
			
	}
	
}

void select_events_without_rules(EventSystem *event_system)
{
	GameData *data = global_app->data;

	// TODO(Sam): Code duplication here :/
	std::unordered_set<u32> event_ids;
	for(auto it = event_system->facts.begin(); it != event_system->facts.end(); ++it)
	{
		Pattern* pattern = it->second.pattern;
		if(pattern && !pattern->symbole && pattern->first->symbole &&
		   pattern->first->type == SymboleType::EVENT_OCCURED)
		{
			event_ids.insert((u32)pattern->first->next->data);
		}
				
	}
	
	for(u32 idx : event_system->events_without_rules)
	{
		Event event = event_system->all_events[idx];

		if(event_ids.count(event.id)>0) continue;

		event.timestamp = 0;
		// TODO(Sam): Ceci ne permet pas de r�cup les perso qui sont dans modifs, c'est un bug
		// qui existe �galement pour les event "normaux"
		event.users.reserve(event.major_variables.size());

		// NOTE(Sam): We select N different users which seems more efficiant than selecting all
		// possible combination of users
		std::vector<u32> user_ids(event.major_variables.size());
		for(u32 idx = 0; idx < user_ids.size(); ++idx)
		{
			bool loop = true;
			while(loop)
			{
				user_ids[idx] = get_random_number_between(0, data->users.size()-1);
				loop = false;
				for(u32 idx2 = 0; idx2 < idx; ++idx2)
				{
					if(user_ids[idx2] == user_ids[idx])
					{
						loop = true;
						break;
					}
				}
			}
		}

		u32 id_user = 0;
		for(char var : event.major_variables)
		{
			event.users[var] = user_ids[id_user];
			++id_user;
		}
		
		for(u32 idx = 0; idx < event.users_modifs.size(); ++idx)
		{
			event.users_modifs[idx].user_id = event.users[(char)event.users_modifs[idx].user_id];

			for(u32 idx_modif = 0; idx_modif < event.users_modifs[idx].modifs.size(); ++idx_modif)
			{
				Modif *modif = &event.users_modifs[idx].modifs[idx_modif];
				if(modif->type == ModifType::RELATION)
				{
					modif->gauge_id = event.users[modif->gauge_id];
				}
			}
		}

		event_system->selected_events.push_back(event);
	}
	

}

// Threaded
void event_selection()
{
	Application* app = global_app;
	EventSystem *event_system = &app->data->event_system;

	if(event_system->first_time_arround)
	{
		event_system->first_time_arround = false;
		random_init(time(0));
		event_system->random_seed = get_random_number_between(0,1024);
	}

	random_init(event_system->random_seed);
	event_system->random_seed = get_random_number_between(0,1024);
	
	
	
#if DEBUG
	global_app->data->event_counters = {};
	global_app->data->event_chrono.restart();
#endif
	
	// NOTE(Sam): This use the inference engine to select the list
	// of intanciable events
	inference(app);
	select_events_without_rules(event_system);

	event_system->thread_mutex.lock();

	if(event_system->selected_events.size() > 0)
	{
	
		//TODO(Sam): We probably will want more control over the probability distribution over events
		Event event = get_random_element(event_system->selected_events);

		Pattern p0 = {};
		p0.symbole = false;
		p0.next = nullptr;
		{
			Pattern *trans = &p0;
			Pattern *tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::EVENT_OCCURED;
			tmp->data = 0;
			p0.first = tmp;
			trans = tmp;

			tmp = new Pattern();
			tmp->symbole = true;
			tmp->variable = false;
			tmp->type = SymboleType::EVENT;
			tmp->data = (u64)(event.id);
			trans->next = tmp;
			trans = tmp;
		
			for(auto const& variable : event.major_variables)
			{
				u32 user_id = event.users[variable];
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

#ifdef DEBUG
		event_system->debut_instancied_events.push_back(event);
#endif

		user_react_to_modifs(&event.users_modifs);
		
		instanciate_social_post_for_event(app, &event);

		

	}

	event_system->event_selection_done = true;


	event_system->thread_mutex.unlock();


	#if DEBUG
	global_app->data->event_duration = global_app->data->event_chrono.getElapsedTime();
	#endif
}

void starter_events(GameData *data)
{
	EventSystem *event_system = &data->event_system;
	SocialPostSystem *social_post_system= &data->social_post_system;
	
	for(auto const& user : data->users)
	{
		for(auto const& personality : user.identity.personalities)
		{
			r32 r = get_random_number_between(0.f, 1.f);

			if(r < .75)
			{
				for(auto& rule : data->event_system.starter_rules)
				{
					Pattern *pattern = rule.conditions[0].first;
					if(pattern->next->type != SymboleType::PERSONALITY_GAUGE)
						continue;
					
					if(pattern->next->data == personality.id
					   && event_condition_inference_function(pattern->type, personality.amount))
					{
						Event event = event_system->starter_events[rule.conclusion.data];
						event.users[event.major_variables[0]] = user.id;
						assert(event.major_variables.size() == 1);
						
						instanciate_starter_social_post_for_event(global_app, &event);
						SocialPost *post = &social_post_system->available_posts.back();
					}
				}
			}
			
		}

		for(auto const& interest : user.identity.interests)
		{
			r32 r = get_random_number_between(0.f, 1.f);

			if(r < .75)
			{
				for(auto& rule : data->event_system.starter_rules)
				{
					Pattern *pattern = rule.conditions[0].first;
					if(pattern->next->type != SymboleType::INTEREST_GAUGE)
						continue;
					
					if(pattern->next->data == interest.id
					   && event_condition_inference_function(pattern->type, interest.amount))
					{
						Event event = event_system->starter_events[rule.conclusion.data];
						event.users[event.major_variables[0]] = user.id;
						assert(event.major_variables.size() == 1);
						
						instanciate_starter_social_post_for_event(global_app, &event);
						SocialPost *post = &social_post_system->available_posts.back();

						r32 r = get_random_number_between(0.f, 1.f);
						if(r < .5)
						{
							//social_post_system->social_feeds[user.id].posts.push_back(*post);
							social_post_system->available_posts.pop_back();
						}
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
		case SymboleType::HIGH:
			result += "HIGH";
			break;
		case SymboleType::LOW:
			result += "LOW";
			break;
		case SymboleType::LIKE:
			result += "LIKE";
			break;
		case SymboleType::DISLIKE:
			result += "DISLIKE";
			break;
		case SymboleType::LOVE:
			result += "LOVE";
			break;
		case SymboleType::HATE:
			result += "HATE";
			break;
		case SymboleType::FRIEND:
			result += "FRIEND";
			break;
		case SymboleType::NEUTRAL:
			result += "NEUTRAL";
			break;
		case SymboleType::ENEMY:
			result += "ENEMY";
			break;
		case SymboleType::EVENT_OCCURED:
			result += "EVENT OCCURED";
			break;
		case SymboleType::SOCIAL_POST_SEEN:
			result += "A VU";
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
		case SymboleType::CMP_SMALLER:
			result += "<";
			break;
		case SymboleType::CMP_GR_OR_EQ:
			result += ">=";
			break;
		case SymboleType::CMP_SM_OR_EQ:
			result += "<=";
			break;
		case SymboleType::CMP_EQ:
			result += "==";
			break;
		case SymboleType::CMP_NOT_EQ:
			result += "!=";
			break;
		case SymboleType::OP_NOT:
			result += "!";
			break;
		case SymboleType::USER:
			result += global_app->data->users[pattern->data].fullname;
			break;
		case SymboleType::EVENT:
			result += "Event";
			{
				std::stringstream ss;
				ss << " [" << pattern->data << "]";
				result += ss.str();
			}
			//result += global_app->data->event_system.all_events[pattern->data].description;
			break;
		case SymboleType::SOCIAL_POST:
			result += "Social post";
			{
				std::stringstream ss;
				ss << " [" << pattern->data << "]";
				result += ss.str();
			}
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
			result += "INTEREST " + global_app->data->interests[pattern->data].name;
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
		case SymboleType::RELATION:
			result += "RELATION";
			
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

void destroy_event_system(EventSystem *event_system)
{
	event_system->thread->terminate();
	delete event_system->thread;
}

void init_event_system(EventSystem *event_system)
{
	GameData *data = global_app->data;

	event_system->time_since_last_inference = sf::seconds(10);
	event_system->fact_next_id = 0;
	event_system->event_selection_done = true;
	event_system->thread = new sf::Thread(&event_selection);
	event_system->first_time_arround = true;

	for(u32 i = 0; i < data->users.size(); ++i)
	{
		Pattern p0 = {};
		p0.symbole = true;
		p0.variable = false;
		p0.type = SymboleType::USER;
		p0.data = (u64)(data->users[i].id);
		Fact f0 = {};
		f0.id = event_system->fact_next_id++;
		f0.pattern = new Pattern(p0);

		event_system->facts.insert({f0.id,f0});
		event_system->facts[f0.id].pattern_proprio = true;
	
	}

#if 0
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

		Event event = {};
		event.id = event_system->all_events.size();
		event.description = "Event A cr�atif";
		event_system->all_events.push_back(event);
		
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = event.id;

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

		Event event = {};
		event.id = event_system->all_events.size();
		event.description = "Event A plus cr�atif que B";
		event_system->all_events.push_back(event);
		
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = event.id;

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

		Event event = {};
		event.id = event_system->all_events.size();
		event.description = "Event Curieux et cr�atif!";
		event_system->all_events.push_back(event);
		
		Pattern pr12 = {};
		{
			pr12.symbole = true;
			pr12.variable = false;
			pr12.type = SymboleType::_SELECT_EVENT;
			pr12.data = event.id;

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
#endif
}
