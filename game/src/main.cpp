/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

// Note: in raylib, 2D coordinate system is x-right, y-down. Angles: positive goes clockwise, negative goes counter
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
// Shape part of a body. Abstract (pure virtual functions), so only Circle / HalfPlane can be created
class Collider
{
public:
    virtual ~Collider() = default; // so delete on a Collider* destroys the Circle/HalfPlane correctly
    virtual ColliderShape GetShape() = 0; // pure virtual function or "abstract" function to use polymorphism
    virtual void Draw(Vector2 position, Color color) = 0; // the body passes in its position and color
};

class Circle : public Collider
{
public:
    float radius;

    Circle(float radius) : radius(radius) {} // radius has to be given, so it's never garbage

    ColliderShape GetShape() override
    {
        return CIRCLE;
    }

    void Draw(Vector2 position, Color color) override
    {
        DrawCircleV(position, radius, color);
    }
};

class HalfPlane : public Collider
{
public:
    Vector2 normal = { 0, -1 };
    Vector2 position2 = { 0, 0 };

    HalfPlane(Vector2 normal) : normal(normal) {}

    ColliderShape GetShape() override
    {
        return HALFPLANE;
    }

    void Draw(Vector2 position, Color color) override {} // TODO: draw the plane's line

    // need to use cross product to get normal
    Vector2 GetNormal()
    {
        return normal;
    }
};

// Composition "has a": every body has a collider that decides its shape
class PhysicsBody
{
public:
    Vector2 position = Vector2 { 0, 0 };
    Vector2 velocity = Vector2 { 0, 0 };
    float mass = 1.0f;
    float drag = 0.0f;
    float gravityScale = 1.0f;
    Color color = RED;
    Color trailColor = BLACK;
    std::vector<Vector2> trail;
    static const int trail_Length = 800;

    Collider* collider = nullptr;

    PhysicsBody(Vector2 position, Collider* collider, Vector2 velocity = { 0, 0 }, float drag = 0.0f, float gravityScale = 1.0f)
        : position(position), velocity(velocity), drag(drag), gravityScale(gravityScale), collider(collider) {}

    ~PhysicsBody()
    {
        delete collider; // the body owns its collider
    }

    // no copies: two copies would both delete the same collider
    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;
};


//using const in this function stops the function from changing the a and b
bool isOverlappingCircle(const PhysicsBody& a, const PhysicsBody& b)
{
    Vector2 displacement = b.position - a.position;
    float distanceSq = Vector2DotProduct(displacement, displacement);

    float radA = ((Circle*)a.collider)->radius; // cast to circle then get radius
    float radB = ((Circle*)b.collider)->radius;

    float sumRadius = radA + radB; // instead of square rooting, just multiply
    return distanceSq < sumRadius * sumRadius; // if the dot product is less than the circle radius its overlapping
}

bool isOverlappingCircleHalfPlane(const PhysicsBody& circle, const PhysicsBody& plane)
{
    return true;
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
                bodies[i]->color = GREEN;
            }
            // check collisions
            for (int i = 0; i < bodies.size(); i++)
            {
                for (int j = i + 1; j < bodies.size(); j++)
                {
                    PhysicsBody* bodyA = bodies[i]; // already a pointer, so no & needed
                    PhysicsBody* bodyB = bodies[j];

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
                        isOverlapping = isOverlappingCircleHalfPlane(*bodyA, *bodyB);
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
                bodies[i]->position += bodies[i]->velocity * FIXED_DELTA_TIME;

                //acceleration is change in velocity over time, gravity is our acceleration in pixels/sec/sec (px/sec)
                bodies[i]->velocity += gravity * bodies[i]->gravityScale * FIXED_DELTA_TIME;

                //apply drag to counteract velocity
                bodies[i]->velocity *= 1.0f - (bodies[i]->drag * FIXED_DELTA_TIME);

                //create trail
                bodies[i]->trail.push_back(bodies[i]->position);
            }

            // loop backwards so erasing a bird doesn't skip the next one
            for (int i = bodies.size() - 1; i >= 0; i--)
            {
                if (bodies[i]->trail.size() > bodies[i]->trail_Length)
                {
                    bodies[i]->trail.erase(bodies[i]->trail.begin()); // Remove oldest point
                }

                //destroy birds when off screen
                Vector2 p = bodies[i]->position;
                if (p.y > screenHeight + 100 || p.x > screenWidth + 100 || p.x < -100 || p.y < -100)
                {
                    delete bodies[i]; // free the body itself before removing its pointer
                    bodies.erase(bodies.begin() + i);
                }
            }
        }

        void Draw()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                //draw trail first so the bird is drawn on top of it
                for (int t = 1; t < bodies[i]->trail.size(); t += 6)
                {
                    DrawCircleV(bodies[i]->trail[t], 2, bodies[i]->trailColor);
                }

                bodies[i]->collider->Draw(bodies[i]->position, bodies[i]->color); // virtual: each shape draws itself
            }
        }

        // the simulation owns the body now and deletes it when it's removed
        void Add(PhysicsBody* body)
        {
            bodies.push_back(body);
        }

        void ClearOutOfBoundsBodies()
        {
            for (auto it = bodies.begin(); it != bodies.end(); )
            {
                if (!bounds.Contains((*it)->position))
                {
                    delete *it;
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
                delete bodies[i];
            }
        }

    private:
    // pointers, because PhysicsBody can't be copied (it owns its collider)
    std::vector<PhysicsBody*> bodies; // A container of all PhysicsBody in the simulation
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

    // position, collider, velocity, drag, gravityScale (0 = static)
    sim.Add(new PhysicsBody({ 800, (float)screenHeight - 300 }, new Circle(80.0f), { 0, 0 }, 0.0f, 0.0f));

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
                // color (RED) and trailColor (BLACK) come from the class defaults
                sim.Add(new PhysicsBody(launchPosition, new Circle(spawnRadius), velocityPreview, spawnDrag));
            }
            
            sim.Update();
            sim.CheckCollisions();
            sim.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
