#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
#include <math.h>
#include <fstream>
using namespace sf;

const int W = 1200;
const int H = 800;
float DEGTORAD = 0.017453f;

enum GameState { MENU, PLAYING, GAMEOVER, HELP };
GameState gameState = MENU;

class Animation {
public:
    float Frame, speed;
    Sprite sprite;
    std::vector<IntRect> frames;

    Animation() {}

    Animation(Texture &t, int x, int y, int w, int h, int count, float Speed) {
        Frame = 0;
        speed = Speed;
        for (int i = 0; i < count; i++)
            frames.push_back(IntRect(x + i * w, y, w, h));

        sprite.setTexture(t);
        sprite.setOrigin(w / 2, h / 2);
        sprite.setTextureRect(frames[0]);
    }

    void update() {
        Frame += speed;
        int n = frames.size();
        if (Frame >= n) Frame -= n;
        if (n > 0) sprite.setTextureRect(frames[int(Frame)]);
    }
    bool isEnd() {
        return Frame + speed >= frames.size();
    }
};

class Entity {
public:
    float x, y, dx, dy, R, angle;
    bool life;
    std::string name;
    Animation anim;

    Entity() {
        life = 1;
    }

    void settings(Animation &a, int X, int Y, float Angle = 0, int radius = 1) {
        anim = a;
        x = X;
        y = Y;
        angle = Angle;
        R = radius;
    }

    virtual void update() {};
    void draw(RenderWindow &app) {
        anim.sprite.setPosition(x, y);
        anim.sprite.setRotation(angle + 90);
        app.draw(anim.sprite);
    }
    virtual ~Entity() {};
};

class asteroid : public Entity {
public:
    asteroid() {
        dx = rand() % 8 - 4;
        dy = rand() % 8 - 4;
        name = "asteroid";
    }

    void update() {
        x += dx;
        y += dy;
        if (x > W) x = 0; if (x < 0) x = W;
        if (y > H) y = 0; if (y < 0) y = H;
    }
};

class bullet : public Entity {
public:
    bullet() {
        name = "bullet";
    }

    void update() {
        dx = cos(angle * DEGTORAD) * 6;
        dy = sin(angle * DEGTORAD) * 6;
        x += dx;
        y += dy;
        if (x > W || x < 0 || y > H || y < 0) life = 0;
    }
};

class player : public Entity {
public:
    bool thrust;

    player() {
        name = "player";
    }

    void update() {
        if (thrust) {
            dx += cos(angle * DEGTORAD) * 0.2;
            dy += sin(angle * DEGTORAD) * 0.2;
        } else {
            dx *= 0.99;
            dy *= 0.99;
        }

        int maxSpeed = 15;
        float speed = sqrt(dx * dx + dy * dy);
        if (speed > maxSpeed) {
            dx *= maxSpeed / speed;
            dy *= maxSpeed / speed;
        }

        x += dx;
        y += dy;
        if (x > W) x = 0; if (x < 0) x = W;
        if (y > H) y = 0; if (y < 0) y = H;
    }
};

bool isCollide(Entity *a, Entity *b) {
    return (b->x - a->x) * (b->x - a->x) +
        (b->y - a->y) * (b->y - a->y) <
        (a->R + b->R) * (a->R + b->R);
}

void resetGame(std::list<Entity *> &entities, Animation &sPlayer, player *&p, Animation &sRock, int &score) {

    entities.clear();

    for (int i = 0; i < 15; i++) {
        asteroid *a = new asteroid();
        a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
        entities.push_back(a);
    }

    p = new player();
    p->settings(sPlayer, W / 2, H / 2, 0, 20);
    entities.push_back(p);

    score = 0;
}

int main() {
    srand(time(0));

    RenderWindow app(VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    Texture t1, t2, t3, t4, t5, t6, t7;
    t1.loadFromFile("images/spaceship.png");
    t2.loadFromFile("images/background.jpg");
    t3.loadFromFile("images/explosions/type_C.png");
    t4.loadFromFile("images/rock.png");
    t5.loadFromFile("images/fire_blue.png");
    t6.loadFromFile("images/rock_small.png");
    t7.loadFromFile("images/explosions/type_B.png");

    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);

    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

    Font font;
    font.loadFromFile("font.ttf");

    Text titleText("Galaxy Defender", font, 120);
    titleText.setFillColor(Color::White);
    titleText.setPosition(W / 2 - titleText.getGlobalBounds().width / 2, H / 3);

    Text startText("Press Enter to Start", font, 50);
    startText.setFillColor(Color::White);
    startText.setPosition(W / 2 - startText.getGlobalBounds().width / 2, H / 2);

    Text helpText("Press Space to View Help", font, 50);
    helpText.setFillColor(Color::White);
    helpText.setPosition(W / 2 - helpText.getGlobalBounds().width / 2, H / 2 + 50);

    Text helpDetails("Welcome to Galaxy Defender!\n\n"
                 "Objective:\n"
                 "  - Survive as long as possible while destroying asteroids.\n"
                 "  - Earn points by breaking asteroids into smaller pieces.\n"
                 "  - Avoid collisions to maintain your score.\n\n"
                 "Controls:\n"
                 "  - Arrow keys: Steer your spaceship.\n"
                 "  - Up Arrow: Accelerate.\n"
                 "  - Spacebar: Shoot lasers to destroy asteroids.\n"
                 "  - Enter: Replay game in Gameover.\n\n"
                 "Gameplay Tips:\n"
                 "  - Small asteroids give fewer points but are easier to avoid.\n"
                 "  - Prioritize avoiding collisions over scoring.\n"
                 "  - Master your speed to outmaneuver larger asteroid clusters.\n\n"
                 "Press Enter to return to the main menu.",
                 font, 25);
    helpDetails.setFillColor(Color::White);
    helpDetails.setPosition(W / 2 - helpDetails.getGlobalBounds().width / 2, H / 4 - 30);

    Text gameOverText("Game Over", font, 120);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(W / 2 - gameOverText.getGlobalBounds().width / 2, H / 3);

    Text replayText("Press Enter to Restart", font, 50);
    replayText.setFillColor(Color::White);
    replayText.setPosition(W / 2 - replayText.getGlobalBounds().width / 2, H / 2 + 50);

    Text retryText("Press Esc to Return to Menu", font, 50);
    retryText.setFillColor(Color::White);
    retryText.setPosition(W / 2 - retryText.getGlobalBounds().width / 2, H / 2 + 100);

    Text scoreText("", font, 30);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(10, 10);

    Text highScoreText("", font, 30);
    highScoreText.setFillColor(Color::White);
    highScoreText.setPosition(10, 40);

    std::list<Entity *> entities;
    player *p = new player();
    int score = 0;
    resetGame(entities, sPlayer, p, sRock, score);

    int highScore = 0;
    std::ifstream infile("highscore.txt");
    if (infile.is_open()) {
        infile >> highScore;
        infile.close();
    }

    while (app.isOpen()) {
        Event event;
        while (app.pollEvent(event)) {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed) {
                if (gameState == MENU && event.key.code == Keyboard::Enter) {
                    gameState = PLAYING;
                    resetGame(entities, sPlayer, p, sRock, score);
                } else if (gameState == MENU && event.key.code == Keyboard::Space) {
                    gameState = HELP;
                } else if (gameState == HELP && event.key.code == Keyboard::Enter) {
                    gameState = MENU;
                } else if (gameState == GAMEOVER && event.key.code == Keyboard::Escape) {
                    gameState = MENU;
                } else if (gameState == GAMEOVER && event.key.code == Keyboard::Enter) {
                    gameState = PLAYING;
                    resetGame(entities, sPlayer, p, sRock, score);
                } else if (gameState == PLAYING && event.key.code == Keyboard::Space) {
                    bullet *b = new bullet();
                    b->settings(sBullet, p->x, p->y, p->angle, 10);
                    entities.push_back(b);
                } else if (gameState == PLAYING && event.key.code == Keyboard::Q) {  
                    gameState = GAMEOVER;
                }
            }
        }

	app.clear();

        if (gameState == MENU) {

            highScoreText.setString("High Score: " + std::to_string(highScore));
            app.draw(highScoreText);
        	app.draw(titleText);
        	app.draw(startText);
            app.draw(helpText);

    } else if (gameState == HELP) {
            
            app.draw(helpDetails);

    } else if (gameState == GAMEOVER) {

        	app.draw(gameOverText);
            app.draw(replayText);
            app.draw(retryText);

            scoreText.setString("Score: " + std::to_string(score));
            app.draw(scoreText);

            highScoreText.setString("High Score: " + std::to_string(highScore));
            app.draw(highScoreText);


    } else if (gameState == PLAYING) {

        if (std::count_if(entities.begin(), entities.end(), [](Entity* e) { return e->name == "asteroid"; }) == 0) {
            for (int i = 0; i < 15; i++) {
                asteroid* a = new asteroid();
                a->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
                entities.push_back(a);
            }
        }

        app.draw(background);

        if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += 3;
        if (Keyboard::isKeyPressed(Keyboard::Left)) p->angle -= 3;
        if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust = true;
        else p->thrust = false;

        for (auto a : entities)
            for (auto b : entities) {
                if (a->name == "asteroid" && b->name == "bullet" && isCollide(a, b)) {
                    a->life = false;
                    b->life = false;

                    score += 10;
                    if (score > highScore) {
                        highScore = score;
                        std::ofstream outfile("highscore.txt");
                        if (outfile.is_open()) {
                            outfile << highScore;
                            outfile.close();
                        }
                        highScoreText.setFillColor(Color::Yellow);
                    }
                    
                    Entity *e = new Entity();
                    e->settings(sExplosion, a->x, a->y);
                    e->name = "explosion";
                    entities.push_back(e);

                    for (int i = 0; i < 2; i++) {
                        if (a->R == 15) continue;
                        Entity *e = new asteroid();
                        e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                        entities.push_back(e);
                    }
                }

                if (a->name == "player" && b->name == "asteroid" && isCollide(a, b)) {
                    b->life = false;
                    Entity *e = new Entity();
                    e->settings(sExplosion_ship, a->x, a->y);
                    e->name = "explosion";
                    entities.push_back(e);

                    gameState = GAMEOVER;
                    highScoreText.setFillColor(Color::White);
                }
            }

        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;

        for (auto e : entities)
            if (e->name == "explosion")
                if (e->anim.isEnd()) e->life = false;

        for (auto i = entities.begin(); i != entities.end();) {
            Entity *e = *i;
            e->update();
            e->anim.update();

            if (e->life == false) {
                i = entities.erase(i);
                delete e;
            } else
                i++;
        }

        if (rand() % 150 == 0) {
            asteroid *a = new asteroid();
            a->settings(sRock, 0, rand() % H, rand() % 360, 25);
            entities.push_back(a);
        }

        for (auto i : entities) i->draw(app);

        scoreText.setString("Score: " + std::to_string(score));
        app.draw(scoreText);

        highScoreText.setString("High Score: " + std::to_string(highScore));
        app.draw(highScoreText);

	}

      	app.display();
	}

    return 0;
}
