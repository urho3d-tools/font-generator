/*
    Эта подсистема управляет интерфейсом.
*/

#pragma once
#include <Urho3D/Urho3DAll.h>
#include "BFGenerator.h"

class UIManager : public Object
{
    URHO3D_OBJECT(UIManager, Object);

public:
    UIManager(Context* context);
    void CreateUI();

private:
    // Фоновая шахматная текстура.
    BorderImage* background_;
    
    // Кнопка выбора шрифта.
    Button* pickFontButton_;
    
    // Поле для ввода высоты шрифта.
    LineEdit* fontHeightLineEdit_;
    
    // Список стилей шрифта.
    DropDownList* styleDDList_;

    // Поля для ввода основного цвета и поясняющая надпись.
    LineEdit* mainColorRLineEdit_;
    LineEdit* mainColorGLineEdit_;
    LineEdit* mainColorBLineEdit_;
    LineEdit* mainColorALineEdit_;
    Text* mainColorLabel_;
    
    // Цвет обводки и поясняющая надпись.
    LineEdit* strokeColorRLineEdit_;
    LineEdit* strokeColorGLineEdit_;
    LineEdit* strokeColorBLineEdit_;
    LineEdit* strokeColorALineEdit_;
    Text* strokeColorLabel_;

    // Числовое значение интерпретируется в зависимости от стиля шрифта.
    LineEdit* value1LineEdit_;
    Text* value1Label_;

    // Второе числовое значение.
    LineEdit* value2LineEdit_;
    Text* value2Label_;

    // Ширина и высота результирующего атласа.
    DropDownList* atlasWidthDDList_;
    DropDownList* atlasHeightDDList_;
    
    // Кнопка для запуска процесса.
    Button* generateButton_;
    
    // Кнопка для сохранения результата.
    Button* saveButton_;

    // Показывает результат (атлас) пользователю.
    Sprite* resultViewer_;

    // Показывает общее число страниц (атласов).
    Text* numPagesText_;

    // Кнопки для переключения между страницами результата.
    Button* prevPageButton_;
    Button* nextPageButton_;

    // Страница, которая отображается в данный момент.
    int currentPage_ = 0;

    // Обновляет положение и содержимое элементов.
    void UpdateElements();

    // Создает текстовое поле ввода.
    LineEdit* CreateLineEdit(const String& text, int x, int y);

    // Создает надпись (на кнопках в том числе).
    Text* CreateText(UIElement* parent, const String& text, bool outlined = false, int x = 0, int y = 0);

    // Создает выпадающий список.
    DropDownList* CreateDropDownList(UIElement* parent, int x, int y, int width, int height);

    // Создает отдельный элемент для выпадающего списка (размер атласа).
    Text* CreateDropDownListItem(DropDownList* list, const String& text);

    // Создает элемент списка (стиль шрифта) и добавляем к нему переменную "Style".
    Text* CreateDropDownListItem(DropDownList* list, const String& text, BF_Style bitmapFontstyle);
    
    // Создает кнопку.
    Button* CreateButton(UIElement* parent, const String& title, int x, int y, int width, int height,
        EventHandler* handler = nullptr);

    void HandleRootElementResized(StringHash eventType, VariantMap& eventData);
    void HandleButtonReleased_PickFont(StringHash eventType, VariantMap& eventData);
    void HandleItemSelected_StylesDDList(StringHash eventType, VariantMap& eventData);
    void HandleButtonReleased_Generate(StringHash eventType, VariantMap& eventData);
    void HandleButtonReleased_Save(StringHash eventType, VariantMap& eventData);
    void HandleButtonReleased_PrevPage(StringHash eventType, VariantMap& eventData);
    void HandleButtonReleased_NextPage(StringHash eventType, VariantMap& eventData);

    // Выбранный исходный шрифт. Если это значение пустое, значит шрифт еще не выбран.
    String sourceFontFileName_;

    // Указатель на окно выбора файла (при сохранении результата или
    // для выбора исходного шрифта). Не нашел способа прятать модальные окна, поэтому
    // приходится каждый раз пересоздавать это окно и уничтожать его при закрытии.
    FileSelector* fileSelector_ = nullptr;

    // Расширение для сохраняемого результата. Для SDF-шрифта это значение равно "sdf",
    // а спрайтового - "fnt". Если эта строка пустая, значит результат
    // еще не сгенерирован и окно сохранения результата не будет открыто.
    String resultExtension_;

    // Стартовые пути для окон выбора файлов.
    String pickFontInitialPath_;
    String saveInitialPath_;

    // Создает окно выбора файла (используется и при выборе шрифта и при сохранении).
    void ShowFileSelector(const String& title, const String& okText, const String& cancelText,
        const String& filter, const String& initialPath);

    // Событие E_FILESELECTED возникает при нажатии любой кнопки в окне выбора файла.
    void HandleFileSelected_PickFont(StringHash eventType, VariantMap& eventData);
    void HandleFileSelected_Save(StringHash eventType, VariantMap& eventData);
};
