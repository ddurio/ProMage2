#include "Game/Game.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/ColorTargetView.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/App.hpp"
#include "Game/CameraController.hpp"

#include "stdarg.h"


Game::Game() {

}


Game::~Game() {

}


void Game::Startup() {
    m_gameClock = new Clock();
    m_playerCamera = new Camera();
    m_debugCamera = new Camera();

    if( m_onAttractScreen ) {
        StartupAttract();
    } else {
        StartupGame();
    }
}


void Game::Shutdown() {
    delete m_playerCamera;
    m_playerCamera = nullptr;

    delete m_debugCamera;
    m_debugCamera = nullptr;
}


void Game::Update() {
    //m_playerCamera->SetOrthoView( Vec2::ZERO + desktopOffset, m_desktopDimensions[m_activeDesktop] + desktopOffset );
    //m_playerCamera->SetOrthoProjection( 10.f );
    m_playerCamera->SetPerspectiveProjection( 90.f, 0.01f, 100.f );
    m_debugCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 2000.f, 1000.f ) );

    if( m_onAttractScreen ) {
        UpdateAttractScreen( m_gameClock->GetDeltaTime() );
    } else {
        UpdateGame( m_gameClock->GetDeltaTime() );
    }

}


void Game::Render() const {
    Camera* activeCamera = GetActiveCamera();

    m_controller->Render();
    g_theRenderer->BeginCamera( activeCamera );

    if( m_onAttractScreen ) {
        RenderAttractScreen();
    } else {
        RenderGame();
    }

    g_theRenderer->EndCamera( activeCamera );
}


bool Game::IsDebugDrawingOn() const {
    return m_debugDrawing;
}


void Game::SetCameraShakeAmount( float newCameraShakeAmount ) {
    m_cameraShakeAmount = fmax( newCameraShakeAmount, CAMERA_SHAKE_MAX );
}


bool Game::HandleKeyPressed( unsigned char keyCode ) {
	switch( keyCode ) {
        case(0x70): { // F1 - Toggle Debug Drawing
            m_debugDrawing = !m_debugDrawing;
            g_theDevConsole->ExecuteCommandString( Stringf( "DebugDrawToggle enabled=%s", (m_debugDrawing ? "true" : "false") ) );
            return true;
        } case(0x71): { // F2 - Render Normals
            m_renderMode = (RenderMode)((m_renderMode + 1) % NUM_RENDER_MODES);
            g_theRenderer->SetRenderMode( m_renderMode );
            return true;
        } case(0x72): { // F3 - Place Point Light
            LightDesc light = g_theRenderer->GetDynamicLight( 2 );
            light.position = m_playerCamera->GetPosition();
            g_theGame->m_forcePosition = true;
            g_theRenderer->SetDynamicLight( 2, light );
            return true;
        } case(0x73): { // F4 - Toggle Debug Camera
            m_useDebugCamera = !m_useDebugCamera;
            return true;
        } case(0xBC): { // F4 - Toggle Debug Camera
            m_ambientColor.a = ClampFloat( m_ambientColor.a - 0.04f, 0.f, 1.f );
            return true;
        } case(0xBE): { // F4 - Toggle Debug Camera
            m_ambientColor.a = ClampFloat( m_ambientColor.a + 0.04f, 0.f, 1.f );
            return true;
        } case('P'): { // P - Pause time
            m_isPaused = !m_isPaused;

            if( m_isPaused ) {
                m_gameClock->Pause();
            } else {
                m_gameClock->Unpause();
            }

            return true;
        } case('T'): { // T - Slow down time while held
            m_gameClock->SetTimeDilation( 0.1f );
            return true;
        } case('Y'): { // Y - Speed up time
            m_gameClock->SetTimeDilation( 4.f );
            return true;
        }
	}

    if( !m_onAttractScreen ) {
        return m_controller->HandleKeyPressed( keyCode );
    } else {
        return false;
    }
}


bool Game::HandleKeyReleased( unsigned char keyCode ) {
    if( !m_onAttractScreen ) {
        bool handled = m_controller->HandleKeyReleased( keyCode );

        if( handled ) {
            return true;
        }
    }

    switch( keyCode ) {
        case('T'): {
            m_gameClock->SetTimeDilation( 1.f );
            return true;
        } case('Y'): {
            m_gameClock->SetTimeDilation( 1.f );
            return true;
        }
    }

    return false;
}


bool Game::HandleQuitRequested() {
	return false;
}


bool Game::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    UNUSED( event );
    UNUSED( scrollAmount );
    return false;
}


void Game::ReturnToAttractScreen() {
    Shutdown();
    m_onAttractScreen = true;
}


Camera* Game::GetActiveCamera() const {
    Camera* activeCamera = m_useDebugCamera ? m_debugCamera : m_playerCamera;
    return activeCamera;
}


Clock* Game::GetGameClock() const {
    return m_gameClock;
}


bool Game::HasGameBeenBeaten() const {
    return m_hasBeatenTheGame;
}


bool Game::Command_SetAmbientLight( EventArgs& args ) {
    g_theGame->m_ambientColor = args.GetValue( "color", g_theGame->m_ambientColor );
    g_theDevConsole->PrintString( Stringf( "- Setting Ambient Light to %s", g_theGame->m_ambientColor.GetAsString().c_str() ), DevConsole::CHANNEL_INFO );

    g_theRenderer->SetAmbientLight( g_theGame->m_ambientColor );
    return false;
}


bool Game::Command_SetEmissiveLight( EventArgs& args ) {
    g_theGame->m_emissiveColor = args.GetValue( "color", g_theGame->m_emissiveColor );
    g_theDevConsole->PrintString( Stringf( "- Setting Emissive Light to %s", g_theGame->m_emissiveColor.GetAsString().c_str() ), DevConsole::CHANNEL_INFO );

    g_theRenderer->SetEmissiveLight( g_theGame->m_emissiveColor );
    return false;
}


bool Game::Command_SetDirectionalLight( EventArgs& args ) {
    LightDesc dirLight = g_theRenderer->GetDynamicLight( 0 );

    dirLight.direction = args.GetValue( "direction", dirLight.direction );
    g_theDevConsole->PrintString( Stringf( "- Setting Directional Light direction to %s", dirLight.direction.GetAsString().c_str() ), DevConsole::CHANNEL_INFO );

    dirLight.color = args.GetValue( "color", dirLight.color );
    g_theDevConsole->PrintString( Stringf( "- Setting Directional Light color to %s", dirLight.color.GetAsString().c_str() ), DevConsole::CHANNEL_INFO );

    g_theRenderer->SetDynamicLight( 0, dirLight );
    return false;
}


bool Game::Command_SetPointLights( EventArgs& args ) {
    int lightIndex = 0;
    LightDesc light;

    do {
        lightIndex++;
        light = g_theRenderer->GetDynamicLight( lightIndex );
    } while( light.color.a != 0.f );

    int numLights = args.GetValue( "number", lightIndex - 1 );

    for( lightIndex = 1; lightIndex < 5; lightIndex++ ) {
        light = g_theRenderer->GetDynamicLight( lightIndex );

        if( lightIndex <= numLights ) {
            light.color.a = 0.5f; // Set to a default on value
        } else {
            light.color.a = 0.f; // Intensity to 0 turns it off
        }

        g_theRenderer->SetDynamicLight( lightIndex, light );
    }

    return false;
}


void Game::StartupAttract() {
}


void Game::StartupGame() {
    StartupParseXMLTests();
    StartupConsoleTests();
    StartupEventTests();

    g_theEventSystem->SubscribeEventCallbackFunction( "SetAmbient", Command_SetAmbientLight );
    g_theEventSystem->SubscribeEventCallbackFunction( "SetEmissive", Command_SetEmissiveLight );
    g_theEventSystem->SubscribeEventCallbackFunction( "SetDirectional", Command_SetDirectionalLight );
    g_theEventSystem->SubscribeEventCallbackFunction( "SetPoint", Command_SetPointLights );
    StartupLights();

    m_materials.resize( 3 );
    m_materials[0] = g_theRenderer->GetOrCreateMaterial( "Data/Materials/Example.xml:couch" );
    m_materials[1] = g_theRenderer->GetOrCreateMaterial( "Data/Materials/Example.xml:brick" );
    m_materials[2] = g_theRenderer->GetOrCreateMaterial( "Data/Materials/Example.xml:green" );

    m_controller = new CameraController( m_playerCamera );
    m_controller->Startup();

    g_theRenderer->CreateTexture( "Data/Images/Globe.jpg" );
    g_theRenderer->CreateTexture( "Data/Images/WoodCrate.jpg" );
}


void Game::StartupParseXMLTests() {
    tinyxml2::XMLDocument xmlTest = new tinyxml2::XMLDocument();
    tinyxml2::XMLError loadSuccess = xmlTest.LoadFile( DATA_XML_TEST );
    GUARANTEE_OR_DIE( loadSuccess == tinyxml2::XML_SUCCESS, Stringf("Failed to load XML file: %s", DATA_PROJECT_CONFIG) );

    tinyxml2::XMLNode* root = xmlTest.FirstChild();
    GUARANTEE_OR_DIE( root != nullptr, "Poorly constructed XML file" );

    //const char* projectName = root->ToElement()->Attribute( "name" );
    std::string projectName = ParseXMLAttribute( *root->ToElement(), "name", "NAME NOT FOUND" );

    const BitmapFont* font =  g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    float cellHeight = 1.5f;
    Vec2 textStart( 100.f, 97.f );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "- Root: name = %s", projectName.c_str() ) );
    textStart.y -= 3.f;

    // Display, fullscreen, resolution: testing bool and IntVec2
    XMLElement* display = root->FirstChildElement( "display" );
    bool isFullscreen = ParseXMLAttribute( *display, "fullscreen", false );
    IntVec2 resolution = ParseXMLAttribute( *display, "resolution", IntVec2(-1, -1) );

    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Display: fullscreen = %d, resolution = %s", isFullscreen, resolution.GetAsString().c_str() ) );
    textStart.y -= 3.f;

    // Item, name, number: testing string and int
    XMLElement* item = root->FirstChildElement( "item" );
    //const char* itemName = item->Attribute( "name" );
    std::string itemName = ParseXMLAttribute( *item, "name", "NAME NOT FOUND" );
    //int itemCount = item->IntAttribute( "number" );
    int itemCount = ParseXMLAttribute( *item, "number", 0 );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Item: name = %s, number = %i", itemName.c_str(), itemCount ) );
    textStart.y -= 3.f;

    // Health: testing internal node data and float
    XMLElement* health = root->FirstChildElement( "health" );
    float healthValue = health->FloatText();
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Health: value = %.02f", healthValue ) );
    textStart.y -= 3.f;

    // Position: testing Vec2
    XMLElement* position2D = root->FirstChildElement( "position" );
    Vec2 positionVec2( position2D->GetText() );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Position2D: value = %s", positionVec2.GetAsString().c_str() ) );
    textStart.y -= 3.f;

    // Position: testing multiple children with same name and Vec3
    XMLElement* position3D = position2D->NextSiblingElement( "position" );
    Vec3 positionVec3( position3D->GetText() );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Position3D: value = %s", positionVec3.GetAsString().c_str() ) );
    textStart.y -= 3.f;

    // Range: testing IntRange
    XMLElement* strength = root->FirstChildElement( "range" );
    //const char* type = strength->Attribute( "type" );
    std::string type = ParseXMLAttribute( *strength, "type", "TYPE NOT FOUND" );
    IntRange strModifier( strength->GetText() );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Range: type = %s, value = %s", type.c_str(), strModifier.GetAsString().c_str() ) );
    textStart.y -= 3.f;

    // Range: testing FloatRange
    XMLElement* damage = strength->NextSiblingElement( "range" );
    //type = damage->Attribute( "type" );
    type = ParseXMLAttribute( *strength, "type", "TYPE NOT FOUND" );
    FloatRange dmgModifier( damage->GetText() );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Range: type = %s, value = %s", type.c_str(), dmgModifier.GetAsString().c_str() ) );
    textStart.y -= 3.f;

    // Hair: testing Rgba
    XMLElement* hair = root->FirstChildElement( "hair" );
    //Rgba color( hair->Attribute( "color" ) );
    Rgba color = ParseXMLAttribute( *hair, "color", Rgba::MAGENTA );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "   - Hair: color = %s", color.GetAsString().c_str() ) );
    textStart.y -= 3.f;
}


void Game::StartupConsoleTests() {
    g_theDevConsole->PrintString( "Hello, world!", Rgba::RED );
    g_theDevConsole->PrintString( "This is a TEST", Rgba::WHITE );
    g_theDevConsole->PrintString( Stringf( "This is a NUMBER: %d", 42 ), Rgba::BLUE );
}


// Test callback functions for StartupEventTests
bool EventTestCallback1( EventArgs& args ) {
    UNUSED( args );
    g_theDevConsole->PrintString( "Callback1 was called" );
    return false;
}


bool EventTestCallback2( EventArgs& args ) {
    UNUSED( args );
    g_theDevConsole->PrintString( "Callback2 was called" );
    return false;
}


bool EventTestCallbackConsuming( EventArgs& args ) {
    UNUSED( args );
    g_theDevConsole->PrintString( "ConsumingCallback was called" );
    return true;
}


void Game::StartupEventTests() {
    g_theDevConsole->PrintString( "Start Event Tests" );
    g_theEventSystem->FireEvent( "EventTest1" ); // No subscribers, no printout
    
    g_theDevConsole->PrintString( "Start Test: 1, 2" );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest1", EventTestCallback1 );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest1", EventTestCallback2 );
    g_theEventSystem->FireEvent( "EventTest1" ); // Two subscribers in order 1, 2

    g_theDevConsole->PrintString( "Start Test: 2, 1" );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest2", EventTestCallback2 );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest2", EventTestCallback1 );
    g_theEventSystem->FireEvent( "EventTest2" ); // Two subscribers in order 2,1 

    g_theDevConsole->PrintString( "Start Test: Consuming, 2, 1" );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallbackConsuming );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallback2 );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallback1 );
    g_theEventSystem->FireEvent( "EventTest3" ); // Three subscribers, first one consumes the event though, only one prints!

    /*
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallback1 );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallback2 );
    g_theEventSystem->SubscribeEventCallbackFunction( "EventTest3", EventTestCallbackConsuming );
    g_theEventSystem->FireEvent( "EventTest3" ); // Setup for unsub
    */

    g_theDevConsole->PrintString( "End Event Tests" );

    g_theDevConsole->PrintString( "Start DevConsole Self Tests" );
    g_theDevConsole->RunCommandTests();
    g_theDevConsole->PrintString( "End DevConsole Self Tests" );
}


void Game::StartupLights() {
    Vec3 atten = Vec3( 1.f, 0.1f, 0.f );

    LightDesc dirLight0;
    dirLight0.color = Rgba( 1.f, 1.f, 1.f, 0.5f );
    dirLight0.direction = Vec3( 0.f, -1.f, 1.f );
    dirLight0.isDirectional = 1.f;
    dirLight0.position = Vec3( 0.f, 10.f, -10.f );
    dirLight0.diffuseAttentuation = atten;
    dirLight0.specularAttenuation = atten;

    LightDesc pointLight1;
    pointLight1.color = Rgba( 1.f, 0.f, 0.f, 0.5f );
    pointLight1.isDirectional = 0.f;
    pointLight1.position = Vec3( -5.f, 2.f, 0.f );
    pointLight1.diffuseAttentuation = atten;
    pointLight1.specularAttenuation = atten;

    LightDesc pointLight2;
    pointLight2.color = Rgba( 0.f, 1.f, 0.f, 0.5f );
    pointLight2.isDirectional = 0.f;
    pointLight2.position = Vec3( -2.5f, 2.f, 0.f );
    pointLight2.diffuseAttentuation = atten;
    pointLight2.specularAttenuation = atten;

    LightDesc pointLight3;
    pointLight3.color = Rgba( 0.f, 0.f, 1.f, 0.5f );
    pointLight3.isDirectional = 0.f;
    pointLight3.position = Vec3( 0.f, 2.f, 0.f );
    pointLight3.diffuseAttentuation = atten;
    pointLight3.specularAttenuation = atten;

    LightDesc pointLight4;
    pointLight4.color = Rgba( 1.f, 1.f, 1.f, 0.5f );
    pointLight4.isDirectional = 0.f;
    pointLight4.position = Vec3( 2.5f, 2.f, 0.f );
    pointLight4.diffuseAttentuation = atten;
    pointLight4.specularAttenuation = atten;

    g_theRenderer->SetDynamicLight( 0, dirLight0 );
    g_theRenderer->SetDynamicLight( 1, pointLight1 );
    g_theRenderer->SetDynamicLight( 2, pointLight2 );
    g_theRenderer->SetDynamicLight( 3, pointLight3 );
    g_theRenderer->SetDynamicLight( 4, pointLight4 );
}


void Game::UpdateAttractScreen( float deltaSeconds ) {
    UpdateFromController( deltaSeconds );
}


void Game::UpdateGame( float deltaSeconds ) {
    // Update Entities
    //UpdateEntityArray( (Entity**)m_bullets, MAX_BULLETS, deltaSeconds );

    // Check Collision
    //CheckCollisionBetweenEntityArrays( (Entity**)shipArray, 1, (Entity**)m_asteroids, MAX_ASTEROIDS );

    //UpdateConsoleChannels( deltaSeconds );
    m_controller->Update( deltaSeconds );


    // Update Dynamic Lights
    float time = (float)GetCurrentTimeSeconds();
    float cosTime = cos( time );
    float sinTime = sin( time );

    for( int lightIndex = 1; lightIndex <= m_numDynamicLights; lightIndex++ ) {
        LightDesc light = g_theRenderer->GetDynamicLight( lightIndex );

        if( lightIndex == 1 ) { // Circle forward
            light.position.z = -cosTime;
            light.position.y = sinTime;
        } else if( lightIndex == 2 ) {
            if( !m_forcePosition ) {
                light.position.z = cosTime;
                light.position.y = sinTime;
            }
        } else if( lightIndex == 3 ) {
            light.position.y = -cosTime;
            light.position.z = -sinTime;
        } else if( lightIndex == 4 ) {
            light.position.y =  2.f * cosTime;
            light.position.z = -2.f * sinTime;
        }

        g_theDebugger->DrawDebugPoint( light.position, 0.f, 0.25f, light.color, light.color );
        g_theRenderer->SetDynamicLight( lightIndex, light );
    }
    

    // Shake Camera
    UpdateCameraShake( deltaSeconds );

    // Clean up Garbage
    CollectGarbage();
}


void Game::UpdateEntityArray( Entity** entityArray, int numEntities, float deltaSeconds ) {
    Entity* entity = nullptr;
	for( int i = 0; i < numEntities; i++ ) {
		entity = entityArray[i];
		if( entity != nullptr ) {
			entity->Update( deltaSeconds );
		}
	}
}


void Game::UpdateFromController( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    for( int i = 0; i < MAX_CONTROLLERS; i++ ) {
        const XboxController& controller = g_theInput->GetController( i );
        
        if( controller.IsConnected() ) {
            const KeyButtonState& startButton = controller.GetKeyButtonState( XBOX_BUTTON_ID_START );

            if( startButton.WasJustPressed() ) {
                StartupGame();
                m_onAttractScreen = false;
                return;
            }
        }
    }
}


void Game::UpdateCameraShake( float deltaSeconds ) {
    m_cameraShakeAmount -= CAMERA_SHAKE_REDUCTION_PER_SECOND * deltaSeconds;
    m_cameraShakeAmount = ClampFloat( m_cameraShakeAmount, 0.f, CAMERA_SHAKE_MAX );

    float randomX = g_RNG->GetRandomFloatInRange( 0.f, m_cameraShakeAmount );
    float randomY = g_RNG->GetRandomFloatInRange( 0.f, m_cameraShakeAmount );
    Camera* activeCamera = GetActiveCamera();
    activeCamera->Translate2D( Vec2( randomX, randomY ) );
}


void Game::UpdateConsoleChannels( float deltaSeconds ) {
    UNUSED( deltaSeconds );

    std::string channels[] = {
        // Switch to this alone
        "Command",
        "Info",
        "Warning",
        "Error",
        "RenderContext",
        // Enable/Disable this channel (additive)
        "Warning,Command",
        "!Command,Error",
        "Info,!Warning",
        "!RenderContext,!Warning",
        "!Warning,!RenderContext,!Command,Info,cOmMaND"
    };

    int channelIndex = (int)(fmod( 0.25 * GetCurrentTimeSeconds(), 10 ));

    if( channelIndex != m_channelIndex ) {
        m_channelIndex = channelIndex;
        std::string channelName = channels[channelIndex];

        if( channelIndex < 5 ) {
            std::string updateMsg = Stringf( "Switching to channel (%s)", channelName.c_str() );
            g_theDevConsole->PrintString( updateMsg.c_str(), Rgba::GREEN, (DevConsoleChannel)0xFFFFFFFF );

            g_theDevConsole->SwitchToChannel( channelName );
        } else {
            std::string updateMsg = Stringf( "Setting active channels (%s)", channelName.c_str() );
            g_theDevConsole->PrintString( updateMsg.c_str(), Rgba::GREEN, (DevConsoleChannel)0xFFFFFFFF );

            g_theDevConsole->SetActiveChannels( channelName );
        }
    }
}


void Game::RenderAttractScreen() const {
    //g_theRenderer->DrawVertexArray( s_numPressNVerts, m_attractPressNVerts );
}


void Game::RenderGame() const {
    g_theRenderer->ClearRenderTarget( Rgba::BLUE );
    g_theRenderer->BindShader( "BuiltIn/Lit" );
    g_theRenderer->SetAmbientLight( m_ambientColor );

    CPUMesh cpuMesh;
    GPUMesh gpuMesh = GPUMesh( g_theRenderer );

    // Floor
    cpuMesh.SetColor( Rgba::WHITE );
    cpuMesh.AddCircle( Vec3(0.f, -1.f, 0.f), 10.f, Vec3( 0.f, 1.f, 0.f ) );
    cpuMesh.SetColor( Rgba::BLUE );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    g_theRenderer->BindTexture();
    g_theRenderer->DrawMesh( &gpuMesh, Matrix44::IDENTITY );

    // Test Cylinders
    cpuMesh.Clear();
    cpuMesh.SetColor( Rgba::WHITE );
    //cpuMesh.AddCylinder( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddCone( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, -Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddHourGlass( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, -Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddNonUniformCylinder( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, 0.5f, Vec3( -1.f, 1.f, 2.f ) );

    g_theRenderer->BindMaterial( m_materials[1] );
    cpuMesh.AddQuad( Vec3( -1.f, -1.f, 0.f ), Vec3( 1.f, 1.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    //g_theRenderer->BindTexture( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
    g_theRenderer->DrawMesh( &gpuMesh, Matrix44::IDENTITY );

    // Draw Wood Box
    Vec3 corner = Vec3( 0.5f, 0.5f, 0.5f );
    cpuMesh.Clear();
    cpuMesh.AddBox( -corner, corner );

    float degrees = fmod( 20 * (float)GetCurrentTimeSeconds(), 360.f );
    Matrix44 cubeModel = Matrix44::MakeRotationDegrees3D( Vec3( degrees, -degrees, degrees * 2 ) );
    cubeModel.SetTranslation( Vec3( -5.f, 0.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    //g_theRenderer->BindTexture( "Data/Images/WoodCrate.jpg" );
    g_theRenderer->DrawMesh( &gpuMesh, cubeModel );

    // Draw Globe
    g_theRenderer->BindShader( "BuiltIn/Lit" );
    g_theRenderer->BindTexture( "Flat", TEXTURE_SLOT_NORMAL );
    g_theRenderer->BindTexture( "Black", TEXTURE_SLOT_EMISSIVE );

    cpuMesh.Clear();
    cpuMesh.AddUVSphere( Vec3::ZERO, 2.f );
    Matrix44 rotation = Matrix44::MakeYRotationDegrees( -degrees );
    rotation.SetTranslation( Vec3( 5.f, 0.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    g_theRenderer->BindTexture( "Data/Images/Globe.jpg" );
    g_theRenderer->DrawMesh( &gpuMesh, rotation );

    RenderDebugDraw();
    g_theDebugger->RenderWorld( m_playerCamera );
}


void Game::RenderDebugDraw() const {
    // World Space
    g_theDebugger->DrawDebugBasis( Matrix44::MakeTranslation3D( Vec3( -5.f, 5.f, 0.f ) ), 0.f, 0.1f );
    //g_theDebugger->DrawDebugPoint( Vec3( 1.f, 0.f, 0.f ), 0.f, 0.05f, Rgba::RED );
    //g_theDebugger->DrawDebugPoint( Vec3( 0.f, 1.f, 0.f ), 0.f, 0.05f, Rgba::GREEN );
    //g_theDebugger->DrawDebugPoint( Vec3( 0.f, 0.f, 1.f ), 0.f, 0.05f, Rgba::BLUE );

    g_theDebugger->DrawDebugArrow( Vec3::ZERO, Vec3( 10.f, 10.f, 10.f ), 0.f, 0.1f, Rgba::YELLOW );
    g_theDebugger->DrawDebugLine( Vec3(1.f, 0.f, 0.f), Vec3( 11.f, 10.f, 10.f ), 0.f, 0.1f, Rgba::YELLOW );
    g_theDebugger->DrawDebugBillboardedQuad( Vec3( -2.f, 1.f, 0.f ), Vec2( 2.f, 1.f ), 0.f, ALIGN_CENTER, Rgba::MAGENTA, Rgba::MAGENTA, "", DRAW_DEPTH_MOD_XRAY );
    g_theDebugger->DrawDebugBillboardedText( Vec3( -2.f, 2.f, 0.f ), Vec2( 2.f, 1.f ), "Hello, world!", 0.f, ALIGN_CENTER, nullptr, Rgba::MAGENTA, Rgba::MAGENTA, DRAW_DEPTH_MODE_ALWAYS );

    // Screen Space
    g_theDebugger->DrawDebugPoint( Vec2( 0.08f, 0.1f ), Vec2::ZERO, 0.f, .2f, Rgba::CYAN );
    g_theDebugger->DrawDebugLine( Vec2( 0.1f, 0.1f ), Vec2::ZERO, Vec2( 0.9f, 0.1f ), Vec2::ZERO, 0.f, 0.1f, Rgba::CYAN );
    g_theDebugger->DrawDebugArrow( Vec2( 0.1f, 0.2f ), Vec2::ZERO, Vec2( 0.9f, 0.2f ), Vec2::ZERO, 0.f, 0.1f, Rgba::CYAN );
    g_theDebugger->DrawDebugQuad( ALIGN_TOP_RIGHT, Vec2::ZERO, Vec2( 4.f, 2.f ), 0.f, Rgba::WHITE, Rgba::WHITE, "Data/Images/Globe.jpg" );
    g_theDebugger->DrawDebugText( ALIGN_TOP_CENTER, Vec2( 0.f, -0.5f ), Vec2( 4.f, 1.f ), "Welcome, Forseth!", 0.f );

    std::string message = Stringf( "Time: %0.2f", (float)GetCurrentTimeSeconds() );
    g_theDebugger->DrawDebugMessage( message, 0.f );
    g_theDebugger->DrawDebugMessage( "Hello again", 0.f );

    // Objects
    // Wood Box
    Vec3 corner = Vec3( 0.5f, 0.5f, 0.5f );
    OBB3 box = OBB3( Vec3(-7.f, 0.f, 0.f), corner, Vec3::RIGHT, Vec3::UP );
    g_theDebugger->DrawDebugBox( box, 0.f, Rgba::WHITE, Rgba::WHITE, "Data/Images/WoodCrate.jpg" );

    // Wireframe Globe
    g_theDebugger->DrawDebugPoint( Vec3( 10.f, 0.f, 0.f ), 0.f, 2.f, Rgba::WHITE, Rgba::WHITE, DRAW_DEPTH_MODE_DEPTH, FILL_MODE_WIREFRAME );
}


void Game::RenderEntityArray( const Entity** entityArray, int numEntities ) const {
	for( int i = 0; i < numEntities; i++ ) {
		const Entity* entity = entityArray[i];
		if( entity != nullptr ) {
			entity->Render();
		}
	}
}


void Game::CheckCollisionBetweenEntityArrays( Entity** entity1Array, int maxEntity1, Entity** entity2Array, int maxEntity2 ) {
    Entity* entity1 = nullptr;
    Vec2 entity1Pos;
    float entity1Radius;

    Entity* entity2 = nullptr;
    Vec2 entity2Pos;
    float entity2Radius;

    for( int i = 0; i < maxEntity1; i++ ) {
        entity1 = entity1Array[i];

        if( entity1 != nullptr && entity1->IsAlive() ) {
            entity1->GetPhysicsDisc( entity1Pos, entity1Radius );

            for( int j = 0; j < maxEntity2; j++ ) {
                entity2 = entity2Array[j];

                if( entity2 != nullptr && entity2->IsAlive() ) {
                    entity2->GetPhysicsDisc( entity2Pos, entity2Radius );

                    if( DoDiscsOverlap( entity1Pos, entity1Radius, entity2Pos, entity2Radius ) ) {
                        entity1->TakeDamage( 1 );
                        entity2->TakeDamage( 1 );
                        break; // Entity is no longer valid, stop looping through
                    }
                }
            }
        }
    }
}


void Game::DestroyEntity( Entity** entityArray, int entityIndex ) {
    Entity* entity = entityArray[entityIndex];

	entity->Shutdown();
    delete entity;
    entityArray[entityIndex] = nullptr;
}


void Game::CollectGarbage() {
}
