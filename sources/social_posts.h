
struct SocialPost {
	u32 id;
	u32 event_id;

	std::string text;

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
