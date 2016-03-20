#pragma once

#include <type_traits>

#include <SFML/Graphics.hpp>
#include "../imgui/imgui.h"

#include "Time.hpp"

struct Game
{
public:
	Game();

	virtual ~Game();

	Game(Game const& other) = delete;

	Game(Game&& other) = delete;

	Game& operator=(Game const& other) = delete;

	Game& operator=(Game&& other) = delete;

	virtual int error_state() const final
	{ return _error_state; }

	virtual bool is_running() const final
	{ return _is_running; }

	virtual void quit(int errorCode = 0) final;

	virtual void init(int argc, char** argv);

	virtual void frame_start();

	virtual void process_event(sf::Event& e);

	virtual void update(Seconds delta_time);

	virtual void frame_end();

private:
	sf::RenderWindow _window;
	int _error_state;
	bool _is_running;
};

template<class GameType>
inline int run(int argc, char** argv)
{
	static_assert(std::is_base_of<Game, GameType>::value, "GameType must be a Game");
	GameType app;
	app.init(argc, argv);

	Seconds last_time = time_now();
	while (app.is_running())
	{
		Seconds current = time_now();
		Seconds frame_time = current - last_time;
		last_time = current;

		app.frame_start();
		app.update(frame_time);
		app.frame_end();
	}

	return app.error_state();
}
