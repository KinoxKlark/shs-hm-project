
struct Renderer
{
	sf::RenderWindow *window;
	sf::View camera;
};

void render(Renderer *renderer);
Renderer* renderer_init(Application *app);
void renderer_shutdown(Renderer *renderer);
		
