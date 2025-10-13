/**
* Author: Eric Cheung
* Assignment: Simple 2D Scene
* Date due: 2025-09-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


/*
Asset Sources:
https://opengameart.org/content/planets-4
https://www.vhv.rs/viewpic/iwwmbwR_pixel-art-asteroid-sprite-hd-png-download/
https://pngtree.com/freepng/pixel-art-ufo-icon-design-vector_8533572.html
*/

#include "cs3113.h"

// Global Constants
constexpr int   SCREEN_WIDTH = 800,
                SCREEN_HEIGHT = 450,
                FPS = 60,
                ASTEROID_COUNT = 5;


constexpr Vector2   ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f },
                    gSunPosition = ORIGIN;

constexpr float SPRITE_SIZE = 200.0f / 3.0f, 
                EARTH_ORBIT_SPEED = 0.5f,
                MOON_ORBIT_SPEED = 2.0f,
                EARTH_ROTATION_SPEED = 50.0f,
                MOON_ROTATION_SPEED = 100.0f;

constexpr char PLANET_TEXTURE_PATH[] = "assets/Planets.png";
constexpr char ASTEROID_TEXTURE_PATH[] = "assets/asteroid.png";
constexpr char UFO_TEXTURE_PATH[] = "assets/ufo.png";

// Global Variables
AppStatus gAppStatus = RUNNING;
float   gPreviousTicks = 0.0f,
        gEarthRotation = 0.0f,
        gMoonRotation = 0.0f,
        gSunScale = 1.0f,
        gSunBaseSize = 100.0f,
        gEarthOrbitRadius = 150.0f,
        gEarthSize = 50.0f,
        gEarthOrbitAngle = 0.0f,
        gMoonOrbitRadius = 30.0f,
        gMoonSize = 20.0f,
        gMoonOrbitAngle = 0.0f,
        gUFOSize = 40.0f,
        gUFOProgress = 0.0f,
        gAsteroidSpeeds[ASTEROID_COUNT],
        gAsteroidSizes[ASTEROID_COUNT];

Vector2 gEarthPosition,
        gMoonPosition,
        gUFOPosition = { -100.0f, SCREEN_HEIGHT / 3.0f },
        gAsteroidPositions[ASTEROID_COUNT];

Texture2D gPlanetsTexture;
Texture2D gAsteroidTexture;
Texture2D gUFOTexture;

// Source rectangles for planets sprite sheet

Rectangle gSunSource = { 60, 50, 80, 80 }; // it's actually Jupiter. Also not quite aligned in the sprite sheet
Rectangle gEarthSource = { SPRITE_SIZE * 2, 0, SPRITE_SIZE, SPRITE_SIZE };
Rectangle gMoonSource = { 0, 0, SPRITE_SIZE, SPRITE_SIZE }; // it's actually Mercury 


Rectangle asteroidSource;
Rectangle ufoSource;

Color bgColor = ColorFromHex("#201F3D");

// Function Prototypes
void initialise();
void processInput();
void update();
void render();
void shutdown();


// added source rectangle and rotation params for convenience
void RenderObject(const Texture2D *texture, const Rectangle *source, const Vector2 *position,
                  const Vector2 *scale, const float rotation)
{

    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    DrawTexturePro(*texture, *source, destinationArea, originOffset,
                   rotation, WHITE);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple 2D Scene");
    SetTargetFPS(FPS);
    
    gPlanetsTexture = LoadTexture(PLANET_TEXTURE_PATH);
    gAsteroidTexture = LoadTexture(ASTEROID_TEXTURE_PATH);
    gUFOTexture = LoadTexture(UFO_TEXTURE_PATH);
    
    asteroidSource = { 0, 0, (float)gAsteroidTexture.width, (float)gAsteroidTexture.height };
    ufoSource = { 0, 0, (float)gUFOTexture.width, (float)gUFOTexture.height };
    
    for (int i = 0; i < ASTEROID_COUNT; i++) {
        // using raylib random functions over std for simplicity
        // starting positions outside the screen
        gAsteroidPositions[i].x = GetRandomValue(-100, SCREEN_WIDTH);
        gAsteroidPositions[i].y = GetRandomValue(-100, 0); 
        gAsteroidSpeeds[i] = GetRandomValue(50, 100); 
        gAsteroidSizes[i] = GetRandomValue(5, 12); 
    }
}

void processInput()
{
    if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose())
        gAppStatus = TERMINATED;
}


void update()
{
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    gSunScale = 1.0f + 0.05f * sin(ticks * 0.5f);

    gEarthRotation += EARTH_ROTATION_SPEED * deltaTime;
    gEarthOrbitAngle += EARTH_ORBIT_SPEED * deltaTime;

    gEarthPosition.x = gSunPosition.x + gEarthOrbitRadius * cos(gEarthOrbitAngle);
    gEarthPosition.y = gSunPosition.y + gEarthOrbitRadius * sin(gEarthOrbitAngle);

    gMoonRotation += MOON_ROTATION_SPEED * deltaTime;  
    gMoonOrbitAngle += MOON_ORBIT_SPEED * deltaTime;

    gMoonPosition.x = gEarthPosition.x + gMoonOrbitRadius * cos(gMoonOrbitAngle);
    gMoonPosition.y = gEarthPosition.y + gMoonOrbitRadius * sin(gMoonOrbitAngle);

    for (int i = 0; i < ASTEROID_COUNT; i++) {
        gAsteroidPositions[i].x += gAsteroidSpeeds[i] * deltaTime;
        gAsteroidPositions[i].y += gAsteroidSpeeds[i] * deltaTime * 0.7f;  // diagonal movement
        
        // wrap around when they go off screen
        if (gAsteroidPositions[i].x > SCREEN_WIDTH + 50 || 
            gAsteroidPositions[i].y > SCREEN_HEIGHT + 50) {
            gAsteroidPositions[i].x = GetRandomValue(-200, -50);
            gAsteroidPositions[i].y = GetRandomValue(-200, -50);
        }
    }

    gUFOProgress += deltaTime * (0.1f * GetRandomValue(10, 30));
    gUFOPosition.x += gUFOProgress;
    gUFOPosition.y = SCREEN_HEIGHT / 3.0f +
                     sin(gUFOProgress * 2.5f) * 60.0f;

    if (gUFOPosition.x > SCREEN_WIDTH + 50) {
        gUFOPosition.x = -100.0f;
        gUFOProgress = 0.0f;
    }

    // bgcolor cycle
    float colorCycle = (sin(ticks * 0.5f) + 1.0f) * 0.5f; // sine wave [0, 1]
    // #201F3D to #000000
    bgColor = {
        (unsigned char)(32 * colorCycle),
        (unsigned char)(31 * colorCycle),
        (unsigned char)(61 * colorCycle),
        255
    };
}


void render()
{
    BeginDrawing();
    ClearBackground(bgColor);


    Vector2 sunScale = { 
        gSunBaseSize * gSunScale, 
        gSunBaseSize * gSunScale 
    };
    RenderObject(&gPlanetsTexture, &gSunSource, &gSunPosition, 
                &sunScale, 0.0f);

    Vector2 earthScale = { gEarthSize, gEarthSize };
    RenderObject(&gPlanetsTexture, &gEarthSource, &gEarthPosition, 
                &earthScale, gEarthRotation);

    Vector2 moonScale = { gMoonSize, gMoonSize };
    RenderObject(&gPlanetsTexture, &gMoonSource, &gMoonPosition, 
                &moonScale, gMoonRotation);

    for (int i = 0; i < ASTEROID_COUNT; i++) {
        Vector2 asteroidScale = { 
            (float)gAsteroidSizes[i], 
            (float)gAsteroidSizes[i]
        };
        float rotation = gAsteroidPositions[i].x + gAsteroidPositions[i].y;
        RenderObject(&gAsteroidTexture, &asteroidSource, &gAsteroidPositions[i],
                    &asteroidScale, rotation);
    }

    Vector2 ufoScale = { gUFOSize, gUFOSize };
    RenderObject(&gUFOTexture, &ufoSource, &gUFOPosition,
                &ufoScale, 0.0f);
    EndDrawing();
}

void shutdown()
{
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