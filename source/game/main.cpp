#include "engine/Game.hpp"

using namespace std;
using namespace sf;

class TestGame : public Game
{
public:
	TestGame() : _main_open(false)
	{ }

	virtual void init(int argc, char** argv) override
	{
		Game::init(argc, argv);
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

		ImGui::Begin("Main", &_main_open, ImGuiWindowFlags_NoResize);
		ImGui::Text("Hello, world!");
		if (ImGui::Button("Exit"))
			quit(0);
		ImGui::End();
	}

	virtual void frame_end() override
	{
		Game::frame_end();
	}

private:
	bool _main_open;
};

int main(int argc, char** argv)
{ return run<TestGame>(argc, argv); }
