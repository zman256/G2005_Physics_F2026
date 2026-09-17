/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int screenWidth = 1200;
int screenHeight = 800;

const unsigned int TARGET_FPS = 50;
const float FIXED_DELTA_TIME = 1.0f / (float)TARGET_FPS;
 
// Struct to contain position and velcotiy state (for exercise 2)
struct PhysicBody
{
    Vector2 position;
    Vector2 velocity;
};

PhysicBody bird = {Vector2{-1000, -1000,}, Vector2{ 0, 0 }};


Vector2 launchPosition; // Slingshot position
float launchPositionAdjustmentSpeed = 50.0f;
float launchSpeed = 0.0f;
float launchAngle = 0.0f; 


int main()
{
    InitWindow(screenWidth, screenHeight, "Physics-1");
    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(Color{ 100, 100, 166, 225});
            // GUI
            DrawRectangle(0, 0, 260, 600, Color{255, 255, 255, 20 });
            GuiSliderBar(Rectangle{100, 5, 100, 20}, "LaunchSpeed", TextFormat("%.2f", launchSpeed), &launchSpeed, 1.0f, 500.0f);
            GuiSliderBar(Rectangle{100, 30, 100, 20}, "LaunchAngle", TextFormat("%.2f", launchSpeed), &launchSpeed, -90.f, 90.0f);

            if (IsKeyDown(KEY_UP))
            {
                launchPosition.y -= launchPositionAdjustmentSpeed * GetFrameTime();
            }
            if (IsKeyDown(KEY_DOWN))
            {
                launchPosition.y += launchPositionAdjustmentSpeed * GetFrameTime();
            }

            Vector2 velocityPreview = { cosf(launchAngle * DEG2RAD) * launchSpeed, sinf(launchAngle * DEG2RAD) * launchSpeed }; // use speed and angle
            DrawCircleV(launchPosition, 5, BROWN);
            DrawLineEx(launchPosition, launchPosition + velocityPreview, 2, BROWN);

            // Spawn Bird for lab 2
            if (IsKeyPressed(KEY_SPACE))
            {
                bird.position = launchPosition;
                bird.velocity = velocityPreview;
            }
            //Vector2 mouseDelta = launchPosition - GetMousePosition();
            



           /*  //Draw circle that follows the mouse
            DrawCircleV(birdPosition, 30, RED);
            Vector2 mousePos = GetMousePosition();
            DrawCircleLines(mousePos.x, mousePos.y, 50, DARKBROWN);

            //Thick lines
            DrawLineEx(Vector2{ 0.0f, 700.0f }, Vector2{ 1200, 700 }, 5, DARKGREEN);

            time += 1;

            GuiSliderBar(Rectangle{ 60, 5, 1000, 10 }, "Time", TextFormat("%.2f", time), &time, 0, 240); */


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
