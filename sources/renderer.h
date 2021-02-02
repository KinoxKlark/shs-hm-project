#include <vector>

#include "HTMLText.cpp"

struct DebugRect {
	rect bounds;
	sf::Color color;
};

struct Renderer
{
	sf::RenderWindow *window;
	sf::View view;

	std::vector<DebugRect> debug_rects;
};

void render(Renderer *renderer);
Renderer* renderer_init(Application *app);
void renderer_shutdown(Renderer *renderer);

