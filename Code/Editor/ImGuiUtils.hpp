#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/Rgba.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"

#include "array"


struct FloatRange;
struct ImVec2;
struct IntRange;


// Return value is if the value has been changed
bool RenderPercent( float& value, const std::string& label = "", float defaultValue = 1.f );
bool RenderIntRange( IntRange& currentValue, const std::string& label = "", int minValue = 0, int maxValue = 10, const IntRange& defaultValue = IntRange::ONE );
bool RenderFloatRange( FloatRange& currentValue, const std::string& label = "", float minValue = 0.f, float maxValue = 10.f, const FloatRange& defaultValue = FloatRange::ONE );
bool RenderTileDropDown( const std::string& uniqueKey, std::string& currentType, const std::string& label = "Tile Type", bool addNoneOption = true, const std::string& defaultValue = "" );
bool RenderDropDown( const std::string& uniqueKey, std::string& currentType, const Strings& ddOptions, const std::string& label, bool addNoneOptions, const std::string& defaultValue );
std::array< bool, 2> RenderTags( const std::string& uniqueKey, Strings& currentTags, bool missingHasChanged, const std::string& label = "" );
bool RenderHeatMaps( const std::string& uniqueKey, HeatMaps& currentHeatMaps );
bool RenderMotifVariable( const std::string& uniqueKey, const std::string& varName, std::string& currentValue, const Strings& motifHierarchy, const std::string& label = "" );

bool RenderPercentOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, float& value, const std::string& label = "", float defaultValue = 1.f );

bool RenderRightClick( std::string& motifVarName, const std::string& itemID );

void RenderChangeText( bool hasChanged );
void SetImGuiTextColor( bool isDefaultValue );
void SetImGuiTextColor( const Rgba& newColor );


// Return value is if the button was pressed
bool RenderImageButton( const char* textureStr, const ImVec2& buttonSize, const Rgba& bgColor = Rgba::CLEAR_BLACK );
