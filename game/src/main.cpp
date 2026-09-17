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
float time = 0;
Vector2 birdPosition;


int main()
{
    InitWindow(screenWidth, screenHeight, "Physics-1");
    SetTargetFPS(TARGET_FPS);
    birdPosition = { 30, (float)(screenHeight - 50) };

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawText("Hello world!", 10, 10, 20, LIGHTGRAY);


            DrawCircleV(birdPosition, 30, RED);


            //Draw circle that follows the mouse
            Vector2 mousePos = GetMousePosition();
            DrawCircleLines(mousePos.x, mousePos.y, 50, DARKBROWN);

            //Thick lines
            DrawLineEx(Vector2{ 0.0f, 700.0f }, Vector2{ 1200, 700 }, 5, DARKGREEN);

            time += 1;

            GuiSliderBar(Rectangle{ 60, 5, 1000, 10 }, "Time", TextFormat("%.2f", time), &time, 0, 240);


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
