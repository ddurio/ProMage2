#pragma once
#include "Editor/EditorCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


struct FloatRange;
struct IntRange;


void RenderPercent( float& value, const std::string& label = "", float defaultValue = 1.f );
void RenderIntRange( IntRange& range, const std::string& label = "", int minValue = 0, int maxValue = 10, const IntRange& defaultValue = IntRange::ONE );
void RenderFloatRange( FloatRange& range, const std::string& label = "", float minValue = 0.f, float maxValue = 10.f, const FloatRange& defaultValue = FloatRange::ONE );
void RenderTileDropDown( const std::string& uniqueKey, std::string& currentType, const std::string& label = "Tile Type", bool addNoneOption = true, const std::string& defaultValue = "" );
void RenderDropDown( const std::string& uniqueKey, std::string& currentType, const Strings& ddOptions, const std::string& label, bool addNoneOptions, const std::string& defaultValue );
void RenderTags( const std::string& uniqueKey, Strings& currentTags, const std::string& label = "" );
void RenderHeatMaps( const std::string& uniqueKey, std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps );

void SetImGuiTextColor( bool isDefaultValue );
