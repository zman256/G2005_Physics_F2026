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

Vector2 launchPosition = { 30, (float)(screenHeight - 50) }; // Slingshot position
float launchPositionAdjustmentSpeed = 50.0f;
float launchSpeed = 0.0f;
float launchAngle = 0.0f;  // Deg
float spawnDrag = 0.0f; // Damping value to apply to physics object velocities

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
    Color trailColor = BLACK;
    std::vector<Vector2> trail;
    static const int trail_Length = 800;
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

                //create trail
                bodies[i].trail.push_back(bodies[i].position);
            }

            // loop backwards so erasing a bird doesn't skip the next one
            for (int i = bodies.size() - 1; i >= 0; i--)
            {
                if (bodies[i].trail.size() > bodies[i].trail_Length)
                {
                    bodies[i].trail.erase(bodies[i].trail.begin()); // Remove oldest point
                }

                //destroy birds when off screen
                Vector2 p = bodies[i].position;
                if (p.y > screenHeight + 100 || p.x > screenWidth + 100 || p.x < -100 || p.y < -100)
                {
                    bodies.erase(bodies.begin() + i);
                }
            }
        }

        void Draw()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                //draw trail first so the bird is drawn on top of it
                for (int t = 1; t < bodies[i].trail.size(); t += 4)
                {
                    DrawCircleV(bodies[i].trail[t], 2, bodies[i].trailColor);                    
                }

                DrawCircleV(bodies[i].position, bodies[i].radius, bodies[i].color);
            }
        }
};

// PhysicsBody bird; // = {Vector2{-1000, -1000,}, Vector2{ 0, 0 }};
PhysicsSimulation sim;



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
            DrawCircleV(launchPosition, 10, BROWN);
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
                birdToLaunch.trailColor = BLACK;

                sim.bodies.push_back(birdToLaunch);
            }
            
            sim.Update();
            sim.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
