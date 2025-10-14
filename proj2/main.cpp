/**
* Author: Eric Cheung
* Assignment: Pong Clone
* Date due: 2025-10-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#include "cs3113.h"
#include "Entity.h"
#include <iostream>


// Global Constants
constexpr int   SCREEN_WIDTH = 800,
                SCREEN_HEIGHT = 450,
                FPS = 60,
                MAX_BALLS = 3;

constexpr float PADDLE_SPEED    = 400.0f;

enum GameState { PLAYING, GAME_OVER };


// Global Variables
AppStatus gAppStatus = RUNNING;
GameState gGameState = PLAYING;
float   gPreviousTicks = 0.0f;
bool gSinglePlayer = false;
int gActiveBallCount = 1;
std::string gWinnerMessage = "";

Entity gPlayer1;
Entity gPlayer2;
Entity gBalls[MAX_BALLS];

void initialise();
void processInput();
void update();
void render();
void shutdown();
void resetBall(int index);


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

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Clone!");
    SetTargetFPS(FPS);

    // Initialise player paddles and balls

    // using default constructor because the 2 paddles uses the
    // same asset and that cause some issues
    gPlayer1 = Entity();

    gPlayer2 = Entity();

    gPlayer1.setTexture("assets/paddle.png");
    gPlayer1.setPosition({ 30.0f, SCREEN_HEIGHT / 2.0f });
    gPlayer1.setScale({ 20.0f, 100.0f });
    gPlayer1.setColliderDimensions({ 20.0f, 100.0f });

    gPlayer2.setTexture("assets/paddle.png");
    gPlayer2.setPosition({ SCREEN_WIDTH - 30.0f, SCREEN_HEIGHT / 2.0f });
    gPlayer2.setScale({ 20.0f, 100.0f });
    gPlayer2.setColliderDimensions({ 20.0f, 100.0f });

    for (int i = 0; i < MAX_BALLS; ++i)
    {
        gBalls[i] = Entity(
            { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, 
            { 20.0f, 20.0f }, 
            "assets/ball.png", 
            EntityType::NONE
        );
        resetBall(i);
        if (i >= gActiveBallCount)
            gBalls[i].deactivate();
    }
}

void processInput()
{
    if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
        gAppStatus = TERMINATED;

    if (gGameState == GAME_OVER) {
        // press R to reset
        if (IsKeyPressed(KEY_R)) {
            gGameState = PLAYING;
            gWinnerMessage = "";
            for (int i = 0; i < gActiveBallCount; ++i)
                resetBall(i);
        }
    }

    gPlayer1.resetMovement();
    gPlayer2.resetMovement();

    if (IsKeyDown(KEY_W)) gPlayer1.moveUp();
    if (IsKeyDown(KEY_S)) gPlayer1.moveDown();

    if (!gSinglePlayer)
    {
        // Player 2 controls
        if (IsKeyDown(KEY_UP)) gPlayer2.moveUp();
        if (IsKeyDown(KEY_DOWN)) gPlayer2.moveDown();
    }
    // Mode toggle
    if (IsKeyPressed(KEY_T))
    {
        gSinglePlayer = !gSinglePlayer;
    }
    // Adjust number of balls
    int newBallCount = gActiveBallCount;
    if (IsKeyPressed(KEY_ONE)) newBallCount = 1;
    if (IsKeyPressed(KEY_TWO)) newBallCount = 2;
    if (IsKeyPressed(KEY_THREE)) newBallCount = 3;

    if (newBallCount != gActiveBallCount)
    {
        gActiveBallCount = newBallCount;
        for (int i = 0; i < MAX_BALLS; ++i)
        {
            if (i < gActiveBallCount)
            {
                resetBall(i);
                gBalls[i].activate();
            }
            else
            {
                gBalls[i].deactivate();
            }
        }
    }
}

void update()
{
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    if (gGameState == GAME_OVER) return;

    // AI for Player 2
    // just move towards the closest ball by x axis
    if (gSinglePlayer)
    {
        Entity *ballToTrack = &gBalls[0];
        float minDistance = SCREEN_WIDTH;
        for (int i = 0; i < gActiveBallCount; ++i)
        {
            float dist = fabs(gBalls[i].getPosition().x - gPlayer2.getPosition().x);
            if (dist < minDistance)
            {
                minDistance = dist;
                ballToTrack = &gBalls[i];
            }
        }
        float diff = ballToTrack->getPosition().y - gPlayer2.getPosition().y;
        float acceleration = 0.0f;
        acceleration = diff * 4.0f;
        gPlayer2.setAcceleration({ 0.0f, acceleration });
        gPlayer2.update(deltaTime, nullptr, 0);
    }

    // update paddles
    Vector2 p1_pos = gPlayer1.getPosition();
    p1_pos.y += gPlayer1.getMovement().y * PADDLE_SPEED * deltaTime;
    gPlayer1.setPosition(p1_pos);

    Vector2 p2_pos = gPlayer2.getPosition();
    p2_pos.y += gPlayer2.getMovement().y * PADDLE_SPEED * deltaTime;
    gPlayer2.setPosition(p2_pos);

    // screen bounds for paddles
    float paddleHeight = gPlayer1.getScale().y;
    if (gPlayer1.getPosition().y < paddleHeight / 2.0f) 
        gPlayer1.setPosition({ p1_pos.x, paddleHeight / 2.0f });
    if (gPlayer1.getPosition().y > SCREEN_HEIGHT - paddleHeight / 2.0f)
    gPlayer1.setPosition({ p1_pos.x, SCREEN_HEIGHT - paddleHeight / 2.0f });
    if (gPlayer2.getPosition().y < paddleHeight / 2.0f)
        gPlayer2.setPosition({ p2_pos.x, paddleHeight / 2.0f });
    if (gPlayer2.getPosition().y > SCREEN_HEIGHT - paddleHeight / 2.0f)
        gPlayer2.setPosition({ p2_pos.x, SCREEN_HEIGHT - paddleHeight / 2.0f });

    // update balls

    for (int i = 0; i < gActiveBallCount; ++i)
    {
        if (!gBalls[i].isActive()) continue;

        // move
        Vector2 b_pos = gBalls[i].getPosition();
        Vector2 b_vel = gBalls[i].getVelocity();
        b_pos.x += b_vel.x * deltaTime;
        b_pos.y += b_vel.y * deltaTime;
        gBalls[i].setPosition(b_pos);

        // collision
        float ballRadius = gBalls[i].getScale().x / 2.0f;

        // top and bottom walls
        if (b_pos.y - ballRadius < 0.0f || b_pos.y + ballRadius > SCREEN_HEIGHT)
        {
            b_vel.y = -b_vel.y;
            gBalls[i].setVelocity(b_vel);
        }

        // paddles
        if ((b_vel.x < 0 && gBalls[i].isCollidingWith(&gPlayer1)) ||
            (b_vel.x > 0 && gBalls[i].isCollidingWith(&gPlayer2)))
        {
            // reflect ball with slightly higher speed
            b_vel.x = -b_vel.x * 1.05f;
            gBalls[i].setVelocity(b_vel);
        }

        // GAME OVER
        if (b_pos.x - ballRadius < 0.0f)
        {
            gGameState = GAME_OVER;
            gWinnerMessage = "Player 2 Wins!\nPress R to Restart";
        }
        if (b_pos.x + ballRadius > SCREEN_WIDTH)
        {
            gGameState = GAME_OVER;
            gWinnerMessage = "Player 1 Wins!\nPress R to Restart";
        }
    }

}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    
    gPlayer1.render();
    gPlayer2.render();

    for (int i = 0; i < gActiveBallCount; ++i) {
        if (gBalls[i].isActive()) {
            gBalls[i].render();
        }
    }
    if (gGameState == GAME_OVER){
        int textWidth = MeasureText(gWinnerMessage.c_str(), 40);
        DrawText(gWinnerMessage.c_str(), SCREEN_WIDTH / 2 - textWidth / 2, SCREEN_HEIGHT / 2 - 20, 40, YELLOW);
    }

    EndDrawing();
}

void shutdown()
{
    CloseWindow();
}

// Resets ball to center with random velocity
void resetBall(int index)
{
    if (index < 0 || index >= MAX_BALLS) return;

    gBalls[index].setPosition({ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f });

    float vel_x = (GetRandomValue(0, 1) == 0 ? -250 : 250);
    float vel_y = GetRandomValue(-150, 150);
    gBalls[index].setVelocity({ vel_x, vel_y });
}