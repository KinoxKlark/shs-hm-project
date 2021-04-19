
enum class ModifType {
	PERSONALITY,
	INTEREST,
	RELATION,
};

struct Modif {
	ModifType type;
	u32 gauge_id;
	i8 nudge_amount;
};

struct Modifs {
	u32 user_id;
	std::vector<Modif> modifs;
};

struct GaugeInfo {
	u32 id;
	std::string name;
};

struct UserGauge {
	u32 id;
	r32 amount;
};

struct UserIdentity {
	std::vector<UserGauge> personalities;
	std::vector<UserGauge> interests;
	std::vector<r32> relations;
};

struct User {
	u32 id;
	UserIdentity identity;
	bool isMan;
	std::string fullname;
};

struct GameData {
	EventSystem event_system;
	SocialPostSystem social_post_system;
	
	std::vector<User> users;
	std::vector<GaugeInfo> personalities;
	std::vector<GaugeInfo> interests;

	u32 click_counter;
	u32 drop_counter;

};

inline GameData* game_data_init();
inline void game_data_shutdown(GameData *data);
