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
// I will use spawn radius to create the expanding bird. for now its just assigned to each birds radius. 
float spawnRadius = 10.0f;

// Struct to contain position and velocity state (for exercise 2)
// to not have to copy pase the values in physics body, u can use inheritance. preferred method is using composition 

// Circle and half plane detection need to work differently
enum ColliderShape
{
    CIRCLE,
    HALFPLANE
};
class Collider
{
public:
    virtual ~Collider() = default;
    virtual ColliderShape GetShape() = 0; // pure virtual function or "abstract" function to use polymorphism

    virtual void Draw() {};
};
class Circle : public Collider
{
public:
    float radius;

private:
    ColliderShape GetShape() override
    {
        return CIRCLE;
    }
};
class HalfPlane : public Collider
{
    Vector2 normal;
    Vector2 position2;

    ColliderShape GetShape() override
    {
        return HALFPLANE;
    }

    // need to use cross product to get normal
    Vector2 GetNormal()
    {
        return {};
    }
};

class PhysicsBody
{
    public:
    Vector2 position = Vector2 { 0, 0 };
    Vector2 velocity = Vector2 { 0, 0 };
    float mass = 1.0f;
    float drag = 0.5f;
    float gravityScale = 1.0f;
    Color color = RED;
    float radius = 10.0f;
    Color trailColor = BLACK;
    std::vector<Vector2> trail;
    static const int trail_Length = 800;

    Collider* collider = nullptr;
};


//using const in this function stops the function from changing the a and b
bool isOverlappingCircle(const PhysicsBody& a, const PhysicsBody &b)
{
    Vector2 displacement = b.position - a.position;
    float distanceSq = Vector2DotProduct(displacement, displacement);

    float radA = ((Circle*)a.collider)->radius; // cast to circle then get radius
    float radB = ((Circle*)b.collider)->radius; 

    float sumRadius = radA + radB; // instead of square rooting, just multiply
    return distanceSq < sumRadius * sumRadius; // if the dot product is less than the circle radius its overlapping
}

bool isOverlappingCircleHalfPlane(const PhysicsBody& b, const PhysicsBody& a)
{
    return true;
}

bool isOverlapping(PhysicsBody a, PhysicsBody b)
{
    Vector2 displacement = b.position - a.position;
    float distanceSq = Vector2DotProduct(displacement, displacement);
    float sumRadius = a.radius + b.radius; // instead of square rooting, just multiply
    return distanceSq < sumRadius * sumRadius; // if the dot product is less than the circle radius its overlapping
}

struct Bounds2D
{
    Bounds2D(float top, float bottom, float left, float right)
    {
        min = { left, bottom};
        max = { right, top };
    }

    bool Contains (Vector2 point)
    {
        return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
    }

private:
    Vector2 min;
    Vector2 max;
};


class PhysicsSimulation
{
    public:
        Vector2 gravity = { 0, 200 }; // Global acceleration due to gravity, in pixels/second/second
        const float FIXED_DELTA_TIME = 1.0f / (float)TARGET_FPS; // A fixed delta time variable is important to physics simulations
        Bounds2D bounds = Bounds2D((float)screenHeight, 0, 0, (float)screenWidth); // Bodies outside this area get removed

        // to check if two vectors overlap, they must be parallel (cross product of their direction vectors equals 0) and the scalar projection of one segment has to fall within the range of the other
        // projection = dividing the dot product of the two vectors by the magnitude of the vector you are projecting to
        void CheckCollisions()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                bodies[i].color = GREEN;
            }
            // check collisions
            for (int i = 0; i < bodies.size(); i++)
            {
                for (int j = i + 1; j < bodies.size(); j++)
                {
                    PhysicsBody* bodyA = &bodies[i];
                    PhysicsBody* bodyB = &bodies[j];

                    bool isOverlapping = false;

                    //Circle collision
                    // Circle A, Circle B
                    // dont use . because bodies is a ptr. bodyA->collider is the same as (*bodyA).collider
                    if (bodyA->collider->GetShape() == CIRCLE && bodyB->collider->GetShape() == CIRCLE)
                    {
                        isOverlapping = isOverlappingCircle(*bodyA, *bodyB);
                    }
                    else if (bodyA->collider->GetShape() == CIRCLE && bodyB->collider->GetShape() == HALFPLANE)
                    {
                        // circle-halfplane collision
                        isOverlappingCircle(*bodyA, *bodyB);
                    } 
                    else if (bodyA->collider->GetShape() == HALFPLANE && bodyB->collider->GetShape() == CIRCLE)
                    {
                        // REMOVE the = later
                        isOverlapping = isOverlappingCircleHalfPlane(*bodyB, *bodyA);
                    }
                    
                    if (isOverlapping)
                    {
                        // bodies[i].color = RED;
                        // bodies[j].color = RED;  
                        bodyA->color = RED;
                        bodyB->color = BLUE;
                    }
                    
                    //HalfPlane collision
                    
                }
            }
        }

        void Update()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                //velocity is defined in pixels/second --> we need pixels/frame
                bodies[i].position += bodies[i].velocity * FIXED_DELTA_TIME;

                //acceleration is change in velocity over time, gravity is our acceleration in pixels/sec/sec (px/sec)
                bodies[i].velocity += gravity * bodies[i].gravityScale * FIXED_DELTA_TIME;

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
                    delete bodies[i].collider;
                    bodies.erase(bodies.begin() + i);
                }
            }
        }

        // drop the radius field in Physics body once Draw gets the radius from the collider
        void Draw()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                //draw trail first so the bird is drawn on top of it
                for (int t = 1; t < bodies[i].trail.size(); t += 6)
                {
                    DrawCircleV(bodies[i].trail[t], 2, bodies[i].trailColor);                    
                }
                    //remove radius once my virtual draw function draws each shape itself
                    float radius = ((Circle*)bodies[i].collider)->radius;
                    DrawCircleV(bodies[i].position, radius, bodies[i].color);
            }
        }

        // Add making a copy
        void Add(PhysicsBody body)
        {
            bodies.push_back(body);
        }

        void ClearOutOfBoundsBodies()
        {
            for (auto it = bodies.begin(); it != bodies.end(); )
            {
                if (!bounds.Contains(it -> position))
                {
                    delete it->collider;
                    it = bodies.erase(it); // erase returns the next body, so don't advance
                }
                else
                {
                    it++;
                }
            }
        }

        ~PhysicsSimulation()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                delete bodies[i].collider;
            }
        }
        
    private:
    std::vector<PhysicsBody> bodies; // A container of all PhysicsBody in the simulation
};

// PhysicsBody bird; // = {Vector2{-1000, -1000,}, Vector2{ 0, 0 }};
PhysicsSimulation sim;
float margin = 0;
// sim.bounds = { Vector2 {0 - margin, screenHeight + margin, 0 - margin, screenWidth + margin} };

int main()
{
    sim.bounds = Bounds2D(screenHeight + margin, 0 - margin, 0 - margin, screenWidth + margin);
    InitWindow(screenWidth, screenHeight, "Physics-1");
    SetTargetFPS(TARGET_FPS);

    PhysicsBody target;
    target.radius = 80.0f;
    target.gravityScale = 0;
    target.position = { 800, (float)screenHeight - 300};
    Circle* targetCircle = new Circle;
    targetCircle->radius = target.radius;
    target.collider = targetCircle;
    sim.Add(target);

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
            GuiSliderBar(Rectangle{100, 120, 100, 20}, "Radius", TextFormat("%.2f", spawnRadius), &spawnRadius, 2, 50);
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
                birdToLaunch.radius = spawnRadius;
                birdToLaunch.position = launchPosition;
                birdToLaunch.velocity = velocityPreview;
                birdToLaunch.drag = spawnDrag;
                birdToLaunch.trailColor = BLACK;

                Circle* birdCircle = new Circle();
                birdCircle->radius = birdToLaunch.radius;
                birdToLaunch.collider = birdCircle; // a Circle* can be stored in a Collider* (Composition "has a")

                sim.Add(birdToLaunch);
            }
            
            sim.Update();
            sim.CheckCollisions();
            sim.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
