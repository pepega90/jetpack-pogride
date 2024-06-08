#include "raylib.h"
#include "iostream"
#include <string>
#include <vector>

enum Screen {
    MENU,
    PLAY
};

struct Particle {
    Texture2D img;
    Vector2 pos;
    Vector2 speed;
};

struct Player {
    Texture2D img;
    Texture2D flyFire;
    Texture2D particleImg;
    Texture2D bekasBulletImg;
    Texture2D playerDieImg;
    Vector2 pos;
    Vector2 vel;
    std::vector<Particle> particles;
    std::vector<Particle> listBekasBullet;
    Rectangle rect;

    Sound flySfx;

    float jumpVel = -150;
    float particleTimer = 0.0f;
    float particleSpawnInterval = 0.1f;
    bool isGround = false;
    bool isFly = false;

    Player(float x, float y, Texture2D img) {
        this->pos = Vector2(x,y);
        this->img = img;
    }

    void Reset() {
        pos.x = -img.width;
    }

    void Update(float &dt, bool &gameOver) {
        if(gameOver) {
            particles.erase(particles.begin(), particles.end());
            listBekasBullet.erase(listBekasBullet.begin(), listBekasBullet.end());
        }

        if(IsKeyDown(KEY_SPACE) && !gameOver) {
            vel.y = 0;
            vel.y += jumpVel;
            isFly = true;

            particleTimer += dt;

            if(particleTimer >= particleSpawnInterval) {
                Particle p;
                p.img = particleImg;
                p.pos = Vector2(pos.x + 15, pos.y + 50);
                int randomValue = GetRandomValue(0, 2);
                float speedX = (randomValue == 0) ? -1.5 : (randomValue == 1) ? 1.5 : 0;
                p.speed = Vector2(speedX, 10);
                particles.push_back(p);
                PlaySound(flySfx);
                particleTimer = 0.0;
            }
        } else {
            isFly = false;
            particleTimer = particleSpawnInterval;
        }

        if(pos.y <= 60) {
            pos.y = 60;
        }

        pos += vel * dt;

        rect.x = pos.x;
        rect.y = pos.y;
        rect.width = img.width;
        rect.height = img.height;
    }

    void Draw(float &dt, float &bgSpeed) {
        if(isFly) {
            DrawTextureV(flyFire, Vector2(pos.x, pos.y + 50), WHITE);
        }
        for(int i =0; i < particles.size(); i++) {
            if(particles[i].pos.y > 470) {
                Particle p;
                p.img = bekasBulletImg;
                p.pos = particles[i].pos;
                p.speed = Vector2(-bgSpeed, 0);
                listBekasBullet.push_back(p);
                particles.erase(particles.begin() + i);
            }
            particles[i].pos += particles[i].speed;
            DrawTextureEx(particles[i].img, particles[i].pos, 0.0, 2.0, WHITE);
        }

        for(int i =0; i < listBekasBullet.size();i++) {
            if(listBekasBullet[i].pos.x < -listBekasBullet[i].img.width) listBekasBullet.erase(listBekasBullet.begin() + i);
            listBekasBullet[i].pos += listBekasBullet[i].speed * dt;
            DrawTextureV(listBekasBullet[i].img, listBekasBullet[i].pos, WHITE);
        }
        // DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, RED);
        DrawTextureV(img, pos, WHITE);
    }

    ~Player() = default; 
};

struct Obstacle {
    std::vector<Texture2D> img;
    Vector2 pos;
    Vector2 speed;
    Rectangle rect;

    int idx = 0;
    float zapperTimer = 0.0;
    float zapperInterval = 0.1f;

    Obstacle(float x, float y, std::vector<Texture2D> imgs) {
        this->img = imgs;
        this->pos = Vector2(x,y);
    }

    ~Obstacle() = default;

    void Reset() {
        pos = Vector2(GetScreenWidth(), GetRandomValue(0,1) == 0 ? 120 : 300);
    }

    void Draw(float &dt) {
        zapperTimer += dt;
        if(zapperTimer >= zapperInterval) {
            idx = (1 + idx) % img.size();
            zapperTimer = 0.0;
        }

        rect.x = pos.x;
        rect.y = pos.y;
        rect.width = img[idx].width;
        rect.height = img[idx].height;

        // DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, RED);
        DrawTextureV(img[idx], pos, WHITE);
    }
};

struct Button {
    Texture2D img;
    Rectangle rect;
    Button(float x, float y, Texture2D img) {
        this->img = img;
        this->rect = Rectangle{x,y,(float)img.width, (float)img.height};
    }

    void Draw() {
        DrawTextureRec(img, Rectangle{0,0,rect.width,rect.height}, Vector2(rect.x, rect.y), WHITE);
        // DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, RED);
    }

    ~Button() = default;
};

struct ParallaxBG {
    Texture2D img;
    float speed;
    float offset;

    ParallaxBG(Texture2D tex, float spd) : img(tex), speed(spd), offset(0) {}
    
    void Update(float &dt) {
        offset -= speed * dt;
        if(offset < -img.width) offset = 0;
    }

    void Draw() {
        DrawTextureEx(img, Vector2(offset, 0), 0.0f, 1.0f, WHITE);
        DrawTextureEx(img, Vector2(offset + img.width , 0), 0.0f, 1.0f, WHITE);
    }
};

void loadVectorImage(int start, int end, std::string path, std::vector<Texture> &textures, float scale = 1);

int main() {
    const int WIDTH = 860;
    const int HEIGHT = 540;
    InitWindow(WIDTH, HEIGHT,"Jetpack joy");
    InitAudioDevice();
    SetTargetFPS(60);

    // load assets
    Texture2D playerFlyImg = LoadTexture("assets/sprites/PlayerFly.png");
    Texture2D playerDeadImg = LoadTexture("assets/sprites/skins/PlayerDead_Blue.png");
    Texture2D bg = LoadTexture("assets/sprites/BackdropMain.png");
    bg.width = WIDTH;
    bg.height = HEIGHT;
    Texture2D flyFire = LoadTexture("assets/sprites/FlyFire.png");
    Texture2D flyFire2 = LoadTexture("assets/sprites/FlyFire2.png");
    Texture2D rocketParticleImg = LoadTexture("assets/sprites/Bullet.png");
    Texture2D bekasBulletImg = LoadTexture("assets/sprites/BulletCollision.png");
    Sound flySfx = LoadSound("assets/sounds/FlyTest.wav");
    Font gameFont = LoadFont("assets/fonts/New Athletic M54.ttf");
    std::vector<Texture2D> zappers;
    loadVectorImage(1, 5, "assets/sprites/", zappers, 1.5);
    // UI assets
    Texture2D playAgainImg = LoadTexture("assets/sprites/ButtonPlayAgain.png");
    Texture2D playGameImg = LoadTexture("assets/sprites/ButtonPlayGame.png");

    // button variabel
    Button playAgainBtn = Button(400, HEIGHT/2 + 15, playAgainImg);
    Button playGameBtn = Button(330, HEIGHT/2 + 30, playGameImg);

    // game variable
    Player player = Player(-playerFlyImg.width, 470, playerFlyImg); // 150, untuk posisi player start playing the game
    player.flyFire = flyFire;
    player.particleImg = rocketParticleImg;
    player.bekasBulletImg = bekasBulletImg;
    player.flySfx = flySfx;
    player.playerDieImg = playerDeadImg;
    int distance = 0;
    int bestDistance = 0;
    float distanceTimer = 0.0;
    float distanceInterval = 0.3f;
    float grav = 500;
    bool gameOver = false;

    Screen currentScreen = Screen::MENU;
    Obstacle zapper = Obstacle(WIDTH, GetRandomValue(0,1) == 0 ? 120 : 300, zappers);
    ParallaxBG parallaxBg = ParallaxBG(bg, 100);

    // TODO: menambahkan menu screen

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        ClearBackground(BLACK);
        BeginDrawing();
        switch (currentScreen)
        {
        case MENU:
            DrawTexture(bg, 0, 0, WHITE);

            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), playGameBtn.rect)) currentScreen = Screen::PLAY;
            
            DrawTextEx(gameFont, "Jetpack", Vector2(370, 150), 50, 0.0, WHITE);
            DrawTextEx(gameFont, "POGRIDE", Vector2(360, 190), 70, 0.0, ORANGE);
            playGameBtn.Draw();
            DrawTextEx(gameFont, "created by aji mustofa @pepega90", Vector2(5, HEIGHT-45), 30, 0.0, YELLOW);
            break;
        case PLAY:
            distanceTimer += dt;

            if(player.pos.x < 150) {
                player.pos.x += 100 * dt;
            } else if(!gameOver) {
                player.pos.x = 150;
                parallaxBg.speed = 400;
                zapper.speed = Vector2(-parallaxBg.speed, 0);
                distanceInterval = 0.1f;
            }

            if(!gameOver && distanceTimer >= distanceInterval) {
                distance += 1;
                distanceTimer = 0.0;
            }
            
            if(!gameOver) {
                zapper.pos += zapper.speed * dt;
            }

            if(zapper.pos.x < -100) {
                zapper.pos.x = WIDTH;
                zapper.pos.y = GetRandomValue(0,1) == 0 ? 120 : 300;
            }

            // 470, posisi y koordinat untuk ground background
            if(player.pos.y + player.img.height < 470) {
                player.vel.y += grav * dt;
                player.isGround = false;
            } else {
                player.vel.y = 0;
                player.pos.y = 470 - player.img.height;
                player.isGround = true;
            }

            // check collision between player and zapper
            if(CheckCollisionRecs(player.rect, zapper.rect)) {
                gameOver = true;
                player.img = player.playerDieImg;
                if(distance > bestDistance) bestDistance = distance;
            }

            if(gameOver) {
                if(parallaxBg.speed > 0) parallaxBg.speed -= 5;
                else parallaxBg.speed = 0;
            }

            // restart game if gameover
            if(gameOver && IsKeyPressed(KEY_R)) {
                player.Reset();
                zapper.Reset();
                distance = 0;
                player.img = playerFlyImg;
                zapper.speed = Vector2(-parallaxBg.speed, 0);
                gameOver = false;
            }

            player.Update(dt, gameOver);
            parallaxBg.Update(dt);

            // game over logic
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && gameOver && CheckCollisionPointRec(GetMousePosition(),playAgainBtn.rect)) {
                player.Reset();
                zapper.Reset();
                distance = 0;
                player.img = playerFlyImg;
                zapper.speed = Vector2(-parallaxBg.speed, 0);
                gameOver = false;
            }

            // ====================== DRAW SECTION ======================
            parallaxBg.Draw();
            player.Draw(dt, parallaxBg.speed);
            zapper.Draw(dt);

            if(gameOver) {
                DrawTextEx(gameFont, "You Flew", Vector2(570, 180), 50, 0.0, WHITE);
                std::string gameOverDistanceText = std::to_string(distance) + "M";
                DrawTextEx(gameFont, gameOverDistanceText.c_str(), Vector2(610, 230), 60, 0.0, YELLOW);
                playAgainBtn.Draw();
            }

            // draw distance text
            std::string distanceText = "Distance " + std::to_string(distance);
            DrawTextEx(gameFont, distanceText.c_str(), Vector2(20, 22), 40, 0.0, WHITE);
            std::string bestDistanceText = "Best " + std::to_string(bestDistance);
            DrawTextEx(gameFont, bestDistanceText.c_str(), Vector2(20, 57), 35, 0.0, WHITE);

            // draw mouse position helper
            // DrawText(("Mouse X = " + std::to_string(GetMouseX())).c_str(), WIDTH - 150, 30, 20, BLACK);
            // DrawText(("Mouse Y = " + std::to_string(GetMouseY())).c_str(), WIDTH - 150, 60, 20, BLACK);
            // ====================== END DRAW SECTION ==================

        break;
        }
        EndDrawing();
    }
    UnloadTexture(flyFire);
    UnloadTexture(flyFire2);
    UnloadTexture(bg);
    UnloadTexture(playerDeadImg);
    UnloadTexture(rocketParticleImg);
    UnloadTexture(playerFlyImg);
    UnloadTexture(bekasBulletImg);
    UnloadTexture(playAgainImg);
    UnloadTexture(playGameImg);
    for(auto &v : zappers)
        UnloadTexture(v);
    UnloadSound(flySfx);
    UnloadFont(gameFont);
    CloseAudioDevice();
    CloseWindow(); 

    return 0;
}

void loadVectorImage(int start, int end, std::string path, std::vector<Texture> &textures, float scale)
{
    for (int i = start; i < end; i++)
    {
        std::string filename = path + "Zapper"  + std::to_string(i) + ".png";
        Texture2D z = LoadTexture(filename.c_str());
        z.width *= scale;
        z.height *= scale;
        textures.push_back(z);
    }
}