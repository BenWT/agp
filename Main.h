#pragma once

#include "Sample.h"
#include "Player.h"

namespace Urho3D {
    class Node;
    class Scene;
}

class Main : public Sample {
    URHO3D_OBJECT(Main, Sample);

public:
    // Constructors
    Main(Context* context);
    ~Main();

    static const unsigned short SERVER_PORT = 2345;
    LineEdit* serverAddress;
    SharedPtr<Window> window_;

    Player serverPlayer;
    Vector<Player> playerList;

    virtual void Start();

private:
    bool menuVisible_ = true;

    void SubscribeToEvents();

    // Object Creators
    void CreateMainMenu(bool isClient);
    void CreateGameScene(bool isClient);
    void CreateCharacter(VariantMap& identity);

    // Urho Event Handlers
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
    void HandleClientConnected(StringHash eventType, VariantMap& eventData);
    void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);

    // Custom Events
    void HandleConnect(StringHash eventType, VariantMap& eventData);
    void HandleDisconnect(StringHash eventType, VariantMap& eventData);
    void HandleStartServer(StringHash eventType, VariantMap& eventData);
    void HandleQuit(StringHash eventType, VariantMap& eventData);

    // Game logic
    void ServerPrePhysics(float timeStep);
    void ClientPrePhysics(float timeStep);
    void ServerUpdate(float timeStep);
    void ClientUpdate(float timeStep);

    Controls ClientToServerControls();
    void ProcessClientControls();
    void GetClientCameraPosition(StringHash eventType, VariantMap& eventData);
};
