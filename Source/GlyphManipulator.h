/*
    После рендеринга глифа библиотекой FreeType структура FT_BitmapGlyph
    конвертируется в данную структуру.
*/

#include <ft2build.h>
#include FT_GLYPH_H

struct GlyphManipulator
{
    // Размеры изображения.
    int width_;
    int height_;

    // Grayscale пиксели изображения, сконвертированные в диапазон 0.0 - 1.0.
    float* pixels_;

    // Конструктор копирует данные из FT_BitmapGlyph. FT_BitmapGlyph - это указатель.
    GlyphManipulator(const FT_BitmapGlyph bitmapGlyph);

    // Переопределяем конструктор копирования, чтобы он клонировал массив пикселей.
    GlyphManipulator(const GlyphManipulator& original);

    ~GlyphManipulator();

    float GetPixel(int x, int y) const { return pixels_[y * width_ + x]; }
    void SetPixel(int x, int y, float gray);

    // Некоторые эффекты требуют увеличения размера битмапа.
    // Битмап расширяется в каждую сторону на halfDist, то есть ширина и высота увеличатся на halfDist * 2.
    // Старое изображение располагается в центре нового.
    void Extend(int halfDist);

    // Размазывает каждый пиксель на radius в каждую сторону (без учета исходного пикселя).
    // Размеры итогового изображения увеличатся на radius * 2 по вертикали и по горизонтали.
    void Blur(int radius);

    // Проверяет, что координаты не выходят за границы изображения.
    bool IsInside(int x, int y);
};
