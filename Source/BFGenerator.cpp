/*
    Family - гарнитура - семейство шрифтов. Например "Courier".
    Face - начертание - конкретный шрифт. Например "Courier Bold".
    Glyph - глиф - изображение определенного символа из шрифта.
    Кернинг - поправка расстояния между парой конкретных символов.

    Размер шрифта задается в пунктах (points). Один пункт = 1/72 дюйма.
    Стандартное разрешение Windows 96x96 dpi (пикселей на дюйм),
    значит 12pt = 12 * 96 / 72 = 16 пикселей. Это размер воображаемой рамки
    вокруг символа. Как символ расположен внутри рамки, зависит от шрифта.
    
    Сначала я использовал для указания размера шрифта пункты (FT_Set_Char_Size),
    но потом переделал в пиксели (FT_Set_Pixel_Sizes). Поэтому абзац выше не важен :).

    В библиотеке координаты и размеры задаются не вещественными числами, а целыми
    с единицей измерения 1/64. Поэтому размеры нужно умножать на 64.
    Операция сдвига ">> 6" равносильна делению на 64 (0b1000000 превращается в 1).
    Формат называется 26.6 (26 бит на целую часть, 6 на дробную).
    Еще есть формат 16.16 (16 бит на целую часть, 16 на дробную). Чтобы округлить
    такое число до нормального целого, его нужно разделить на 65536.

    См. также: https://www.freetype.org/freetype2/docs/documentation.html
*/

#include "BFGenerator.h"
#include FT_TRUETYPE_TABLES_H
#include "Aliases.h"
#include "GlyphManipulator.h"

BFGenerator::BFGenerator(Context* context) : Object(context)
{
    FT_Init_FreeType(&library_);
}

BFGenerator::~BFGenerator()
{
    FT_Done_FreeType(library_);
}

Image* BFGenerator::GetResult(int page)
{
    return pages_[page];
}

void BFGenerator::BeginNewPage()
{
    areaAllocator_.Reset(atlasWidth_, atlasHeight_, 0, 0, false);
    SharedPtr<Image> newPage(new Image(context_));
    newPage->SetSize(atlasWidth_, atlasHeight_, 4);

    // Страница очищается внешним (а не черным) прозрачным цветом.
    // Это уменьшает артефакты при размытии, которое происходит при FILTER_TRILINEAR.
    // Разница заметна между символами ╬╬╬.
    Color clearColor = strokeColor_;
    if (style_ != BFS_CONTOUR && style_ != BFS_OUTLINED)
        clearColor == mainColor_;

    clearColor.a_ = 0.0f;
    newPage->Clear(clearColor);

    pages_.Push(newPage);
}

// Проверка на выход за границы не производится.
void BFGenerator::Blit(Image* dest, int x, int y, int width, int height, Image* source, int sourceX, int sourceY)
{
    // Число байт на один пиксель. Используется только такой формат.
    const unsigned components = 4;

    unsigned char* destData = dest->GetData() + (y * dest->GetWidth() + x) * components;
    unsigned char* sourceData = source->GetData() + (sourceY * source->GetWidth() + sourceX) * components;

    for (int i = 0; i < height; ++i)
    {
        memcpy(destData, sourceData, (size_t)(width * components));
        destData += dest->GetWidth() * components;
        sourceData += source->GetWidth() * components;
    }
}

void BFGenerator::PackGlyph(Image* image)
{
    // Ищем свободное место на атласе.
    int x, y;
    if (!areaAllocator_.Allocate(image->GetWidth() + 1, image->GetHeight() + 1, x, y))
    {
        // Место на картинке кончилось, создаем новую.
        BeginNewPage();

        // Повторная попытка.
        if (!areaAllocator_.Allocate(image->GetWidth() + 1, image->GetHeight() + 1, x, y))
            return;
    }

    // Вставляем глиф в изображение-атлас.
    Image* currentPage = pages_.Back();
    Blit(currentPage, x, y, image->GetWidth(), image->GetHeight(), image, 0, 0);

    // Дополняем информацию о текущем глифе.
    x_ = x;
    y_ = y;
    width_ = image->GetWidth();
    height_ = image->GetHeight();
    page_ = pages_.Size() - 1;
}

void BFGenerator::RenderGlyph_Simpe()
{
    int blurDistance = Abs(value2_);

    // Загружаем глиф.
    FT_Load_Glyph(face_, glyphIndex_, FT_LOAD_DEFAULT);
    
    // Метрики можно извлечь и без рендеринга.
    CalculateMetrics();

    // Реднерим глиф.
    FT_Glyph glyph;
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    GlyphManipulator glyphManipulator(bitmapGlyph);
    FT_Done_Glyph(glyph);

    // Размываем, если нужно.
    if (blurDistance > 0)
    {
        glyphManipulator.Blur(blurDistance);

        // Так как размытые глифы предназначены для создания теней, то их центры должны
        // совпадать с центрами неразмытых глифов.
        xOffset_ -= blurDistance;
        yOffset_ -= blurDistance;
    }

    SharedPtr<Image> resultImage = ConvertToImage(glyphManipulator, mainColor_);
    PackGlyph(resultImage);
}

void BFGenerator::RenderGlyph_Contour()
{
    int strokeThickness = Abs(value1_);
    int blurDistance = Abs(value2_);

    FT_Load_Glyph(face_, glyphIndex_, FT_LOAD_DEFAULT);
    CalculateMetrics();

    // Рендерим контур.
    FT_Glyph glyph;
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Stroker stroker;
    FT_Stroker_New(library_, &stroker);
    FT_Stroker_Set(stroker, strokeThickness * 64 / 2, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    FT_Glyph_Stroke(&glyph, stroker, true);
    FT_Stroker_Done(stroker);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    GlyphManipulator glyphManipulator(bitmapGlyph);
    FT_Done_Glyph(glyph);

    // Глиф стал больше примерно на половину толщины контура в каждую сторону.
    // Необходимо вручную модифицировать метрики.
    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    xAdvance_ += strokeThickness;
    lineHeight_ += strokeThickness;

    if (blurDistance > 0)
    {
        glyphManipulator.Blur(blurDistance);
        xOffset_ -= blurDistance;
        yOffset_ -= blurDistance;
    }

    SharedPtr<Image> resultImage = ConvertToImage(glyphManipulator, strokeColor_);
    PackGlyph(resultImage);
}

void BFGenerator::RenderGlyph_Outlined()
{
    int strokeThickness = Abs(value1_);
    int blurDistance = Abs(value2_);

    FT_Load_Glyph(face_, glyphIndex_, FT_LOAD_DEFAULT);
    CalculateMetrics();
    FT_Glyph glyph;
    FT_BitmapGlyph bitmapGlyph;

    // Реднерим внутренний глиф обычным способом.
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    
    // Используем манипулятор просто как место временного хранения.
    GlyphManipulator normalGlyph(bitmapGlyph);

    int normalGlyphLeft = bitmapGlyph->left;
    int normalGlyphTop = bitmapGlyph->top;
    FT_Done_Glyph(glyph);

    // Рендерим раздутый глиф.
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Stroker stroker;
    FT_Stroker_New(library_, &stroker);
    FT_Stroker_Set(stroker, strokeThickness * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    FT_Stroker_Done(stroker);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    GlyphManipulator inflatedGlyph(bitmapGlyph);
    int inflatedGlyphLeft = bitmapGlyph->left;
    int inflatedGlyphTop = bitmapGlyph->top;
    FT_Done_Glyph(glyph);

    // Смещение нормального изображения относительно раздутого.
    // Оно не всегда равно толщине обводки. Поэтому вычисляем так.
    int deltaX = normalGlyphLeft - inflatedGlyphLeft;
    int deltaY = inflatedGlyphTop - normalGlyphTop;

    if (blurDistance > 0)
    {
        inflatedGlyph.Blur(blurDistance);
        deltaX += blurDistance;
        deltaY += blurDistance;
        xOffset_ -= blurDistance;
        yOffset_ -= blurDistance;
    }
    
    SharedPtr<Image> resultImage = ConvertToImage(inflatedGlyph, strokeColor_);

    // Специальный случай - цвета внутри и снаружи совпадают. При этом не выводим внутренний глиф.
    // При размытии обводки внутренний глиф будет видно, даже если их цвет одинаковый.
    // Это на случай, если будет нужна размытая тень для глифа с обводкой.
    if (mainColor_ != strokeColor_)
    {
        // Накладываем нормальный глиф на раздутый.
        // Это не альфа-блендинг. Тут пиксели нормального (внтуренннего) глифа перезаписывают
        // пиксели раздутого глифа. Но учитывается альфа крайних полупрозрачных пикселей.
        for (int x = 0; x < normalGlyph.width_; ++x)
        {
            for (int y = 0; y < normalGlyph.height_; ++y)
            {
                Color backColor = resultImage->GetPixel(x + deltaX, y + deltaY);
                float mask = normalGlyph.GetPixel(x, y);
                float r = mask * mainColor_.r_ + backColor.r_ * (1.0f - mask);
                float g = mask * mainColor_.g_ + backColor.g_ * (1.0f - mask);
                float b = mask * mainColor_.b_ + backColor.b_ * (1.0f - mask);
                float a = mask * mainColor_.a_ + backColor.a_ * (1.0f - mask);
                resultImage->SetPixel(x + deltaX, y + deltaY, Color(r, g, b, a));
            }
        }
    }

    PackGlyph(resultImage);

    // Необходимо вручную скорректировать метрики.
    lineHeight_ += strokeThickness * 2;
    xAdvance_ += strokeThickness * 2;
}

// Наложение полупрозрачного изображения на другое полупрозрачное изображение
// (полная версия альфаблендинга без упрощения, что фон считается непрозрачным).
// На входе ожидаются цвета с straight альфой (не premultiplied).
// Рендеринг глифов с тенью я убрал, но функцию оставил. Вдруг когда-то пригодится.
static inline Color MixColors(const Color& front, const Color& back)
{
    float a = front.a_ + back.a_ * (1.0f - front.a_);

    // Абсолютно прозрачное изображение накладывается на другое абсолютно прозрачное.
    if (a < M_EPSILON)
        return Color(0.0f, 0.0f, 0.0f, 0.0f);

    float invA = 1.0f / a;

    float r = (front.r_ * front.a_ + back.r_ * back.a_ * (1.0f - front.a_)) * invA;
    float g = (front.g_ * front.a_ + back.g_ * back.a_ * (1.0f - front.a_)) * invA;
    float b = (front.b_ * front.a_ + back.b_ * back.a_ * (1.0f - front.a_)) * invA;

    return Color(r, g, b, a);
}

SharedPtr<Texture2D> BFGenerator::PrepareGraphics(String psDefines, int targetWidth, int targetHeight)
{
    graphics->SetLineAntiAlias(false);
    graphics->SetClipPlane(false);
    graphics->SetScissorTest(false);
    graphics->SetStencilTest(false);
    graphics->SetDepthWrite(false);
    graphics->SetDepthTest(CMP_ALWAYS);
    graphics->SetCullMode(CULL_NONE);
    graphics->SetFillMode(FILL_SOLID);
    graphics->SetBlendMode(BLEND_REPLACE);
    graphics->ResetRenderTargets();
    graphics->ClearTransformSources();

    // Чтобы избежать всяких багов, рендертаргет надо создавать прежде всего.
    // В частности, SetSize() затирает уже установленную текстуру 0, так как в OpenGL
    // нельзя произвести манипуляции с текстурой, предварительно не забиндив ее.
    SharedPtr<Texture2D> renderTarget(new Texture2D(context_));
    // Использование 4х каналов избыточно, так как информация хранится только в альфе.
    // Также можно использовать формат Float32 для повышения точности. Но вроде и так норм.
    renderTarget->SetSize(targetWidth, targetHeight, graphics->GetRGBAFormat(), TEXTURE_RENDERTARGET);
    // Фильтрация FILTER_TRILINEAR искажает карту расстояния при уменьшении размера изображения.
    // Я не заметил какой-то разницы между FILTER_NEAREST и FILTER_BILINEAR.
    // Нужно больше тестов, чтобы выбрать что лучше.
    renderTarget->SetFilterMode(FILTER_BILINEAR);
    
    // Кажется это не нужно, но пусть будет.
    renderTarget->SetMipsToSkip(QUALITY_LOW, 0); // Нет снижения качества.
    renderTarget->SetNumLevels(1); // Нет мипмаппинга.

    graphics->SetRenderTarget(0, renderTarget);
    graphics->SetViewport(IntRect(0, 0, targetWidth, targetHeight));
    
    // Можно и не очищать, так как каждый пиксель вьюпорта перезаписывается в шейдере.
    // Но в тестовых целях оставил заливку красным цветом.
    graphics->Clear(CLEAR_COLOR, Color(1.0f, 0.0f, 0.0f, 1.0f));

    // SetShaderParameter() нужно использовать только после установки шейдера.
    ShaderVariation* vs = graphics->GetShader(VS, "SdfCalculator");
    ShaderVariation* ps = graphics->GetShader(PS, "SdfCalculator", psDefines);
    graphics->SetShaders(vs, ps);

    Matrix3x4 modelMatrix = Matrix3x4::IDENTITY;

#ifdef URHO3D_OPENGL
    modelMatrix.m23_ = 0.0f;
#else
    model.m23_ = 0.5f;
#endif

    // Прямоугольник с текстурой накроет весь рендертаргет.
    graphics->SetShaderParameter(VSP_MODEL, modelMatrix);
    graphics->SetShaderParameter(VSP_VIEWPROJ, Matrix4::IDENTITY);

    return renderTarget;
}

SharedPtr<Texture2D> BFGenerator::CalcField(Texture2D* inputTexture, int radius, const String& psDefines)
{
    int textureWidth = inputTexture->GetWidth();
    int textureHeight = inputTexture->GetHeight();

    // Размеры входной и выходной текстур совпадают.
    SharedPtr<Texture2D> result = PrepareGraphics(psDefines, textureWidth, textureHeight);
    graphics->SetShaderParameter("TextureInvSize", Vector2(1.0f / textureWidth, 1.0f / textureHeight));
    graphics->SetShaderParameter("Radius", radius);
    graphics->SetTexture(0, inputTexture);
    
    // Рендерим прямоугольник с наложенной входной текстурой и установленным шейдером.
    Geometry* geometry = renderer->GetQuadGeometry();
    geometry->Draw(graphics);

    return result;
}

SharedPtr<Image> BFGenerator::CombineFields(Texture2D* externalField, Texture2D* internalField, int targetWidth, int targetHeight)
{
    // Обычно целевой размер меньше размера входных текстур. Так производится сжатие итогового изображения.
    SharedPtr<Texture2D> resultTexture = PrepareGraphics("COMBINE", targetWidth, targetHeight);
    graphics->SetTexture(TU_DIFFUSE, externalField);
    graphics->SetTexture(TU_NORMAL, internalField);

    Geometry* geometry = renderer->GetQuadGeometry();
    geometry->Draw(graphics);

    return resultTexture->GetImage();
}

void BFGenerator::RenderGlyph_Sdf()
{
    int radius = Abs(value1_);

    FT_Load_Glyph(face_, glyphIndex_, FT_LOAD_DEFAULT);
    CalculateMetrics();

    // Учитываем, что глиф рендерится в увеличенном масштабе.
    lineHeight_ /= scale_;
    xAdvance_ /= scale_;
    xOffset_ /= scale_;
    yOffset_ /= scale_;

    // Реднерим монохромный глиф (он рендерится в увеличенном масштабе).
    FT_Glyph glyph;
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, nullptr, true);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    GlyphManipulator glyphManipulator(bitmapGlyph);
    FT_Done_Glyph(glyph);

    // У глифа может не быть изображения, но он может занимать какую-то площадь
    // при выводе текста (например пробел), поэтому он будет сохранен в XML-файл.
    if (glyphManipulator.width_ == 0 || glyphManipulator.height_ == 0)
    {
        x_ = 0;
        y_ = 0;
        width_ = 0;
        height_ = 0;
        page_ = 0;
        return;
    }

    // Так как работа ведется с масштабированной версией глифа, то и радиус надо масштабировать.
    glyphManipulator.Extend(radius * scale_);

    SharedPtr<Image> srcGlyph = ConvertToImage(glyphManipulator, Color::WHITE);
    SharedPtr<Texture2D> srcTexture(new Texture2D(context_));
    srcTexture->SetData(srcGlyph, true);
    srcTexture->SetFilterMode(FILTER_NEAREST);

    // Смотрите пояснения в шейдере SdfCalculator.
    SharedPtr<Texture2D> horizontalDist = CalcField(srcTexture, radius * scale_, "HORIZONTAL");
    SharedPtr<Texture2D> externalField = CalcField(horizontalDist, radius * scale_, "VERTICAL");
    horizontalDist = CalcField(srcTexture, radius * scale_, "HORIZONTAL INVERT");
    SharedPtr<Texture2D> internalField = CalcField(horizontalDist, radius * scale_, "VERTICAL");
    SharedPtr<Image> resultImage = CombineFields(externalField, internalField,
        srcGlyph->GetWidth() / scale_, srcGlyph->GetHeight() / scale_);

    PackGlyph(resultImage);

    // Граница расширяется на 1 лишний пиксель в каждую сторону, иначе некоторые глифы (о, а, ...)
    // некоторых кривых шрифтов (Anonimous Pro) будут обрезаны при отображении в Урхо.
    x_ = x_ + radius - 1;
    y_ = y_ + radius - 1;
    width_ = width_ - radius * 2 + 2;
    height_ = height_ - radius * 2 + 2;
}

SharedPtr<Image> BFGenerator::ConvertToImage(const GlyphManipulator& glyphManipulator, const Color& color)
{
    SharedPtr<Image> result(new Image(context_));
    result->SetSize(glyphManipulator.width_, glyphManipulator.height_, 4);
    
    for (int y = 0; y < glyphManipulator.height_; ++y)
    {
        for (int x = 0; x < glyphManipulator.width_; ++x)
        {
            float mask = glyphManipulator.GetPixel(x, y);
            result->SetPixel(x, y, Color(color.r_, color.g_, color.b_, mask * color.a_));
        }
    }

    return result;
}

void BFGenerator::Generate(const char* fontFileName, int fontHeight, BF_Style style,
    const Color& mainColor, const Color& strokeColor, int value1, int value2,
    int atlasWidth, int atlasHeight)
{
    fontHeight_ = Abs(fontHeight);
    style_ = style;
    mainColor_ = mainColor;
    strokeColor_ = strokeColor;
    value1_ = Abs(value1);
    value2_ = Abs(value2);
    atlasWidth_ = Abs(atlasWidth);
    atlasHeight_ = Abs(atlasHeight);
    count_ = 0;
    pages_.Clear();
    BeginNewPage();

    FT_New_Face(library_, fontFileName, 0, &face_);

    scale_ = 1;

    // SDF шрифт рендерится в увеличенном масштабе для повышения точности, а потом сжимается.
    if (style_ == BFS_SDF)
    {
        scale_ = Abs(value2_);

        if (scale_ < 1)
            scale_ = 1;
    }

    FT_Set_Pixel_Sizes(face_, 0, fontHeight_ * scale_);

    xmlFile_ = new XMLFile(context_);
    XMLElement rootElem = xmlFile_->CreateRoot("font");
    XMLElement infoElem = rootElem.CreateChild("info");
    infoElem.SetAttribute("face", String(face_->family_name) + " " + String(face_->style_name));

    // Вообще-то здесь принято записывать размер в пунктах, но ладно.
    // Это просто информация и значение нигде не используется.
    infoElem.SetAttribute("size", String(fontHeight));
    
    FT_ULong charCode = FT_Get_First_Char(face_, &glyphIndex_);
   
    while (glyphIndex_ != 0)
    {
        id_ = charCode;

        if (style == BFS_SIMPLE)
            RenderGlyph_Simpe();
        else if (style == BFS_CONTOUR)
            RenderGlyph_Contour();
        else if (style == BFS_OUTLINED)
            RenderGlyph_Outlined();
        else if (style == BFS_SDF)
            RenderGlyph_Sdf();

        count_++;
        StoreGlyph();
        charCode = FT_Get_Next_Char(face_, charCode, &glyphIndex_);
    }

    StoreKerning();

    FT_Done_Face(face_);

    XMLElement charsElem = rootElem.GetOrCreateChild("chars");
    charsElem.SetAttribute("count", String(count_));

    XMLElement commonElem = rootElem.GetOrCreateChild("common");
    commonElem.SetAttribute("lineHeight", String(lineHeight_));
    commonElem.SetAttribute("pages", String(pages_.Size()));
}

void BFGenerator::Save(const String& fileName)
{
    if (!xmlFile_ || pages_.Size() == 0 || count_ == 0)
        return;

    String path, file, extension;
    SplitPath(fileName, path, file, extension);

    // При повторном сохранении имена страниц могут отличаться,
    // если пользователь будет сохранять шрифт под другим именем.
    XMLElement rootElem = xmlFile_->GetOrCreateRoot("font");
    rootElem.RemoveChildren("pages");

    XMLElement pagesElem = rootElem.CreateChild("pages");

    for (unsigned i = 0; i < pages_.Size(); ++i)
    {
        pages_[i]->SavePNG(path + file + "_" + String(i) + ".png");
    
        XMLElement pageElem = pagesElem.CreateChild("page");
        pageElem.SetAttribute("id", String(i));
        pageElem.SetAttribute("file", file + "_" + String(i) + ".png");
    }

    xmlFile_->SaveFile(fileName);
}

void BFGenerator::StoreGlyph()
{
    XMLElement element = xmlFile_->GetOrCreateRoot("font").GetOrCreateChild("chars").CreateChild("char");
    element.SetAttribute("id", String(id_));
    element.SetAttribute("x", String(x_));
    element.SetAttribute("y", String(y_));
    element.SetAttribute("width", String(width_));
    element.SetAttribute("height", String(height_));
    element.SetAttribute("xoffset", String(xOffset_));
    element.SetAttribute("yoffset", String(yOffset_));
    element.SetAttribute("xadvance", String(xAdvance_));
    element.SetAttribute("page", String(page_));
}

inline int RoundToPixels(FT_Pos value)
{
    // В библиотеке FreeType пиксель равен 64 условным единицам.
    // value >> 6 - это деление нацело на 64 (0b1000000 становится 1).
    // value & 0x3f - остаток от деления на 64 (0b1000111 & 0b0111111 равно 0b000111)
    // Значит, если остаток от деления >= полпикселя, то добавляем пиксель.
    return (int)(value >> 6) + (((value & 0x3f) >= 32) ? 1 : 0);
}

// Код повторяет UI/FontFaceFreeType.cpp, чтобы полностью соответствовать движку.
void BFGenerator::CalculateMetrics()
{
    // Расстояние от базовой линии до самой верхней точки шрифта.
    int ascender = RoundToPixels(face_->size->metrics.ascender);
    // Расстояние от базовой линии до самой нижней точки шрифта.
    int descender = RoundToPixels(face_->size->metrics.descender);

    // Какие-то особенности TrueType OS/2 шрифтов.
    TT_OS2* os2Info = (TT_OS2*)FT_Get_Sfnt_Table(face_, ft_sfnt_os2);
    if (os2Info)
    {
        ascender = Max(ascender, os2Info->usWinAscent * face_->size->metrics.y_ppem / face_->units_per_EM);
        ascender = Max(ascender, os2Info->sTypoAscender * face_->size->metrics.y_ppem / face_->units_per_EM);
        descender = Max(descender, os2Info->usWinDescent * face_->size->metrics.y_ppem / face_->units_per_EM);
        descender = Max(descender, os2Info->sTypoDescender * face_->size->metrics.y_ppem / face_->units_per_EM);
    }

    // Высота строки повторно вычисляется при обработке каждого символа, но не страшно.
    lineHeight_ = Max(ascender + descender, RoundToPixels(face_->size->metrics.height));

    xOffset_ = RoundToPixels(face_->glyph->metrics.horiBearingX);
    yOffset_ = ascender - RoundToPixels(face_->glyph->metrics.horiBearingY);
    xAdvance_ = face_->glyph->metrics.horiAdvance >> 6;

    // x_, y_, width_ и height_ устанавливаются позже в BFGenerator::PackGlyph().
}

// Код написан на основе функции FontFaceFreeType::Load() движка.
// Кернинг виден между символами AV (они должны быть расположены близко).
void BFGenerator::StoreKerning()
{
    if (!FT_HAS_KERNING(face_))
        return;

    // Получаем размер памяти, занимаемой таблицами (их может быть несколько).
    FT_ULong tagKern = FT_MAKE_TAG('k', 'e', 'r', 'n');
    FT_ULong memLength = 0;
    FT_Error error = FT_Load_Sfnt_Table(face_, tagKern, 0, 0, &memLength);

    if (error)
        return;

    // Загружаем таблицы.
    SharedArrayPtr<unsigned char> kernData(new unsigned char[memLength]);
    error = FT_Load_Sfnt_Table(face_, tagKern, 0, kernData, &memLength);

    if (error)
        return;

    // Преобразуем big endian в little endian.
    for (unsigned i = 0; i < memLength; i += 2)
        Swap(kernData[i], kernData[i + 1]);

    // Читаем версию.
    MemoryBuffer deserializer(kernData, (unsigned)memLength);
    unsigned short version = deserializer.ReadUShort();

    if (version != 0)
        return;

    // Невозможно получить код символа по индексу (нет однозначного соответствия),
    // поэтому нужно создать таблицу самостоятельно.
    int numGlyphs = face_->num_glyphs;
    PODVector<unsigned> charCodes(numGlyphs);
    
    for (int i = 0; i < count_; ++i)
        charCodes[i] = 0;

    FT_UInt glyphIndex;
    FT_ULong charCode = FT_Get_First_Char(face_, &glyphIndex);
    
    while (glyphIndex != 0)
    {
        charCodes[glyphIndex] = (unsigned)charCode;
        charCode = FT_Get_Next_Char(face_, charCode, &glyphIndex);
    }

    // Число таблиц.
    unsigned numKerningTables = deserializer.ReadUShort();

    XMLElement kerningsElement = xmlFile_->GetOrCreateRoot("font").CreateChild("kernings");
    
    // Общее число пар.
    int numPairs = 0;

    for (unsigned i = 0; i < numKerningTables; ++i)
    {
        unsigned short version = deserializer.ReadUShort();
        unsigned short length = deserializer.ReadUShort();
        unsigned short coverage = deserializer.ReadUShort();

        // Такой тип таблицы не поддерживается, пропускаем её.
        if (version != 0 || coverage != 1)
            continue;

        unsigned numKerningPairs = deserializer.ReadUShort();
        
        // Пропуск searchRange, entrySelector и rangeShift.
        deserializer.Seek((unsigned)(deserializer.GetPosition() + 3 * sizeof(unsigned short)));

        for (unsigned j = 0; j < numKerningPairs; ++j)
        {
            unsigned leftIndex = deserializer.ReadUShort();
            unsigned leftCharCode = charCodes[leftIndex];
            unsigned rightIndex = deserializer.ReadUShort();
            unsigned rightCharCode = charCodes[rightIndex];

            //short amount = RoundToPixels(deserializer.ReadShort());
            // Я не знаю, почему это работает в движке для ttf шрифтов.
            // Может быть это связано с тем, что я задаю размер шрифта в пикселях, а не в пунктах.
            // Поэтому определяю иначе.
            // На данном этапе мы узнали, что между символами есть кернинг
            // (просто перебирать все пары символов - это дикий брутфорс).
            // А теперь используем стандартную функцию.
            FT_Vector akerning;
            FT_Get_Kerning(face_, leftIndex, rightIndex, FT_KERNING_DEFAULT, &akerning);
            short amount = RoundToPixels(akerning.x);

            // Не забываем учитывать увеличенный масштаб для SDF-шрифта.
            amount /= scale_;

            // Кернинг так мал, что после округления стал нулевым. Не сохраняем.
            if (amount == 0)
                continue;

            XMLElement childElement = kerningsElement.CreateChild("kerning");
            childElement.SetAttribute("first", String(leftCharCode));
            childElement.SetAttribute("second", String(rightCharCode));
            childElement.SetAttribute("amount", String(amount));

            ++numPairs;
        }
    }

    kerningsElement.SetAttribute("count", String(numPairs));
}
