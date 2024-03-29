
struct User;

void user_score_up(User *user, r32 dx);
void user_score_down(User *user, r32 dx);

void user_react_to_modifs(std::vector<Modifs> *users_modifs);
void user_see_post(EventSystem *event_system, SocialFeed *feed, SocialPost *post);
