#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <cmath>

#include "Boids.h"

float Boid::separationRange = 30.0f; // Separation Range
float Boid::separationFactor = 4.0f; // Seperation Factor
float Boid::cohesionRange = 30.0f; // Cohesion Range
float Boid::cohesionFactor = 5.0f; // Cohesion Factor
float Boid::alignmentRange = 5.0f; // Alignment Range
float Boid::alignmentFactor = 2.0f; // Alignment Factor
float Boid::velocityMax = 5.0f; // Maximum Velocity

Boid::Boid() {
    pNode = nullptr;
    pObject = nullptr;
    pRigidBody = nullptr;
    pCollisionShape = nullptr;
}

void Boid::Initialise(ResourceCache *pRes, Scene *pScene, bool isBig) {
    this->isBig = isBig;

    if (isBig) pNode = pScene->CreateChild("BoidBig");
    else pNode = pScene->CreateChild("BoidSmall");
    pNode->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(70.0f) + 15.0f, Random(180.0f) - 90.0f));

    pObject = pNode->CreateComponent<StaticModel>();

    if (isBig) {
        pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
        pNode->SetScale(1.0);
    } else {
        pObject->SetModel(pRes->GetResource<Model>("Models/Box.mdl"));
        pNode->SetScale(1.0);
    }

    pObject->SetMaterial(pRes->GetResource<Material>("Materials/Red-Scales.xml"));
    pObject->SetCastShadows(true);

    pRigidBody = pNode->CreateComponent<RigidBody>();
    pRigidBody->SetUseGravity(false);
    pRigidBody->SetMass(1.0f);
    pRigidBody->SetCollisionLayer(2);

    pCollisionShape = pNode->CreateComponent<CollisionShape>();
    // pCollisionShape->SetTriangleMesh(pObject->GetModel(), 0);
    pCollisionShape->SetBox(pNode->GetScale()); // TODO: change for optimisation tests
}
void Boid::ComputeForce(Boid *pBoidList, Vector<Vector3> playerPositions, Vector<int> neighbours) {
    force = Vector3(0,0,0); // Reset total force
    Vector3 fs, fc, fa; // Separation, Cohesion and Alignment forces

    Vector3 pos = pRigidBody->GetPosition(); // Position of current Boid()
    Vector3 pMean, vMean; // Position and Velocity means
    int pN = 0, vN = 0; // Neighbour count for Cohesion and Alignment calculations

    for (Vector<Vector3>::Iterator p = playerPositions.Begin(); p < playerPositions.End(); p++) {
        Vector3 pDelta = pos - *p;

        if (pDelta.Length() < separationRange) {
            fs += 10 * (pDelta / pDelta.Length());
        }
    }

    for (Vector<int>::Iterator n = neighbours.Begin(); n != neighbours.End(); ++n) {
        int i = *n.ptr_;


        if (this == &pBoidList[i] || !pNode->IsEnabled() || !pBoidList[i].pNode->IsEnabled()) continue;

        Vector3 pDelta = pos - pBoidList[i].pRigidBody->GetPosition(); // Get difference in position between boids

        if (isBig == pBoidList[i].isBig) {
            if (pDelta.Length() < cohesionRange) {
                pMean += pBoidList[i].pRigidBody->GetPosition();
                pN++;
            } if (pDelta.Length() < alignmentRange) {
                vMean += pBoidList[i].pRigidBody->GetLinearVelocity();
                vN++;
            } if (pDelta.Length() < separationRange) {
                fs += (pDelta / pDelta.Length());
            }
        } else continue;
    }

    // for (int i = 0; i < NumBoids; i++) {
    //     if (this == &pBoidList[i] || !pNode->IsEnabled() || !pBoidList[i].pNode->IsEnabled()) continue;
    //
    //     Vector3 pDelta = pos - pBoidList[i].pRigidBody->GetPosition(); // Get difference in position between boids
    //
    //     if (isBig == pBoidList[i].isBig) {
    //         if (pDelta.Length() < cohesionRange) {
    //             pMean += pBoidList[i].pRigidBody->GetPosition();
    //             pN++;
    //         } if (pDelta.Length() < alignmentRange) {
    //             vMean += pBoidList[i].pRigidBody->GetLinearVelocity();
    //             vN++;
    //         } if (pDelta.Length() < separationRange) {
    //             fs += (pDelta / pDelta.Length());
    //         }
    //     } else continue;
    // }

    // Calculate Cohesion Average
    if (pN > 0) {
        pMean /= pN;
        fc = (((pMean - pos) / (pMean - pos).Length()) * velocityMax) - pRigidBody->GetLinearVelocity();
    }

    // Calculate Alignment Average
    if (vN > 0) {
        vMean /= vN;
        fa = vMean - pRigidBody->GetLinearVelocity();
    }

    // Stop Boid Passing Edges
    if (pos.x_ > 90) fs += Vector3(-(abs(pos.x_) - 90), 0, 0);
    else if (pos.x_ < -90) fs += Vector3((abs(pos.x_) - 90), 0, 0);
    if (pos.y_ > 40) fs += Vector3(0, -(abs(pos.y_) - 40), 0);
    else if (pos.y_ < 10) fs += Vector3(0, 10 - (abs(pos.y_)), 0);
    if (pos.z_ > 90) fs += Vector3(0, 0, -(abs(pos.z_) - 90));
    else if (pos.z_ < -90) fs += Vector3(0, 0, (abs(pos.z_) - 90));

    // Sum Forces and apply respective factor
    force = (fs * separationFactor) + (fc * cohesionFactor) + (fa * alignmentFactor);
}
void Boid::Update(float tm) {
    pRigidBody->ApplyForce(force);

    Vector3 vel = pRigidBody->GetLinearVelocity();
    float d = vel.Length();

    if (d < 10.0f) d = 10.0f;
    else if (d > 150.0f) d = 150.0f;

    pRigidBody->SetLinearVelocity(vel.Normalized() * d);
    vel = pRigidBody->GetLinearVelocity();

    Vector3 vn = vel.Normalized();
    Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
    float dp = cp.DotProduct(vn);
    pRigidBody->SetRotation(Quaternion(Acos(dp), cp));
}

void BoidSet::Initialise(ResourceCache *pRes, Scene *pScene) {
    boidGrid.Resize(GridSize);
    for (int i = 0; i < GridSize; i++) {
        boidGrid[i].Resize(GridSize);
    }

    for (int i = 0; i < NumBoids; i++) {
        if (i < NumSmall) boidList[i].Initialise(pRes, pScene, false);
        else boidList[i].Initialise(pRes, pScene, true);
    }

    UpdateGrid();
}

void BoidSet::Update(float tm, Vector<Vector3> playerPositions) {
    for (int i = 0; i < NumBoids; i++) {
        if (boidList[i].pNode != NULL) {

            Vector<int> neighbours = boidGrid[boidList[i].gridX][boidList[i].gridZ];

            if (boidList[i].gridX + 1 < 20 && boidList[i].gridZ + 1 < 20) neighbours += boidGrid[boidList[i].gridX + 1][boidList[i].gridZ + 1];
            if (boidList[i].gridX - 1 > 0 && boidList[i].gridZ + 1 < 20) neighbours += boidGrid[boidList[i].gridX - 1][boidList[i].gridZ + 1];
            if (boidList[i].gridX + 1 < 20 && boidList[i].gridZ - 1 > 0) neighbours += boidGrid[boidList[i].gridX + 1][boidList[i].gridZ - 1];
            if (boidList[i].gridX - 1 > 0 && boidList[i].gridZ - 1 > 0) neighbours += boidGrid[boidList[i].gridX - 1][boidList[i].gridZ - 1];
            if (boidList[i].gridX + 1 < 20) neighbours += boidGrid[boidList[i].gridX + 1][boidList[i].gridZ];
            if (boidList[i].gridX - 1 > 0) neighbours += boidGrid[boidList[i].gridX - 1][boidList[i].gridZ];
            if (boidList[i].gridZ + 1 < 20) neighbours += boidGrid[boidList[i].gridX][boidList[i].gridZ + 1];
            if (boidList[i].gridZ - 1 > 0) neighbours += boidGrid[boidList[i].gridX][boidList[i].gridZ - 1];

            boidList[i].ComputeForce(&boidList[0], playerPositions, neighbours);
            boidList[i].Update(tm);
        }
    }

    UpdateGrid();
}

void BoidSet::UpdateGrid() {
    for (int i = 0; i < GridSize; i++) {
        for (int j = 0; j < GridSize; j++) {
            boidGrid[i][j].Clear();
        }
    }

    for (int i = 0; i < NumBoids; i++) {
        Vector3 pos = boidList[i].pRigidBody->GetPosition();
        int x = (pos.x_ + 100.0) / 10.0;
        int z = (pos.z_ + 100.0) / 10.0;

        if (x < 0) x = 0;
        else if (x > GridSize - 1) x = GridSize - 1;
        if (z < 0) z = 0;
        else if (z > GridSize - 1) z = GridSize - 1;

        boidList[i].gridX = x;
        boidList[i].gridZ = z;
        boidGrid[x][z].Push(i);
    }
}
