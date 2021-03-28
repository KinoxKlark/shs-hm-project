
/*
  - Peut etre on veut discocier l'evolution des perso (changement des jauges)
    des events, i.e. avoir un moteur d'event qui crée les actions et avoir un
	moteur de personnages qui modifie les persos en fonctions des events

	
	Exemple de conditions :
	A, B,
	creatif(A,<,.3)
	humour(A,>,.6)
	event(42)
 */

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <functional>

/*
  NOTE(Sam):
  - Le Pattern est propriétaire de 'first' et 'next'  
  - Un Fact possede un pattern
  - Une condition est un pattern
  - Une conclusion est un pattern (pour le moment)
 */

enum class SymboleType {
    // Must not be used
	NONE = 0, 

	// Inferrences
	PERE,
	FRERE,
	ONCLE,
	EVENT_OCCURED,

	// Compilables
	NUMBER,
	USER,
	EVENT,
	PERSONALITY_GAUGE,
	INTEREST_GAUGE,
	CMP_GREATER,
	OP_NOT, // TODO(Sam): Implement this!

	// Special Action
	_SELECT_EVENT,
};

struct Pattern {
	union {
		struct {
			// Symbole
			union {
				SymboleType type; // id de symbole !
				char name; // Nom de variable
			};
			u64 data;
			bool variable;
		};
		struct {
			// Pattern = liste
			Pattern *first;
		};
	};
	Pattern *next;
	bool symbole;

	Pattern() {
		memset(this, 0, sizeof(Pattern));
	}
	
	Pattern(Pattern const& p) {
		memcpy(this, &p, sizeof(Pattern));
		if(symbole)
		{
			//if(type == ???) data = new ???(*((*???)data));
		}
		if(!symbole && first)
			first = new Pattern(*first);
		if(next)
			next = new Pattern(*next);
	}

	~Pattern() {
		if(!symbole) delete first;
		else
		{
			//if(type == ???) delete (*???)data;
		}
		delete next;
	}

	
};

bool operator!=(Pattern const& pat1, Pattern const& pat2);

bool operator==(Pattern const& pat1, Pattern const& pat2)
{
	if(&pat1 == &pat2) return true;

	if(pat1.symbole != pat2.symbole) return false;
	if(pat1.symbole)
	{
		if(pat1.variable != pat2.variable) return false;
		if(pat1.variable)
		{
			if(pat1.name != pat2.name) return false;
		}
		else
		{
			if(pat1.type != pat2.type) return false;
			if(pat1.data != pat2.data) return false;
			// TODO(Sam): Special care must be taken if data is
			// a pointer !
		}
	}
	else
	{
		if(pat1.next && *(pat1.next) != *(pat2.next)) return false;
	}

	if(pat1.next == nullptr) return true;
	return *(pat1.next) == *(pat1.next);
}

inline
bool operator!=(Pattern const& pat1, Pattern const& pat2)
{
	bool result = !(pat1 == pat2);
	return result;
}

// DEBUG
std::string convert_pattern_to_string(Pattern *pattern);

struct Fact {

	u32 id;
	Pattern *pattern;
	bool pattern_proprio;

	// Debug
	std::string description;

	Fact() : description() {
		id = 0;
		pattern = nullptr;
		pattern_proprio = false;
	}

	Fact(Fact const& f) {
		id = f.id;
		pattern = f.pattern;
		pattern_proprio = false;

		description = f.description;
	}
	
	~Fact() {
		if(pattern_proprio)
			delete pattern;
	}
};

void fact_swap_property(Fact& f1, Fact& f2)
{
	bool tmp = f1.pattern_proprio;
	f1.pattern_proprio = f2.pattern_proprio;
	f2.pattern_proprio = tmp;
}

bool operator==(Fact f1, Fact f2)
{
	return f1.id == f2.id;
}

namespace std
{
	template<>
	struct hash<Fact> {
		std::size_t operator()(Fact const fact) const
		{
			return std::hash<u32>{}(fact.id);
		}
	};
}

struct EnvironementAssociation {
	char variable_name;
	// NOTE(Sam): Le Fact est proprio du pattern
	Pattern *datum;
};

struct Environement {
	std::vector<EnvironementAssociation> associations;
	std::unordered_set<u32> conditions_to_compile;
};

struct ConditionWithEnvironement {
	// On va peut etre plutot se trimbaler l'id de la condition dans le tableau des conditions de Rule
	u32 condition;
	Environement environement;
};

struct Rule {
	std::vector<Pattern> conditions;
	Pattern conclusion;
};

struct Event {
	u32 id;
	std::vector<u32> users;
	u64 timestamp;
	
	// Debug
	std::string description;
};


struct EventSystem {
	std::vector<Rule> rules;
	std::unordered_map<Fact, Fact> facts;
	u32 fact_next_id;

	std::vector<Event> all_events;
	std::vector<Event> selected_events;
};
