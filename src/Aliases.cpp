#include "Aliases.h"

Audio* audio = nullptr;
ResourceCache* cache = nullptr;
Engine* engine = nullptr;
FileSystem* fileSystem = nullptr;
Graphics* graphics = nullptr;
Input* input = nullptr;
Localization* localization = nullptr;
Renderer* renderer = nullptr;
UI* ui = nullptr;

DebugHud* debugHud = nullptr;

BFGenerator* bfGenerator = nullptr;
UIManager* uiManager = nullptr;

void InitAliases(Context* context)
{
    audio = context->GetSubsystem<Audio>();
    cache = context->GetSubsystem<ResourceCache>();
    engine = context->GetSubsystem<Engine>();
    fileSystem = context->GetSubsystem<FileSystem>();
    graphics = context->GetSubsystem<Graphics>();
    input = context->GetSubsystem<Input>();
    localization = context->GetSubsystem<Localization>();
    renderer = context->GetSubsystem<Renderer>();
    ui = context->GetSubsystem<UI>();

    debugHud = engine->CreateDebugHud();
    debugHud->SetDefaultStyle(GET_XML_FILE("UI/DefaultStyle.xml"));

    context->RegisterSubsystem(bfGenerator = new BFGenerator(context));
    context->RegisterSubsystem(uiManager = new UIManager(context));
}
