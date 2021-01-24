
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

	ImGui::SFML::Init(window);
	
	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			if(event.type == sf::Event::Closed)
				window.close();
		}

		ImGui::SFML::Update(window, update_clock.restart());

		window.clear();

		ImGui::ShowDemoWindow();
		ImGui::Begin("Test");
		ImGui::Text("Yolo");
		ImGui::End();

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	
    return 0;
}
