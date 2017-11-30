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
#include <Urho3D/Container/Vector.h>

namespace Urho3D {
    class Node;
    class Scene;
    class RigidBody;
    class CollisionShape;
    class ResourceCache;
}

const int CTRL_FORWARD = 1;
const int CTRL_BACK = 2;
const int CTRL_LEFT = 4;
const int CTRL_RIGHT = 8;

using namespace Urho3D;

class Player {
public:
    VariantMap identity;

    Node* pNode;
    StaticModel* pObject;
    RigidBody* pRigidBody;
    CollisionShape* pCollisionShape;

    // Methods
    Player();
    void Initialise(ResourceCache *pRes, Scene *pScene, VariantMap& identity);
    void ApplyControls(const Controls& controls);
};
