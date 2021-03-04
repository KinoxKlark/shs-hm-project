
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

struct Fact {
	u32 id;

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

struct Condition {
	// Possède un pattern ?
};

struct Environement {
	// Associe un fait a une variable
};

struct ConditionWithEnvironement {
	// On va peut etre plutot se trimbaler l'id de la condition dans le tableau des conditions de Rule
	u32 condition;
	Environement environement;
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
