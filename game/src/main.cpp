/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

// Note: in raylib, 2D coordinate system is x-right, y-down. Angles: positive goes clockwise, negative goes counter
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector>

int screenWidth = 1600;
int screenHeight = 1400;

const unsigned int TARGET_FPS = 50;

Vector2 launchPosition = { 30, (float)(screenHeight - 50) }; // Slingshot position
float launchPositionAdjustmentSpeed = 50.0f;
float launchSpeed = 250.0f;
float launchAngle = -45.0f;  // Deg
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
    virtual void Draw(Vector2 position, Color color) = 0; 
};

class Circle : public Collider
{
public:
    float radius;

    Circle(float radius) : radius(radius) {}

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
    Vector2 normal; // unit vector pointing to the open side of the halfplane ({0, -1} = up, since y goes down in raylib)

    HalfPlane(Vector2 normal) : normal(Vector2Normalize(normal)) {} // normalize so it's always length 1

    ColliderShape GetShape() override
    {
        return HALFPLANE;
    }

    void Draw(Vector2 position, Color color) override
    {
        Vector2 along = { -normal.y, normal.x }; // perpendicular to the normal = direction of the surface
        DrawLineEx(position - along * 5000, position + along * 5000, 3, color);
        DrawLineEx(position, position + normal * 60, 3, YELLOW); // normal arrow
        DrawCircleV(position + normal * 60, 5, YELLOW);
    }
/* 
    use later when needing to find the normal of a rotating plane
    Vector2 n = ((HalfPlane*)plane.collider)->normal;
    Vector2 GetNormal() { Vector2 dir = position2 - position; return Vector2Normalize({ dir.y, -dir.x }); }
 */
};

// Composition "has a": every body has a collider that decides its shape
class PhysicsBody
{
public:
    Collider* collider = nullptr;
    Vector2 position = Vector2 { 0, 0 };
    Vector2 velocity = Vector2 { 0, 0 };
    float mass = 1.0f;
    float drag = 0.0f;
    float gravityScale = 1.0f;
    float maxLifeTime = 10.0f;
    float currentLifeTime = 0.0f;
    float lifeTimeAfterHit = -1.0f; // maxLifeTime gets set to this when hit; -1 = hits don't affect lifetime
    float hitCooldown = 0.2f; // overlaps within this window count as the same hit
    Color color = RED;
    Color trailColor = BLACK;
    std::vector<Vector2> trail;
    static const int trail_Length = 800;

    bool isStatic = false;
    float bounce = 0.8f;


    float GetInverseMass() const { return isStatic ? 0.0f : 1.0f / mass; }

    PhysicsBody(Collider* collider, Vector2 position, Vector2 velocity = { 0, 0 }, float drag = 0.0f, float gravityScale = 1.0f, bool isStatic = false, float maxLifeTime = 10.0f)
        : collider(collider), position(position), velocity(velocity), drag(drag), gravityScale(isStatic ? 0.0f : gravityScale), isStatic(isStatic), maxLifeTime(isStatic ? -1.0f : maxLifeTime) {}

    virtual ~PhysicsBody()
    {
        delete collider; // the body owns its collider
    }

    virtual void OnHit(PhysicsBody& other) {} // normal bodies ignore hits

protected:
    // shared by Bird and Pig: start (or restart) the countdown to deletion
    void StartHitCountdown()
    {
        maxLifeTime = lifeTimeAfterHit;
        currentLifeTime = 0.0f; // currentLifeTime now doubles as "time since last hit"
    }

public:
    // no copies: two copies would both delete the same collider
    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;
};

class Bird : public PhysicsBody
{
public:
    bool hasBeenHit = false;

    Bird(Collider* collider, Vector2 position, Vector2 velocity, float drag = 0.0f, float lifeTime = 10.0f, float lifeTimeAfterHit = 3.0f)
        : PhysicsBody(collider, position, velocity, drag, 1.0f, false, lifeTime) // default lifetime from spawn
    {
        this->lifeTimeAfterHit = lifeTimeAfterHit; // base class member, so it's set here instead of the initializer list
    }

    void OnHit(PhysicsBody& other) override
    {
        if (hasBeenHit && other.isStatic) return; // lying on the ground shouldn't keep resetting it
        if (hasBeenHit && currentLifeTime < hitCooldown) return; // still the same impact

        hasBeenHit = true;
        StartHitCountdown(); // switch from the default lifetime to the hit countdown
    }
};

class Pig : public PhysicsBody
{
public:
    //float health = 100.0f;

    Pig(Collider* collider, Vector2 position, float lifeTimeAfterHit = 2.0f)
        : PhysicsBody(collider, position, { 0, 0 }, 0.0f, 1.0f, false, -1.0f) // -1 = infinite until hit
    {
        this->lifeTimeAfterHit = lifeTimeAfterHit; // base class member, so it's set here instead of the initializer list
    }

    void OnHit(PhysicsBody& other) override
    {
        if (other.collider->GetShape() == HALFPLANE) return; // ground check

        bool countdownRunning = maxLifeTime > 0.0f;
        if (countdownRunning && currentLifeTime < hitCooldown) return; // still the same impact

        StartHitCountdown(); // start countdown or restart on new hit
    }
};

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
struct Contacts
{
    Vector2 normal = { 0, 0 };
    float howMuchOverlap = 0.0f;
};

//using const in this function stops the function from changing the a and b
bool isOverlappingCircle(const PhysicsBody& a, const PhysicsBody& b, Contacts& contacts)
{
    Vector2 displacement = b.position - a.position;
    float distanceSq = Vector2DotProduct(displacement, displacement);

    float radA = ((Circle*)a.collider)->radius; // cast to circle then get radius
    float radB = ((Circle*)b.collider)->radius;

    float sumRadius = radA + radB; // instead of square rooting, just multiply

    if (distanceSq >= sumRadius * sumRadius) return false;

    // if they overlap
    float distance = sqrtf(distanceSq);
    contacts.normal = (distance > 0.0001f) ? displacement * (1.0f / distance) : Vector2 { 0 , -1 };
    contacts.howMuchOverlap = sumRadius - distance;
    return true;
}

bool isOverlappingCircleHalfPlane(const PhysicsBody& circle, const PhysicsBody& plane, Contacts& contacts)
{
    Vector2 normal = ((HalfPlane*)plane.collider)->normal;
    float radius = ((Circle*)circle.collider)->radius;

    Vector2 displacement = circle.position - plane.position; // d = C - P
    float distance = Vector2DotProduct(displacement, normal); // projecting d onto the normal = how far C is above the plane

    if (distance >= radius) return false;

    contacts.normal = Vector2Negate(normal); // circle -> plane
    contacts.howMuchOverlap = radius - distance;
    return true;
}

void ResolveCollision(PhysicsBody& a, PhysicsBody& b, const Contacts& c, float restingSpeed)
{
    float invA = a.GetInverseMass();
    float invB = b.GetInverseMass();
    float invSum = invA + invB;
    if (invSum == 0.0f) return; // if two static bodies, do nothing

    // positional correction: push apart, split by mass. lighter objects move more, static dont move
    Vector2 correction = c.normal * (c.howMuchOverlap / invSum);
    a.position -= correction * invA;
    b.position += correction * invB;

    // set impulse. change velocities to bounce
    Vector2 relVel = b.velocity - a.velocity;
    float velAlongNorm = Vector2DotProduct(relVel, c.normal);

    // if already moving apart. dont pull them back together
    if (velAlongNorm > 0.0f) return;

    float e = fminf(a.bounce, b.bounce);

    if (-velAlongNorm < restingSpeed) e = 0.0f;

    float j = -(1.0f + e) * velAlongNorm / invSum; // impulse magnitude

    Vector2 impulse = c.normal * j;
    a.velocity -= impulse * invA;
    b.velocity += impulse * invB;
}

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
                    PhysicsBody* bodyA = bodies[i];
                    PhysicsBody* bodyB = bodies[j];
                    Contacts contacts;

                    bool isOverlapping = false;

                    //Circle collision
                    // Circle A, Circle B
                    // dont use . because bodies is a ptr. bodyA->collider is the same as (*bodyA).collider
                    if (bodyA->collider->GetShape() == CIRCLE && bodyB->collider->GetShape() == CIRCLE)
                    {
                        isOverlapping = isOverlappingCircle(*bodyA, *bodyB, contacts);
                    }
                    else if (bodyA->collider->GetShape() == CIRCLE && bodyB->collider->GetShape() == HALFPLANE)
                    {
                        // circle-halfplane collision
                        isOverlapping = isOverlappingCircleHalfPlane(*bodyA, *bodyB, contacts);
                    }
                    else if (bodyA->collider->GetShape() == HALFPLANE && bodyB->collider->GetShape() == CIRCLE)
                    {
                        // REMOVE the = later
                        isOverlapping = isOverlappingCircleHalfPlane(*bodyB, *bodyA, contacts);
                        std::swap(bodyA, bodyB);
                    }
                    
                    if (isOverlapping)
                    {
                        float restingSpeed = Vector2Length((gravity) * FIXED_DELTA_TIME * 2.0f);
                        ResolveCollision(*bodyA, *bodyB, contacts, restingSpeed);
                        bodyA->OnHit(*bodyB);
                        bodyB->OnHit(*bodyA);
                        bodyA->color = RED;
                        bodyB->color = BLUE;
                    }
                    
                }
            }
        }

        void Update()
        {
            for (int i = 0; i < bodies.size(); i++)
            {
                if (bodies[i]->isStatic) continue; // if static dont move
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

                // destroy with lifetime
                bodies[i]->currentLifeTime += FIXED_DELTA_TIME;
                bool isLifeTimeExpired = bodies[i]->maxLifeTime > 0.0f && bodies[i]->currentLifeTime >= bodies[i]->maxLifeTime;

                //destroy birds when off screen
                Vector2 p = bodies[i]->position;
                bool offScreenCheck = p.y > screenHeight + 100 || p.x > screenWidth + 100 || p.x < -100 || p.y < -100;
                if (isLifeTimeExpired || offScreenCheck)
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

int main()
{
    sim.bounds = Bounds2D(screenHeight + margin, 0 - margin, 0 - margin, screenWidth + margin);
    InitWindow(screenWidth, screenHeight, "Physics-1");
    SetTargetFPS(TARGET_FPS);
    
    PhysicsBody* ground = new PhysicsBody(new HalfPlane({ 0, -1 }), { (float)screenWidth / 2, (float)screenHeight - 25 }, { 0, 0 }, 0.0f, 0.0f, true);
    sim.Add(ground);

    // static circle
    PhysicsBody* staticCircle = new PhysicsBody(new Circle(30.0f), { 800, (float)screenHeight - 300 }, { 0, 0 }, 0.0f, 0.0f); 
    sim.Add(staticCircle);

    // moving circle
    PhysicsBody* bigCircle = new PhysicsBody(new Circle(80.0f), { 800, (float)screenHeight - 300 }, { 0, 0 }, 0.0f, 0.0f, true);
    sim.Add(bigCircle);

    Pig* enemy = new Pig(new Circle(30.0f), { 1200, (float)screenHeight - 200 }, 2.0f);
    sim.Add(enemy);



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

            // spawn bird
            if (IsKeyPressed(KEY_SPACE))
            {
                // bird: dies after 10 seconds, or 3 seconds after it hits something
                sim.Add(new Bird(new Circle(spawnRadius), launchPosition, velocityPreview, spawnDrag, 10.0f, 3.0f));
            }
            
            sim.Update();
            sim.CheckCollisions();
            sim.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}