#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/sfml-rendering.h"
#include "imgui/sfml-events.h"

using namespace std;
using namespace sf;

int main(int, char**)
{
	RenderWindow app(VideoMode(960, 640), "ProjectRune - Game", Style::Titlebar | Style::Close);
	app.setVerticalSyncEnabled(true);

	ImGui::SFML::SetRenderTarget(app);
	ImGui::SFML::InitImGuiRendering();
	ImGui::SFML::SetWindow(app);
	ImGui::SFML::InitImGuiEvents();

	while(app.isOpen())
	{
		ImGui::SFML::UpdateImGui();
		ImGui::SFML::UpdateImGuiRendering();

		Event e;
		while(app.pollEvent(e))
		{
			ImGui::SFML::ProcessEvent(e);
			if(e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
				app.close();
		}

		bool tmp = true;
		ImGui::Begin("window", &tmp);
		ImGui::Text("Hello, world!");
		ImGui::Button("Is this working?!");
		ImGui::End();

		app.clear(Color(64, 64, 64));
		ImGui::Render();
		app.display();
	}
	
	ImGui::SFML::Shutdown();
	return 0;
}
