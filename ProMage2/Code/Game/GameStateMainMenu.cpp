#include "Game/GameStateMainMenu.hpp"

#include "Engine/Debug/DebugDraw.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "Game/Game.hpp"
#include "Game/GameStatePlay.hpp"
#include "Game/UIButton.hpp"
#include "Game/UILabel.hpp"
#include "Game/UIWidget.hpp"


GameStateMainMenu::GameStateMainMenu() {
    m_uiCamera->SetOrthoProjection( 10.f, -100.f, 100.f, CLIENT_ASPECT );

    Shader* shader = g_theRenderer->GetOrCreateShader( "BuiltIn/Unlit" );
    shader->SetDepthMode( COMPARE_ALWAYS, false );
    g_theRenderer->BindShader( shader );

    BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
    g_theRenderer->BindTexture( font->GetTexturePath() );

    BuildLoadingMesh();
}


GameStateMainMenu::~GameStateMainMenu() {
    CLEAR_POINTER( m_uiParent );
}


void GameStateMainMenu::Startup() {
    // Load textures and sounds
    // ...

    g_theEventSystem->SubscribeEventCallbackFunction( "changeGameState", Command_ChangeGameState );

    // Load UI
    m_uiParent = new UIWidget( ALIGN_CENTER, Vec2::ZERO, Vec2( 0.9f, 0.9f ), Vec2::ZERO );
    m_uiParent->SetColor( Rgba::CLEAR );

    // Title
    UIWidget* title = new UILabel( ALIGN_CENTER_LEFT, Vec2::ZERO, Vec2( 0.5f, 0.25f ), Vec2::ZERO, "Adventure" );
    UIWidget* titleBG = new UIWidget( ALIGN_CENTER_LEFT, Vec2::ZERO, Vec2( 0.5f, 0.25f ), Vec2::ZERO );
    titleBG->SetColor( Rgba::RED );

    m_uiParent->AddChild( titleBG );
    m_uiParent->AddChild( title );

    // Play Button
    UIWidget* play = new UIButton( Vec2( 1.f, 0.75f ), Vec2::ZERO, Vec2( 0.25f, 0.25f ), Vec2::ZERO, "changeGameState state=play" );
    play->SetColor( Rgba::RED );
    m_uiParent->AddChild( play );

    UIWidget* playText = new UILabel( ALIGN_CENTER, Vec2::ZERO, Vec2::ONE, Vec2::ZERO, "Play" );
    play->AddChild( playText );
}


void GameStateMainMenu::Shutdown() {

}


void GameStateMainMenu::Update() {
    if( m_loadState != LOAD_STATE_DONE ) {
        switch( m_loadState ) {
            case(LOAD_STATE_PRE_INIT): {
                m_loadState = LOAD_STATE_INIT;
                return;
            } case(LOAD_STATE_INIT): {
                Startup();
                m_loadState = LOAD_STATE_DONE;
            }
        }
    }

    m_uiParent->UpdateBounds( m_uiCamera->GetBounds() );
}


void GameStateMainMenu::Render() {
    g_theRenderer->BeginCamera( m_uiCamera );

    if( m_loadState == LOAD_STATE_INIT ) {
        g_theRenderer->ClearRenderTarget( Rgba::CYAN );

        BitmapFont* font = g_theRenderer->GetOrCreateBitmapFont( FONT_NAME_SQUIRREL );
        g_theRenderer->BindTexture( font->GetTexturePath() );

        g_theRenderer->DrawMesh( m_loadingMesh );
        g_theRenderer->EndCamera( m_uiCamera );
        return;
    }

    g_theRenderer->ClearRenderTarget( Rgba::GREEN );

    m_uiParent->Render();

    g_theDebugger->RenderWorld( m_uiCamera );
    g_theRenderer->EndCamera( m_uiCamera );
}


bool GameStateMainMenu::HandleKeyPressed( unsigned char keyCode ) {
    switch( keyCode ) {
        case('P'): {
            EventArgs args;
            args.SetValue( "state", "play" );
            Command_ChangeGameState( args );
            return true;
        }
    }

    return false;
}


bool GameStateMainMenu::HandleKeyReleased( unsigned char keyCode ) {
    UNUSED( keyCode );
    return false;
}


bool GameStateMainMenu::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    return m_uiParent->HandleMouseButton( event, scrollAmount );
}


bool GameStateMainMenu::HandleQuitRequested() {
    return false;
}


bool GameStateMainMenu::Command_ChangeGameState( EventArgs& args ) {
    std::string newState = args.GetValue( "state", "" );
    newState = StringToLower( newState );

    if( newState == "play" ) {
        GameState* playState = new GameStatePlay();
        g_theGame->ChangeGameState( playState );
        return true;
    }

    return false;
}
