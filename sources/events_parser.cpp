#include <fstream>
#include <stack>

enum class EventCharParserState {
	Default,
	Comments
};

enum class EventTokenParserState {
	Default,

	EventHeader,
	EventCore,
	EventId,
	EventFieldPerso,
	EventFieldDescr,
	EventFieldConds,
	EventFieldModifs,
				
};

enum class ItemType{
	NONE,

	VARIABLE,
	USER_FIELD,
	NUMBER,
	EVENT_ID,

	ACCESSOR,
	BINARY_OPERATOR,

	UNARY_OPERATOR,

	FUNCTION,

	_INVALID,
};

struct Item {
	ItemType type;
	std::string str;
};

struct Node {
	Item item;
	std::vector<Node> children;
};

#define break_with_error(msg) std::cout << "ERROR: " << (msg) << "\n"; assert(false);
#define stop_with_error(msg) std::cout << "ERROR: " << (msg) << "\n"; return false;
#define breaking_error(expr, msg) if((expr)) { break_with_error(msg) }
#define stoping_error(expr, msg) if((expr)) { stop_with_error(msg) }

const std::unordered_set<char> white_chars = {' ', '\t', '\r', '\b'};
const std::unordered_set<char> special_chars = {
	'{', '}', '[', ']', ':', '=', '(', ')', '"', '.',';','+','-',
	',','?','>','<','!', '\\', '\n'
};


bool parseString(std::vector<std::string> const& tokens, u32& idx, std::string& str, char terminator = '"');
bool parseVariableName(std::vector<std::string> const& tokens, u32& idx, std::string& str,
					   std::unordered_map<std::string, char>& current_variables, char& variable_id);
bool parseStringNumber(std::vector<std::string> const& tokens, u32& idx, std::string &str);
bool parseNumber(std::string const& str, r32 &number);

Node convertExpressionToTree(std::vector<Item> const& expression, u32 idstart, u32 idend);
void displayTree(Node const& node, std::string const& prefix = "");
Pattern* convertTreeToPattern(Node const& node,
							  std::unordered_map<std::string, char>& current_variables,
							  std::unordered_map<std::string, u32>& event_ids);


bool importEventsFile(EventSystem *event_system, std::string const& filename)
{
	std::cout << "Importing " << filename << std::endl;

	std::vector<std::string> tokens;
	
	{
		std::ifstream file;
		file.open(filename,std::ifstream::in);
		if(!file.good())
			return false;

		EventCharParserState state = EventCharParserState::Default;
		char c;
		std::string token = "";
		while(file.get(c))
		{
			switch(state)
			{
			case EventCharParserState::Default:
			{
				if(c == '#')
				{
					if(!token.empty()) tokens.push_back(token);
					token = "";
					state = EventCharParserState::Comments;
					break;
				}

				if(special_chars.count(c) > 0 || white_chars.count(c) > 0)
				{
					if(!token.empty()) tokens.push_back(token);
					token = "";
					tokens.push_back(token + c);
					break;
				}
			
				token += c;
			} break;
			case EventCharParserState::Comments:
			{
				if(c == '\n')
				{
					state = EventCharParserState::Default;
					break;
				}
			} break;
			InvalidDefaultCase;
			};
		}

		file.close();
	}

	{
		u32 next_event_id = event_system->all_events.size();
		std::unordered_map<std::string, u32> event_ids;

		char variable_id = 'A';
		Event constructed_event = {};
		std::unordered_map<std::string, char> current_variables;
		
		std::stack<EventTokenParserState> states;
		states.push(EventTokenParserState::Default);
		for(u32 idx = 0; idx < tokens.size(); ++idx)
		{
			std::string& token = tokens[idx];
			switch(states.top())
			{
			case EventTokenParserState::Default:
			{
				if(white_chars.count(token[0]) > 0 || token == "\n") break;

				if(token == "event")
				{
					states.push(EventTokenParserState::EventHeader);
					break;
				}
				else if(token == "post")
				{
					// TODO(Sam): Implements...
					break;
				}

				std::cout << "ERROR:" << "New block should start with one of the accepted keywords: 'event' / 'post' but we found '" << token << "'!" << "\n";
				return false;
				
			} break;

			case EventTokenParserState::EventHeader:
			{
				if(white_chars.count(token[0]) > 0 || token == "\n") break;
				if(token == ":")
				{
					while(white_chars.count(tokens[++idx][0]) > 0) continue;
					--idx;
					states.push(EventTokenParserState::EventId);
					break;	
				}
				else if(token == "{")
				{
					states.pop();
					states.push(EventTokenParserState::EventCore);
					break;
				}

				std::cout << "ERROR:" << "Expected ':' followeb by 'event id' or '{', got '" << token << "'." << "\n";
				return false;
				
			} break;
			case EventTokenParserState::EventId:
			{
				if(special_chars.count(token[0]) > 0)
				{
					std::cout << "ERROR:" << "Expected an 'event id', got '" << tokens[idx][0] << "'." << "\n";
					return false;
				}
					
				std::string event_id = "";
				--idx;
				while(white_chars.count(tokens[++idx][0]) == 0 && tokens[idx] != "{" && tokens[idx] != "\n")
				{
					event_id += tokens[idx];
				}
				--idx;

				// TODO(Sam): Log system pour debug
				std::cout << "EVENT ID: " << event_id << "\n";
				if(event_ids.count(event_id) > 0)
				{
					std::cout << "ERROR:" << "This event id is already used: '" << event_id << "'." << "\n";
					return false;
				}
				u32 id = next_event_id++;
				event_ids[event_id] = id;
				constructed_event.id = id;
				constructed_event.timestamp = 0;
		
				while(white_chars.count(tokens[++idx][0]) > 0) continue;
				--idx;

				states.pop();
				break;
				
			} break;
			case EventTokenParserState::EventCore:
			{
				if(white_chars.count(token[0]) > 0 || token == "\n") break;

				if(token == "}")
				{
					event_system->all_events.push_back(constructed_event);
					constructed_event.id = 0;
					constructed_event.description = "";
					constructed_event.major_variables.clear();
					current_variables.clear();
					variable_id = 'A';
					
					states.pop();
					break;
				}

				if(token == "perso")
				{
					states.push(EventTokenParserState::EventFieldPerso);
				}
				else if(token == "descr")
				{
					states.push(EventTokenParserState::EventFieldDescr);
				}
				else if(token == "conds" || token == "cond")
				{
					states.push(EventTokenParserState::EventFieldConds);
				}
				else if(token == "modifs" || token == "modif")
				{
					states.push(EventTokenParserState::EventFieldModifs);
				}
				else
				{
					std::cout << "ERROR:" << "Expected on of: ('perso','descr','conds','modifs') or '}', got '" << token << "'." << "\n";
					return false;
				}

				while(white_chars.count(tokens[++idx][0]) > 0) continue;

				if(tokens[idx] != "=")
				{
					std::cout << "ERROR:" << "Expected '=', got '" << tokens[idx] << "'." << "\n";
					return false;
				}

				while(white_chars.count(tokens[++idx][0]) > 0) continue;
				--idx;
				
			} break;
			case EventTokenParserState::EventFieldPerso:
			{
				std::string variable;
				if(!parseVariableName(tokens, idx, variable, current_variables, variable_id))
					return false;

				std::cout << "- VARIABLE: " << variable << "\n";
				constructed_event.major_variables.push_back(current_variables[variable]);

				while(white_chars.count(tokens[++idx][0]) > 0)
					continue;

				if(tokens[idx] == ",")
				{
					break;
				}
				else if(tokens[idx] == "\n")
				{
					states.pop();
					break;
				}

				std::cout << "ERROR:" << "Expected ',' or end of line, got '" << tokens[idx] << "'." << "\n";
				return false;
				
			} break;
			case EventTokenParserState::EventFieldDescr:
			{
				--idx;
				while(white_chars.count(tokens[++idx][0]) > 0) continue;

				char terminator = '\n';
				if(tokens[idx] == "\"")
				{
					terminator = '"';
					++idx;
				}

				std::string str;
				if(!parseString(tokens, idx, str, terminator))
					return false;

				std::cout << "- DESCRIPTION: \"" << str << "\"\n";
				constructed_event.description = str;

				if(terminator == '\n') --idx;
				while(white_chars.count(tokens[++idx][0]) > 0)
					continue;

				if(tokens[idx] != "\n")
				{
					std::cout << "ERROR:" << "Expected end of line, got '" << tokens[idx] << "'." << "\n";
					return false;
				}

				states.pop();
				break;
				
			} break;
			case EventTokenParserState::EventFieldConds:
			{
				--idx;
				while(white_chars.count(tokens[++idx][0]) > 0) continue;

				bool array = tokens[idx] == "[";

				if(array) ++idx;
				
				Rule rule = {};
				while(tokens[idx] != "]" && (array || tokens[idx] != "\n"))
				{
					--idx; while(white_chars.count(tokens[++idx][0])>0) continue;
					if(array && (tokens[idx] == "\n" || tokens[idx] == ","))
					{
						++idx;
						continue;
					}
					
					Pattern condition = {};
					condition.symbole = false;
					condition.next = nullptr;

					std::vector<Item> expression;
					i32 parenthesis_depth = 0;
					
					while((parenthesis_depth > 0 || tokens[idx] != ",")
						  && tokens[idx] != "\n")
					{
						--idx; while(white_chars.count(tokens[++idx][0]) > 0) continue;

						if(tokens[idx][0] == '?') // Variable
						{
							Item item;
							item.type = ItemType::VARIABLE;
							if(!parseVariableName(tokens, idx, item.str, current_variables, variable_id))
								return false;
							expression.push_back(item);

							if(tokens[++idx] == ".")
							{
								expression.push_back({ItemType::ACCESSOR, "."});
								item.type = ItemType::USER_FIELD;
								if(tokens[++idx][0] == '"')
								{
									++idx;
									item.str = "";
									if(!parseString(tokens, idx, item.str, '"'))
										return false;
									
								}
								else
								{
									item.str = tokens[idx];
								}
								expression.push_back(item);
							}
							else --idx;
						}
						else if((tokens[idx][0] >= '0' && tokens[idx][0] <= '9') || tokens[idx][0] == '.') // Number
						{
							Item item;
							item.type = ItemType::NUMBER;
							if(!parseStringNumber(tokens, idx, item.str))
								return false;
							expression.push_back(item);
						}
						else if(tokens[idx] == "event") // Event
						{
							Item item;
							item.type = ItemType::FUNCTION;
							item.str = tokens[idx];
							expression.push_back(item);

							if(tokens[++idx] != "(")
							{
								std::cout << "ERROR:" << "Expected openning parenthesis.\n";
								return false;
							}
							expression.push_back({ItemType::NONE, "("});

							item.type = ItemType::EVENT_ID;
							item.str = "";
							while((tokens[++idx] != "," && tokens[idx] != ")")
								  && white_chars.count(tokens[idx][0]) == 0)
							{
								item.str += tokens[idx];
							}
							expression.push_back(item);
							--idx; while(white_chars.count(tokens[++idx][0])>0) continue;
							if(tokens[idx] != "," && tokens[idx] != ")")
							{
								std::cout << "ERROR:" << "Expected a comma ',' or closing parenthesis ')'.\n";
								return false;
							}

							while(tokens[idx] == ",")
							{
								expression.push_back({ItemType::NONE, ","});
								while(white_chars.count(tokens[++idx][0])>0) continue;
							
								item.type = ItemType::VARIABLE;
								item.str = "";
								if(!parseVariableName(tokens, idx, item.str, current_variables, variable_id))
									return false;
								expression.push_back(item);
								
								while(white_chars.count(tokens[++idx][0])>0) continue;
							}
							
							--idx; while(white_chars.count(tokens[++idx][0])>0) continue;
							if(tokens[idx] != ")")
							{
								std::cout << "ERROR:" << "Expected closing parenthesis.\n";
								return false;
							}
							expression.push_back({ItemType::NONE, ")"});
						}
						else if(tokens[idx] == ">"
							|| tokens[idx] == "<") // Operateur
						{
							Item item;
							item.type = ItemType::BINARY_OPERATOR;
							item.str = tokens[idx];
							expression.push_back(item);
						}
						else if((tokens[idx] == "=" && tokens[idx+1] == "=")
							|| (tokens[idx] == "!" && tokens[idx+1] == "=")
							|| (tokens[idx] == "<" && tokens[idx+1] == "=")
							|| (tokens[idx] == ">" && tokens[idx+1] == "=")) // Operateur
						{
							Item item;
							item.type = ItemType::BINARY_OPERATOR;
							item.str = tokens[idx]+tokens[idx+1];
							expression.push_back(item);
							++idx;
						}
						else if(tokens[idx] == "!")
						{
							expression.push_back({ItemType::UNARY_OPERATOR, "!"});
						}
						// Various simple tokens
						else if(tokens[idx] == "("
								|| tokens[idx] == ")")
						{
							if(tokens[idx] == "(") ++parenthesis_depth;
							if(tokens[idx] == ")") --parenthesis_depth;

							if(parenthesis_depth < 0)
							{
								std::cout << "ERROR: " << "Unexpected closing parenthesis.\n";
								return false;
							}
							
							Item item;
							item.type = ItemType::NONE;
							item.str = tokens[idx];
							expression.push_back(item);
						}
						else if((array && (tokens[idx] == "," || tokens[idx] == "]")) || tokens[idx] == "\n")
						{
							if(parenthesis_depth > 0)
							{
								std::cout << "ERROR: " << "Unexpected end of condition (missing parenthesis).\n";
								return false;
							}
							
							break;
						}
						else
						{
							std::cout << "ERROR: " << "Unexpected token in condition: '" << tokens[idx] << "'." << "\n";
							return false;
						}

						++idx;
					}

					if(expression.size() == 0)
						continue;
					
#if 1
					std::cout << "- Cond: ";
					for(Item& i : expression)
					{
						std::cout << "\"" << i.str << "\" ";
						if(i.type == ItemType::NUMBER)
						{
							r32 nb = 0;
							parseNumber(i.str, nb);
							std::cout << "[" << nb << "] ";
						}
					}
					std::cout << "\n";
#endif
					
					Node expression_tree = convertExpressionToTree(expression, 0, expression.size()-1);
					
#if 1
					displayTree(expression_tree, "        ");
#endif

					Pattern *condition_pattern =  convertTreeToPattern(expression_tree, current_variables, event_ids);
					rule.conditions.push_back(*condition_pattern);
					delete condition_pattern;
				}
					
				rule.conclusion.symbole = true;
				rule.conclusion.variable = false;
				rule.conclusion.type = SymboleType::_SELECT_EVENT;
				rule.conclusion.data = constructed_event.id;

				event_system->rules.push_back(rule);
				
				if(array && tokens[idx] != "]")
				{
					std::cout << "ERROR:" << "Expected end of array ']', got '" << tokens[idx] << "'." << "\n";
					return false;
				}

				while(white_chars.count(tokens[++idx][0]) > 0) continue;
				--idx;

				states.pop();
				
			} break;
			case EventTokenParserState::EventFieldModifs:
			{
				--idx;
				while(white_chars.count(tokens[++idx][0]) > 0) continue;

				bool array = tokens[idx] == "[";

				// TODO(Sam): Les modifs...
				while(tokens[++idx] != "]" && (array || tokens[idx] != "\n")) continue;
				// TODO(Sam): tmp code, remove it!
				
				if(array && tokens[idx] != "]")
				{
					std::cout << "ERROR:" << "Expected end of array ']', got '" << tokens[idx] << "'." << "\n";
					return false;
				}
				
				while(white_chars.count(tokens[++idx][0]) > 0) continue;
				--idx;

				states.pop();
			} break;

			InvalidDefaultCase;
			};
		}
		
	}
	
	return true;
}

bool parseString(std::vector<std::string> const& tokens, u32& idx, std::string& str, char terminator)
{
	--idx;
	while(tokens[++idx][0] != terminator)
	{
		if(tokens[idx] == "\\")
		{
			if(tokens[idx+1][0] == terminator)
				++idx;
		}

		str += tokens[idx];

		if(idx+1 == tokens.size())
		{
			std::cout << "ERROR:" << "Missing string terminator!" << "\n";
			return false;
		}
	}

	return true;
}

bool parseVariableName(std::vector<std::string> const& tokens, u32& idx, std::string& str,
					   std::unordered_map<std::string, char>& current_variables, char& variable_id)
{
	--idx;
	while(white_chars.count(tokens[++idx][0]) > 0) continue;
	
	if(tokens[idx] != "?")
	{
		std::cout << "ERROR:" << "Expected '?', got '" << tokens[idx] << "'." << "\n";
		return false;
	}

	++idx;
	if(white_chars.count(tokens[idx][0]) > 0 || special_chars.count(tokens[idx][0]) > 0)
	{
		std::cout << "ERROR:" << "Expected alphanumerical chars for variable name, got '" << tokens[idx] << "'." << "\n";
		return false;
	}

	str = "?"+tokens[idx];

	if(current_variables.count(str) == 0)
	{
		current_variables[str] = variable_id++;
	}

	return true;
}

bool parseStringNumber(std::vector<std::string> const& tokens, u32& idx, std::string &str)
{
	--idx;
	while(white_chars.count(tokens[++idx][0]) > 0) continue;

	std::string digits = "";
	std::string decimals = "";

	if(tokens[idx][0] != '.')
	{
		for(char c : tokens[idx])
		{
			if(!(c>='0' && c <= '9'))
			{
				std::cout << "ERROR:" << "Unexpected caracter in number parsing: '" << c << "'.\n";
				return false;
			}

			digits += c;
		}
		++idx;
	}

	if(tokens[idx][0] == '.')
	{
		char c = tokens[++idx][0];
		if(!(c>='0' && c <= '9'))
		{
			--idx;
		}
		else
		for(char c : tokens[idx])
		{
			if(!(c>='0' && c <= '9'))
			{
				
				std::cout << "ERROR:" << "Unexpected caracter in number parsing: '" << c << "'.\n";
				return false;
			}

			decimals += c;
		}
	}


	str = digits + "." + decimals;

	return true;
}

u32 fromStringToNumber(std::string str)
{
	u32 pow = 1;
	u32 number = 0;

	for(char c : str)
	{
		pow *= 10;
	}
	pow /= 10;

	for(char c : str)
	{
		number += pow*(c-'0');
		pow /= 10;
	}

	return number;
}

bool parseNumber(std::string const& str, r32 &number)
{
	std::string digits = "";
	std::string decimals = "";

	bool do_decimals = false;
	for(char c : str)
	{
		if(c == '.')
		{
			do_decimals = true;
			continue;
		}

		if(!(c>='0' && c <= '9'))
		{
			std::cout << "ERROR:" << "Expected digits, got '" << c << "'." << "\n";
			return false;
		}
		
		if(do_decimals)
			decimals += c;
		else
			digits += c;
	}
	
	number = (r32)(fromStringToNumber(digits));
	if(decimals.size() > 0)
	{
		r32 pow = 1;
		for(u32 i = 0; i < decimals.size(); ++i) pow *= 10;
		number += (r32)(fromStringToNumber(decimals))/pow;
	}
	
	return true;
}

u32 getItemPrecedence(Item const& item)
{
	// Operator precedence:
	// >, <, >=, <=    0
	// !               1
	// func(...)       2
	// .               3
	// other...        4
	
	static std::unordered_map<std::string, u32> str_precedence = {
		{ ">", 0 }, { "<", 0 }, { ">=", 0 }, { "<=", 0 }, { "==", 0 }, { "!=", 0 },
		{ "!", 1 },
		//{ FUNCTION , 2 },
		{ ".", 3 },
	};

	u32 precedence = 4;
	if(item.type == ItemType::FUNCTION)
		precedence = 2;
	else if(str_precedence.count(item.str) > 0)
		precedence = str_precedence[item.str];

	return precedence;
}

u32 getIdOfNextTokenBeforeGreaterPrecedence(std::vector<Item> const& expression, Item const& ref_item,
											u32 idstart, u32 idend)
{

	u32 ref_precedence = getItemPrecedence(ref_item);
	
	i32 parenthesis_depth = 0;
	u32 result = idend;

	for(u32 i = idstart; i <= idend; ++i)
	{
		if(expression[i].str == "(") ++parenthesis_depth;
		if(expression[i].str == ")") --parenthesis_depth;
		if(parenthesis_depth == 0)
		{		
			u32 precedence = getItemPrecedence(expression[i]);
		
			if(precedence <= ref_precedence)
			{
				result = i-1;
				break;
			}
		}
	}
	
	return result;
}

Node convertExpressionToTree(std::vector<Item> const& expression, u32 idstart, u32 idend)
{
	
	Node root = {};
	root.item.type = ItemType::_INVALID;
	
	for(u32 i = idstart; i < idend+1; ++i)
	{
		switch(expression[i].type)
		{
		case ItemType::NONE:
		{
			if(expression[i].str == "(")
			{
				breaking_error(root.item.type != ItemType::_INVALID, "Invalid expression tree");
				u32 id_end_parenthesis = i;
				i32 parenthesis_depth = 1;
				for(u32 j = i+1; j <= idend; ++j)
				{
					if(expression[j].str == "(") ++parenthesis_depth;
					if(expression[j].str == ")") --parenthesis_depth;
					if(parenthesis_depth == 0)
					{
						id_end_parenthesis = j;
						break;
					}
				}
				if(id_end_parenthesis <= i)
				{
					std::cout << "EXPR ERROR: " << "Missing closing parenthesis" << "\n";
					assert(false);
				}
			
				root = convertExpressionToTree(expression, i+1, id_end_parenthesis-1);
				i = id_end_parenthesis;
			}
			else if(expression[i].str == ",")
			{	break_with_error("Unexpected comma"); }
			else
			{	break_with_error("Unexpected token"); }
		} break;
		case ItemType::VARIABLE:
		case ItemType::USER_FIELD:
		case ItemType::NUMBER:
		case ItemType::EVENT_ID:
		{
			breaking_error(root.item.type != ItemType::_INVALID, "Invalid expression tree!");
			root.item = expression[i];
		} break;
		case ItemType::ACCESSOR:
		case ItemType::BINARY_OPERATOR:
		{
			u32 end_rhs = getIdOfNextTokenBeforeGreaterPrecedence(expression, expression[i], i+1,idend);
			
			Node lhs = root;
			Node rhs = convertExpressionToTree(expression, i+1, end_rhs);

			breaking_error(lhs.item.type == ItemType::_INVALID, "Invalid expression tree: binary operators expects a left hand side");
			breaking_error(rhs.item.type == ItemType::_INVALID, "Invalid expression tree: binary operators expects a right hand side");

			root.item = expression[i];
			root.children.clear();
			root.children.push_back(lhs);
			root.children.push_back(rhs);

			i = end_rhs;
		} break;
		case ItemType::UNARY_OPERATOR:
		{
			breaking_error(root.item.type != ItemType::_INVALID, "Invalid expression tree");

			u32 end_rhs = getIdOfNextTokenBeforeGreaterPrecedence(expression, expression[i], i+1,idend);
			Node rhs = convertExpressionToTree(expression, i+1, end_rhs);

			breaking_error(rhs.item.type == ItemType::_INVALID, "Invalid expression tree: binary operators expects a right hand side");

			root.item = expression[i];
			root.children.push_back(rhs);

			i = end_rhs;
		} break;
		case ItemType::FUNCTION:
		{
			breaking_error(root.item.type != ItemType::_INVALID, "Invalid expression tree");

			root.item = expression[i];
			breaking_error(expression[i+1].str != "(", "Functions must be followed by an opening parenthesis");

			i32 parenthesis_depth = 1;
			u32 subexpr_start_id = i+2;
			u32 subexpr_end_id = i;
			for(u32 j = subexpr_start_id; j <= idend; ++j)
			{
				if(expression[j].str == "(") ++parenthesis_depth;
				if(expression[j].str == ")") --parenthesis_depth;
				if((parenthesis_depth == 0) || (parenthesis_depth == 1 && expression[j].str == ","))
				{
					subexpr_end_id = j;
					Node child = convertExpressionToTree(expression, subexpr_start_id, subexpr_end_id-1);
					root.children.push_back(child);
					subexpr_start_id = j+1;
				}				
			}
			breaking_error(parenthesis_depth > 0, "Invalid expression tree, missing closing parenthesis.");

			i = subexpr_end_id;

		} break;
		}
	}

	return root;
}

Pattern* convertTreeToPattern(Node const& node,
							  std::unordered_map<std::string, char>& current_variables,
							  std::unordered_map<std::string, u32>& event_ids)
{	
	Pattern *pattern = new Pattern();
	pattern->next = nullptr;
	pattern->data = 0;

	switch(node.item.type)
	{
	case ItemType::VARIABLE:
	{
		breaking_error(current_variables.count(node.item.str)==0, "Variable does not exist"); // Memory leak
		pattern->symbole = true;
		pattern->variable = true;
		pattern->name = current_variables[node.item.str];
	} break;
	case ItemType::USER_FIELD:
	{
		pattern->symbole = true;
		pattern->variable = false;
		for(u32 i = 0; i < global_app->data->personalities.size(); ++i)
		{
			GaugeInfo &gauge = global_app->data->personalities[i];
			if(gauge.name == node.item.str)
			{
				pattern->type = SymboleType::PERSONALITY_GAUGE;
				pattern->data = gauge.id;
				goto correct_user_field;
			}
		}

		for(u32 i = 0; i < global_app->data->interests.size(); ++i)
		{
			GaugeInfo &interest = global_app->data->interests[i];
			if(interest.name == node.item.str)
			{
				pattern->type = SymboleType::INTEREST_GAUGE;
				pattern->data = interest.id;
				goto correct_user_field;
			}
		}

		breaking_error(false, "User Field does not exist");
	correct_user_field:
		break;
	} break;
	case ItemType::NUMBER:
	{
		pattern->symbole = true;
		pattern->variable = false;
		pattern->type = SymboleType::NUMBER;

		r32 nb = 0;
		parseNumber(node.item.str, nb);
		pattern->data = *((u64*)(&nb));
	} break;
	case ItemType::EVENT_ID:
	{
		breaking_error(event_ids.count(node.item.str)==0, "Event id does not exist"); // Memory leak
		pattern->symbole = true;
		pattern->variable = false;
		pattern->type = SymboleType::EVENT;
		pattern->data = (u64)(event_ids[node.item.str]);

	} break;
	case ItemType::ACCESSOR:
	{
		breaking_error(node.children.size() != 2, "Accessor operator is a binary operator"); // Memory leak
		pattern->symbole = false;
		pattern->first = convertTreeToPattern(node.children[1], current_variables, event_ids);
		pattern->first->next = convertTreeToPattern(node.children[0], current_variables, event_ids);
	} break;
	case ItemType::BINARY_OPERATOR:
	{
		breaking_error(node.children.size() != 2, "Binary operator must have 2 children"); // Memory leak

		Pattern *first = new Pattern();
		first->next = nullptr;
		
		first->symbole = true;
		first->variable = false;
		first->data = 0;

		if(node.item.str == ">")
			first->type = SymboleType::CMP_GREATER;
		else if(node.item.str == "<")
			first->type = SymboleType::CMP_SMALLER;
		else if(node.item.str == ">=")
			first->type = SymboleType::CMP_GR_OR_EQ;
		else if(node.item.str == "<=")
			first->type = SymboleType::CMP_SM_OR_EQ;
		else if(node.item.str == "==")
			first->type = SymboleType::CMP_EQ;
		else if(node.item.str == "!=")
			first->type = SymboleType::CMP_NOT_EQ;
		else
		{ break_with_error("Unknown binary operator"); }

		first->next = convertTreeToPattern(node.children[0], current_variables, event_ids);
		first->next->next = convertTreeToPattern(node.children[1], current_variables, event_ids);

		pattern->symbole = false;
		pattern->first = first;
	} break;
	case ItemType::UNARY_OPERATOR:
	{
		breaking_error(node.children.size() != 1, "Binary operator must have 1 child"); // Memory leak

		Pattern *first = new Pattern();
		first->next = nullptr;
		
		first->symbole = true;
		first->variable = false;
		first->data = 0;

		if(node.item.str == "!")
			first->type = SymboleType::OP_NOT;
		else
		{ break_with_error("Unknown unary operator"); }

		first->next = convertTreeToPattern(node.children[0], current_variables, event_ids);

		pattern->symbole = false;
		pattern->first = first;
		
	} break;
	case ItemType::FUNCTION:
	{
		Pattern *first = new Pattern();
		first->next = nullptr;
		
		first->symbole = true;
		first->variable = false;
		first->data = 0;

		if(node.item.str == "event")
			first->type = SymboleType::EVENT_OCCURED;
		else
		{ break_with_error("Unknown function"); }

		Pattern *list_elem = first;
		for(u32 i = 0; i < node.children.size(); ++i)
		{
			Pattern *pat = convertTreeToPattern(node.children[i], current_variables, event_ids);
			list_elem->next = pat;
			list_elem = pat;
		}

		pattern->symbole = false;
		pattern->first = first;
	} break;
	InvalidDefaultCase;
	}
	
	return pattern;
}

void displayTree(Node const& node, std::string const& prefix)
{
	std::cout << prefix << "- " << node.item.str << "\n";
	std::string next_prefix = prefix + "  ";
	for(auto const& child : node.children)
		displayTree(child, next_prefix);
}

void importEvents(EventSystem *event_system)
{
	// TODO(Sam): Get all .txt files
	if(!importEventsFile(event_system, "data/events/exemple.txt"))
		std::cout << "Error";
}
