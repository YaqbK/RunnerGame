#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

enum GameState {
    Playing,
    GameOver
};

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
        move(0, 250.0f * dt);
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

        // Initialize the next jump time
        nextJumpTime = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (4.0f - 1.0f)));

        // Load font and initialize score text
        if (!font.loadFromFile("D:/Documents/Projects/fonts/Roboto/Roboto-Black.ttf")) {
            std::cerr << "Could not load font: path/to/font.ttf" << std::endl;
            std::exit(1);
        }

        scoreText.setFont(font);
        scoreText.setCharacterSize(24); // Set the character size
        scoreText.setFillColor(sf::Color::Red); // Set the text color
        scoreText.setPosition(15, 15); // Position the score text in the top-left corner
        updateScoreText(); // Initial update to display the score
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
                renderGameOver();
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
    float timeSinceLastJump = 0.0f;  // Time elapsed since the last jump
    float nextJumpTime = 0.0f;       // Time interval for the next jump
    GameState gameState = Playing;  // Track the current game state
    int score = 0;  // Track the score based on obstacles passed
    sf::Font font;  // Font for rendering the score and game over text
    sf::Text scoreText;  // Text object for displaying the score

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
                }
            }
        }
    }

    void update(const sf::Time& elapsed) {
        if (gameState == Playing) {
            float dt = elapsed.asSeconds();
            player.update(elapsed);
            for (auto obj : objects) {
                obj->update(elapsed);
            }

            // Update the timer for random lane jumps
            timeSinceLastJump += dt;
            if (timeSinceLastJump >= nextJumpTime) {
                player.randomMove();
                timeSinceLastJump = 0.0f;  // Reset the timer

                // Set the next jump time to a random value between 2 and 6 seconds
                nextJumpTime = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (3.0f - 1.0f)));
            }

            // Check if player has passed the current obstacle
            if (currentObstacleIndex < objects.size()) {
                if (player.getPosition().y < objects[currentObstacleIndex]->getPosition().y) {
                    // Player has passed the obstacle
                    generateNewObstacle();
                    currentObstacleIndex++;
                    score++;  // Increment score
                    updateScoreText();  // Update the displayed score
                }
            }

            if (checkCollision()) {
                gameState = GameOver;
            }
        }else if (gameState == GameOver) {
            // Check for restart key press
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                restartGame();
            }
        }
    }

    void updateScoreText() {
        scoreText.setString("Score: " + std::to_string(currentObstacleIndex));
    }

    void generateNewObstacle() {
        int randomLane = rand() % 3; // Random lane (0, 1, or 2)
        Obstacle* newObstacle = new Obstacle(randomLane * laneWidth + laneMargin, -50.0f); // Start off-screen
        //newObstacle->setLane(randomLane);
        objects.push_back(newObstacle);
    }


    void render() {
        if (gameState == Playing) {
            window.clear(sf::Color::Black);

            sf::Texture grassTexture;
            if (!grassTexture.loadFromFile("D:/Documents/Projects/PUT/RunnerGame/RunnerGame/grass.png")) {
                std::cerr << "Could not load texture" << std::endl;
                return;
            }
            grassTexture.setRepeated(true);

            sf::Sprite sprite;
            sprite.setTexture(grassTexture);
            sprite.setTextureRect(sf::IntRect(0, 0, 800, 600));

            window.draw(sprite);
            window.draw(player);
            for (const auto& obj : objects) {
                window.draw(*obj);
            }

            window.draw(scoreText);

            window.display();
        } else if (gameState == GameOver) {
            renderGameOver();
        }
    }

    void renderGameOver() {
        window.clear(sf::Color::Black);

        // Display the score
        sf::Text gameOverText;
        gameOverText.setFont(font);
        gameOverText.setCharacterSize(30);
        gameOverText.setFillColor(sf::Color::White);
        gameOverText.setString("Game Over\nScore: " + std::to_string(score) + "\nPress R to Restart");
        gameOverText.setPosition(window.getSize().x / 2.0f - gameOverText.getGlobalBounds().width / 2.0f,
                                 window.getSize().y / 2.0f - gameOverText.getGlobalBounds().height / 2.0f);

        window.draw(gameOverText);
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
    void restartGame() {
        gameState = Playing;
        score = 0;
        objects.clear();

        // Reinitialize the first obstacle
        Obstacle* obstacle = new Obstacle(150.0f + laneMargin, 0.0f);
        obstacle->setLane(1);
        objects.push_back(obstacle);

        // Reset player position
        player.setPosition(150.0f, 500.0f);
        player.randomMove();

        // Reset the jump timer
        timeSinceLastJump = 0.0f;
        nextJumpTime = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (6.0f - 2.0f)));
    }

};

int main() {
    Game game;
    game.run();
    return 0;
}
