/*
    Данная подсистема рендерит шрифт в набор атласов.
*/

#pragma once
#include <Urho3D/Urho3DAll.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

struct GlyphManipulator;

// Стиль спрайтового шрифта.
enum BF_Style
{
    BFS_SIMPLE = 0,
    BFS_CONTOUR,
    BFS_OUTLINED,
    BFS_SDF,
};

class BFGenerator : public Object
{
    URHO3D_OBJECT(BFGenerator, Object);

public:
    BFGenerator(Context* context);
    ~BFGenerator();

    // Доступ к созданным атласам, чтобы их можно было показать пользователю.
    Image* GetResult(int page);
    unsigned GetNumPages() const { return pages_.Size(); }

    void Generate(const char* fontFileName, int fontHeight, BF_Style style,
        const Color& mainColor, const Color& strokeColor, int value1, int value2,
        int atlasWidth, int atlasHeight);

    void Save(const String& fileName);

private:
    // Результат работы генератора. В процессе генерации
    // последняя страница является текущим атласом, куда добавляются новые глифы.
    Vector<SharedPtr<Image> > pages_;
    SharedPtr<XMLFile> xmlFile_;

    // Библиотека FreeType.
    FT_Library library_;
    
    // Текущий шрифт.
    FT_Face face_;

    // Индекс текущего глифа.
    // Библиотека FreeType оперирует индексами из таблицы символов, а не кодами символов.
    FT_UInt glyphIndex_;

    // Высота шрифта в пикселях.
    int fontHeight_;

    // Ищет свободные места в атласе.
    AreaAllocator areaAllocator_;

    // Размеры атласа.
    int atlasWidth_;
    int atlasHeight_;

    BF_Style style_;

    // Запоминаем значения, которые позже будут сохранены в XML-файл. Названия соответствуют названиям в файле.
    // ------------------------------------------------------------------------------НАЧАЛО---
    //  Код текущего символа в кодировке UTF-32.
    unsigned id_;
    //
    // Положение изображения текущего глифа в атласе.
    int x_;
    int y_;
    //
    // Размеры изображения текущего глифа в аталасе.
    int width_;
    int height_;
    //
    // Индекс атласа, в который упакован текущий глиф.
    int page_;
    //
    // Смещения изображения текущего глифа от левого верхнего угла
    // вправо вниз при выводе на экран.
    int xOffset_;
    int yOffset_;
    //
    // Расстояние между origin текущего глифа и origin следующего глифа.
    int xAdvance_;
    //
    // Высота строки.
    int lineHeight_;
    //
    // Общее количество глифов.
    int count_;
    // -------------------------------------------------------------------------------КОНЕЦ---

    // Окраска текущего символа. Значения интерпретируются в зависимости от стиля.
    // Могут вообще не использоваться.
    Color mainColor_;
    Color strokeColor_;

    // Значения интерпретируются в зависимости от стиля шрифта.
    // Могут вообще не использоваться.
    int value1_;
    int value2_;

    // SDF шрифт рендерится в увеличенном масштабе для повышения точности, а затем уменьшается.
    int scale_ = 1;

    // Обычный рендеринг глифа.
    void RenderGlyph_Simpe();

    // Рендерит только контур глифа. Увеличивает размер изображения на половину толщины контура в каждую сторону.
    void RenderGlyph_Contour();

    // Глиф с обводкой вокруг него. Увеличивает размер изображения на толщину обводки в каждую сторону.
    void RenderGlyph_Outlined();

    void RenderGlyph_Sdf();

    // Конвертируют в четырехканальный Image.
    SharedPtr<Image> ConvertToImage(const GlyphManipulator& glyphManipulator, const Color& color);

    // Устанавливает настроки рендеринга, общие для CalcField() и CombineFields().
    // Возвращает подготовленный рендертаргет.
    SharedPtr<Texture2D> PrepareGraphics(String psDefines, int targetWidth, int targetHeight);

    // Обрабатывает текстуру шейдером SdfCalculator.
    SharedPtr<Texture2D> CalcField(Texture2D* inputTexture, int radius, const String& psDefines);

    // Комбинирует две карты расстояний (расстояния от точек фона до глифа и расстояния от точек глифа до фона).
    SharedPtr<Image> CombineFields(Texture2D* externalField, Texture2D* internalField, int targetWidth, int targetHeight);

    // Добавляет подготовленный глиф в финальный атлас. Создает новые страницы по мере надобности.
    void PackGlyph(Image* image);

    // Создает новое изображение-атлас.
    void BeginNewPage();

    // Вставляет одно изображение в другое.
    void Blit(Image* dest, int x, int y, int width, int height, Image* source, int sourceX, int sourceY);

    // Определяет метрики шрифта. Устанавливает не все метрики,
    // а только lineHeight_, xOffset_, yOffset_ и xAdvance_.
    void CalculateMetrics();

    // Добавляет метрики текущего глифа в xmlFile_.
    void StoreGlyph();

    // Добавляет кернинг текущего шрифта в xmlFile_.
    void StoreKerning();
};
