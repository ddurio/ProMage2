#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Definition.hpp"


class Actor;


template<>
Definition<Actor>::Definition( const XMLElement& element );

template<>
void Definition<Actor>::Define( Actor& theObject ) const;
