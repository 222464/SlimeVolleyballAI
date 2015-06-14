#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <iostream>
#include <random>

struct PhyObj {
	sf::Vector2f _position;
	sf::Vector2f _velocity;

	PhyObj()
		: _position(0.0f, 0.0f), _velocity(0.0f, 0.0f)
	{}
};

int main(int argc, char* argv[]) {
	if (argc < 1) {
		std::cout << "Invalid number of commandline arguments." << std::endl;

		return 0;
	}

	sf::VideoMode videoMode(1280, 720);

	unsigned short port = 53000;

	int blueResolution = 8;
	int redResolution = 8;

	int blueWidth = blueResolution * static_cast<float>(videoMode.width) / static_cast<float>(videoMode.height);
	int redWidth = redResolution * static_cast<float>(videoMode.width) / static_cast<float>(videoMode.height);

	int gameFramerate = 60;
	int skipFrames = 3;

	bool greyscale = false;

	bool show = true;

	bool printData = true;

	for (int v = 1; v < argc; v++) {
		std::string argstr = argv[v];

		if (argstr == "--port") {
			if (v + 1 >= argc)
				std::cout << "--port: Port must be provided." << std::endl;

			port = std::atoi(argv[v++]);
		}

		if (argstr == "--show" || argstr == "-s") {
			std::cout << "--show/-s: Showing the game." << std::endl;

			show = true;
		}

		if (argstr == "--hide" || argstr == "-h") {
			std::cout << "--hide/-h: Hiding the game." << std::endl;

			show = false;
		}

		if (argstr == "--gameFramerate") {
			if (v + 1 >= argc)
				std::cout << "--gameFramerate: Framerate must be provided." << std::endl;

			gameFramerate = std::atoi(argv[v++]);

			std::cout << "--gameFramerate: Set game framerate to " << gameFramerate << "." << std::endl;
		}

		if (argstr == "--skipFrames") {
			if (v + 1 >= argc)
				std::cout << "--skipFrames: Skip frames must be provided." << std::endl;

			skipFrames = std::atoi(argv[v++]);

			std::cout << "--skipFrames: Set skip frames to " << skipFrames << "." << std::endl;
		}

		if (argstr == "--blueResolution") {
			if (v + 1 >= argc)
				std::cout << "--blueResolution: Resolution must be provided." << std::endl;

			blueResolution = std::atoi(argv[v++]);

			blueWidth = blueResolution * static_cast<float>(videoMode.width) / static_cast<float>(videoMode.height);

			std::cout << "--blueResolution: Set blue resolution to " << blueWidth << "x" << blueResolution << "." << std::endl;
		}

		if (argstr == "--redResolution") {
			if (v + 1 >= argc)
				std::cout << "--redResolution: Resolution must be provided." << std::endl;

			redResolution = std::atoi(argv[v++]);
			
			redWidth = redResolution * static_cast<float>(videoMode.width) / static_cast<float>(videoMode.height);

			std::cout << "--redResolution: Set red resolution to " << redWidth << "x" << redResolution << "." << std::endl;
		}

		if (argstr == "--printData") {
			std::cout << "--printData: Printing data." << std::endl;

			printData = true;
		}

		if (argstr == "--hideData") {
			std::cout << "--hideData: Do not print data." << std::endl;

			printData = false;
		}

		if (argstr == "--greyscale") {
			std::cout << "--greyscale: Greyscale bot input." << std::endl;

			greyscale = true;
		}

		if (argstr == "--colored") {
			std::cout << "--colored: Colored (RGB) bot input." << std::endl;

			greyscale = false;
		}
	}

	sf::TcpListener listener;
	sf::TcpSocket blueSocket;
	sf::TcpSocket redSocket;

	std::cout << "Waiting for connections..." << std::endl;

	listener.listen(port);

	listener.accept(blueSocket);
	listener.accept(redSocket);

	std::cout << "Sending startup data..." << std::endl;

	sf::Packet blueStartupPacket;
	blueStartupPacket << bool(true) << greyscale << sf::Int32(blueWidth) << sf::Int32(blueResolution);

	sf::Packet redStartupPacket;
	redStartupPacket << bool(false) << greyscale << sf::Int32(redWidth) << sf::Int32(redResolution);

	blueSocket.send(blueStartupPacket);
	redSocket.send(redStartupPacket);

	std::cout << "Ready to go!" << std::endl;

	std::mt19937 generator(time(nullptr));

	sf::RenderWindow renderWindow;

	renderWindow.create(videoMode, "Reinforcement Learning", sf::Style::Default);

	if (show)
		renderWindow.setVerticalSyncEnabled(true);

	renderWindow.setFramerateLimit(gameFramerate);

	// --------------------------------- Game Init -----------------------------------

	const float slimeRadius = 94.5f;
	const float ballRadius = 23.5f;
	const float wallRadius = 22.5f;
	const float fieldRadius = 640.0f;

	const float gravity = 900.0f;
	const float slimeBounce = 100.0f;
	const float wallBounceDecay = 0.8f;
	const float slimeJump = 500.0f;
	const float maxSlimeSpeed = 1000.0f;
	const float slimeMoveAccel = 5000.0f;
	const float slimeMoveDeccel = 8.0f;

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	sf::Vector2f fieldCenter = sf::Vector2f(renderWindow.getSize().x * 0.5f, renderWindow.getSize().y * 0.5f + 254.0f);
	sf::Vector2f wallCenter = fieldCenter + sf::Vector2f(0.0f, -182.0f);

	PhyObj blue;
	PhyObj red;
	PhyObj ball;

	blue._position = fieldCenter + sf::Vector2f(-200.0f, 0.0f);
	blue._velocity = sf::Vector2f(0.0f, 0.0f);
	red._position = fieldCenter + sf::Vector2f(200.0f, 0.0f);
	red._velocity = sf::Vector2f(0.0f, 0.0f);
	ball._position = fieldCenter + sf::Vector2f(2.0f, -300.0f);
	ball._velocity = sf::Vector2f((dist01(generator)) * 600.0f, -(dist01(generator)) * 500.0f);

	sf::Texture backgroundTexture;
	backgroundTexture.loadFromFile("resources/slimevolleyball/background.png");

	sf::Texture blueSlimeTexture;
	blueSlimeTexture.loadFromFile("resources/slimevolleyball/slimeBodyBlue.png");

	sf::Texture redSlimeTexture;
	redSlimeTexture.loadFromFile("resources/slimevolleyball/slimeBodyRed.png");

	sf::Texture ballTexture;
	ballTexture.loadFromFile("resources/slimevolleyball/ball.png");

	sf::Texture eyeTexture;
	eyeTexture.loadFromFile("resources/slimevolleyball/slimeEye.png");
	eyeTexture.setSmooth(true);

	sf::Texture arrowTexture;
	arrowTexture.loadFromFile("resources/slimevolleyball/arrow.png");

	sf::Font scoreFont;
	scoreFont.loadFromFile("resources/slimevolleyball/scoreFont.ttf");

	int scoreRed = 0;
	int scoreBlue = 0;

	int prevScoreRed = 0;
	int prevScoreBlue = 0;

	float prevBallX = fieldCenter.x;

	sf::RenderTexture blueRT;

	blueRT.create(blueWidth, blueResolution);

	sf::RenderTexture redRT;

	redRT.create(redWidth, redResolution);

	// ------------------------------- Simulation Loop -------------------------------

	bool quit = false;

	float dt = 0.017f;

	int skipFrameCounter = 0;

	sf::Packet redPacket;
	sf::Packet bluePacket;

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

		// ---------------------------------- Physics ----------------------------------

		bool blueBounced = false;
		bool redBounced = false;

		// Ball
		{
			ball._velocity.y += gravity * dt;
			ball._position += ball._velocity * dt;

			// To floor (game restart)
			if (ball._position.y + ballRadius > fieldCenter.y) {
				if (ball._position.x < fieldCenter.x)
					scoreRed++;
				else
					scoreBlue++;

				blue._position = fieldCenter + sf::Vector2f(-200.0f, 0.0f);
				blue._velocity = sf::Vector2f(0.0f, 0.0f);
				red._position = fieldCenter + sf::Vector2f(200.0f, 0.0f);
				red._velocity = sf::Vector2f(0.0f, 0.0f);
				ball._position = fieldCenter + sf::Vector2f(2.0f, -300.0f);
				ball._velocity = sf::Vector2f((dist01(generator)) * 600.0f, -(dist01(generator)) * 500.0f);
			}

			// To wall
			if (((ball._position.x + ballRadius) > (wallCenter.x - wallRadius) && ball._position.x < wallCenter.x) || ((ball._position.x - ballRadius) < (wallCenter.x + wallRadius) && ball._position.x > wallCenter.x)) {
				// If above rounded part
				if (ball._position.y < wallCenter.y) {
					sf::Vector2f delta = ball._position - wallCenter;

					float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

					if (dist < wallRadius + ballRadius) {
						sf::Vector2f normal = delta / dist;

						// Reflect velocity
						sf::Vector2f reflectedVelocity = ball._velocity - 2.0f * (ball._velocity.x * normal.x + ball._velocity.y * normal.y) * normal;

						ball._velocity = reflectedVelocity * wallBounceDecay;

						ball._position = wallCenter + normal * (wallRadius + ballRadius);
					}
				}
				else {
					// If on left side
					if (ball._position.x < wallCenter.x) {
						ball._velocity.x = wallBounceDecay * -ball._velocity.x;
						ball._position.x = wallCenter.x - wallRadius - ballRadius;
					}
					else {
						ball._velocity.x = wallBounceDecay * -ball._velocity.x;
						ball._position.x = wallCenter.x + wallRadius + ballRadius;
					}
				}
			}

			// To blue slime			
			{
				sf::Vector2f delta = ball._position - blue._position;

				float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

				if (dist < slimeRadius + ballRadius) {
					sf::Vector2f normal = delta / dist;

					// Reflect velocity
					sf::Vector2f reflectedVelocity = ball._velocity - 2.0f * (ball._velocity.x * normal.x + ball._velocity.y * normal.y) * normal;

					float magnitude = std::sqrt(reflectedVelocity.x * reflectedVelocity.x + reflectedVelocity.y * reflectedVelocity.y);

					sf::Vector2f normalizedReflected = reflectedVelocity / magnitude;

					ball._velocity = blue._velocity + (magnitude > slimeBounce ? reflectedVelocity : normalizedReflected * slimeBounce);

					ball._position = blue._position + normal * (wallRadius + slimeRadius);

					blueBounced = true;
				}
			}

			// To red slime			
			{
				sf::Vector2f delta = ball._position - red._position;

				float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

				if (dist < slimeRadius + ballRadius) {
					sf::Vector2f normal = delta / dist;

					// Reflect velocity
					sf::Vector2f reflectedVelocity = ball._velocity - 2.0f * (ball._velocity.x * normal.x + ball._velocity.y * normal.y) * normal;

					float magnitude = std::sqrt(reflectedVelocity.x * reflectedVelocity.x + reflectedVelocity.y * reflectedVelocity.y);

					sf::Vector2f normalizedReflected = reflectedVelocity / magnitude;

					ball._velocity = red._velocity + (magnitude > slimeBounce ? reflectedVelocity : normalizedReflected * slimeBounce);

					ball._position = red._position + normal * (wallRadius + slimeRadius);

					redBounced = true;
				}
			}

			// Out of field, left and right
			{
				if (ball._position.x - ballRadius < fieldCenter.x - fieldRadius) {
					ball._velocity.x = wallBounceDecay * -ball._velocity.x;
					ball._position.x = fieldCenter.x - fieldRadius + ballRadius;
				}
				else if (ball._position.x + ballRadius > fieldCenter.x + fieldRadius) {
					ball._velocity.x = wallBounceDecay * -ball._velocity.x;
					ball._position.x = fieldCenter.x + fieldRadius - ballRadius;
				}
			}
		}

		// Blue slime
		{		
			blue._velocity.y += gravity * dt;
			blue._velocity.x += -slimeMoveDeccel * blue._velocity.x * dt;
			blue._position += blue._velocity * dt;

			bool moveLeft;
			bool moveRight;
			bool jump;

			bluePacket >> moveLeft >> moveRight >> jump;

			if (moveLeft) {
				blue._velocity.x += -slimeMoveAccel * dt;

				if (blue._velocity.x < -maxSlimeSpeed)
					blue._velocity.x = -maxSlimeSpeed;
			}
			else if (moveRight) {
				blue._velocity.x += slimeMoveAccel * dt;

				if (blue._velocity.x > maxSlimeSpeed)
					blue._velocity.x = maxSlimeSpeed;
			}

			if (blue._position.y > fieldCenter.y) {
				blue._velocity.y = 0.0f;
				blue._position.y = fieldCenter.y;
				
				if (jump)
					blue._velocity.y -= slimeJump;
			}

			if (blue._position.x - slimeRadius < fieldCenter.x - fieldRadius) {
				blue._velocity.x = 0.0f;
				blue._position.x = fieldCenter.x - fieldRadius + slimeRadius;
			}

			if (blue._position.x + slimeRadius > wallCenter.x - wallRadius) {
				blue._velocity.x = 0.0f;
				blue._position.x = wallCenter.x - wallRadius - slimeRadius;
			}
		}

		// Red slime
		{
			red._velocity.y += gravity * dt;
			red._velocity.x += -slimeMoveDeccel * red._velocity.x * dt;
			red._position += red._velocity * dt;

			bool moveLeft;
			bool moveRight;
			bool jump;

			redPacket >> moveLeft >> moveRight >> jump;

			if (moveLeft) {
				red._velocity.x += -slimeMoveAccel * dt;

				if (red._velocity.x < -maxSlimeSpeed)
					red._velocity.x = -maxSlimeSpeed;
			}
			else if (moveRight) {
				red._velocity.x += slimeMoveAccel * dt;

				if (red._velocity.x > maxSlimeSpeed)
					red._velocity.x = maxSlimeSpeed;
			}
			
			if (red._position.y > fieldCenter.y) {
				red._velocity.y = 0.0f;
				red._position.y = fieldCenter.y;

				if (jump)
					red._velocity.y -= slimeJump;
			}

			if (red._position.x + slimeRadius > fieldCenter.x + fieldRadius) {
				red._velocity.x = 0.0f;
				red._position.x = fieldCenter.x + fieldRadius - slimeRadius;
			}

			if (red._position.x - slimeRadius < wallCenter.x + wallRadius) {
				red._velocity.x = 0.0f;
				red._position.x = wallCenter.x + wallRadius + slimeRadius;
			}
		}

		prevScoreRed = scoreRed;
		prevScoreBlue = scoreBlue;
		prevBallX = ball._position.x;

		// --------------------------------- Rendering ---------------------------------

		if (show) {
			renderWindow.clear();

			{
				sf::Sprite s;
				s.setTexture(backgroundTexture);
				s.setOrigin(backgroundTexture.getSize().x * 0.5f, backgroundTexture.getSize().y * 0.5f);
				s.setPosition(renderWindow.getSize().x * 0.5f, renderWindow.getSize().y * 0.5f);

				renderWindow.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(blueSlimeTexture);
				s.setOrigin(blueSlimeTexture.getSize().x * 0.5f, blueSlimeTexture.getSize().y);
				s.setPosition(blue._position);

				renderWindow.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(blue._position + sf::Vector2f(50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				renderWindow.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(redSlimeTexture);
				s.setOrigin(redSlimeTexture.getSize().x * 0.5f, redSlimeTexture.getSize().y);
				s.setPosition(red._position);

				renderWindow.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(red._position + sf::Vector2f(-50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				renderWindow.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(ballTexture);
				s.setOrigin(ballTexture.getSize().x * 0.5f, ballTexture.getSize().y * 0.5f);
				s.setPosition(ball._position);

				renderWindow.draw(s);
			}

			if (ball._position.y + ballRadius < 0.0f) {
				sf::Sprite s;
				s.setTexture(arrowTexture);
				s.setOrigin(arrowTexture.getSize().x * 0.5f, 0.0f);
				s.setPosition(ball._position.x, 0.0f);

				renderWindow.draw(s);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreBlue));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f - 100.0f, 10.0f);

				scoreText.setColor(sf::Color(100, 133, 255));

				renderWindow.draw(scoreText);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreRed));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f + 100.0f, 10.0f);

				scoreText.setColor(sf::Color(255, 100, 100));

				renderWindow.draw(scoreText);
			}
		}

		blueRT.setView(renderWindow.getView());
		redRT.setView(renderWindow.getView());

		{
			blueRT.clear();

			{
				sf::Sprite s;
				s.setTexture(backgroundTexture);
				s.setOrigin(backgroundTexture.getSize().x * 0.5f, backgroundTexture.getSize().y * 0.5f);
				s.setPosition(blueRT.getSize().x * 0.5f, blueRT.getSize().y * 0.5f);

				blueRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(blueSlimeTexture);
				s.setOrigin(blueSlimeTexture.getSize().x * 0.5f, blueSlimeTexture.getSize().y);
				s.setPosition(blue._position);

				blueRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(blue._position + sf::Vector2f(50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				blueRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(redSlimeTexture);
				s.setOrigin(redSlimeTexture.getSize().x * 0.5f, redSlimeTexture.getSize().y);
				s.setPosition(red._position);

				blueRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(red._position + sf::Vector2f(-50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				blueRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(ballTexture);
				s.setOrigin(ballTexture.getSize().x * 0.5f, ballTexture.getSize().y * 0.5f);
				s.setPosition(ball._position);

				blueRT.draw(s);
			}

			if (ball._position.y + ballRadius < 0.0f) {
				sf::Sprite s;
				s.setTexture(arrowTexture);
				s.setOrigin(arrowTexture.getSize().x * 0.5f, 0.0f);
				s.setPosition(ball._position.x, 0.0f);

				blueRT.draw(s);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreBlue));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f - 100.0f, 10.0f);

				scoreText.setColor(sf::Color(100, 133, 255));

				blueRT.draw(scoreText);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreRed));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f + 100.0f, 10.0f);

				scoreText.setColor(sf::Color(255, 100, 100));

				blueRT.draw(scoreText);
			}

			blueRT.display();
		}

		{
			redRT.clear();

			{
				sf::Sprite s;
				s.setTexture(backgroundTexture);
				s.setOrigin(backgroundTexture.getSize().x * 0.5f, backgroundTexture.getSize().y * 0.5f);
				s.setPosition(redRT.getSize().x * 0.5f, redRT.getSize().y * 0.5f);

				redRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(blueSlimeTexture);
				s.setOrigin(blueSlimeTexture.getSize().x * 0.5f, blueSlimeTexture.getSize().y);
				s.setPosition(blue._position);

				redRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(blue._position + sf::Vector2f(50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				redRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(redSlimeTexture);
				s.setOrigin(redSlimeTexture.getSize().x * 0.5f, redSlimeTexture.getSize().y);
				s.setPosition(red._position);

				redRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(eyeTexture);
				s.setOrigin(eyeTexture.getSize().x * 0.5f, eyeTexture.getSize().y * 0.5f);
				s.setPosition(red._position + sf::Vector2f(-50.0f, -28.0f));

				sf::Vector2f delta = ball._position - s.getPosition();

				float angle = std::atan2(delta.y, delta.x);

				s.setRotation(angle * 180.0f / 3.141596f);

				redRT.draw(s);
			}

			{
				sf::Sprite s;
				s.setTexture(ballTexture);
				s.setOrigin(ballTexture.getSize().x * 0.5f, ballTexture.getSize().y * 0.5f);
				s.setPosition(ball._position);

				redRT.draw(s);
			}

			if (ball._position.y + ballRadius < 0.0f) {
				sf::Sprite s;
				s.setTexture(arrowTexture);
				s.setOrigin(arrowTexture.getSize().x * 0.5f, 0.0f);
				s.setPosition(ball._position.x, 0.0f);

				redRT.draw(s);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreBlue));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f - 100.0f, 10.0f);

				scoreText.setColor(sf::Color(100, 133, 255));

				redRT.draw(scoreText);
			}

			{
				sf::Text scoreText;
				scoreText.setFont(scoreFont);
				scoreText.setString(std::to_string(scoreRed));
				scoreText.setCharacterSize(100);

				float width = scoreText.getLocalBounds().width;

				scoreText.setPosition(fieldCenter.x - width * 0.5f + 100.0f, 10.0f);

				scoreText.setColor(sf::Color(255, 100, 100));

				redRT.draw(scoreText);
			}

			redRT.display();
		}

		// --------------------------------- Send/Receive Data ---------------------------------

		if (skipFrameCounter <= 0) {
			skipFrameCounter = skipFrames;

			// Transmit data
			{
				sf::Image imgBlue;

				imgBlue = blueRT.getTexture().copyToImage();

				sf::Packet blueSend;

				if (greyscale) {
					for (int x = 0; x < imgBlue.getSize().x; x++) {
						for (int y = 0; y < imgBlue.getSize().y; y++) {
							blueSend << sf::Uint8((imgBlue.getPixel(x, y).r + imgBlue.getPixel(x, y).g + imgBlue.getPixel(x, y).b) * 0.333f);
						}
					}
				}
				else {
					for (int x = 0; x < imgBlue.getSize().x; x++) {
						for (int y = 0; y < imgBlue.getSize().y; y++) {
							blueSend << imgBlue.getPixel(x, y).r << imgBlue.getPixel(x, y).g << imgBlue.getPixel(x, y).b;
						}
					}
				}

				sf::Image imgRed;

				imgRed = redRT.getTexture().copyToImage();

				sf::Packet redSend;

				if (greyscale) {
					for (int x = 0; x < imgRed.getSize().x; x++) {
						for (int y = 0; y < imgRed.getSize().y; y++) {
							redSend << sf::Uint8((imgRed.getPixel(x, y).r + imgRed.getPixel(x, y).g + imgRed.getPixel(x, y).b) * 0.333f);
						}
					}
				}
				else {
					for (int x = 0; x < imgRed.getSize().x; x++) {
						for (int y = 0; y < imgRed.getSize().y; y++) {
							redSend << imgRed.getPixel(x, y).r << imgRed.getPixel(x, y).g << imgRed.getPixel(x, y).b;
						}
					}
				}

				blueSocket.send(blueSend);
				redSocket.send(redSend);
			}

			// Collect new data	
			redPacket.clear();
			bluePacket.clear();

			redSocket.receive(redPacket);
			blueSocket.receive(bluePacket);
		}

		skipFrameCounter--;

		renderWindow.display();
	} while (!quit);

	return 0;
}