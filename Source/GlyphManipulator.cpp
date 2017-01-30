#include "GlyphManipulator.h"
#include <Urho3D/Urho3DAll.h>

GlyphManipulator::GlyphManipulator(const FT_BitmapGlyph bitmapGlyph)
{
    width_ = bitmapGlyph->bitmap.width;
    height_ = bitmapGlyph->bitmap.rows;
    pixels_ = new float[width_ * height_];
    
    if (bitmapGlyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        // В монохромном изображении один пиксель занимает 1 бит (не байт).
        // pitch - это число байт, занимаемых одной линией изображения.
        for (int y = 0; y < height_; ++y)
        {
            unsigned char* src = bitmapGlyph->bitmap.buffer + bitmapGlyph->bitmap.pitch * y;
            float* rowDest = pixels_ + y * width_;

            for (int x = 0; x < width_; ++x)
            {
                // 1) В одном байте умещается 8 пикселей. x >> 3 эквивалентно делению
                //    на 8 (0b1000 превращается в 0b1). Так мы получаем байт, в котором находится пиксель.
                // 2) x & 7 - это остаток от деления на 8. Допустим x = 12 = 0b1100.
                //    0b1100 & 0b0111 = 0b100 = 4. Так мы получаем номер бита внутри байта.
                // 3) 0x80 == 0b10000000. Единица внутри этого числа сдвигается на номер бита
                //    и побитовой операцией определяется значение бита.
                rowDest[x] = (src[x >> 3] & (0x80 >> (x & 7))) ? 255.0f : 0.0f;
            }
        }
    }
    else
    {
        // В grayscale изображении каждый пиксель занимает один байт,
        // а значит pitch эквивалентен width.
        for (int i = 0; i < width_ * height_; ++i)
            pixels_[i] = (float)bitmapGlyph->bitmap.buffer[i] / 255.0f;
    }
}

GlyphManipulator::GlyphManipulator(const GlyphManipulator& original)
{
    width_ = original.width_;
    height_ = original.height_;
    pixels_ = new float[width_ * height_];
    memcpy(pixels_, original.pixels_, width_ * height_ * sizeof(float));
}

GlyphManipulator::~GlyphManipulator()
{
    delete[] pixels_;
}

inline void GlyphManipulator::SetPixel(int x, int y, float gray)
{
    pixels_[y * width_ + x] = gray;
}

void GlyphManipulator::Extend(int halfDist)
{
    halfDist = abs(halfDist);
    int newWidth = width_ + halfDist * 2;
    int newHeight = height_ + halfDist * 2;
    float* newPixels = new float[newWidth * newHeight];
    memset(newPixels, 0, newWidth * newHeight * sizeof(float));
    
    for (int x = 0; x < width_; ++x)
    {
        int newX = x + halfDist;

        for (int y = 0; y < height_; ++y)
        {
            int newY = y + halfDist;
            newPixels[newY * newWidth + newX] = pixels_[y * width_ + x];
        }
    }

    delete[] pixels_;
    pixels_ = newPixels;
    width_ = newWidth;
    height_ = newHeight;
}

inline bool GlyphManipulator::IsInside(int x, int y)
{
    if (x < 0 || y < 0)
        return false;

    if (x >= width_ || y >= height_)
        return false;

    return true;
}

// Упрощает обращение к пикселям в любом буфере, который по размеру
// равен width_ * height_ (то есть такого же размера, что и pixels_).
#define PIXEL(buffer, x, y) buffer[width_ * (y) + (x)]

// Пробовал все три алгоритма с http://www.gamedev.ru/code/articles/blur .
// Обычное усреднение (первый метод) выглядит уродливо, изображение как будто двоится.
// Гауссово размытие (третий метод) выглядит красиво, но интенсивность пикселей очень
// быстро и нелинейно затухает с расстоянием. Бо'льшая часть радиуса размытия почти
// полностью прозрачна и не видна. Приходится нереально накручивать радиус размытия,
// чтобы хоть немного увеличить видимую площадь эффекта. А это бьет по производительности.
// Поэтому используется треугольный закон распределения (второй метод).
void GlyphManipulator::Blur(int radius)
{
    if (radius == 0)
        return;

    radius = Abs(radius);
    Extend(radius);
    float* tmpBuffer = new float[width_ * height_];

    // Пиксель на каждом проходе размывается в две стороны, и получается отрезок
    // длиной radius + 1 + radius.
    // У крайних пикселей вес самый маленький и равен 1.
    // У центрального пикселя вес самый большой и равен radius + 1.
    // Вес соседних пикселей отличается на 1.
    // Сумму весов всех пикселей в этом отрезке можно определить сразу.
    int totalWeight = (radius + 1) * (radius + 1);

    // Размываем по вертикали и сохраняем результат в tmpBuffer.
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            // Сразу записываем вклад центрального пикселя.
            // Его вес равен radius + 1.
            float value = GetPixel(x, y) * (radius + 1);
            int dist = 1;

            while (dist <= radius)
            {
                int weight = 1 + radius - dist;

                // Пиксель вне изображения черный, ноль можно не плюсовать.
                // Так что тут все корректно.
                if (IsInside(x, y + dist))
                    value += GetPixel(x, y + dist) * weight;

                if (IsInside(x, y - dist))
                    value += GetPixel(x, y - dist) * weight;

                dist++;
            }

            // Сумму нужно поделить на общий вес, иначе изменится яркость изображения.
            PIXEL(tmpBuffer, x, y) = value / totalWeight;
        }
    }

    // Размываем по горизонтали и сохраняем результат назад в структуру.
    for (int x = 0; x < width_; ++x)
    {
        for (int y = 0; y < height_; ++y)
        {
            float value = PIXEL(tmpBuffer, x, y) * (radius + 1);
            int dist = 1;

            while (dist <= radius)
            {
                int weight = 1 + radius - dist;

                if (IsInside(x + dist, y))
                    value += PIXEL(tmpBuffer, x + dist, y) * weight;

                if (IsInside(x - dist, y))
                    value += PIXEL(tmpBuffer, x - dist, y) * weight;

                dist++;
            }

            SetPixel(x, y, value / totalWeight);
        }
    }

    delete[] tmpBuffer;
}
