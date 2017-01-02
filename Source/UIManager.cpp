#include "UIManager.h"
#include "Aliases.h"

UIManager::UIManager(Context* context) : Object(context)
{
}

void UIManager::CreateUI()
{
    XMLFile* style = GET_XML_FILE("UI/DefaultStyle.xml");
    UI_ROOT->SetDefaultStyle(style);

    background_ = UI_ROOT->CreateChild<BorderImage>();
    background_->SetPriority(-10);
    background_->SetTexture(GET_TEXTURE_2D("Textures/Chess.png"));
    background_->SetTiled(true);

    int x = 10;
    int y = 10;
    pickFontButton_ = CreateButton(UI_ROOT, "Pick Font…", x, y, 190, 30,
        URHO3D_HANDLER(UIManager, HandleButtonReleased_PickFont));

    y += 40;
    fontHeightLineEdit_ = CreateLineEdit("60", x, y);
    CreateText(UI_ROOT, "Font Height (Pixels)", true, x + 70, y + 2);

    y += 40;
    styleDDList_ = CreateDropDownList(UI_ROOT, x, y, 190, 30);
    CreateDropDownListItem(styleDDList_, "Simple", BFS_SIMPLE);
    CreateDropDownListItem(styleDDList_, "Contour", BFS_CONTOUR);
    CreateDropDownListItem(styleDDList_, "Outlined", BFS_OUTLINED);
    CreateDropDownListItem(styleDDList_, "SDF", BFS_SDF);
    styleDDList_->SetSelection(3);
    SubscribeToEvent(styleDDList_, E_ITEMSELECTED, URHO3D_HANDLER(UIManager, HandleItemSelected_StylesDDList));

    y += 40;
    mainColorRLineEdit_ = CreateLineEdit("1.0", x, y);
    mainColorGLineEdit_ = CreateLineEdit("1.0", x + 70, y);
    mainColorBLineEdit_ = CreateLineEdit("1.0", x + 70 * 2, y);
    mainColorALineEdit_ = CreateLineEdit("1.0", x + 70 * 3, y);
    mainColorLabel_ = CreateText(UI_ROOT, "", true, x + 70 * 4, y + 2);

    y += 40;
    strokeColorRLineEdit_ = CreateLineEdit("0.0", x, y);
    strokeColorGLineEdit_ = CreateLineEdit("0.0", x + 70, y);
    strokeColorBLineEdit_ = CreateLineEdit("0.0", x + 70 * 2, y);
    strokeColorALineEdit_ = CreateLineEdit("1.0", x + 70 * 3, y);
    strokeColorLabel_ = CreateText(UI_ROOT, "", true, x + 70 * 4, y + 2);

    y += 40;
    value1LineEdit_ = CreateLineEdit("10", x, y);
    value1Label_ = CreateText(UI_ROOT, "", true, x + 70, y + 2);

    y += 40;
    value2LineEdit_ = CreateLineEdit("8", x, y);
    value2Label_ = CreateText(UI_ROOT, "", true, x + 70, y + 2);

    y += 40;
    atlasWidthDDList_ = CreateDropDownList(UI_ROOT, x, y, 70, 30);
    CreateDropDownListItem(atlasWidthDDList_, "128");
    CreateDropDownListItem(atlasWidthDDList_, "256");
    CreateDropDownListItem(atlasWidthDDList_, "512");
    CreateDropDownListItem(atlasWidthDDList_, "1024");
    CreateDropDownListItem(atlasWidthDDList_, "2048");
    atlasWidthDDList_->SetSelection(3);

    atlasHeightDDList_ = CreateDropDownList(UI_ROOT, x + 80, y, 70, 30);
    CreateDropDownListItem(atlasHeightDDList_, "128");
    CreateDropDownListItem(atlasHeightDDList_, "256");
    CreateDropDownListItem(atlasHeightDDList_, "512");
    CreateDropDownListItem(atlasHeightDDList_, "1024");
    CreateDropDownListItem(atlasHeightDDList_, "2048");
    atlasHeightDDList_->SetSelection(3);

    CreateText(UI_ROOT, "Texture Size", true, x + 160, y + 2);

    resultViewer_ = UI_ROOT->CreateChild<Sprite>();
    resultViewer_->SetPriority(-5);
    resultViewer_->SetBlendMode(BLEND_ALPHA);

    y += 40;
    generateButton_ = CreateButton(UI_ROOT, "Generate!", x, y, 190, 30,
        URHO3D_HANDLER(UIManager, HandleButtonReleased_Generate));

    y += 40;
    saveButton_ = CreateButton(UI_ROOT, "Save", x, y, 190, 30,
        URHO3D_HANDLER(UIManager, HandleButtonReleased_Save));

    numPagesText_ = CreateText(UI_ROOT, "", true);

    prevPageButton_ = CreateButton(UI_ROOT, "<", 0, 0, 40, 30,
        URHO3D_HANDLER(UIManager, HandleButtonReleased_PrevPage));

    nextPageButton_ = CreateButton(UI_ROOT, ">", 0, 0, 40, 30,
        URHO3D_HANDLER(UIManager, HandleButtonReleased_NextPage));

    UpdateElements();

    pickFontInitialPath_ = fileSystem->GetProgramDir() + "Data/Fonts";
    saveInitialPath_ = fileSystem->GetProgramDir();

    SubscribeToEvent(UI_ROOT, E_RESIZED, URHO3D_HANDLER(UIManager, HandleRootElementResized));
}

DropDownList* UIManager::CreateDropDownList(UIElement* parent, int x, int y, int width, int height)
{
    DropDownList* result = UI_ROOT->CreateChild<DropDownList>();
    result->SetStyleAuto();
    result->SetResizePopup(true);
    result->SetSize(width, height);
    result->SetPosition(x, y);
    return result;
}

Text* UIManager::CreateDropDownListItem(DropDownList* list, const String& text)
{
    SharedPtr<Text> item(new Text(context_));
    item->SetFont("Fonts/Anonymous Pro.ttf", 18);
    item->SetText(text);
    item->SetSelectionColor(Color(0.2f, 0.225f, 0.35f, 1.0f));
    item->SetHoverColor(Color(0.3f, 0.4f, 0.7f, 1.0f));
    list->AddItem(item);
    return item;
}

Text* UIManager::CreateDropDownListItem(DropDownList* list, const String& text, BF_Style bitmapFontstyle)
{
    Text* item = CreateDropDownListItem(list, text);
    item->SetVar("Style", bitmapFontstyle);
    return item;
}

LineEdit* UIManager::CreateLineEdit(const String& text, int x, int y)
{
    LineEdit* result = UI_ROOT->CreateChild<LineEdit>();
    result->SetStyleAuto();
    result->GetTextElement()->SetFont("Fonts/Anonymous Pro.ttf", 18);
    result->SetSize(60, 30);
    result->SetPosition(x, y);
    result->SetText(text);
    return result;
}

Text* UIManager::CreateText(UIElement* parent, const String& text, bool outlined/* = false*/, int x/* = 0*/, int y/* = 0*/)
{
    Text* result = parent->CreateChild<Text>();
    result->SetFont("Fonts/Anonymous Pro.ttf", 18);
    result->SetText(text);
    result->SetPosition(x, y);
    
    if (outlined)
    {
        result->SetTextEffect(TE_STROKE);
        result->SetEffectStrokeThickness(2);
        result->SetEffectRoundStroke(true);
    }

    return result;
}

Button* UIManager::CreateButton(UIElement* parent, const String& title, int x, int y, int width, int height,
    EventHandler* handler/* = nullptr*/)
{
    Button* result = parent->CreateChild<Button>();
    result->SetStyleAuto();
    result->SetPosition(x, y);
    result->SetSize(width, height);

    Text* text = CreateText(result, title);
    text->SetAlignment(HA_CENTER, VA_CENTER);

    if (handler)
        SubscribeToEvent(result, E_RELEASED, handler);

    return result;
}

void UIManager::UpdateElements()
{
    background_->SetSize(UI_ROOT->GetSize());

    int numPages = bfGenerator->GetNumPages();

    if (numPages == 0)
    {
        resultViewer_->SetVisible(false);
    }
    else
    {
        // Создаем текстуру из изображения.
        SharedPtr<Texture2D> texture(new Texture2D(context_));
        texture->SetMipsToSkip(QUALITY_LOW, 0); // Нет снижения качества.
        texture->SetNumLevels(1); // Нет мипмаппинга.
        texture->SetAddressMode(COORD_U, ADDRESS_BORDER);
        texture->SetAddressMode(COORD_V, ADDRESS_BORDER);
        texture->SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
        texture->SetData(bfGenerator->GetResult(currentPage_));

        // И показываем ее.
        resultViewer_->SetTexture(texture);
        resultViewer_->SetSize(texture->GetWidth(), texture->GetHeight());
        resultViewer_->SetFullImageRect();
        resultViewer_->SetPosition((float)(UI_ROOT->GetWidth() - texture->GetWidth()), 0.0f);
        resultViewer_->SetVisible(true);
    }

    nextPageButton_->SetPosition(UI_ROOT->GetWidth() - nextPageButton_->GetWidth() - 10,
        UI_ROOT->GetHeight() - nextPageButton_->GetHeight() - 10);

    prevPageButton_->SetPosition(nextPageButton_->GetPosition().x_ - prevPageButton_->GetWidth() - 10,
        nextPageButton_->GetPosition().y_);

    numPagesText_->SetText("Num Pages: " + String(numPages));
    numPagesText_->SetPosition(prevPageButton_->GetPosition().x_ - numPagesText_->GetWidth() - 10,
        UI_ROOT->GetHeight() - 40);

    BF_Style bitmapFontStyle = (BF_Style)styleDDList_->GetSelectedItem()->GetVar("Style").GetInt();
    
    if (bitmapFontStyle == BFS_SIMPLE)
    {
        mainColorLabel_->SetText("Color");
        strokeColorLabel_->SetText("Not Used");
        value1Label_->SetText("Not Used");
        value2Label_->SetText("Blur Radius");
    }
    else if (bitmapFontStyle == BFS_CONTOUR)
    {
        mainColorLabel_->SetText("Not Used");
        strokeColorLabel_->SetText("Contour Color");
        value1Label_->SetText("Contour Thickness");
        value2Label_->SetText("Blur Radius");
    }
    else if (bitmapFontStyle == BFS_OUTLINED)
    {
        mainColorLabel_->SetText("Main Color");
        strokeColorLabel_->SetText("Outline Color");
        value1Label_->SetText("Outline Thickness");
        value2Label_->SetText("Outline Blur");
    }
    else if (bitmapFontStyle == BFS_SDF)
    {
        mainColorLabel_->SetText("Not Used");
        strokeColorLabel_->SetText("Not Used");
        value1Label_->SetText("Max Field Distance");
        value2Label_->SetText("Font Scale");
    }
}

void UIManager::HandleRootElementResized(StringHash eventType, VariantMap& eventData)
{
    UpdateElements();
}

void UIManager::HandleItemSelected_StylesDDList(StringHash eventType, VariantMap& eventData)
{
    UpdateElements();
}

void UIManager::HandleButtonReleased_PrevPage(StringHash eventType, VariantMap& eventData)
{
    if (bfGenerator->GetNumPages() == 0)
        return;

    currentPage_ = Clamp<int>(currentPage_ - 1, 0, bfGenerator->GetNumPages() - 1);
    UpdateElements();
}

void UIManager::HandleButtonReleased_NextPage(StringHash eventType, VariantMap& eventData)
{
    if (bfGenerator->GetNumPages() == 0)
        return;

    currentPage_ = Clamp<int>(currentPage_ + 1, 0, bfGenerator->GetNumPages() - 1);
    UpdateElements();
}

void UIManager::ShowFileSelector(const String& title, const String& okText, const String& cancelText,
    const String& filter, const String& initialPath)
{
    fileSelector_ = new FileSelector(context_);
    XMLFile* style = GET_XML_FILE("UI/DefaultStyle.xml");
    fileSelector_->SetDefaultStyle(style);
    fileSelector_->SetTitle(title);
    fileSelector_->SetButtonTexts(okText, cancelText);
    fileSelector_->SetPath(initialPath);

    StringVector strVector;
    strVector.Push(filter);
    fileSelector_->SetFilters(strVector, 0);

    // Центрируем диалог.
    IntVector2 size = fileSelector_->GetWindow()->GetSize();
    int x = (graphics->GetWidth() - size.x_) / 2;
    int y = (graphics->GetHeight() - size.y_) / 2;
    fileSelector_->GetWindow()->SetPosition(x, y);
}

void UIManager::HandleButtonReleased_PickFont(StringHash eventType, VariantMap& eventData)
{
    ShowFileSelector("Pick Font", "Open", "Cancel", "*.ttf", pickFontInitialPath_);
    SubscribeToEvent(fileSelector_, E_FILESELECTED, URHO3D_HANDLER(UIManager, HandleFileSelected_PickFont));
}

void UIManager::HandleButtonReleased_Save(StringHash eventType, VariantMap& eventData)
{
    // Результат еще не сгенерирован.
    if (resultExtension_.Empty())
        return;

    ShowFileSelector("Save Result", "Save", "Cancel", String("*.") + resultExtension_, saveInitialPath_);
    SubscribeToEvent(fileSelector_, E_FILESELECTED, URHO3D_HANDLER(UIManager, HandleFileSelected_Save));
}

void UIManager::HandleFileSelected_PickFont(StringHash eventType, VariantMap& eventData)
{
    using namespace FileSelected;
    String fileName = eventData[P_FILENAME].GetString();
    bool ok = eventData[P_OK].GetBool();

    // Нажата кнопка Ok (а не Cancel).
    if (ok)
    {
        sourceFontFileName_ = fileName;
        pickFontInitialPath_ = GetPath(fileName);
        Text* text = static_cast<Text*>(pickFontButton_->GetChild(0));
        text->SetText(GetFileName(fileName));
    }

    delete fileSelector_;
    fileSelector_ = nullptr;
}

void UIManager::HandleFileSelected_Save(StringHash eventType, VariantMap& eventData)
{
    using namespace FileSelected;
    String fileName = eventData[P_FILENAME].GetString();
    String filter = eventData[P_FILTER].GetString();
    bool ok = eventData[P_OK].GetBool();

    if (ok)
    {
        // Дописываем расширение к имени файла, если отсутствует.
        if (GetExtension(fileName).Empty())
            fileName += filter.Substring(1);

        bfGenerator->Save(fileName);
        saveInitialPath_ = GetPath(fileName);
    }

    delete fileSelector_;
    fileSelector_ = nullptr;
}

void UIManager::HandleButtonReleased_Generate(StringHash eventType, VariantMap& eventData)
{
    // Исходный шрифт еще не выбран.
    if (sourceFontFileName_.Empty())
        return;

    BF_Style bitmapFontStyle = (BF_Style)styleDDList_->GetSelectedItem()->GetVar("Style").GetInt();
    int fontHeight = ToInt(fontHeightLineEdit_->GetText());
    
    int atlasWidth = ToInt(static_cast<Text*>(atlasWidthDDList_->GetSelectedItem())->GetText());
    int atlasHeight = ToInt(static_cast<Text*>(atlasHeightDDList_->GetSelectedItem())->GetText());
    
    float mainColorR = ToFloat(mainColorRLineEdit_->GetText());
    float mainColorG = ToFloat(mainColorGLineEdit_->GetText());
    float mainColorB = ToFloat(mainColorBLineEdit_->GetText());
    float mainColorA = ToFloat(mainColorALineEdit_->GetText());
    Color mainColor = Color(mainColorR, mainColorG, mainColorB, mainColorA);
    
    float strokeColorR = ToFloat(strokeColorRLineEdit_->GetText());
    float strokeColorG = ToFloat(strokeColorGLineEdit_->GetText());
    float strokeColorB = ToFloat(strokeColorBLineEdit_->GetText());
    float strokeColorA = ToFloat(strokeColorALineEdit_->GetText());
    Color strokeColor = Color(strokeColorR, strokeColorG, strokeColorB, strokeColorA);
    
    int value1 = ToInt(value1LineEdit_->GetText());
    int value2 = ToInt(value2LineEdit_->GetText());

    bfGenerator->Generate(sourceFontFileName_.CString(), fontHeight, bitmapFontStyle, mainColor,
        strokeColor, value1, value2, atlasWidth, atlasHeight);

    if (bitmapFontStyle == BFS_SDF)
        resultExtension_ = "sdf";
    else
        resultExtension_ = "fnt";

    currentPage_ = 0;
    UpdateElements();
}
