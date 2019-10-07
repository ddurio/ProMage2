#pragma once
#include "Editor/EditorCommon.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"


struct FloatRange;
struct IntRange;


void RenderPercent( float& value, const std::string& label = "" );
void RenderIntRange( IntRange& range, const std::string& label = "", int minValue = 0, int maxValue = 10 );
void RenderFloatRange( FloatRange& range, const std::string& label = "", float minValue = 0.f, float maxValue = 10.f );
void RenderTileDropDown( const std::string& uniqueKey, std::string& currentType, const std::string& label = "Tile Type", bool addNoneOption = true );
void RenderDropDown( const std::string& uniqueKey, std::string& currentType, const Strings& ddOptions, const std::string& label, bool addNoneOptions );
void RenderTags( const std::string& uniqueKey, Strings& currentTags, const std::string& label = "" );
void RenderHeatMaps( const std::string& uniqueKey, std::map< std::string, FloatRange, StringCmpCaseI >& currentHeatMaps );
