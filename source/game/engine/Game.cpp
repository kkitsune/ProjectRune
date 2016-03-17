#include "Game.hpp"

#include "../imgui/sfml-rendering.h"
#include "../imgui/sfml-events.h"

using namespace sf;

Game::Game() : _error_state(0), _is_running(true)
{
}

void Game::quit(int errorCode)
{
	if (!is_running()) return;

	_error_state = errorCode;
	_is_running = false;
	_window.close();
	ImGui::SFML::Shutdown();
}

void Game::init(int, char**)
{
	_window.create({960, 640}, "ProjectRune - Game", Style::Titlebar | Style::Close);
	_window.setVerticalSyncEnabled(true);

	ImGui::SFML::SetRenderTarget(_window);
	ImGui::SFML::InitImGuiRendering();
	ImGui::SFML::SetWindow(_window);
	ImGui::SFML::InitImGuiEvents();
}

void Game::frame_start()
{
	ImGui::SFML::UpdateImGui();
	ImGui::SFML::UpdateImGuiRendering();

	Event e;
	while (_window.pollEvent(e))
	{
		ImGui::SFML::ProcessEvent(e);
		process_event(e);
	}
}

void Game::process_event(Event& e)
{
	if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
		quit();
	if (e.type == Event::Closed)
		quit();
}

void Game::update(Seconds)
{
	_window.clear(Color(32, 32, 32));
}

void Game::frame_end()
{
	ImGui::Render();
	_window.display();
}
