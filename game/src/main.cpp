/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <vector>

int screenWidth = 1600;
int screenHeight = 1400;

const unsigned int TARGET_FPS = 50;


// Struct to contain position and velocity state (for exercise 2)
class PhysicsBody
{
    public:
    Vector2 position = Vector2 { 0, 0 };
    Vector2 velocity = Vector2 { 0, 0 };
    float mass = 1.0f;
    float drag = 0.5f;
    Color color = RED;
    float radius = 10.0f;
};

class PhysicsSimulation
{
    public:
        std::vector<PhysicsBody> bodies; // A container of all PhysicsBody in the simulation
        Vector2 gravity = { 0, 200 }; // Global acceleration due to gravity, in pixels/second/second
        const float FIXED_DELTA_TIME = 1.0f / (float)TARGET_FPS; // A fixed delta time variable is important to physics simulations

        void Update()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                //velocity is defined in pixels/second --> we need pixels/frame
                bodies[i].position += bodies[i].velocity * FIXED_DELTA_TIME;
                //acceleration is change in velocity over time, gravity is our acceleration in pixels/sec/sec (px/sec)
                bodies[i].velocity += gravity * FIXED_DELTA_TIME;

                //apply drag to counteract velocity
                bodies[i].velocity *= 1.0f - (bodies[i].drag * FIXED_DELTA_TIME);
            }
        }

        void Draw()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                DrawCircleV(bodies[i].position, bodies[i].radius, bodies[i].color);
            }
        }
};

// PhysicsBody bird; // = {Vector2{-1000, -1000,}, Vector2{ 0, 0 }};
PhysicsSimulation sim;

Vector2 launchPosition = { 30, (float)(screenHeight - 50) }; // Slingshot position
float launchPositionAdjustmentSpeed = 50.0f;
float launchSpeed = 0.0f;
float launchAngle = 0.0f;  // Deg

float spawnDrag = 0.0f; // Damping value to apply to physics object velocities
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
            GuiSliderBar(Rectangle{100, 0, 100, 20}, "LaunchSpeed", TextFormat("%.2f", launchSpeed), &launchSpeed, 1.0f, 500.0f);
            GuiSliderBar(Rectangle{100, 30, 100, 20}, "LaunchAngle", TextFormat("%.2f", launchAngle), &launchAngle, -89.7f, 89);
            GuiSliderBar(Rectangle{100, 60, 100, 20}, "Gravity", TextFormat("%.2f", sim.gravity.y), &sim.gravity.y, -1000, 1000);
            GuiSliderBar(Rectangle{100, 90, 100, 20}, "Drag", TextFormat("%.2f", spawnDrag), &spawnDrag, 0, 10);
            DrawText("Game Physics - Massan Kudsia-Meade 101620204", 1000, 20, 20, BLACK);

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
            DrawLineEx(launchPosition, launchPosition + velocityPreview, 2, GREEN);

            // Spawn Bird for lab 2
            if (IsKeyPressed(KEY_SPACE))
            {
                PhysicsBody birdToLaunch;
                birdToLaunch.color = RED;
                birdToLaunch.radius = 10;
                birdToLaunch.position = launchPosition;
                birdToLaunch.velocity = velocityPreview;
                birdToLaunch.drag = spawnDrag;

                sim.bodies.push_back(birdToLaunch);
            }
            
            sim.Update();
            sim.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
