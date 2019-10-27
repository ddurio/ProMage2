#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/FloatRange.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


class Image;
class TileDef;

struct AABB2;


class MGS_FromImage : public MapGenStep {
    friend class EditorMapGenStep;

    public:
    MGS_FromImage( const XMLElement& element, const Strings& motifHierarchy );
    MGS_FromImage( const MGS_FromImage& copyFrom );
    ~MGS_FromImage();

    void SaveToXml( XmlDocument& document, XMLElement& element ) const override;
    bool RecalculateMotifVars( EventArgs& args ) override;


    private:
    static std::map< std::string, const TileDef* > s_tileDefsByTexelColor;

    std::string m_imageFilePath = "";
    mutable Image* m_image = nullptr;
    FloatRange m_alignX = FloatRange::ZEROTOONE;
    FloatRange m_alignY = FloatRange::ZEROTOONE;
    IntRange m_numRotations = IntRange::ZERO;


    void Startup() const;
    void Shutdown() const;

    void RunOnce( Map& map ) const;

    bool IsAlignmentValid( Map& theMap, const AABB2& imageBounds ) const;
    IntVec2 GetImageOffset( Map& theMap ) const;

    static const TileDef* GetTileDef( const Rgba& texelColor );
};
