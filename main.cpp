///Includes
#include <SFML/Graphics.hpp>
#include <cstdlib> //for rand
#include <ctime> //for seeding rand

#include "gameSys.hpp"
#include "gameParams.hpp"
#include <tuple>

int test;

int main () {
	srand(time(0));

	//create the window
	sf::RenderWindow window(sf::VideoMode({Params::gameW, Params::gameH}), "Space Invaders");
	window.setVerticalSyncEnabled(true);

    //initialise and load
	GameSys::init();

	while (window.isOpen())
	{
		//process window events
      	sf::Event event;
      	while (window.pollEvent(event))
	  	{
      		if (event.type == sf::Event::Closed)
			{
        		window.close();
      		}
    	}

		//Calculate dt
		static sf::Clock clock;
		const float dt = clock.restart().asSeconds();
		window.clear();
		GameSys::update(dt);
		GameSys::render(window);
		window.display();
	}

	//Unload and shutdown
	GameSys::clean();
}