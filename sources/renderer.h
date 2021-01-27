
#define GUI_STRETCH -1

enum class GuiCmdType {
	BEGIN_CONTAINER,
	END_CONTAINER
};

struct GuiObject
{
	v2 size;
	v4 margin;
	v4 padding;
	sf::Color bg_color;
};

struct GuiCmd {
	GuiCmdType type;
	GuiObject obj;
};

struct Renderer
{
	sf::RenderWindow *window;
	sf::View view;

	std::vector<GuiCmd> gui_cmd;
};

void render(Renderer *renderer);
Renderer* renderer_init(Application *app);
void renderer_shutdown(Renderer *renderer);

inline
void GuiPushCmd(Renderer* renderer, GuiCmdType type, GuiObject obj)
{
	GuiCmd cmd = {type, obj};
	renderer->gui_cmd.push_back(cmd);
}

inline
void GuiClearCmd(Renderer* renderer)
{
	renderer->gui_cmd.clear();
}

inline
void GuiBeginContainer(Renderer *renderer, GuiObject obj)
{
	GuiPushCmd(renderer, GuiCmdType::BEGIN_CONTAINER, obj);
}

inline
void GuiEndContainer(Renderer *renderer)
{
	GuiPushCmd(renderer, GuiCmdType::END_CONTAINER, {});
}
