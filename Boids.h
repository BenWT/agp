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

#include "Player.h"

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

const static int GridSize = 20;
const static int NumSmall = 375;
const static int NumMedium = 375;
const static int NumBoids = NumSmall + NumMedium;

class Boid {
    static float separationRange; // Separation Range
    static float separationFactor; // Seperation Factor
    static float cohesionRange; // Cohesion Range
    static float cohesionFactor; // Cohesion Factor
    static float alignmentRange; // Alignment Range
    static float alignmentFactor; // Alignment Factor
    static float velocityMax; // Maximum Velocity

public:
    Vector3 force;
    Node* pNode;
    StaticModel* pObject;
    RigidBody* pRigidBody;
    CollisionShape* pCollisionShape;
    bool isBig;
    int gridX, gridZ;

    // Methods
    Boid();
    void Initialise(ResourceCache *pRes, Scene *pScene, bool isBig);
    void Update(float tm);
    void ComputeForce(Boid *b, Vector<Vector3> playerPositions, Vector<int> neighbours);
};

class BoidSet {
public:
    Boid boidList[NumBoids];
    Vector<Vector<Vector<int>>> boidGrid;

    BoidSet() {};
    void Initialise(ResourceCache *pRes, Scene *pScene);
    void Update(float tm, Vector<Vector3> playerPositions);
    void UpdateGrid();
};
