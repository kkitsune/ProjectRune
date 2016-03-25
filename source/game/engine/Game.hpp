#pragma once

#include <type_traits>
#include <functional>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>
#include "imgui/imgui.h"

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

class GameState
{
	friend class GameStateStack;

public:
	GameState() : _game(nullptr)
	{ }

	virtual ~GameState()
	{ }

	GameState(GameState const& other) = delete;

	GameState(GameState&& other) = delete;

	GameState& operator=(GameState const& other) = delete;

	GameState& operator=(GameState&& other) = delete;

	virtual void init()
	{ }

	virtual void load_resources()
	{ }

	virtual void unload_resources()
	{ }

	virtual void update(Seconds)
	{ }

	virtual void render()
	{ }

	virtual void on_pause()
	{ }

	virtual void on_resume()
	{ }

protected:
	template<class GameType>
	GameType& game()
	{
		static_assert(std::is_base_of<Game, GameType>::value, "GameType must be a Game");
		return *dynamic_cast<GameType*>(_game);
	}

	template<class GameType>
	Game const& game() const
	{
		static_assert(std::is_base_of<Game, GameType>::value, "GameType must be a Game");
		return *dynamic_cast<GameType*>(_game);
	}

private:
	Game* _game;
};

/// \brief An enumeration used for a stack
///
/// This describes different ways to push
/// a game state on a stack
enum class PushType
{
	/// Pushes on a new GameState,
	/// without popping off the previous GameState
	/// Acts like a regular stack push operation
	PushWithoutPopping,

	/// Pushes on a new GameState, and pops off the
	/// previous GameState that was pushed on the stack
	/// (if there is a previous GameState).
	PushAndPop,

	/// Pushes on a new GameState,
	/// and pops off all previous states on the stack
	PushAndPopAllPreviousStates,

	/// Pushes on a new GameState, without popping off the last GameState
	/// Unlike `PushWithoutPopping`, this enumeration makes it so that
	/// the previous GameState is still updated, whilst the new GameState is updated
	PushWithoutPoppingSilenty
};

class GameStateStackListener
{
public:
	virtual ~GameStateStackListener()
	{ }

	virtual void on_gamestate_will_be_pushed(class GameStateStack& sender, GameState& state) = 0;

	virtual void on_gamestate_was_pushed(class GameStateStack& sender, GameState& state) = 0;

	virtual void on_gamestate_will_be_removed(class GameStateStack& sender, GameState& state) = 0;

	virtual void on_stack_will_be_popped(class GameStateStack& sender) = 0;

	virtual void on_stack_will_be_cleared(class GameStateStack& sender) = 0;
};

class GameStateStack
{
public:
	GameStateStack(Game* game) : _game(game)
	{ }

	~GameStateStack()
	{ clear(); }

	GameStateStack(GameStateStack const& other) = default;

	GameStateStack(GameStateStack&& other) = default;

	GameStateStack& operator=(GameStateStack const& other) = default;

	GameStateStack& operator=(GameStateStack&& other) = default;

	template<class StateT, PushType Push, class... Args>
	void push(Args&&... args)
	{
		push(new StateT{std::forward<Args>(args)...}, Push);
	}

	void push(GameState* state, PushType pushType = PushType::PushWithoutPopping);

	void pop();

	void update(Seconds delta_time);

	void render();

	void clear();

	void remove(GameState* state);

	Game& game() const { return *_game; }

	void add_listener(GameStateStackListener* listener);

	void remove_listener(GameStateStackListener* listener);

private:
	template <typename F>
	void perform_f_on_stack(F f)
	{
		// we're going to loop through the stack backwards
		// if the top is silently pushed on, we will iterate again
		for(size_t i = _stack.size(); i-- > 0;)
		{
			f(_stack[i].first.get());

			// if we no longer need to continue to iterate
			if(_stack[i].second != PushType::PushWithoutPoppingSilenty)
				break;
		}
	}

	struct GameStateDeleter
	{
		void operator()(GameState* state) const
		{
			state->unload_resources();
			delete state;
		}
	};

	typedef std::unique_ptr<GameState, GameStateDeleter> GameStatePtrImpl;
	typedef std::pair<GameStatePtrImpl, PushType> GameStatePair;
	typedef std::vector<GameStatePair> StackImpl;
	typedef std::vector<GameStateStackListener*> ListenerArray;

	ListenerArray _listeners;
	StackImpl _stack;
	Game* _game;
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
