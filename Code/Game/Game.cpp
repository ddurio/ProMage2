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


void Game::Update( float deltaSeconds ) {
    Vec2 desktopOffset = GetDesktopOffset( m_activeDesktop );

    //m_playerCamera->SetOrthoView( Vec2::ZERO + desktopOffset, m_desktopDimensions[m_activeDesktop] + desktopOffset );
    //m_playerCamera->SetOrthoProjection( 10.f );
    m_playerCamera->SetPerspectiveProjection( 90.f, 0.00001f, 100.f );
    m_debugCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 2000.f, 1000.f ) );

    if( m_onAttractScreen ) {
        UpdateAttractScreen( deltaSeconds );
    } else {
        UpdateGame( deltaSeconds );
    }

}


void Game::Render() const {
    Camera* activeCamera = GetActiveCamera();

    // Only necessary if something other than the back buffer is used
    /*
    ColorTargetView* targetView = g_theRenderer->GetNewColorTarget();
    activeCamera.SetColorTarget( nullptr );
    */

    m_cameraPos->Render();
    g_theRenderer->BeginCamera( activeCamera );

    // TODO: Remove after changing to D3D11
    g_theRenderer->ClearColorTarget( Rgba::BLUE );
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
    cpuMesh.SetColor( Rgba::GRAY );
    //cpuMesh.AddCylinder( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddCone( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, -Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddHourGlass( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, -Vec3( 1.f, 1.f, 1.f ) );
    //cpuMesh.AddNonUniformCylinder( Vec3( 0.f, 1.f, -5.f ), 1.f, 0.25f, 0.5f, Vec3( -1.f, 1.f, 2.f ) );

    cpuMesh.AddQuad( Vec3( -1.f, -1.f, 0.f ), Vec3( 1.f, 1.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    g_theRenderer->BindTexture( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
    g_theRenderer->DrawMesh( &gpuMesh, Matrix44::IDENTITY );

    // Draw Wood Box
    Vec3 corner = Vec3( 0.5f, 0.5f, 0.5f );
    cpuMesh.Clear();
    cpuMesh.AddBox( -corner, corner );

    float degrees = fmod( 20 * (float)GetCurrentTimeSeconds(), 360.f );
    Matrix44 cubeModel = Matrix44::MakeRotationDegrees3D( Vec3( degrees, -degrees * 0.5f, degrees * 2 ) );
    cubeModel.SetTranslation( Vec3( -5.f, 0.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    g_theRenderer->BindTexture( "Data/Images/WoodCrate.jpg" );
    g_theRenderer->DrawMesh( &gpuMesh, cubeModel );

    // Draw Globe
    cpuMesh.Clear();
    cpuMesh.AddUVSphere( Vec3::ZERO, 2.f );
    Matrix44 rotation = Matrix44::MakeYRotationDegrees( -degrees );
    rotation.SetTranslation( Vec3( 5.f, 0.f, 0.f ) );

    gpuMesh.CopyVertsFromCPUMesh( &cpuMesh );
    g_theRenderer->BindTexture( "Data/Images/Globe.jpg" );
    g_theRenderer->DrawMesh( &gpuMesh, rotation );

    //RenderDebugDraw();
    g_theDebugger->RenderWorld( activeCamera );
    // Remove up to here


    // TODO: Uncomment after changing to D3D11
    /*
    g_theRenderer->ClearColorTarget( Rgba::BLUE );
    if( m_onAttractScreen ) {
        RenderAttractScreen();
    } else {
        RenderGame();
    }
    */

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
            m_renderNormals = !m_renderNormals;
            g_theRenderer->SetRenderNormals( m_renderNormals );
            return true;
        } case(0x72): { // F3 - Go To Next Desktop
            m_activeDesktop = ++m_activeDesktop % m_numDesktops;
            return true;
        } case(0x73): { // F4 - Toggle Debug Camera
            m_useDebugCamera = !m_useDebugCamera;
            return true;
        } case(0xBC): { // F4 - Toggle Debug Camera
            m_ambientColor.a = ClampFloat( m_ambientColor.a - 0.02f, 0.f, 1.f );
            return true;
        } case(0xBE): { // F4 - Toggle Debug Camera
            m_ambientColor.a = ClampFloat( m_ambientColor.a + 0.02f, 0.f, 1.f );
            return true;
        }
	}

    if( !m_onAttractScreen ) {
        return m_cameraPos->HandleKeyPressed( keyCode );
    } else {
        return false;
    }
}


bool Game::HandleKeyReleased( unsigned char keyCode ) {
    UNUSED( keyCode );

    if( !m_onAttractScreen ) {
	    return m_cameraPos->HandleKeyReleased( keyCode );
    } else {
        return false;
    }
}


bool Game::HandleQuitRequested() {
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


bool Game::HasGameBeenBeaten() const {
    return m_hasBeatenTheGame;
}


bool Game::Command_SetAmbientLight( EventArgs& args ) {
    g_theGame->m_ambientColor = args.GetValue( "color", g_theGame->m_ambientColor );
    g_theDevConsole->PrintString( Stringf( "- Setting Ambient Light to %s", g_theGame->m_ambientColor.GetAsString().c_str() ), DevConsole::CHANNEL_INFO );

    g_theRenderer->SetAmbientLight( g_theGame->m_ambientColor );
    return false;
}


void Game::StartupAttract() {
}


void Game::StartupGame() {
    StartupParseXMLTests();
    StartupConsoleTests();
    StartupEventTests();

    g_theEventSystem->SubscribeEventCallbackFunction( "SetAmbient", Command_SetAmbientLight );
    StartupLights();

    m_cameraPos = new CameraController( m_playerCamera );
    m_cameraPos->Startup();

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
    dirLight0.positioin = Vec3( 0.f, 10.f, -10.f );
    dirLight0.diffuseAttentuation = atten;
    dirLight0.specularAttenuation = atten;

    LightDesc pointLight1;
    pointLight1.color = Rgba( 1.f, 0.f, 0.f, 0.5f );
    pointLight1.isDirectional = 0.f;
    pointLight1.positioin = Vec3( -5.f, 2.f, 0.f );
    pointLight1.diffuseAttentuation = atten;
    pointLight1.specularAttenuation = atten;

    LightDesc pointLight2;
    pointLight2.color = Rgba( 0.f, 1.f, 0.f, 0.5f );
    pointLight2.isDirectional = 0.f;
    pointLight2.positioin = Vec3( -2.5f, 2.f, 0.f );
    pointLight2.diffuseAttentuation = atten;
    pointLight2.specularAttenuation = atten;

    LightDesc pointLight3;
    pointLight3.color = Rgba( 0.f, 0.f, 1.f, 0.5f );
    pointLight3.isDirectional = 0.f;
    pointLight3.positioin = Vec3( 0.f, 2.f, 0.f );
    pointLight3.diffuseAttentuation = atten;
    pointLight3.specularAttenuation = atten;

    LightDesc pointLight4;
    pointLight4.color = Rgba( 1.f, 1.f, 1.f, 0.5f );
    pointLight4.isDirectional = 0.f;
    pointLight4.positioin = Vec3( 2.5f, 2.f, 0.f );
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
    m_cameraPos->Update( deltaSeconds );


    // Update Dynamic Lights
    float time = (float)GetCurrentTimeSeconds();
    float cosTime = cos( time );
    float sinTime = sin( time );

    for( int lightIndex = 1; lightIndex <= m_numDynamicLights; lightIndex++ ) {
        LightDesc light = g_theRenderer->GetDynamicLight( lightIndex );

        if( lightIndex == 1 ) { // Circle forward
            light.positioin.z = -cosTime;
            light.positioin.y = sinTime;
        } else if( lightIndex == 2 ) {
            light.positioin.z = cosTime;
            light.positioin.y = sinTime;
        } else if( lightIndex == 3 ) {
            light.positioin.y = -cosTime;
            light.positioin.z = -sinTime;
        } else if( lightIndex == 4 ) {
            light.positioin.y =  2.f * cosTime;
            light.positioin.z = -2.f * sinTime;
        }

        g_theDebugger->DrawDebugPoint( light.positioin, 0.f, 0.25f, light.color, light.color );
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
    RenderTexture( 0 );
    RenderSpriteAnimations( 0 );
    RenderAdditiveVenn( 0 );
    RenderTextInBox( 0 );
    RenderXML( 0 );
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


void Game::RenderTexture( int desktopID ) const {
    g_theRenderer->BindTexture( TEXTURE_STBI_TEST );

    AABB2 box1Bounds = AABB2( Vec2( 50.f, 50.f ), Vec2( 100.f, 100.f ) );
    std::vector<VertexPCU> boxVerts;
    AddVertsForAABB2D( boxVerts, box1Bounds, Rgba( 1.f, 1.f, 1.f, 1.f ) );

    TransformVertexArrayToDesktop( desktopID, 1, boxVerts.size(), boxVerts.data() );
    g_theRenderer->DrawVertexArray( (int)boxVerts.size(), &boxVerts[0] );
}


void Game::RenderSpriteAnimations( int desktopID ) const {
    g_theRenderer->CreateTexture( TEXTURE_ANIMATION_TEST );
    SpriteSheet spriteSheet = SpriteSheet( TEXTURE_ANIMATION_TEST, IntVec2( 8, 2 ) );

    Vec2 uvBL;
    Vec2 uvTR;

    // Play once
    SpriteAnimDef* animation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_ONCE );
    SpriteDef spriteDef = animation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    std::vector<VertexPCU> spriteVerts;
    AABB2 spriteBox = AABB2( Vec2( 0.f, 0.f ), Vec2( 10.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );

    // Loop Animations
    SpriteAnimDef* loopAnimation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_LOOP );
    spriteDef = loopAnimation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    spriteBox = AABB2( Vec2( 10.f, 0.f ), Vec2( 20.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );

    // PingPong Animations
    SpriteAnimDef* pingPongAnimation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_PINGPONG );
    spriteDef = pingPongAnimation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    spriteBox = AABB2( Vec2( 20.f, 0.f ), Vec2( 30.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );

    TransformVertexArrayToDesktop( desktopID, 1, spriteVerts.size(), spriteVerts.data() );
    g_theRenderer->BindTexture( TEXTURE_ANIMATION_TEST );
    g_theRenderer->DrawVertexArray( spriteVerts );
}


void Game::RenderAdditiveVenn( int desktopID ) const {
    // Draw background (needs to be black or the additive blend picks up other initial colors)
    AABB2 desktopBounds = GetDesktopBounds( desktopID );
    AABB2 outerBoxBounds = desktopBounds.GetBoxWithin( Vec2( 34.f, 34.f ), ALIGN_TOP_LEFT );
    AABB2 boxBounds = outerBoxBounds.GetBoxWithin( Vec2( 30.f, 30.f ), ALIGN_CENTER );
    VertexList boxVerts;
    AddVertsForAABB2D( boxVerts, outerBoxBounds, Rgba::GRAY );
    AddVertsForAABB2D( boxVerts, boxBounds, Rgba::BLACK );

    g_theRenderer->BindTexture();
    g_theRenderer->DrawVertexArray( boxVerts );

    // Draw Circles
    VertexList vennVerts;
    float circleRadius = 8.f;

    // Red Circle
    Rgba circleColor = Rgba::RED;
    circleColor.a = 0.75f;
    Vec2 circleCenter = boxBounds.GetPointWithin( Vec2( 0.5f, 0.66f ) );
    AddVertsForDisc2D( vennVerts, circleCenter, circleRadius, circleColor );

    // Green Circle
    circleColor = Rgba::GREEN;
    circleColor.a = 0.75f;
    circleCenter = boxBounds.GetPointWithin( Vec2( 0.33f, 0.33f ) );
    AddVertsForDisc2D( vennVerts, circleCenter, circleRadius, circleColor );

    // Blue Circle
    circleColor = Rgba::BLUE;
    circleColor.a = 0.75f;
    circleCenter = boxBounds.GetPointWithin( Vec2( 0.66f, 0.33f ) );
    AddVertsForDisc2D( vennVerts, circleCenter, circleRadius, circleColor );

    double modeFreq = 0.5f * GetCurrentTimeSeconds();
    BlendMode modeIndex = (BlendMode)((int)modeFreq % 2);

    g_theRenderer->BindTexture();
    g_theRenderer->DrawVertexArray( vennVerts, modeIndex );
}


void Game::RenderTextInBox( int desktopID ) const {
    RenderTextAlignment( desktopID );
    RenderTextDrawMode( desktopID );
    RenderTextMultiLine( desktopID );
}


void Game::RenderTextAlignment( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    std::vector<VertexPCU> textVerts;
    AABB2 desktopBounds = GetDesktopBounds( desktopID );
    Vec2 desktopDimensions = desktopBounds.GetDimensions();

    float desktopCenterX = (desktopDimensions.x * 0.5f);
    float desktopCenterY = (desktopDimensions.y * 0.5f);

    // Text Alignment
    float cellHeight = 2.f;
    std::string text = "Alignment";
    AABB2 boxBounds( Vec2( desktopCenterX * 1.25f, desktopCenterY * 0.5f ), Vec2( desktopCenterX * 1.75f, desktopCenterY * 0.75f ) );
    std::vector<VertexPCU> boxVerts;
    AABB2 borderBounds = boxBounds.GetPaddedAABB2( 2.f );
    AddVertsForAABB2D( boxVerts, borderBounds, Rgba::GRAY );
    AddVertsForAABB2D( boxVerts, boxBounds, Rgba::BLACK );

    float alignmentX = cos( (float)GetCurrentTimeSeconds() ) + 0.5f;
    float alignmentY = sin( (float)GetCurrentTimeSeconds() ) + 0.5f;
    alignmentX = ClampFloat( alignmentX, 0.f, 1.f );
    alignmentY = ClampFloat( alignmentY, 0.f, 1.f );

    Vec2 movingAlignment = Vec2( alignmentX, alignmentY );
    text = Stringf( "%s: %.02f, %.02f", text.c_str(), movingAlignment.x, movingAlignment.y );

    double glyphRate = 10.f * GetCurrentTimeSeconds();
    int numGlyphs = (int)glyphRate % 30;

    font->AddVeretsForTextInBox2D( textVerts, boxBounds, cellHeight, text, Rgba::WHITE, 1.f, movingAlignment, TEXT_DRAW_OVERRUN, numGlyphs );


    TransformVertexArrayToDesktop( desktopID, 2, boxVerts.size(), boxVerts.data(), textVerts.size(), textVerts.data() );

    g_theRenderer->BindTexture( "" );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexturePath() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderTextDrawMode( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    std::vector<VertexPCU> textVerts;
    AABB2 desktopBounds = GetDesktopBounds( desktopID );
    Vec2 desktopDimensions = desktopBounds.GetDimensions();

    float desktopCenterX = (desktopDimensions.x * 0.5f);
    float desktopCenterY = (desktopDimensions.y * 0.5f);

    VertexList boxVerts;
    AABB2 boxBounds2( Vec2( desktopCenterX * 0.5f, desktopCenterY * 0.5f ), Vec2( desktopCenterX * 1.f, desktopCenterY * 0.75f ) );
    AABB2 borderBounds2 = boxBounds2.GetPaddedAABB2( 2.f );
    AddVertsForAABB2D( boxVerts, borderBounds2, Rgba::GRAY );
    AddVertsForAABB2D( boxVerts, boxBounds2, Rgba::BLACK );

    std::string text = "In a Box??";
    float cellHeight = 8.f * sin( (float)GetCurrentTimeSeconds() );

    Vec2 alignments[] = { ALIGN_CENTER, ALIGN_CENTER_LEFT, ALIGN_BOTTOM_CENTER, ALIGN_CENTER_RIGHT, ALIGN_TOP_CENTER };
    int alignmentIndex = (int)(fmod( 0.25 * GetCurrentTimeSeconds(), 5 ));

    font->AddVeretsForTextInBox2D( textVerts, boxBounds2, cellHeight, text, Rgba::GRAY,  1.f, alignments[alignmentIndex], TEXT_DRAW_OVERRUN );
    font->AddVeretsForTextInBox2D( textVerts, boxBounds2, cellHeight, text, Rgba::WHITE, 1.f, alignments[alignmentIndex], TEXT_DRAW_SHRINK_TO_FIT );


    TransformVertexArrayToDesktop( desktopID, 2, boxVerts.size(), boxVerts.data(), textVerts.size(), textVerts.data() );

    g_theRenderer->BindTexture( "" );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexturePath() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderTextMultiLine( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );

    std::vector<VertexPCU> textVerts;
    AABB2 desktopBounds = GetDesktopBounds( desktopID );
    Vec2 desktopDimensions = desktopBounds.GetDimensions();

    float desktopCenterX = (desktopDimensions.x * 0.5f);
    float desktopCenterY = (desktopDimensions.y * 0.5f);

    VertexList boxVerts;
    AABB2 boxBounds2( Vec2( desktopCenterX * 1.25f, desktopCenterY * 0.9f ), Vec2( desktopCenterX * 1.75f, desktopCenterY * 1.25f ) );
    AABB2 borderBounds2 = boxBounds2.GetPaddedAABB2( 2.f );
    AddVertsForAABB2D( boxVerts, borderBounds2, Rgba::GRAY );
    AddVertsForAABB2D( boxVerts, boxBounds2, Rgba::BLACK );

    std::string textOptions[] = {
        "\nThis is\na string \nwith multiple\n lines \n and odd spaces\n",
        "This string\nhas less odd spacing\nand should be more obviously\naligned correctly!"
    };
    int textIndex = (int)(fmod( 0.1 * GetCurrentTimeSeconds(), 2 ));
    std::string text = textOptions[textIndex];

    float cellHeight = 2.f;

    float alignmentX = cos( (float)GetCurrentTimeSeconds() ) + 0.5f;
    float alignmentY = sin( (float)GetCurrentTimeSeconds() ) + 0.5f;
    alignmentX = ClampFloat( alignmentX, 0.f, 1.f );
    alignmentY = ClampFloat( alignmentY, 0.f, 1.f );
    Vec2 movingAlignment = Vec2( alignmentX, alignmentY );

    double glyphRate = 30.f * GetCurrentTimeSeconds();
    int numGlyphs = (int)glyphRate % 125;

    font->AddVeretsForTextInBox2D( textVerts, boxBounds2, cellHeight, text, Rgba::WHITE, 1.f, movingAlignment, TEXT_DRAW_SHRINK_TO_FIT, numGlyphs );

    TransformVertexArrayToDesktop( desktopID, 2, boxVerts.size(), boxVerts.data(), textVerts.size(), textVerts.data() );

    g_theRenderer->BindTexture( "" );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexturePath() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderXML( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    std::vector<VertexPCU> verts = m_xmlVerts;

    TransformVertexArrayToDesktop( desktopID, 1, verts.size(), verts.data() );
    g_theRenderer->BindTexture( font->GetTexturePath() );
    g_theRenderer->DrawVertexArray( verts );
}


void Game::RenderEntityArray( const Entity** entityArray, int numEntities ) const {
	for( int i = 0; i < numEntities; i++ ) {
		const Entity* entity = entityArray[i];
		if( entity != nullptr ) {
			entity->Render();
		}
	}
}


const Vec2 Game::GetDesktopOffset( int desktopID ) const {
    Vec2 desktopOffset( 0.f, 0.f );

    for( int desktopIndex = 0; desktopIndex < desktopID; desktopIndex++ ) {
        float desktopHeight = m_desktopDimensions[desktopIndex].y;
        desktopOffset += Vec2( 0.f, desktopHeight );
    }

    return desktopOffset;
}


const AABB2 Game::GetDesktopBounds( int desktopID ) const {
    Vec2 mins = GetDesktopOffset( desktopID );
    Vec2 maxs = mins + m_desktopDimensions[desktopID];
    return AABB2( mins, maxs );
}


void Game::TransformVertexArrayToDesktop( int desktopID, int numArrays, ... ) const {
    Vec2 desktopOffset = GetDesktopOffset( desktopID );

    va_list vertexArrays;
    va_start( vertexArrays, numArrays );

    for( int arrayIndex = 0; arrayIndex < numArrays; arrayIndex++ ) {
        int numVerts = (int)va_arg( vertexArrays, size_t );
        VertexPCU* verts = va_arg( vertexArrays, VertexPCU* );
        TransformVertexArray( numVerts, verts, 1.f, 0.f, desktopOffset );
    }
    
    va_end( vertexArrays );
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
