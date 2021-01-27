
struct GameData {
	std::vector<i32> users;
	std::vector<i32> users_duration;
	i32 next_user_duration;
};

inline GameData* game_data_init();
inline void game_data_shutdown(GameData *data);
