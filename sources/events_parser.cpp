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

const std::unordered_set<char> white_chars = {' ', '\t', '\r', '\b'};
const std::unordered_set<char> special_chars = {
	'{', '}', '[', ']', ':', '=', '(', ')', '"', '.',';','+','-',
	',','?','>','<','!', '\\', '\n'
};



bool parseString(std::vector<std::string> const& tokens, u32& idx, std::string& str, char terminator = '"');
bool parseVariableName(std::vector<std::string> const& tokens, u32& idx, std::string& str);

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

				// TODO(Sam): Que faire de l'event id
				std::cout << "EVENT ID: " << event_id << "\n";
		
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
				if(!parseVariableName(tokens, idx, variable))
					return false;

				// TODO(Sam): Faire qqch avec 'variable'
				std::cout << "- VARIABLE: " << variable << "\n";

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

				// TODO(Sam): Faire qqch avec 'str'
				std::cout << "- DESCRIPTION: \"" << str << "\"\n";

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

				// TODO(Sam): Les conds...
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

bool parseVariableName(std::vector<std::string> const& tokens, u32& idx, std::string& str)
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

	return true;
}

void importEvents(EventSystem *event_system)
{
	// TODO(Sam): Get all .txt files
	if(!importEventsFile(event_system, "data/events/exemple.txt"))
		std::cout << "Error";
}
