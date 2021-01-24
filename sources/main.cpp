
#include "imgui_includes.cpp"


#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    std::cout << "Hello World\n";

#ifdef DEBUG
	std::cout << "Debug\n";
#endif

	sf::RenderWindow window(sf::VideoMode(800,600), "Hello");

	sf::Clock update_clock;

#ifdef DEBUG
	ImGui::SFML::Init(window);
#endif
	
	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
#ifdef DEBUG
			ImGui::SFML::ProcessEvent(event);
#endif
			if(event.type == sf::Event::Closed)
				window.close();
		}

#ifdef DEBUG
		ImGui::SFML::Update(window, update_clock.restart());
#endif
		
		window.clear();

#ifdef DEBUG
		ImGui::ShowDemoWindow();
		ImGui::Begin("Test");
		ImGui::Text("Yolo");
		ImGui::End();
#endif

#ifdef DEBUG
		ImGui::SFML::Render(window);
#endif
		window.display();
	}

#ifdef DEBUG
	ImGui::SFML::Shutdown();
#endif
	
    return 0;
}
