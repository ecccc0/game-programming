/**
* Author: Eric Zhang
* Assignment: Lunar Lander!
* Date due: 2025-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Entity.h"
#include <vector>
#include <cmath>
#include <string>


/*
* Asset Credits:
* https://guardian5.itch.io/spaceship-asset
*/



enum  GameStatus   { RUN, FAILED, SUCCESS };


// terrain would just be lines, like the original game
struct TerrainSegment
{
    Vector2 start;
    Vector2 end;
    bool is_safe;
};

struct GameState
{
    Entity *player;
    Entity *platform;
    std::vector<TerrainSegment> terrain;
    float fuel;
};

std::string message = "";
constexpr int   SCREEN_WIDTH  = 1000,
                SCREEN_HEIGHT = 600,
                FPS           = 60;
    

constexpr char BG_COLOR[] = "#040e1bff";

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

constexpr float GRAVITY = 20.0f,
                UP_THRUST_POWER = 80.0f,
                SIDE_THRUST_POWER = 150.0f,
                ROTATION_SPEED = 3.0f,
                // physics
                MAX_LANDING_SPEED_Y = 60.0f, 
                MAX_LANDING_SPEED_X = 60.0f,
                MAX_LANDING_ANGLE = 30.0f,
                MAX_ANGLE = 30.0f,
                // win conditions
                FUEL_START = 500.0f,
                FUEL_CONSUMPTION_RATE = 0.4f; 
                // fuel mechanics

constexpr Vector2 PLAY_START_POS = { ORIGIN.x + 100.0f, 100.0f };
constexpr Vector2 PLAYER_SCALE    = { 40.0f, 40.0f };

// terrain gen parameters
constexpr int TERRAIN_START_Y   = 450;
constexpr int MIN_SEGMENT_WIDTH = 50;
constexpr int MAX_SEGMENT_WIDTH = 120;
constexpr int MAX_ALTITUDE_DELTA = 75;


// floating platform
constexpr Vector2 PLATFORM_START_POS = { 700.0f, 300.0f };
constexpr Vector2 PLATFORM_SIZE = { 100.0f, 20.0f };
constexpr float PLATFORM_SPEED = 1.5f; 


AppStatus gAppStatus = RUNNING;
GameStatus gGameStatus = RUN;

float   gPreviousTicks = 0.0f;

GameState *gState;

void initialise();
void processInput();
void update();
void render();
void shutdown();
std::vector<TerrainSegment> generateTerrain
(int screenWidth, int screenHeight, int startY, int minSegWidth, int maxSegWidth, int maxDeltaY);

// simple terrain gen algorithm
std::vector<TerrainSegment> generateTerrain
(int screenWidth, int screenHeight, int startY, int minSegWidth, int maxSegWidth, int maxDeltaY)
{
    
    std::vector<TerrainSegment> terrain;
    float currentX = 0.0f;
    float currentY = (float)GetRandomValue(startY - 40, startY + 40);
    // clamp to screen
    const float MIN_Y = 100.0f;
    const float MAX_Y = (float)screenHeight - 20.0f;

    while (currentX < screenWidth)
    {
        // random width
        float segWidth = (float)GetRandomValue(minSegWidth, maxSegWidth);
        // clamp to screen width
        if (currentX + segWidth > screenWidth)
        {
            segWidth = screenWidth - currentX;
            if (segWidth <= 0.0f) break;
        }

        Vector2 start = { currentX, currentY };
        Vector2 end;

        bool isSafe = (GetRandomValue(0, 99) < 20);
        // 20% chance of generating a platform
        // so if you're unlucky you might not get any safe platforms 

        if (isSafe)
        {
            end = { currentX + segWidth, currentY };
            // flat
        }
        else
        {
            float deltaY = (float)GetRandomValue(-maxDeltaY, maxDeltaY);
            // random altitude change
            float newY = currentY + deltaY;

            newY = fmaxf(newY, MIN_Y);
            newY = fminf(newY, MAX_Y); 
            // clamp to screen

            end = { currentX + segWidth, newY };
        }

        terrain.push_back({ start, end, isSafe });

        currentX = end.x;
        currentY = end.y;
        // to next segment
    }

    return terrain;

}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Proj3 - Lunar Lander");

    SetTargetFPS(FPS);
    gState = new GameState();

    // 3 states, idle, drift, and thrust, mapped to directions cuz idk what else
    std::map<Direction, std::vector<int>> animationAtlas;
    animationAtlas[DOWN] = {0};
    animationAtlas[RIGHT] = {1};
    animationAtlas[LEFT] = {1};
    animationAtlas[UP] = {2};

    gState->player = new Entity(
        PLAY_START_POS,
        PLAYER_SCALE,
        "assets/Spaceship_Asset.png",
        ATLAS,
        { 4.0f, 4.0f}, // 1 empty column
        animationAtlas
    );

    // initializing player spaceship
    gState->player->setAcceleration({ 0.0f, GRAVITY });
    gState->player->setDirection(DOWN);
    gState->player->setVelocity({ 0.0f, 0.0f });
    gState->player->setAngle(0.0f);

    // generating terrain
    gState->terrain = generateTerrain(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        TERRAIN_START_Y,
        MIN_SEGMENT_WIDTH,
        MAX_SEGMENT_WIDTH,
        MAX_ALTITUDE_DELTA
    );

    gState->platform = new Entity(
        PLATFORM_START_POS,
        PLATFORM_SIZE,
        nullptr
    );

    gState->platform->setVelocity({ PLATFORM_SPEED, 0.0f });
    gState->fuel = FUEL_START;
}

void processInput()
{
    if (IsKeyDown(KEY_ESCAPE) || WindowShouldClose()) gAppStatus = TERMINATED;
    if (gGameStatus != RUN) 
    {
        // reset game after win/loss
        if (IsKeyPressed(KEY_R))
        {
            // reset player
            gState->player->setPosition(PLAY_START_POS);
            gState->player->setVelocity({ 0.0f, 0.0f });
            gState->player->setAcceleration({ 0.0f, GRAVITY });
            gState->player->setAngle(0.0f);
            gState->player->setDirection(DOWN);

            // reset fuel
            gState->fuel = FUEL_START;

            // regenerate terrain
            gState->terrain = generateTerrain(
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                TERRAIN_START_Y,
                MIN_SEGMENT_WIDTH,
                MAX_SEGMENT_WIDTH,
                MAX_ALTITUDE_DELTA
            );

            // reset platform
            gState->platform->setPosition(PLATFORM_START_POS);
            gState->platform->setVelocity({ PLATFORM_SPEED, 0.0f });

            message = "";
            gGameStatus = RUN;
        }
        return; 
    }
    // default state
    Vector2 newAccel = { 0.0f, GRAVITY }; 
    gState->player->setDirection(DOWN);

    if (gState->fuel > 0.0f)
    {
        if (IsKeyDown(KEY_LEFT))
        {
            // just rotate left
            gState->player->setAngle(gState->player->getAngle() - ROTATION_SPEED);
            gState->player->setDirection(LEFT);
            gState->fuel -= FUEL_CONSUMPTION_RATE / 2.0f;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            // just rotate right
            gState->player->setAngle(gState->player->getAngle() + ROTATION_SPEED);
            gState->player->setDirection(RIGHT);
            gState->fuel -= FUEL_CONSUMPTION_RATE / 2.0f;
        }

        if (IsKeyDown(KEY_UP))
        {
            // get new acceleration based on rotation and a fixed thrust magnitude
            float angleRad = gState->player->getAngle() * (PI / 180.0f);
            float thrustX = sinf(angleRad) * UP_THRUST_POWER;
            float thrustY = -cosf(angleRad) * UP_THRUST_POWER;

            newAccel.x += thrustX; 
            newAccel.y += thrustY;

            gState->player->setDirection(UP);
            gState->fuel -= FUEL_CONSUMPTION_RATE;
            
        }
    }

    gState->player->setAcceleration(newAccel);
}

void update()
{
    if (gAppStatus != RUNNING) return;
    if (gGameStatus != RUN) return;
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // out of fuel
    if (gState->fuel <= 0.0f && gGameStatus == RUN)
    {
        gState->player->setAcceleration({ 0.0f, GRAVITY });
        gState->player->setDirection(DOWN);

    }

    // platform stuff
    Vector2 platPos = gState->platform->getPosition();
    Vector2 platVel = gState->platform->getVelocity();
    platPos.x += platVel.x * deltaTime;

    // handle platform out of screen bounds
    float halfW = PLATFORM_SIZE.x / 2.0f;
    if (platPos.x - halfW < 0.0f)
    {
        platPos.x = halfW;
        platVel.x = -platVel.x;
        gState->platform->setVelocity(platVel);
    }
    else if (platPos.x + halfW > SCREEN_WIDTH)
    {
        platPos.x = SCREEN_WIDTH - halfW;
        platVel.x = -platVel.x;
        gState->platform->setVelocity(platVel);
    }

    gState->platform->setPosition(platPos);

    gState->player->update(deltaTime, nullptr, 0);
    // collisions stuff
    Vector2 playerPos = gState->player->getPosition();
    Vector2 playerSize = gState->player->getScale();
    Rectangle playerRect = {
        playerPos.x - playerSize.x / 2.0f,
        playerPos.y - playerSize.y / 2.0f,
        playerSize.x,
        playerSize.y
    };
    
    Rectangle platformRect = {
        gState->platform->getPosition().x,
        gState->platform->getPosition().y,
        PLATFORM_SIZE.x,
        PLATFORM_SIZE.y
    };
    // win by landing on floating platform
    if (CheckCollisionRecs(playerRect, platformRect))
    {
        Vector2 velocity = gState->player->getVelocity();
        
        float angle = gState->player->getAngle();
        // angle constraint
        bool isLevel = (fmodf(angle, 360.0f) < MAX_ANGLE && fmodf(angle, 360.0f) > -MAX_ANGLE);
        // velocity constraint
        if (fabsf(velocity.y) < MAX_LANDING_SPEED_Y &&
            fabsf(velocity.x) < MAX_LANDING_SPEED_X && isLevel)
        {
            gGameStatus = SUCCESS;
        }
        else
        {
            gGameStatus = FAILED;
        }
        gState->player->setAcceleration({ 0.0f, 0.0f });
        gState->player->setVelocity({ 0.0f, 0.0f });
    }

    if (gGameStatus == RUN)
    {
        // win by landing on safe terrain
        // similar logic to platform landing
        Vector2 botLeft = { playerRect.x + 5.0f, playerRect.y + playerRect.height };
        Vector2 botRight = { playerRect.x + playerRect.width - 5.0f, playerRect.y + playerRect.height };

        for (const auto& segment : gState->terrain)
        {
            if (CheckCollisionPointLine(botLeft, segment.start, segment.end, 2) ||
                CheckCollisionPointLine(botRight, segment.start, segment.end, 2))
            {
                Vector2 velocity = gState->player->getVelocity();
                float angle = gState->player->getAngle();
                bool isLevel = (fmodf(angle, 360.0f) < MAX_ANGLE && fmodf(angle, 360.0f) > -MAX_ANGLE);

                if (segment.is_safe && 
                    fabsf(velocity.y) < MAX_LANDING_SPEED_Y &&
                    fabsf(velocity.x) < MAX_LANDING_SPEED_X && isLevel)
                {
                    gGameStatus = SUCCESS;
                }
                else
                {
                    gGameStatus = FAILED;
                }
                
                gState->player->setAcceleration({ 0.0f, 0.0f });
                gState->player->setVelocity({ 0.0f, 0.0f });
                break;
            }
        }
    }

}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOR));

    gState->player->render();
    // draw terrain with lines
    for (const TerrainSegment &segment : gState->terrain)
    {
        Color lineColor = segment.is_safe ? GREEN : WHITE;
        DrawLineV(segment.start, segment.end, lineColor);
    }

    // draw platform
    Vector2 platTopLeft = {
        gState->platform->getPosition().x - PLATFORM_SIZE.x / 2.0f,
        gState->platform->getPosition().y - PLATFORM_SIZE.y / 2.0f
    };
    DrawRectangleV(
        platTopLeft,
        PLATFORM_SIZE,
        BLUE
    );

    // fuel hud
    DrawText("FUEL", 20, 20, 20, LIGHTGRAY);

    float fuelPercent = gState->fuel / FUEL_START;

    // warning for low fuel
    if (fuelPercent < 0.2f && gGameStatus == RUN)
    {
        if (fmodf(GetTime() * 2.0f, 1.0f) < 0.5f) // blink effect, twice per second
        {
            DrawText("LOW FUEL!", SCREEN_WIDTH / 2 - MeasureText("LOW FUEL!", 20) / 2,
            SCREEN_HEIGHT / 2 - 60, 20, RED);
        }
        
    }
    // fuel bar color from green to red
    // b=0 always, rising red, falling green, linear
    // using slope = 2 but clamped to 0-255

    unsigned char r = (unsigned char)fminf(255.0f, (1.0f - fuelPercent) * 2.0f * 255.0f);

    unsigned char g = (unsigned char)fminf(255.0f, fuelPercent * 2.0f * 255.0f);

    unsigned char b = 0;

    Color fuelColor = { r, g, b, 255 };

    DrawRectangle(80, 20, 200, 20, DARKGRAY); // background bar
    DrawRectangle(80, 20, (int)(fuelPercent * 200.0f), 20, fuelColor); // fuel level

    // show velocity and angle
    Vector2 velocity = gState->player->getVelocity();
    std::string velX = "Vel X: " + std::to_string((int)velocity.x);
    std::string velY = "Vel Y: " + std::to_string((int)velocity.y);
    std::string angle = "Angle: " + std::to_string((int)gState->player->getAngle() % 360);
    DrawText(velX.c_str(), 20, 50, 20, LIGHTGRAY);
    DrawText(velY.c_str(), 20, 75, 20, LIGHTGRAY);
    DrawText(angle.c_str(), 20, 100, 20, LIGHTGRAY);

    // end game messages
    if (gGameStatus == FAILED)
    {
        message = "MISSION FAILED\nPress R to Restart";
        DrawText(message.c_str(), 
                 SCREEN_WIDTH / 2 - MeasureText(message.c_str(), 40) / 2, 
                 SCREEN_HEIGHT / 2 - 20, 40, RED);
    }
    else if (gGameStatus == SUCCESS)
    {
        message = "MISSION ACCOMPLISHED\nPress R to Restart";
        DrawText(message.c_str(), 
                 SCREEN_WIDTH / 2 - MeasureText(message.c_str(), 40) / 2, 
                 SCREEN_HEIGHT / 2 - 20, 40, GREEN);
    }
    EndDrawing();
}

void shutdown()
{
    delete gState->player;
    delete gState->platform;
    delete gState;
    CloseWindow();
}

int main()
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}