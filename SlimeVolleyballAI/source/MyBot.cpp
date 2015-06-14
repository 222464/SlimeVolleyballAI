#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <iostream>
#include <random>

int main() {
	sf::VideoMode videoMode(1280, 720);

	std::cout << "Enter IP to connect to: ";

	std::string ip;

	std::cin >> ip;

	std::cout << "Enter port: ";

	std::string portstr;

	std::cin >> portstr;

	unsigned short port = std::stoi(portstr);

	sf::TcpSocket socket;

	sf::IpAddress address = ip;

	socket.connect(address, port, sf::seconds(180));

	// Receive startup packet
	sf::Packet startupPacket;

	socket.receive(startupPacket);

	bool isBlue;
	bool greyscale;
	sf::Int32 width;
	sf::Int32 height;

	startupPacket >> isBlue >> greyscale >> width >> height;
	
	sf::RenderWindow renderWindow(videoMode, "MyBot");

	bool quit = false;

	do {
		sf::Event event;

		while (renderWindow.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			quit = true;

		// Receive data
		sf::Packet packet;

		socket.receive(packet);

		// Send data
		bool moveLeft;
		bool moveRight;
		bool jump;

		moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
		moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
		jump = sf::Keyboard::isKeyPressed(sf::Keyboard::W);

		sf::Packet sendData;

		sendData << moveLeft << moveRight << jump;

		socket.send(sendData);

		renderWindow.clear();

		// Draw received data
		sf::Image img;
		img.create(width, height);

		if (greyscale) {
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++) {
					sf::Color c;

					sf::Uint8 g;

					packet >> g;

					c.r = c.g = c.b = g;

					c.a = 255;

					img.setPixel(x, y, c);
				}
		}
		else {
			for (int x = 0; x < width; x++)
				for (int y = 0; y < height; y++) {
					sf::Color c;

					packet >> c.r >> c.g >> c.b;

					c.a = 255;

					img.setPixel(x, y, c);
				}
		}

		sf::Texture t;

		t.loadFromImage(img);

		sf::Sprite s;

		s.setTexture(t);

		renderWindow.draw(s);

		renderWindow.display();

	} while (!quit);

	return 0;
}