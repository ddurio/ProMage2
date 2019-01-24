#include "Game/Game.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDef.hpp"
#include "Engine/Renderer/SpriteDef.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/App.hpp"

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
}


void Game::Update( float deltaSeconds ) {
    Vec2 desktopOffset = GetDesktopOffset( m_activeDesktop );

    m_playerCamera->SetOrthoView( Vec2::ZERO + desktopOffset, m_desktopDimensions[m_activeDesktop] + desktopOffset );
    m_debugCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 2000.f, 1000.f ) );

    if( m_onAttractScreen ) {
        UpdateAttractScreen( deltaSeconds );
    } else {
        UpdateGame( deltaSeconds );
    }

}


void Game::Render() const {
    Camera activeCamera = GetActiveCamera();
    g_theRenderer->BeginCamera( activeCamera );

    // TODO: Remove after changing to D3D11
    Rgba colors[] = { Rgba::RED, Rgba::GREEN, Rgba::BLUE };
    int colorIndex = (int)(fmod( GetCurrentTimeSeconds(), 3 ));

    g_theRenderer->ClearScreen( colors[colorIndex] );
    // Remove up to here

    /* TODO: Uncomment after changing to D3D11
    g_theRenderer->ClearScreen( Rgba::BLUE );
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
            return false;
        } case(0x71): { // F2 - Go To Previous Desktop
            m_activeDesktop--;

            if( m_activeDesktop < 0 ) {
                m_activeDesktop = m_numDesktops - 1;
            }
            return false;
        } case(0x72): { // F3 - Go To Next Desktop
            m_activeDesktop = ++m_activeDesktop % m_numDesktops;
            return false;
        } case(0x73): { // F4 - Toggle Debug Camera
            m_useDebugCamera = !m_useDebugCamera;
            return false;
        } case('S'): {
            SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
            g_theAudio->PlaySound( testSound );
            return false;
        }
	}

    if( !m_onAttractScreen ) {
        //return m_thePlayerShip->HandleKeyPressed( keyCode );
        return true;
    } else {
        return true;
    }
}


bool Game::HandleKeyReleased( unsigned char keyCode ) {
    UNUSED( keyCode );

    if( !m_onAttractScreen ) {
	    //return m_thePlayerShip->HandleKeyReleased( keyCode );
        return 1;
    } else {
        return 1;
    }
}


bool Game::HandleQuitRequested() {
	return 1;
}


void Game::ReturnToAttractScreen() {
    Shutdown();
    m_onAttractScreen = true;
}


const Camera& Game::GetActiveCamera() const {
    Camera* activeCamera = m_useDebugCamera ? m_debugCamera : m_playerCamera;
    return *activeCamera;
}


bool Game::HasGameBeenBeaten() const {
    return m_hasBeatenTheGame;
}


void Game::StartupAttract() {
}


void Game::StartupGame() {
    StartupParseXMLTests();
    StartupConsoleTests();
    StartupEventTests();
}


void Game::StartupParseXMLTests() {
    tinyxml2::XMLDocument xmlTest = new tinyxml2::XMLDocument();
    tinyxml2::XMLError loadSuccess = xmlTest.LoadFile( DATA_XML_TEST );
    GUARANTEE_OR_DIE( loadSuccess == tinyxml2::XML_SUCCESS, Stringf("Failed to load XML file: %s", DATA_PROJECT_CONFIG) );

    tinyxml2::XMLNode* root = xmlTest.FirstChild();
    GUARANTEE_OR_DIE( root != nullptr, "Poorly constructed XML file" );

    //const char* projectName = root->ToElement()->Attribute( "name" );
    std::string projectName = ParseXMLAttribute( *root->ToElement(), "name", "NAME NOT FOUND" );

    const BitmapFont* font =  g_theRenderer->CreateOrGetBitmapFontFromFile( FONT_NAME_SQUIRREL );
    float cellHeight = 1.5f;
    Vec2 textStart( 100.f, 97.f );
    font->AddVertsForText2D( m_xmlVerts, textStart, cellHeight, Stringf( "- Root: name = %s", projectName.c_str() ) );
    textStart.y -= 3.f;

    // Display, fullscreen, resolution: testing bool and IntVec2
    XMLElement* display = root->FirstChildElement( "display" );
    //bool isFullscreen = display->BoolAttribute( "fullscreen" );
    bool isFullscreen = ParseXMLAttribute( *display, "fullscreen", false );
    //IntVec2 resolution( display->Attribute( "resolution" ) );
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
void Game::UpdateAttractScreen( float deltaSeconds ) {
    UpdateFromController( deltaSeconds );
}


void Game::UpdateGame( float deltaSeconds ) {
    // Update Entities
    //UpdateEntityArray( (Entity**)m_bullets, MAX_BULLETS, deltaSeconds );

    // Check Collision
    //CheckCollisionBetweenEntityArrays( (Entity**)shipArray, 1, (Entity**)m_asteroids, MAX_ASTEROIDS );

    UpdateConsoleChannels( deltaSeconds );

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
    Camera activeCamera = GetActiveCamera();
    activeCamera.Translate2D( Vec2( randomX, randomY ) );
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
    RenderTextInBox( 0 );
    RenderXML( 0 );
}


void Game::RenderTexture( int desktopID ) const {
    Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile( TEXTURE_STBI_TEST );
    g_theRenderer->BindTexture( testTexture );

    AABB2 box1Bounds = AABB2( Vec2( 50.f, 50.f ), Vec2( 100.f, 100.f ) );
    std::vector<Vertex_PCU> boxVerts;
    AddVertsForAABB2D( boxVerts, box1Bounds, Rgba( 1.f, 1.f, 1.f, 1.f ), Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

    TransformVertexArrayToDesktop( desktopID, 1, boxVerts.size(), boxVerts.data() );
    g_theRenderer->DrawVertexArray( (int)boxVerts.size(), &boxVerts[0] );
}


void Game::RenderSpriteAnimations( int desktopID ) const {
    Texture* testTexture2 = g_theRenderer->CreateOrGetTextureFromFile( TEXTURE_ANIMATION_TEST );
    SpriteSheet spriteSheet = SpriteSheet( testTexture2, IntVec2( 8, 2 ) );

    Vec2 uvBL;
    Vec2 uvTR;

    // Play once
    SpriteAnimDef* animation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_ONCE );
    SpriteDef spriteDef = animation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    std::vector<Vertex_PCU> spriteVerts;
    AABB2 spriteBox = AABB2( Vec2( 0.f, 0.f ), Vec2( 10.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );

    //g_theRenderer->BindTexture( testTexture2 );
    //g_theRenderer->DrawVertexArray( spriteVerts );

    // Loop Animations
    SpriteAnimDef* loopAnimation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_LOOP );
    spriteDef = loopAnimation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    spriteBox = AABB2( Vec2( 10.f, 0.f ), Vec2( 20.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );
    //g_theRenderer->BindTexture( testTexture2 );
    //g_theRenderer->DrawVertexArray( (int)spriteVerts.size(), spriteVerts.data() );

    // PingPong Animations
    SpriteAnimDef* pingPongAnimation = new SpriteAnimDef( spriteSheet, 0, 15, 16.f, SPRITE_ANIM_PLAYBACK_PINGPONG );
    spriteDef = pingPongAnimation->GetSpriteDefAtTime( (float)GetCurrentTimeSeconds() );

    spriteDef.GetUVs( uvBL, uvTR );
    spriteBox = AABB2( Vec2( 20.f, 0.f ), Vec2( 30.f, 20.f ) );
    AddVertsForAABB2D( spriteVerts, spriteBox, Rgba( 1.f, 1.f, 1.f, 1.f ), uvBL, uvTR );

    TransformVertexArrayToDesktop( desktopID, 1, spriteVerts.size(), spriteVerts.data() );
    g_theRenderer->BindTexture( testTexture2 );
    g_theRenderer->DrawVertexArray( spriteVerts );
}


void Game::RenderTextInBox( int desktopID ) const {
    RenderTextAlignment( desktopID );
    RenderTextDrawMode( desktopID );
    RenderTextMultiLine( desktopID );
}


void Game::RenderTextAlignment( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->CreateOrGetBitmapFontFromFile( FONT_NAME_SQUIRREL );

    std::vector<Vertex_PCU> textVerts;
    AABB2 desktopBounds = GetDesktopBounds( desktopID );
    Vec2 desktopDimensions = desktopBounds.GetDimensions();

    float desktopCenterX = (desktopDimensions.x * 0.5f);
    float desktopCenterY = (desktopDimensions.y * 0.5f);

    // Text Alignment
    float cellHeight = 2.f;
    std::string text = "Alignment";
    AABB2 boxBounds( Vec2( desktopCenterX * 1.25f, desktopCenterY * 0.5f ), Vec2( desktopCenterX * 1.75f, desktopCenterY * 0.75f ) );
    std::vector<Vertex_PCU> boxVerts;
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

    g_theRenderer->BindTexture( nullptr );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexture() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderTextDrawMode( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->CreateOrGetBitmapFontFromFile( FONT_NAME_SQUIRREL );

    std::vector<Vertex_PCU> textVerts;
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

    g_theRenderer->BindTexture( nullptr );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexture() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderTextMultiLine( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->CreateOrGetBitmapFontFromFile( FONT_NAME_SQUIRREL );

    std::vector<Vertex_PCU> textVerts;
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

    g_theRenderer->BindTexture( nullptr );
    g_theRenderer->DrawVertexArray( boxVerts );

    g_theRenderer->BindTexture( font->GetTexture() );
    g_theRenderer->DrawVertexArray( textVerts );
}


void Game::RenderXML( int desktopID ) const {
    const BitmapFont* font = g_theRenderer->CreateOrGetBitmapFontFromFile( FONT_NAME_SQUIRREL );
    std::vector<Vertex_PCU> verts = m_xmlVerts;

    TransformVertexArrayToDesktop( desktopID, 1, verts.size(), verts.data() );
    g_theRenderer->BindTexture( font->GetTexture() );
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
        Vertex_PCU* verts = va_arg( vertexArrays, Vertex_PCU* );
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
