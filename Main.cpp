#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

#include "Main.h"
#include "Boids.h"

static const StringHash PLAYER_ID("IDENTITY");
static const StringHash PLAYER_SCORE("SCORE");
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
static const StringHash E_CLIENTISREADY("ClientReadyToStart");
static const StringHash E_CLIENTSCORECHANGE("ClientScoreIncrease");

URHO3D_DEFINE_APPLICATION_MAIN(Main)

Text* fpsCounter;
Text* scoreCounter;
BoidSet boids;

Button* CreateButton(const String& text, int pHeight, Font* font, Urho3D::Window* window) {
    Button* button = window->CreateChild<Button>();
    button->SetMinHeight(pHeight);
    button->SetStyleAuto();
    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);
    window->AddChild(button);
    return button;
}
Text* CreateText(const String& text, int pHeight, Font* font, Urho3D::Window* window) {
    Text* textView = window->CreateChild<Text>();
    textView->SetFont(font, 12);
    textView->SetAlignment(HA_CENTER, VA_CENTER);
    textView->SetText(text);
    window->AddChild(textView);
    return textView;
}
LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* window) {
    LineEdit* lineEdit = window->CreateChild<LineEdit>();
    lineEdit->SetMinHeight(pHeight);
    lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
    lineEdit->SetText(text);
    lineEdit->SetStyleAuto();
    window->AddChild(lineEdit);
    return lineEdit;
}

Main::Main(Context* context) : Sample(context) {}
Main::~Main() {}

void Main::Start() {
    Sample::Start();
    CreateMainMenu();
}
void Main::SubscribeToEvents() {
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Main, HandlePhysicsPreStep));
    SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(Main, HandleClientConnected));
    SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Main, HandleClientDisconnected));

    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(Main, HandleServerDisconnected));

    SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(Main, HandleClientToServerReadyToStart));
    GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);
    SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(Main, HandleServerToClientObjectID));
    GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);
    SubscribeToEvent(E_CLIENTSCORECHANGE, URHO3D_HANDLER(Main, HandleServerToClientScoreIncreased));
    GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTSCORECHANGE);
}

void Main::CreateMainMenu() {
    Sample::InitMouseMode(MM_RELATIVE);
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();
    XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
    root->SetDefaultStyle(uiStyle);

    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto(uiStyle);
    ui->SetCursor(cursor);

    window_ = new Window(context_);
    root->AddChild(window_);
    window_->SetMinWidth(300);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_CENTER, VA_CENTER);
    window_->SetName("Window");
    window_->SetStyleAuto();

    Button* connectButton = CreateButton("Connect", 24, font, window_);
    serverAddress = CreateLineEdit("localhost", 24, window_);
    Button* disconnectButton = CreateButton("Disconnect", 24, font, window_);
    Button* startButton = CreateButton("Start Server", 24, font, window_);
    Button* quitButton = CreateButton("Quit", 24, font, window_);

    fps_ = new Window(context_);
    root->AddChild(fps_);
    fps_->SetMinWidth(100);
    fps_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    fps_->SetAlignment(HA_LEFT, VA_TOP);
    fps_->SetName("Window");
    fps_->SetStyleAuto();

    fpsCounter = CreateText("FPS: 0", 24, font, fps_);

    ready_ = new Window(context_);
    root->AddChild(ready_);
    ready_->SetMinWidth(300);
    ready_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    ready_->SetAlignment(HA_CENTER, VA_CENTER);
    ready_->SetName("Window");
    ready_->SetStyleAuto();
    ready_->SetVisible(false);

    Text* readyMessage = CreateText("Press Ready...", 24, font, ready_);
    Button* readyButton = CreateButton("Ready", 24, font, ready_);

    hud_ = new Window(context_);
    root->AddChild(hud_);
    hud_->SetMinWidth(10);
    hud_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    hud_->SetAlignment(HA_RIGHT, VA_TOP);
    hud_->SetName("Window");
    hud_->SetStyleAuto();
    hud_->SetVisible(false);

    scoreCounter = CreateText("Score: 0", 24, font, hud_);

    SubscribeToEvent(connectButton, E_RELEASED, URHO3D_HANDLER(Main, HandleConnect));
    SubscribeToEvent(disconnectButton, E_RELEASED, URHO3D_HANDLER(Main, HandleDisconnect));
    SubscribeToEvent(startButton, E_RELEASED, URHO3D_HANDLER(Main, HandleStartServer));
    SubscribeToEvent(quitButton, E_RELEASED, URHO3D_HANDLER(Main, HandleQuit));
    SubscribeToEvent(readyButton, E_RELEASED, URHO3D_HANDLER(Main, HandleClientStartGame));
    SubscribeToEvents();

    CreateGameScene();
}
void Main::CreateGameScene() {
    Graphics* graphics = GetSubsystem<Graphics>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>(LOCAL);
    scene_->CreateComponent<PhysicsWorld>(LOCAL);

    cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
    camera->SetFarClip(300.0f);

    Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
    Zone* zone = zoneNode->CreateComponent<Zone>(LOCAL);
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.55f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.95f));
    zone->SetFogStart(20.0f);
    zone->SetFogEnd(300.0f);
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

    Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    Light* light = lightNode->CreateComponent<Light>(LOCAL);
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    Node* floorNode = scene_->CreateChild("Floor", LOCAL);
    floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
    floorNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
    StaticModel* floorObject = floorNode->CreateComponent<StaticModel>(LOCAL);
    floorObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    floorObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

    RigidBody* rigidbody = floorNode->CreateComponent<RigidBody>(LOCAL);
    rigidbody->SetCollisionLayer(2);
    CollisionShape* shape = floorNode->CreateComponent<CollisionShape>(LOCAL);
    shape->SetBox(Vector3::ONE);

    Node* wallNode1 = scene_->CreateChild("Wall", LOCAL);
    wallNode1->SetPosition(Vector3(100.5f, 25.0f, 0.0f));
    wallNode1->SetScale(Vector3(1.0f, 50.0f, 200.0f));
    StaticModel* wallObject1 = wallNode1->CreateComponent<StaticModel>(LOCAL);
    wallObject1->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    wallObject1->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

    Node* wallNode2 = scene_->CreateChild("Wall", LOCAL);
    wallNode2->SetPosition(Vector3(-100.5f, 25.0f, 0.0f));
    wallNode2->SetScale(Vector3(1.0f, 50.0f, 200.0f));
    StaticModel* wallObject2 = wallNode2->CreateComponent<StaticModel>(LOCAL);
    wallObject2->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    wallObject2->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

    Node* wallNode3 = scene_->CreateChild("Wall", LOCAL);
    wallNode3->SetPosition(Vector3(0.0f, 25.0f, 100.5f));
    wallNode3->SetScale(Vector3(200.0f, 50.0f, 1.0f));
    StaticModel* wallObject3 = wallNode3->CreateComponent<StaticModel>(LOCAL);
    wallObject3->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    wallObject3->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

    Node* wallNode4 = scene_->CreateChild("Wall", LOCAL);
    wallNode4->SetPosition(Vector3(0.0f, 25.0f, -100.5f));
    wallNode4->SetScale(Vector3(200.0f, 50.0f, 1.0f));
    StaticModel* wallObject4 = wallNode4->CreateComponent<StaticModel>(LOCAL);
    wallObject4->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    wallObject4->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

    RigidBody* wallBody1 = wallNode1->CreateComponent<RigidBody>(LOCAL);
    RigidBody* wallBody2 = wallNode2->CreateComponent<RigidBody>(LOCAL);
    RigidBody* wallBody3 = wallNode3->CreateComponent<RigidBody>(LOCAL);
    RigidBody* wallBody4 = wallNode4->CreateComponent<RigidBody>(LOCAL);
    wallBody1->SetCollisionLayer(2);
    wallBody2->SetCollisionLayer(2);
    wallBody3->SetCollisionLayer(2);
    wallBody4->SetCollisionLayer(2);
    CollisionShape* wallShape1 = wallNode1->CreateComponent<CollisionShape>(LOCAL);
    CollisionShape* wallShape2 = wallNode2->CreateComponent<CollisionShape>(LOCAL);
    CollisionShape* wallShape3 = wallNode3->CreateComponent<CollisionShape>(LOCAL);
    CollisionShape* wallShape4 = wallNode4->CreateComponent<CollisionShape>(LOCAL);
    wallShape1->SetBox(Vector3::ONE);
    wallShape2->SetBox(Vector3::ONE);
    wallShape3->SetBox(Vector3::ONE);
    wallShape4->SetBox(Vector3::ONE);

    waterNode_ = scene_->CreateChild("Water", LOCAL);
    waterNode_->SetScale(Vector3(2048.0f, 1.0f, 2048.0f));
    waterNode_->SetPosition(Vector3(0.0f, 50.0f, 0.0f));
    waterNode_->SetRotation(Quaternion(180.0f, 0.0, 0.0f));
    StaticModel* water = waterNode_->CreateComponent<StaticModel>();
    water->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    water->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));

    water->SetViewMask(0x80000000);

    waterPlane_ = Plane(waterNode_->GetWorldRotation() * Vector3(0.0f, 1.0f, 0.0f), waterNode_->GetWorldPosition());

    waterClipPlane_ = Plane(waterNode_->GetWorldRotation() * Vector3(0.0f, 1.0f, 0.0f),
    waterNode_->GetWorldPosition() - Vector3(0.0f, 0.01f, 0.0f));

    reflectionCameraNode_ = cameraNode_->CreateChild();
    Camera* reflectionCamera = reflectionCameraNode_->CreateComponent<Camera>();
    reflectionCamera->SetFarClip(50.0);
    reflectionCamera->SetViewMask(0x7fffffff);
    reflectionCamera->SetAutoAspectRatio(true);
    reflectionCamera->SetUseReflection(true);
    reflectionCamera->SetReflectionPlane(waterPlane_);
    reflectionCamera->SetUseClipping(true);
    reflectionCamera->SetClipPlane(waterClipPlane_);

    reflectionCamera->SetAspectRatio((float)graphics->GetWidth() / (float)graphics->GetHeight());

    int texSize = 1024;
    SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
    renderTexture->SetSize(texSize, texSize, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
    renderTexture->SetFilterMode(FILTER_BILINEAR);
    RenderSurface* surface = renderTexture->GetRenderSurface();
    SharedPtr<Viewport> rttViewport(new Viewport(context_, scene_, reflectionCamera));
    surface->SetViewport(0, rttViewport);
    Material* waterMat = cache->GetResource<Material>("Materials/Water.xml");
    waterMat->SetTexture(TU_DIFFUSE, renderTexture);

    Node* skyNode = scene_->CreateChild("Sky", LOCAL);
    skyNode->SetScale(500.0f); // The scale actually does not matter
    Skybox* skybox = skyNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
}
void Main::CreateClientObjects() {}
void Main::CreateServerObjects() {
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    boids.Initialise(cache, scene_);
}
Player* Main::CreateCharacter() {
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Player* p = new Player();
    p->Initialise(cache, scene_);

    return p;
}

void Main::HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData) {
    using namespace Update;

    Network* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    if (serverConnection) ClientPrePhysics(eventData[P_TIMESTEP].GetFloat());
    else if (network->IsServerRunning()) ServerPrePhysics(eventData[P_TIMESTEP].GetFloat());
}
void Main::HandleUpdate(StringHash eventType, VariantMap& eventData) {
    using namespace Update;

    Network* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    if (serverConnection) ClientUpdate(eventData[P_TIMESTEP].GetFloat());
    else if (network->IsServerRunning()) ServerUpdate(eventData[P_TIMESTEP].GetFloat());
}
void Main::HandleClientConnected(StringHash eventType, VariantMap& eventData) {
    using namespace ClientConnected;

    Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    newConnection->SetScene(scene_);
}
void Main::HandleClientDisconnected(StringHash eventType, VariantMap& eventData) {
    using namespace ClientConnected;

    Connection* connection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

    Player* playerObject = serverObjects_[connection];
    if (playerObject) playerObject->pNode->Remove();
}
void Main::HandleServerDisconnected(StringHash eventType, VariantMap& eventData) {
    printf("HandleServerDisconnected\n");

    scene_->Clear(true, false);
    clientObjectID_ = 0;

    window_->SetVisible(true);
    fps_->SetVisible(false);
    hud_->SetVisible(false);


    UI* ui = GetSubsystem<UI>();
    ui->GetCursor()->SetVisible(true);
}

void Main::HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData) {
    clientObjectID_ = eventData[PLAYER_ID].GetUInt();
    ready_->SetVisible(false);
    hud_->SetVisible(true);

    UI* ui = GetSubsystem<UI>();
    ui->GetCursor()->SetVisible(false);
}
void Main::HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData) {
    using namespace ClientConnected;
    Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION]. GetPtr());

    Player* newPlayer = CreateCharacter();
    serverObjects_[newConnection] = newPlayer;

    VariantMap remoteEventData;
    remoteEventData[PLAYER_ID] = newPlayer->pNode->GetID();
    newConnection->SendRemoteEvent (E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}
void Main::HandleServerToClientScoreIncreased(StringHash eventType, VariantMap& eventData) {
    int score = eventData[PLAYER_SCORE].GetInt();
    scoreCounter->SetText("Score: " + String(score));
}

void Main::HandleConnect(StringHash eventType, VariantMap& eventData) {
    CreateClientObjects();

    Network* network = GetSubsystem<Network>();
    String address = serverAddress->GetText().Trimmed();
    network->Connect((address.Empty()) ? "localhost" : address, SERVER_PORT, scene_);

    window_->SetVisible(false);
    ready_->SetVisible(true);
}
void Main::HandleDisconnect(StringHash eventType, VariantMap& eventData) {
    printf("HandleDisconnect");

    Network* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    if (serverConnection) {
        serverConnection->Disconnect();
        scene_->Clear(true, false);
        clientObjectID_ = 0;
    } else if (network->IsServerRunning()) {
        network->StopServer();
        scene_->Clear(true, false);
    }
}
void Main::HandleStartServer(StringHash eventType, VariantMap& eventData) {
    Network* network = GetSubsystem<Network>();
    network->StartServer(SERVER_PORT);

    window_->SetVisible(false);

    CreateServerObjects();
}
void Main::HandleClientStartGame(StringHash eventType, VariantMap & eventData) {
    if (clientObjectID_ == 0) {
        Network* network = GetSubsystem<Network>();
        Connection* serverConnection = network->GetServerConnection();

        if (serverConnection) {
            VariantMap remoteEventData;
            remoteEventData[PLAYER_ID] = 0;
            serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
        }
    }
}
void Main::HandleQuit(StringHash eventType, VariantMap& eventData) {
    engine_->Exit();
}

void Main::ServerPrePhysics(float timeStep) {
    ProcessClientControls(timeStep);

    Network* network = GetSubsystem<Network>();
    const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
    Vector<Vector3> playerPositions;

    for (unsigned i = 0; i < connections.Size(); ++i) {
        Connection* connection = connections[i];
        Player* playerObject = serverObjects_[connection];

        if (!playerObject || playerObject->pNode == NULL) continue;

        playerPositions.Push(serverObjects_[connection]->pRigidBody->GetPosition());
    }

    boids.Update(timeStep, playerPositions);
}
void Main::ClientPrePhysics(float timeStep) {
    Network* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();

    if (serverConnection) {
        // serverConnection->SetPosition(cameraNode_->GetPosition());
        serverConnection->SetControls(ClientToServerControls());
    }
}
void Main::ServerUpdate(float timeStep) {
    FrameInfo frameInfo = GetSubsystem<Renderer>()->GetFrameInfo();
    // fpsCounter->SetText("FPS: " + String((int)(1.0 / frameInfo.timeStep_)));
    fpsCounter->SetText("FPS: " + String((int)(1.0 / timeStep)));
}
void Main::ClientUpdate(float timeStep) {
    if (clientObjectID_ > 0) {
        Node* playerNode = this->scene_->GetNode(clientObjectID_);

        if (playerNode) {
            cameraNode_->SetPosition(playerNode->GetPosition() + playerNode->GetRotation() * Vector3::BACK * 25.0);
            cameraNode_->LookAt(playerNode->GetPosition());
            cameraNode_->SetPosition(cameraNode_->GetPosition() + playerNode->GetRotation() * Vector3::UP * 2.0);
        }
    }

    FrameInfo frameInfo = GetSubsystem<Renderer>()->GetFrameInfo();
    fpsCounter->SetText("FPS: " + String((int)(1.0 / frameInfo.timeStep_)));
}

Controls Main::ClientToServerControls() {
    Input* input = GetSubsystem<Input>();
    Controls controls;

    controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
    controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
    controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
    controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));

    controls.yaw_ = input->GetMouseMove().x_;
    controls.pitch_ = input->GetMouseMove().y_;

    return controls;
}
void Main::ProcessClientControls(float timeStep) {
    Network* network = GetSubsystem<Network>();
    const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();

    for (unsigned i = 0; i < connections.Size(); ++i) {
        Connection* connection = connections[i];
        Player* playerObject = serverObjects_[connection];

        if (!playerObject) continue;

        playerObject->ApplyControls(connection->GetControls(), timeStep);

        Ray cameraRay(playerObject->pNode->GetPosition(), playerObject->pNode->GetPosition() + playerObject->pNode->GetRotation() * Vector3::FORWARD * 100.0);
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->SphereCast (result, cameraRay, 2.0, 5.0, 2);
        if (result.body_) {
            Component* component = result.body_->GetComponent("StaticModel");
            Node* node = component->GetNode();

            if (node->GetName() == "BoidBig" || node->GetName() == "BoidSmall") {
                if (node->GetName() == "BoidBig") playerObject->score += 5;
                else if (node->GetName() == "BoidSmall") playerObject->score += 10;

                ResourceCache* cache = GetSubsystem<ResourceCache>();

                ParticleEffect* particleEffect = cache->GetResource<ParticleEffect>("Particle/SnowExplosionBig.xml");
                Node* particleNode_ = scene_->CreateChild("ParticleEmitter");
                ParticleEmitter* particleEmitter = particleNode_->CreateComponent<ParticleEmitter>();
                particleEmitter->SetEffect(particleEffect);
                particleNode_->SetPosition(node->GetPosition());

                node->SetEnabled(false);
                VariantMap remoteEventData;
                remoteEventData[PLAYER_SCORE] = playerObject->score;
                connection->SendRemoteEvent (E_CLIENTSCORECHANGE, true, remoteEventData);
            }
        }
    }
}
