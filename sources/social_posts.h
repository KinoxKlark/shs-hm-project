
struct Modifs;

enum class ImageType {
	NONE, // TODO(Sam): Get some images here!		
};

enum class PostType {
	NONE,
	PUBLICATION,
	PARTAGE,
	PHOTO,
	ARTICLE,
	PUB,
	LOCALISATION,
};

struct SocialPost {
	u32 id;

	PostType type;
	u32 author_id;
	u32 receiver_id;
	ImageType image_type;
	sf::String article_origin;
	sf::String article_title;
	sf::String localisation;
	sf::String text;

	
	// NOTE(Sam): Id of raw event, not instanciated event. Thus can't be used to access
	// info about users, maybe this should be changed?
	u32 event_id;
	u32 social_post_id;
	bool free;
	
	std::vector<u32> major_user_ids;
	std::vector<Modifs> users_modifs;
	
    // TODO(Sam): faire un vrai system de types
	sf::Color color;	
};

struct SocialFeed {
	u32 user_id;
	std::vector<SocialPost> posts;
};

struct SocialPostSystem {
	std::vector<SocialPost> all_posts;
	std::vector<SocialPost> starter_posts;
	
	std::vector<SocialPost> available_posts;
	std::vector<SocialFeed> social_feeds;
};

	
void instanciate_social_post_for_event(Application *app, Event *event);
void instanciate_starter_social_post_for_event(Application *app, Event *event);
