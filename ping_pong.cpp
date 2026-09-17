#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <random>

sf::RenderWindow window;
std::mt19937 gen{std::random_device{}()};

class Player : public sf::RectangleShape {
public:
	constexpr static float width = 16, height = 128;

	Player() : sf::RectangleShape({width, height}) {
		setOrigin({0, height/2});
	}

	void setPosition(float y) {
		sf::RectangleShape::setPosition({0, y});
	}
} player;

class Ball : public sf::CircleShape {
	float move_x, move_y;
public:
	constexpr static float radius = Player::width;
	enum BallHit : char {NOTHING, HIT, FAIL};

	inline Ball() : sf::CircleShape(radius) {
		setOrigin({radius, radius});
	}

	constexpr inline void setVelocity(float x, float y) {
		move_x = x;
		move_y = y;
	}

	BallHit move() {
		sf::CircleShape::move({move_x, move_y});
		const auto [x, y] = getPosition();
		if(y + radius >= window.getSize().y || y <= radius)
			move_y *= -1;
		if(x + radius >= window.getSize().x)
			move_x *= -1;
		else if(x <= radius)
			return FAIL;
		else if(const auto [px, py] = player.getPosition(); x-radius <= px + Player::width && y+radius >= py - Player::height/2 && y-radius <= py + Player::height/2) {
			move_x = -move_x + 0.1f;
			move_y += move_y < 0 ? -0.1f : 0.1f;
			return HIT;
		}
		return NOTHING;
	}
};

class ColorSwitch {
	sf::Color c1, c2;
public:
	constexpr inline ColorSwitch(sf::Color c1, sf::Color c2) : c1(c1), c2(c2) {}
	constexpr inline operator sf::Color() const {
		return c1;
	}
	constexpr inline sf::Color operator!() const {
		return c2;
	}
	void toggle() {
		std::swap(c1, c2);
	}
};

int main() {
	window.create(sf::VideoMode({1080, 720}), "Ping Pong");
	window.setMaximumSize(sf::Vector2u{1080, 720});
	window.setMinimumSize(sf::Vector2u{1080, 720});
	window.setFramerateLimit(60);

	ColorSwitch color(sf::Color::White, sf::Color::Black);

	const sf::Font font("./font.ttf");
	sf::Text text(font);
	text.setFillColor(color);
	text.setCharacterSize(30);
	text.setPosition({window.getSize().x/2.f, 15.f});


	player.setFillColor(color);

	std::normal_distribution<float> dist_n(7.f, 1.f);
	std::uniform_real_distribution<float> dist_u(-7.f, 7.f);

	Ball ball;
	ball.setFillColor(color);

RESET:
	int score = 0;
	bool game_lost = false;
	text.setString("0");
	text.setOrigin({15, 15});
	ball.setPosition({window.getSize().x / 2.f, window.getSize().y / 2.f});
	ball.setVelocity(dist_n(gen), dist_u(gen));

	while(window.isOpen()) {
		while(const std::optional event = window.pollEvent()) {
			if(event->is<sf::Event::Closed>())
				return 0;
			else if(const auto *key = event->getIf<sf::Event::KeyPressed>()) {
				using sc = sf::Keyboard::Scancode;
				switch(key->scancode) {
					case sc::Escape:
						return 0;
					case sc::C:
						color.toggle();
						player.setFillColor(color);
						text.setFillColor(color);
						ball.setFillColor(color);
						break;
					case sc::R:
						goto RESET;
					default:
						break;
				}
			} else if(const auto *mouse = event->getIf<sf::Event::MouseMoved>())
				player.setPosition(mouse->position.y);
		}
		std::string score_str;
		if(!game_lost) {
			switch(ball.move()) {
				case Ball::NOTHING:
					break;
				case Ball::HIT:
					score_str = std::to_string(++score);
					text.setString(score_str);
					text.setOrigin({15.f * score_str.length(), 15});
					break;
				case Ball::FAIL:
					game_lost = true;
					break;
			}
		}
		window.clear(!color);
		window.draw(player);
		window.draw(text);
		window.draw(ball);
		window.display();
	}
}