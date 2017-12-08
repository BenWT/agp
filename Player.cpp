#include "Player.h"

Player::Player() {
    pNode = nullptr;
    pObject = nullptr;
    pRigidBody = nullptr;
    pCollisionShape = nullptr;
}

void Player::Initialise(ResourceCache *pRes, Scene *pScene) {
    pNode = pScene->CreateChild("Player");
    pNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 15.0f, Random(40.0f) - 20.0f));

    pObject = pNode->CreateComponent<StaticModel>();
    pObject->SetModel(pRes->GetResource<Model>("Models/Box.mdl"));
    pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
    pObject->SetCastShadows(true);

    pRigidBody = pNode->CreateComponent<RigidBody>();
    pRigidBody->SetUseGravity(false);
    pRigidBody->SetMass(15.0f);
    pRigidBody->SetCollisionLayer(1);

    pCollisionShape = pNode->CreateComponent<CollisionShape>();
    pCollisionShape->SetTriangleMesh(pObject->GetModel(), 0);

    pNode->SetEnabled(true);
}

void Player::ApplyControls(const Controls& controls, float timeStep) {
    const float MOVE_SPEED = 5.0f;
    const float MOUSE_SENSITIVITY = 1.1f;

    yaw += controls.yaw_ * MOUSE_SENSITIVITY;
    pitch += controls.pitch_ * MOUSE_SENSITIVITY;

    if (pitch > 45.0f) pitch = 45.0f;
    if (pitch < -45.0f) pitch = -45.0f;

    Quaternion rotation = Quaternion(pitch, yaw, 0.0f);
    pRigidBody->SetRotation(rotation);

    Vector3 movement;

    // if (controls.buttons_ & CTRL_FORWARD) pNode->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    // if (controls.buttons_ & CTRL_BACK) pNode->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    // if (controls.buttons_ & CTRL_LEFT) pNode->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    // if (controls.buttons_ & CTRL_RIGHT) pNode->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

    if (controls.buttons_ & CTRL_FORWARD) movement += Vector3::FORWARD * MOVE_SPEED;
    if (controls.buttons_ & CTRL_BACK) movement += Vector3::BACK * MOVE_SPEED;
    if (controls.buttons_ & CTRL_LEFT) movement += Vector3::LEFT * MOVE_SPEED;
    if (controls.buttons_ & CTRL_RIGHT) movement += Vector3::RIGHT * MOVE_SPEED;

    pRigidBody->SetLinearVelocity(pRigidBody->GetRotation() * movement);
}
