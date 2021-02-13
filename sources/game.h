
struct SocialPost {
	u32 id;
	sf::Color color;
};

struct SocialFeed {
	std::vector<SocialPost> posts;
};

struct GaugeInfo {
	u32 id;
	std::string name;
};

struct UserGauge {
	u32 id;
	float amount;
};

struct UserIdentity {
	std::vector<UserGauge> personalities;
	std::vector<UserGauge> interests;
};

struct User {
	u32 id;
	UserIdentity identity;
};

struct GameData {
	std::vector<User> users;
	std::vector<GaugeInfo> personalities;
	std::vector<GaugeInfo> interests;

	u32 click_counter;
	u32 drop_counter;

	std::vector<SocialPost> available_posts;
	std::vector<SocialFeed> social_feeds;
};

inline GameData* game_data_init();
inline void game_data_shutdown(GameData *data);
