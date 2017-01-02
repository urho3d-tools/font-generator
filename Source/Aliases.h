/*
    Простой доступ к подсистемам.
*/

#pragma once
#include <Urho3D/Urho3DAll.h>
#include "BFGenerator.h"
#include "UIManager.h"

// Стандартные подсистемы движка.
extern Audio* audio;
extern ResourceCache* cache;
extern Engine* engine;
extern FileSystem* fileSystem;
extern Graphics* graphics;
extern Input* input;
extern Localization* localization;
extern Renderer* renderer;
extern UI* ui;

extern DebugHud* debugHud;

// Собственные подсистемы.
extern BFGenerator* bfGenerator;
extern UIManager* uiManager;

// Инициализирует указатели на подсистемы.
void InitAliases(Context* context);

#define UI_ROOT ui->GetRoot()

#define GET_FONT cache->GetResource<Font>
#define GET_IMAGE cache->GetResource<Image>
#define GET_JSON_FILE cache->GetResource<JSONFile>
#define GET_MATERIAL cache->GetResource<Material>
#define GET_MODEL cache->GetResource<Model>
#define GET_PARTICLE_EFFECT cache->GetResource<ParticleEffect>
#define GET_PARTICLE_EFFECT_2D cache->GetResource<ParticleEffect2D>
#define GET_SOUND cache->GetResource<Sound>
#define GET_TEXTURE_2D cache->GetResource<Texture2D>
#define GET_XML_FILE cache->GetResource<XMLFile>

