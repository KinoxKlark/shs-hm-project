
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

#include <unordered_set>
#include <queue>
#include <vector>
#include <functional>

/*
  NOTE(Sam):
  - Le Pattern est propriétaire de 'first' et 'next'
  - TODO Est-ce qu'on veux un "pattern manager" qui soit proprio
         des patterns ROOT ?
  
  - Le Fact possède 1 pattern
 */

struct Pattern {
	union {
		struct {
			// Symbole
			union {
				u32 id; // id de symbole !
				char name; // Nom de variable
			};
			bool variable;
		};
		struct {
			// Pattern = liste
			Pattern *first;
		};
	};
	Pattern *next;
	bool symbole;

	~Pattern() {
		if(!symbole) delete first;
		delete next;
	}
};

bool operator==(Pattern const& pat1, Pattern const& pat2)
{
	if(&pat1 == &pat2) return true;

   // TODO(Sam): On veut pouvoir comparer 2 patterns qui ne sont pas
   // au meme endroit en mémoire
	
	bool cmp = memcmp(&part1, &part2, sizeof(Pattern)) == 0;

	if(pat1.next == nullptr) return true;
	return *(pat1->next) == *(pat1->next);
}

struct Fact {
	u32 id;

	// TODO(Sam): Attention aux copies de Fact !
	Pattern pattern;

	// Un fact est un pattern ?
	
	// Debug
	std::string description;
};

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
	// TODO(Sam): Qui est propriétaire des Patterns ?
	Pattern *datum
};

struct Environement {
	std::vector<EnvironementAssociation> associations;
};

struct ConditionWithEnvironement {
	// On va peut etre plutot se trimbaler l'id de la condition dans le tableau des conditions de Rule
	u32 condition;
	Environement environement;
};

struct Condition {
	// Possède un pattern ?
	// TODO(Sam): Une condition est un pattern ? Donc on pourrait pas dire que
	// le proprio du pattern c'est la règle ?
};

struct Rule {
	std::vector<Fact> conditions;
	Fact conclusion;
};

struct Event {
	u32 id;

	// Debug
	std::string description;
};

struct EventSystem {
	std::vector<Rule> rules;
	std::unordered_set<Fact> facts;
};
