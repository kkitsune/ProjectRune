#include <SFML/Graphics.hpp>

#include <iostream>

using namespace std;
using namespace sf;

int main(int /*argc*/, char* /*argv*/[])
{
	RenderWindow app(VideoMode(960, 640), "ProjectRune - Game", Style::Titlebar | Style::Close);
	app.setVerticalSyncEnabled(true);

	while(app.isOpen())
	{
		Event e;
		while(app.pollEvent(e))
		{
			if(e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
				app.close();
		}

		app.clear(Color::Black);

		app.display();
	}

	return 0;
}
