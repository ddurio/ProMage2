#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"


class Camera;


class HelpWindow : public EditorWindow {
    public:
    HelpWindow();
    ~HelpWindow();

    bool IsOpen() const;


    private:
    Camera* m_camera = nullptr;
    bool m_typeIndex = 0;

    const Strings m_typeNames = {
        TEXTURE_EDITOR_HELP_CA,
        TEXTURE_EDITOR_HELP_DF,
        TEXTURE_EDITOR_HELP_FI,
        TEXTURE_EDITOR_HELP_PN,
        TEXTURE_EDITOR_HELP_RnP,
        TEXTURE_EDITOR_HELP_Spr
    };


    bool UpdateChild( float deltaSeconds ) override;
};
