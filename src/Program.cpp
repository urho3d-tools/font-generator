/*
    Точка входа.
*/

#include "Aliases.h"

class Program : public Application
{
    URHO3D_OBJECT(Program, Application);

    Program(Context* context) : Application(context)
    {
    }

    void Setup()
    {
        engineParameters_[EP_RESOURCE_PATHS] = "AppData;Data;CoreData";
        engineParameters_[EP_WINDOW_TITLE] = "Urho3D Bitmap Font Generator";
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_RESIZABLE] = true;
        engineParameters_[EP_VSYNC] = true;
    }

    void Start()
    {
        InitAliases(context_);
        input->SetMouseVisible(true);
        uiManager->CreateUI();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Program)
