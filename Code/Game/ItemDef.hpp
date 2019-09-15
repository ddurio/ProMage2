#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Definition.hpp"
#include "Engine/Core/Tags.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/SpriteDef.hpp"


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


class ItemDef : public Definition< Item, ItemDef > {
    friend class Definition< Item, ItemDef >;

    public:
    void DefineObject( Item& theObject ) const override;

    ItemSlot GetItemSlot() const;
    std::vector< Tags > GetItemSets() const;

    std::string GetSpriteSheet() const;
    const SpriteDef GetPortrait() const;
    float GetSpritePPU() const;

    std::string GetAttackAnim() const;
    float GetAttackRange() const;
    float GetAttackDamage() const;
    float GetAttackConeWidth() const;
    float GetAttackConeDot() const;

    bool IsConsumable() const;
    std::string GetConsumptionItemSet() const;

    IntRange GetDropFloors() const;
    std::string GetDropBias() const;

    static std::vector< const ItemDef* > GetAllDefinitions();

    private:
    static float s_slotBaseDefense[NUM_ITEM_SLOTS - 1];

    ItemSlot m_slot = ITEM_SLOT_NONE;
    std::vector< Tags > m_itemSets;

    int m_baseValue           = 100;
    float m_qualityOverride   = 1.f;
    float m_proficiency       = -1.f;

    std::string m_spriteSheet = "";
    SpriteDef m_portrait      = SpriteDef( Vec2::ZERO, Vec2::ONE, "" );
    float m_spritePPU         = 40.f;

    std::string m_attackAnim  = ANIM_PAPER_DOLL_IDLE;
    float m_attackRange       = -1.f;
    float m_attackDamage      = 0.f;
    float m_attackConeWidth   = 0.f;
    float m_attackConeDot     = 1.f;

    bool m_isConsumable       = false;
    std::string m_onConsumeItemSet = "";

    IntRange m_dropFloors     = IntRange::NEGONE;
    std::string m_dropBias    = "early";


    ItemDef( const XMLElement& element );
};
