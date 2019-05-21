#pragma once
#include "Game/GameCommon.hpp"

#include "Game/UIWidget.hpp"


class UIButton : public UIWidget {
    public:
    UIButton( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize, const std::string& commandString );
    ~UIButton();

    bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f ) override;

    void OnClick();

    void SetCommandString( const std::string& commandString );

    private:
    std::string m_commandString = "";
};
