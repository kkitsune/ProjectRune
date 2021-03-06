#include <iostream>

#include "engine/Game.hpp"
#include "engine/sol.hpp"

#include "engine/ginseng.hpp"

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
	ginseng::Database<> _db;
	bool _main_open;
};

class TestGame : public Game
{
public:
	TestGame() : _stack(this)
	{
		_lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::math, sol::lib::string, sol::lib::table);
	}

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

	GameStateStack& stack()
	{ return _stack; }

	sol::state& lua()
	{ return _lua; }

private:
	sol::state _lua;
	GameStateStack _stack;
};

int main(int argc, char** argv)
{ return run<TestGame>(argc, argv); }

struct Position
{
	Position(Vector2f const& p) : position(p)
	{ }

	Vector2f position;
};

void TestState::init()
{
	game<TestGame>().lua().script("print('Hello Lua!')");
	auto e = _db.create_entity();
	_db.create_component(e, Position{Vector2f{100, 100}});

	_db.visit([](Position const& p)
	          {
		          cout << "Entity with position (" << p.position.x << ", " << p.position.y << ") visited" << endl;
	          });
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
