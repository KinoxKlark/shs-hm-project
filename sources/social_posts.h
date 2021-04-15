
struct Modifs;

struct SocialPost {
	u32 id;
	
	// NOTE(Sam): Id of raw event, not instanciated event. Thus can't be used to access
	// info about users, maybe this should be changed?
	u32 event_id;
	u32 social_post_id;

	std::string text;

	std::vector<u32> major_user_ids;
	std::vector<Modifs> users_modifs;
	
    // TODO(Sam): faire un vrai system de types
	std::string type;
	sf::Color color;	
};

struct SocialFeed {
	u32 user_id;
	std::vector<SocialPost> posts;
};

struct SocialPostSystem {
	std::vector<SocialPost> all_posts;
	
	std::vector<SocialPost> available_posts;
	std::vector<SocialFeed> social_feeds;
};

	
void instanciate_social_post_for_event(Application *app, Event *event);
