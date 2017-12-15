#pragma once
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
namespace Urho3D
{
    class Node;
    class Scene;
    class RigidBody;
    class CollisionShape;
    class ResourceCache;
}
// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

const static int NumBoids = 100;

class Boid {
    static float rs; // Separation Range
    static float cs; // Seperation Factor
    static float rc; // Cohesion Range
    static float cc; // Cohesion Factor
    static float ra; // Alignment Range
    static float ca; // Alignment Factor
    static float vmax; // Maximum Velocity

public:
    Vector3 force;
    Node* pNode;
    StaticModel* pObject;
    RigidBody* pRigidBody;
    CollisionShape* pCollisionShape;;

    // Methods
    Boid();
    void Initialise(ResourceCache *pRes, Scene *pScene);
    void Update(float tm);
    void ComputeForce(Boid *b);
};

class BoidSet {
public:
    Boid boidList[NumBoids];

    BoidSet() {};
    void Initialise(ResourceCache *pRes, Scene *pScene);
    void Update(float tm);
};
