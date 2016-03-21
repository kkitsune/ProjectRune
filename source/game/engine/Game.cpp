#include "Game.hpp"

#include "../imgui/sfml-rendering.h"
#include "../imgui/sfml-events.h"

using namespace sf;

Game::Game() : _error_state(0), _is_running(true)
{
}

Game::~Game()
{
	ImGui::SFML::Shutdown();
}

void Game::quit(int errorCode)
{
	if (!is_running()) return;

	_error_state = errorCode;
	_is_running = false;
	_window.close();
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

void GameStateStack::push(GameState* state, PushType pushType)
{
	assert(state && "GameState is null, please offer a non-null GameState");

	for(auto& listener : _listeners)
		listener->on_gamestate_will_be_pushed(*this, *state);

	switch(pushType)
	{
		case PushType::PushAndPop:
			pop();
			break;
		case PushType::PushAndPopAllPreviousStates:
			clear();
			break;
		default:
			break;
	}

	// if the stack isn't empty and we're not silently pushing
	// then tell the stack we're gonna pause everyone
	if(!_stack.empty() && pushType != PushType::PushWithoutPoppingSilenty)
		perform_f_on_stack([](GameState* state) { state->on_pause(); });

	_stack.emplace_back(GameStatePtrImpl{state}, pushType);
	state->_game = _game;

	state->load_resources();
	state->init();
	state->on_resume();
}

void GameStateStack::pop()
{
	if(_stack.empty()) return;

	for(auto& listener : _listeners)
		listener->on_stack_will_be_popped(*this);

	// if the last state was silent
	bool wasSilent = _stack.back().second == PushType::PushWithoutPoppingSilenty;
	_stack.pop_back();

	// call onResume on every other state in the stack that was on previous top
	if(!wasSilent)
		perform_f_on_stack([](GameState* state) { state->on_resume(); });
}

void GameStateStack::update(Seconds delta_time)
{
	perform_f_on_stack([&](GameState* state) { state->update(delta_time); });
}

void GameStateStack::render()
{
	perform_f_on_stack([](GameState* state) { state->render(); });
}

void GameStateStack::clear()
{
	for(auto& listener : _listeners)
		listener->on_stack_will_be_cleared(*this);

	_stack.clear();
}

void GameStateStack::remove(GameState* state)
{
	assert(state);
	auto elementToRemove = std::find_if(_stack.begin(), _stack.end(), [&](GameStatePair& p) { return p.first.get() == state; });

	if(elementToRemove == _stack.end())
		return;

	for(auto& listener : _listeners)
		listener->on_gamestate_will_be_removed(*this, *state);

	_stack.erase(elementToRemove);
}

void GameStateStack::add_listener(GameStateStackListener* listener)
{
	assert(listener);
	_listeners.push_back(listener);
}

void GameStateStack::remove_listener(GameStateStackListener* listener)
{
	assert(listener);
	_listeners.erase(std::remove(_listeners.begin(), _listeners.end(), listener), _listeners.end());
}
