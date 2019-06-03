#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"


enum ItemSlot : int;


ItemSlot ParseXMLAttribute( const XMLElement& element, const char* attributeName, ItemSlot defaultValue );
