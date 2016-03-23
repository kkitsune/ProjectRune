#include "engine/selene/selene.h"
#include "engine/Game.hpp"

using namespace std;
using namespace sf;

class TestState : public GameState
{
public:
	TestState() : _main_open(false)
	{ }

	virtual void init() override;

	virtual void load_resources() override;

	virtual void unload_resources() override;

	virtual void update(Seconds) override;

	virtual void render() override;

protected:
	bool _main_open;
};

class TestGame : public Game
{
public:
	TestGame() : _lua(true), _stack(this)
	{ }

	virtual void init(int argc, char** argv) override
	{
		Game::init(argc, argv);
		_stack.push<TestState, PushType::PushWithoutPopping>();
	}

	virtual void frame_start() override
	{
		Game::frame_start();
	}

	virtual void process_event(sf::Event& e) override
	{
		Game::process_event(e);
	}

	virtual void update(Seconds delta_time) override
	{
		Game::update(delta_time);
		_stack.update(delta_time);
	}

	virtual void frame_end() override
	{
		_stack.render();
		Game::frame_end();
	}

	sel::State& lua()
	{ return _lua; }

private:
	sel::State _lua;
	GameStateStack _stack;
};

int main(int argc, char** argv)
{ return run<TestGame>(argc, argv); }

void TestState::init()
{
	game<TestGame>().lua()("print('Hello Lua!')");
}

void TestState::load_resources()
{ }

void TestState::unload_resources()
{ }

void TestState::update(Seconds)
{
	ImGui::Begin("Main", &_main_open, ImGuiWindowFlags_NoResize);
	ImGui::Text("Hello, world!");
	if (ImGui::Button("Exit"))
		game<TestGame>().quit(0);
	ImGui::End();
}

void TestState::render()
{ }
