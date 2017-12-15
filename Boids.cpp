#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "Boids.h"

float Boid::rs = 20.0f; // Separation Range
float Boid::cs = 2.0f; // Seperation Factor
float Boid::rc = 30.0f; // Cohesion Range
float Boid::cc = 4.0f; // Cohesion Factor
float Boid::ra = 5.0f; // Alignment Range
float Boid::ca = 2.0f; // Alignment Factor
float Boid::vmax = 5.0f; // Maximum Velocity

Boid::Boid() {
    pNode = nullptr;
    pObject = nullptr;
    pRigidBody = nullptr;
    pCollisionShape = nullptr;
}

void Boid::Initialise(ResourceCache *pRes, Scene *pScene) {
    pNode = pScene->CreateChild("Boid");
    pNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 15.0f, Random(40.0f) - 20.0f));

    pObject = pNode->CreateComponent<StaticModel>();
    pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
    pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
    pObject->SetCastShadows(true);

    pRigidBody = pNode->CreateComponent<RigidBody>();
    pRigidBody->SetUseGravity(false);
    pRigidBody->SetMass(1.0f);
    pRigidBody->SetCollisionLayer(1);

    pCollisionShape = pNode->CreateComponent<CollisionShape>();
    pCollisionShape->SetTriangleMesh(pObject->GetModel(), 0);
}
void Boid::ComputeForce(Boid *pBoidList) {
    force = Vector3(0,0,0); // Reset total force
    Vector3 fs, fc, fa; // Separation, Cohesion and Alignment forces

    Vector3 pMean, vMean; // Position and Velocity means
    int pN = 0, vN = 0; // Neighbour count for Cohesion and Alignment calculations

    for (int i = 0; i < NumBoids; i++) {
        if (this == &pBoidList[i]) continue;

        Vector3 pDelta = pRigidBody->GetPosition() - pBoidList[i].pRigidBody->GetPosition(); // Get difference in position between boids

        // Separation
        if (pDelta.Length() < rs) {
            fs += (pDelta / pDelta.Length());
        }

        // Cohesion
        if (pDelta.Length() < rc) {
            pMean += pBoidList[i].pRigidBody->GetPosition();
            pN++;
        }

        // Alignment
        if (pDelta.Length() < ra) {
            vMean += pBoidList[i].pRigidBody->GetLinearVelocity();
            vN++;
        }
    }

    // Cohesion
    if (pN > 0) {
        // pMean /= pN;
        pMean = pMean / pN;
        fc = (((pMean - pRigidBody->GetPosition()) / (pMean - pRigidBody->GetPosition()).Length()) * vmax) - pRigidBody->GetLinearVelocity();
    }

    // Alignment
    if (vN > 0) {
        vMean /= vN;
        fa = vMean - pRigidBody->GetLinearVelocity();
    }

    // if (near wall) fs += wall repel * distance from wall;
    Vector3 pos = pRigidBody->GetPosition();
    float wallX, wallY, wallZ;

    if (pos.x_ > 90) {
        wallX = abs(pos.x_) - 90;
        fs += Vector3(-wallX * wallX, 0, 0);
    } else if (pos.x_ < -90) {
        wallX = abs(pos.x_) - 90;
        fs += Vector3(wallX * wallX, 0, 0);
    }

    if (pos.y_ > 40) {
        wallY = pos.y_ - 40;
        fs += Vector3(0, -(wallY * wallY), 0);
    } else if (pos.y_ < 10) {
        wallY = 10 - pos.y_;
        fs += Vector3(0, (wallY * wallY), 0);
    }

    if (pos.z_ > 90) {
        wallZ = abs(pos.z_) - 90;
        fs += Vector3(0, 0, -(wallZ * wallZ));
    } else if (pos.z_ < -90) {
        wallZ = abs(pos.z_) - 90;
        fs += Vector3(0, 0, (wallZ * wallZ));
    }

    // Sum Forces and apply respective factor
    force = (fs * cs) + (fc * cc) + (fa * ca);
}
void Boid::Update(float tm) {
    pRigidBody->ApplyForce(force);

    Vector3 vel = pRigidBody->GetLinearVelocity();
    float d = vel.Length();

    if (d < 10.0f) {
        d = 10.0f;
        pRigidBody->SetLinearVelocity(vel.Normalized() * d);
    } else if (d > 50.0f) {
        d = 50.0f;
        pRigidBody->SetLinearVelocity(vel.Normalized() * d);
    }

    Vector3 vn = vel.Normalized();
    Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
    float dp = cp.DotProduct(vn);
    pRigidBody->SetRotation(Quaternion(Acos(dp), cp));
}

void BoidSet::Initialise(ResourceCache *pRes, Scene *pScene) {
    for (int i = 0; i < NumBoids; i++) {
        boidList[i].Initialise(pRes, pScene);
    }
}

void BoidSet::Update(float tm) {
    for (int i = 0; i < NumBoids; i++) {
        if (boidList[i].pNode != NULL) {
            boidList[i].ComputeForce(&boidList[0]);
            boidList[i].Update(tm);
        }
    }
}
