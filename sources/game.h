
struct SocialPost {
	u32 id;
	sf::Color color;
};

struct SocialFeed {
	std::vector<SocialPost> posts;
};

struct GameData {
	std::vector<i32> users;
	std::vector<i32> users_duration;
	i32 next_user_duration;
	u32 click_counter;
	u32 drop_counter;

	std::vector<SocialPost> available_posts;
	std::vector<SocialFeed> social_feeds;
};

inline GameData* game_data_init();
inline void game_data_shutdown(GameData *data);
