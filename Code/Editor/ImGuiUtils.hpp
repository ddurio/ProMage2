#pragma once
#if defined(_EDITOR)
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
bool RenderFilePath( std::string& currentPath, const Strings& filter );
bool RenderMotifVariable( const std::string& uniqueKey, const std::string& varName, std::string& currentValue, const Strings& motifHierarchy, const std::string& label = "" );
bool RenderCheckbox( bool& currentValue, const std::string& label = "", bool defaultValue = false );

bool RenderIntOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, int& currentValue, const std::string& label = "", int defaultValue = 1.f );
bool RenderPercentOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, float& currentValue, const std::string& label = "", float defaultValue = 1.f );
bool RenderIntRangeOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, IntRange& currentValue, const std::string& label = "", int minValue = 0, int maxValue = 10, const IntRange& defaultValue = IntRange::ONE );
bool RenderFloatRangeOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, FloatRange& currentValue, const std::string& label = "", float minValue = 0.f, float maxValue = 10.f, const FloatRange& defaultValue = FloatRange::ONE );
bool RenderTileDropDownOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, std::string& currentType, const std::string& label = "Tile Type", bool addNoneOption = true, const std::string& defaultValue = "" );
bool RenderDropDownOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, std::string& currentType, const Strings& ddOptions, const std::string& label, bool addNoneOptions, const std::string& defaultValue );
std::array< bool, 2> RenderTagsOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, Strings& currentTags, bool missingHasChanged, const std::string& label = "" );
bool RenderHeatMapsOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, HeatMaps& currentHeatMaps );
bool RenderFilePathOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& motifHierarchy, std::string& currentPath, const Strings& filter, const std::string& label = "" );
bool RenderCheckboxOrVar( const std::string& uniqueKey, NamedStrings& stepVars, const std::string& attrName, const Strings& morifHierarchy, bool& currentValue, const std::string& label = "", bool defaultValue = true );

bool RenderRightClick( std::string& motifVarName, const std::string& itemID );

void RenderChangeText( bool hasChanged );
void SetImGuiTextColor( bool isDefaultValue );
void SetImGuiTextColor( const Rgba& newColor );


// Return value is if the button was pressed
bool RenderImageButton( const char* textureStr, const ImVec2& buttonSize, const Rgba& bgColor = Rgba::CLEAR_BLACK );

#endif
