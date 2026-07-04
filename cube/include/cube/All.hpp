#pragma once

// Umbrella header — all cube:: classes ported from classes/cube__*.h

#include "cube/AIBehaviors.hpp"
#include "cube/Behavior.hpp"
#include "cube/Camera.hpp"
#include "cube/ActionConfig.hpp"
#include "cube/DecompiledConstants.hpp"
#include "cube/DecompiledLayouts.hpp"
#include "cube/ItemRegistry.hpp"
#include "cube/Controller.hpp"
#include "cube/Creature.hpp"
#include "cube/CubeShader.hpp"
#include "cube/Database.hpp"
#include "cube/Field.hpp"
#include "cube/GameController.hpp"
#include "cube/LookAtPlayerBehavior.hpp"
#include "cube/QuestText.hpp"
#include "cube/SpawnLocationBehavior.hpp"
#include "cube/Sprite.hpp"
#include "cube/Terrain.hpp"
#include "cube/Types.hpp"
#include "cube/WalkPathBehavior.hpp"
#include "cube/Widgets.hpp"
#include "cube/World.hpp"
#include "cube/WorldInfo.hpp"
#include "cube/WorldMap.hpp"
#include "cube/Zone.hpp"
#include "cube/ZoneTile.hpp"
#include "cube/Region.hpp"
#include "cube/XAudio2Engine.hpp"

// Grouped in Terrain.hpp: Chunk, ChunkBuffer, Dungeon, House, LandscapeTile
// Dedicated headers: Zone, ZoneTile, Region
// Grouped in Widgets.hpp: AdaptionWidget, BlueprintPreviewWidget, CharacterPreviewWidget,
// CharacterStyleWidget, CharacterWidget, ChatWidget, EnchantWidget, InventoryWidget,
// MapOverlayWidget, ObjectiveWidget, OptionsWidget, PreviewWidget, SkillWidget, SpeechWidget,
// SpriteWidget, StartMenuWidget, StatisticsWidget, SystemWidget, VoxelWidget, WorldPreviewWidget
// Grouped in AIBehaviors.hpp: RandomWalkBehavior, RandomInteractionBehavior, CompanionBehavior,
// CombatBehavior
// Grouped in QuestText.hpp: QuestTextNode, Speech
// Grouped in XAudio2Engine.hpp: audio::Sound, audio::Music
