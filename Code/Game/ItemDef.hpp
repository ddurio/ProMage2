#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/Definition.hpp"


enum ItemSlot {
    ITEM_SLOT_NONE = -1,
    ITEM_SLOT_HELM,
    ITEM_SLOT_CHEST,
    ITEM_SLOT_SHOULDER,
    ITEM_SLOT_LEGS,
    ITEM_SLOT_FEET,
    ITEM_SLOT_WEAPON,

    NUM_ITEM_SLOTS
};


class Item;


template<>
Definition<Item>::Definition( const XMLElement& element );

template<>
void Definition<Item>::Define( Item& theObject ) const;
