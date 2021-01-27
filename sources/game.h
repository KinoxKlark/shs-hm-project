
struct GameData {
	std::vector<int> users;
	std::vector<int> users_duration;
	int next_user_duration;
};

inline GameData* game_data_init();
inline void game_data_shutdown(GameData *data);
