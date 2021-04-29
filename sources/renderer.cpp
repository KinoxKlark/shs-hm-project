#include <stack>

class RoundedRectangleShape : public sf::Shape
{
	v2 size;
	std::vector<v2> points;
	v4 radius;

	void updateGeometry()
		{
			u32 nb_points = points.size()/4;

			points.clear();

			r32 width = size.x;
			r32 height = size.y;

			float X=0,Y=0;
			for(int i=0; i<nb_points; i++)
			{
				Y+=radius[0]/nb_points;
				X=sqrt(radius[0]*radius[0]-Y*Y+1e-5);
				points.push_back({radius[0]-X,radius[0]-Y});
			} // 0,0
			
			X=0;
			for(int i=0; i<nb_points; i++)
			{
				X+=radius[1]/nb_points;
				Y=sqrt(radius[1]*radius[1]-X*X);
				points.push_back({width+X-radius[1],radius[1]-Y});
			} // width, 0
			
			Y=0;
			for(int i=0; i<nb_points; i++)
			{
				Y+=radius[2]/nb_points;
				X=sqrt(radius[2]*radius[2]-Y*Y+1e-5);
				points.push_back({width+X-radius[2],height+Y-radius[2]});
			} // width, height
			X=0;
			for(int i=0; i<nb_points; i++)
			{
				X+=radius[3]/nb_points;
				Y=sqrt(radius[3]*radius[3]-X*X+1e-5);
				points.push_back({radius[3]-X,height+Y-radius[3]});
			}  // 0, height
			
			update();
		}
	
	public:

	RoundedRectangleShape(v4 r = {}, u32 np = 10) : points(4*np), radius(r), size({r[0]+r[2],r[1]+r[3]}) 
		{
			updateGeometry();
		}

	v2 getSize() const { return size; }
	void setSize(v2 s) { size = s; updateGeometry(); }
	
	v4 getRadius() const { return radius; }
	void setRadius(v4 r) { radius = r; updateGeometry(); }
	
	std::size_t getPointCount() const override
		{
			return points.size();
		}
	
	sf::Vector2f getPoint(std::size_t index) const override
		{
			return points[index];
		}
};

void render(Renderer *renderer)
{
	v2u render_region = renderer->window->getSize();
	renderer->view = sf::View(rect(0.f, 0.f, render_region.x, render_region.y));
	renderer->window->setView(renderer->view);

	renderer->window->clear();

	GuiManager *gui = global_gui_manager;

	struct viewport_info {
		u32 last_elem_id;
		sf::View view;
	};

	std::stack<viewport_info> viewport_infos;
	viewport_infos.push({ gui->elements_count, renderer->view });

	//RoundedRectangleShape rect_shape;
	RoundedRectangleShape rect_shape;
	for(u32 idx(0); idx < gui->elements_count; ++idx)
	{
		GuiElement *element = &gui->elements[idx];

		while(idx == viewport_infos.top().last_elem_id)
		{
			viewport_infos.pop();
			renderer->window->setView(viewport_infos.top().view);
		}
		
		if(!element->render) continue;

		if(element->set_viewport)
		{
			sf::View view;
			rect viewport;
			viewport_infos.top().view.getViewport().intersects({
					element->bounds.left / render_region.x,
					element->bounds.top / render_region.y,
					element->bounds.width / render_region.x,
					element->bounds.height / render_region.y }, viewport);

			view.reset(element->bounds);
			view.setViewport(viewport);
			renderer->window->setView(view);

			viewport_infos.push({element->container_one_past_last, view});
		}

		if(element->always_show)
			renderer->window->setView(renderer->view);

		rect_shape.setSize(rect_size(element->bounds));
		rect_shape.setPosition(rect_pos(element->bounds));
		rect_shape.setFillColor(element->obj.bg_color);
		rect_shape.setRadius((r32)(gui->current_size.y)*element->obj.box_radius);
		rect_shape.setOutlineColor(element->obj.border_color);
		rect_shape.setOutlineThickness(element->obj.border_width*gui->current_size.y);
			
		renderer->window->draw(rect_shape);

		
		if(element->obj.text.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.text.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.text.setPosition(vec2_round_to_int(rect_pos(element->inner_bounds) + offset));
			renderer->window->draw(element->obj.text);
		}

		if(element->obj.htmltext.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.htmltext.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.htmltext.setPosition(vec2_round_to_int(rect_pos(element->inner_bounds) + offset));
			renderer->window->draw(element->obj.htmltext);
		}
		
		if(element->always_show)
			renderer->window->setView(viewport_infos.top().view);
	}

	while(viewport_infos.size() > 0 && gui->elements_count == viewport_infos.top().last_elem_id)
	{
		viewport_infos.pop();
		if(viewport_infos.size() > 0)
			renderer->window->setView(viewport_infos.top().view);
	}
	
	assert(viewport_infos.size() == 0);

	for(auto const& sprite : gui->sprites)
	{
		renderer->window->draw(sprite);
	}
	
	viewport_infos.push({ gui->elements_count, renderer->view });
	
	// Drag and Drop Payload
	for(u32 idx = 0; idx < gui->dragging_payload_size; ++idx)
	{
		GuiElement *element = &gui->elements[gui->dragging_payload + idx];

		while(idx == viewport_infos.top().last_elem_id)
		{
			viewport_infos.pop();
			renderer->window->setView(viewport_infos.top().view);
		}
		
		if(element->set_viewport)
		{
			sf::View view;
			rect viewport;
			viewport_infos.top().view.getViewport().intersects({
					element->bounds.left / render_region.x,
					element->bounds.top / render_region.y,
					element->bounds.width / render_region.x,
					element->bounds.height / render_region.y }, viewport);

			view.reset(element->bounds);
			view.setViewport(viewport);
			renderer->window->setView(view);

			viewport_infos.push({element->container_one_past_last, view});
		}

		if(element->always_show)
			renderer->window->setView(renderer->view);

		rect_shape.setSize(rect_size(element->bounds));
		rect_shape.setPosition(rect_pos(element->bounds));
		rect_shape.setFillColor(element->obj.bg_color);
		rect_shape.setRadius((r32)(gui->current_size.y)*element->obj.box_radius);
		rect_shape.setOutlineColor(element->obj.border_color);
		rect_shape.setOutlineThickness(element->obj.border_width*gui->current_size.y);			
		renderer->window->draw(rect_shape);

		if(element->obj.text.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.text.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.text.setPosition(vec2_round_to_int(rect_pos(element->inner_bounds) + offset));
			renderer->window->draw(element->obj.text);
		}

		if(element->obj.htmltext.getString().getSize() > 0)
		{
			rect ref_bounds = global_gui_manager
				->font.getGlyph(L'A', element->obj.htmltext.getCharacterSize(), false).bounds;
			v2 offset = v2(0, .5*ref_bounds.top);
			element->obj.htmltext.setPosition(vec2_round_to_int(rect_pos(element->inner_bounds) + offset));
			renderer->window->draw(element->obj.htmltext);
		}
		
		if(element->always_show)
			renderer->window->setView(viewport_infos.top().view);
	}


	// Debug
#ifdef DEBUG
	for(u32 idx = 0; idx < renderer->debug_rects.size(); ++idx)
	{
		DebugRect *rect = &renderer->debug_rects[idx];
		rect_shape.setSize(rect_size(rect->bounds));
		rect_shape.setPosition(rect_pos(rect->bounds));
		rect_shape.setFillColor(rect->color);
		renderer->window->draw(rect_shape);
	}
	renderer->debug_rects.clear();
#endif

#ifdef DEBUG
	ImGui::SFML::Render(*renderer->window);
#endif
	
	renderer->window->display();
}

Renderer* renderer_init(Application *app)
{
	Renderer *renderer = new Renderer();
	renderer->window = app->window;
	renderer->view = sf::View(sf::FloatRect(0.f, 0.f, app->window->getSize().x, app->window->getSize().y));
	global_renderer = renderer;
	
#ifdef DEBUG
	ImGui::SFML::Init(*app->window);
#endif

	return renderer;
}

void renderer_shutdown(Renderer *renderer)
{
#ifdef DEBUG
	ImGui::SFML::Shutdown();
#endif

	delete renderer;
	global_renderer = nullptr;
}

