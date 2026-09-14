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

const unsigned int TARGET_FPS = 60;
float time = 0;
Vector2 birdPosition;

int main()
{
    InitWindow(screenWidth, screenHeight, "Massan");
    SetTargetFPS(TARGET_FPS);
    birdPosition = {30.0f, (float)(screenHeight)};
    float circleRadius = 40.0f;
    Vector2 circlePosition = { (float)screenWidth / 2, (float)screenHeight / 2 };
    Vector2 circleSpeed = { 5.0f, 4.0f };
    

    while (!WindowShouldClose())
    {
        //bouncing circle
        circlePosition.x += circleSpeed.x;
        circlePosition.y += circleSpeed.y;

        if ((circlePosition.x >= (screenWidth - circleRadius)) || (circlePosition.x <= circleRadius))
        {
            circleSpeed.x *= -1.0f;
        }
        if ((circlePosition.y >= (screenHeight - circleRadius)) || (circlePosition.y <= circleRadius))
        {
            circleSpeed.y *= -1.0f;
        }

        // 3D CAMERA
        Camera3D camera = {0};
        camera.position = (Vector3) { 0.0f, 10.0f, 10.0f};
        camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
        camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
        camera.fovy = 50.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

        BeginDrawing();
            ClearBackground(WHITE);
            DrawText("Hello world!", 10, 10, 20, LIGHTGRAY);
            DrawCircleV(birdPosition, 30.0f, RED);
            //Bouncy Ball
            DrawCircleV(circlePosition, circleRadius, BLUE); 

            // Draw circle that follows the mouse
            Vector2 mousePos = GetMousePosition();
            DrawCircleLines(mousePos.x, mousePos.y, 30, BLACK);

            time += 1;

            GuiSliderBar(Rectangle{ 60, 5, 1000, 10 }, "Time", TextFormat("%.2f", time), &time, 0, 240);

            BeginMode3D(camera);
                
            DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
            DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

            DrawGrid(10, 1.0f);

            EndMode3D();


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
