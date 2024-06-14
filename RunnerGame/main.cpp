#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

// Base class for game objects
class GameObject : public sf::Drawable {
public:
    GameObject(float x, float y) {
        shape.setSize(sf::Vector2f(50.0f, 50.0f));
        shape.setPosition(x, y);
    }

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }

    void setSpeed(const int& x_speed, const int& y_speed) {
        x_speed_ = x_speed;
        y_speed_ = y_speed;
    }

    virtual void update(const sf::Time& elapsed) {
        float dt = elapsed.asSeconds();
        move(x_speed_ * dt, y_speed_ * dt);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

protected:
    sf::RectangleShape shape;
    int x_speed_ = 0;
    int y_speed_ = 0;

    void move(float dx, float dy) {
        shape.move(dx, dy);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(shape, states);
    }
};

// Player class
class Player : public GameObject {
public:
    Player(float x, float y, float margin) : GameObject(x, y), lane(1), laneWidth(200.0f), laneMargin(170.0f) {
        shape.setFillColor(sf::Color::Green);
        updatePosition();
    }

    void moveLeft() {
        if (lane > 0) {
            lane--;
            updatePosition();
        }
    }

    void moveRight() {
        if (lane < 2) {
            lane++;
            updatePosition();
        }
    }

    void randomMove() {
        lane = rand() % 3;
        updatePosition();
    }

    void update(const sf::Time& elapsed) override {
        // Player-specific updates can be handled here
    }

private:
    int lane;
    float laneWidth;
    float laneMargin;

    void updatePosition() {
        shape.setPosition(lane * laneWidth + laneMargin, shape.getPosition().y);
    }
};

// Obstacle class
class Obstacle : public GameObject {
public:
    Obstacle(float x, float y) : GameObject(x, y) {
        shape.setFillColor(sf::Color::Red);
    }

    void update(const sf::Time& elapsed) override {
        float dt = elapsed.asSeconds();
        move(0, 175.0f * dt);
    }

    void setLane(int lane) {
        this->lane = lane;
        updatePosition();
    }

private:
    int lane = 0;
    float laneWidth;
    float laneMargin;

    void updatePosition() {
        shape.setPosition(lane * laneWidth + laneMargin, shape.getPosition().y);
    }
};

// Game class
class Game {
public:
    Game() : window(sf::VideoMode(800, 600), "SFML Game"), player(150.0f, 500.0f, laneMargin) {
        srand(static_cast<unsigned int>(time(nullptr)));
        objects.push_back(new Obstacle( laneMargin, 300.0f)); // Example object
    }

    ~Game() {
        for (auto obj : objects) {
            delete obj;
        }
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            processEvents();
            sf::Time elapsed = clock.restart();
            update(elapsed);
            render();
            if (checkCollision()) {
                window.close(); // Close the window if a collision is detected
            }
        }
    }

private:
    sf::RenderWindow window;
    Player player;
    std::vector<GameObject*> objects;
    const float laneMargin = 170.0f;
    float laneWidth = 200.0f;
    int currentObstacleIndex = 0; // Track the index of the current obstacle

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::A) {
                    player.moveLeft();
                } else if (event.key.code == sf::Keyboard::D) {
                    player.moveRight();
                } else if (event.key.code == sf::Keyboard::Space) {
                    player.randomMove();
                }
            }
        }
    }

    void update(const sf::Time& elapsed) {
        player.update(elapsed);
        for (auto obj : objects) {
            obj->update(elapsed);
        }

        // Check if player has passed the current obstacle
        if (currentObstacleIndex < objects.size()) {
            if (player.getPosition().y < objects[currentObstacleIndex]->getPosition().y) {
                // Player has passed the obstacle
                generateNewObstacle();
                currentObstacleIndex++;
            }
        }
    }
    void generateNewObstacle() {
        int randomLane = rand() % 3; // Random lane (0, 1, or 2)
        Obstacle* newObstacle = new Obstacle(randomLane * laneWidth + laneMargin, -50.0f); // Start off-screen
        //newObstacle->setLane(randomLane);
        objects.push_back(newObstacle);
    }


    void render() {
        window.clear(sf::Color::Black);

        sf::Texture texture;
        if (!texture.loadFromFile("D:/Documents/Projects/PUT/RunnerGame/RunnerGame/grass.png")) {
            std::cerr << "Could not load texture" << std::endl;
            return;
        }
        texture.setRepeated(true);

        sf::Sprite sprite;
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, 800, 600));

        window.draw(sprite);
        window.draw(player);
        for (const auto& obj : objects) {
            window.draw(*obj);
        }

        window.display();
    }

    bool checkCollision() {
        for (const auto& obj : objects) {
            if (player.getBounds().intersects(obj->getBounds())) {
                return true;
            }
        }
        return false;
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
