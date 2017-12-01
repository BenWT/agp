#include "Player.h"

Player::Player() {
    // TODO: implement constructor
    pNode = nullptr;
    pObject = nullptr;
    pRigidBody = nullptr;
    pCollisionShape = nullptr;
}

void Player::Initialise(ResourceCache *pRes, Scene *pScene) {
    pNode = pScene->CreateChild("Player");
    pNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 15.0f, Random(40.0f) - 20.0f));

    pObject = pNode->CreateComponent<StaticModel>();
    pObject->SetModel(pRes->GetResource<Model>("Models/Sphere.mdl"));
    pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
    pObject->SetCastShadows(true);

    pRigidBody = pNode->CreateComponent<RigidBody>();
    pRigidBody->SetUseGravity(true);
    pRigidBody->SetMass(1.0f);
    pRigidBody->SetCollisionLayer(1);

    pCollisionShape = pNode->CreateComponent<CollisionShape>();
    pCollisionShape->SetTriangleMesh(pObject->GetModel(), 0);

    pNode->SetEnabled(true);
}

void Player::ApplyControls(const Controls& controls) {
    // if (controls.buttons_ & CTRL_FORWARD) printf("Received from Client: Controls buttons FORWARD \n");
    // if (controls.buttons_ & CTRL_BACK) printf("Received from Client: Controls buttons BACK \n");
    // if (controls.buttons_ & CTRL_LEFT) printf("Received from Client: Controls buttons LEFT \n");
    // if (controls.buttons_ & CTRL_RIGHT) printf("Received from Client: Controls buttons RIGHT \n");

    pRigidBody->SetPosition(pNode->GetPosition());
}
