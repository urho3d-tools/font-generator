// Используется метод https://habrahabr.ru/post/245729/ .

// Цвет текстуры всегда белый, а значения хранятся только в альфе
// (то есть r, g и b всегда равны 1.0).

// Проход первый. Определен дефайн HORIZONTAL.
// На вход подается глиф - монохромное изображение, хранящееся в альфе.
// Альфа == 1.0 - это глиф, а альфа == 0.0 - фон.
// Результат: в альфе выходной текстуры хранятся расстояния
// по горизонтали до глифа.

// Проход второй. Определен дефайн VERTICAL.
// На вход подается текстура с предыдущего шага.
// Результат: в альфе выходной текстуры хранятся расстояния до глифа.

// Проход третий. Определены дефайны HORIZONTAL INVERT.
// На вход подается глиф как на первом проходе, но он инвертируется.
// Результат: в альфе выходной текстуры хранятся расстояния
// по горизонтали до фона.

// Проход четвертый. Определен дефайн VERTICAL.
// На вход подается текстура с предыдущего шага.
// Результат: в альфе выходной текстуры хранятся расстояния до фона.

// Проход пятый. Определен дефайн COMBINE.
// На вход подаются текстуры с шагов 2 и 4 и комбинируются в одну текстуру.

#line 28

#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

// Эти два значения определены также для прохода COMBINE, где они не нужны,
// нужно будет исправить.
uniform int cRadius;
// Инвертированный размер входной текстуры используется для обращения к конкретным пикселям.
uniform vec2 cTextureInvSize;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

#if defined(COMPILEPS) && defined(HORIZONTAL)

    bool IsGlyph(vec2 texCoord)
    {
        float alpha = texture2D(sDiffMap, texCoord).a;
        
        #ifdef INVERT
            alpha = 1.0 - alpha;
        #endif
        
        return (alpha > 0.0);
    }

#endif


#if defined(COMPILEPS) && defined(VERTICAL)

    // Вычисляет гипотенузу треугольника по катетам.
    float CalcC(float a, float b)
    {
        return sqrt(a * a + b * b);
    }
    
    // Извлекает значение из текстуры, созданной на предыдущем шаге (из карты расстояний по горизонтали).
    // offset - это смещение от текущего пикселя по вертикали.
    float GetHDist(int offset)
    {
        vec2 coord = vTexCoord + vec2(0.0, offset) * cTextureInvSize;
        return texture2D(sDiffMap, coord).a;
    }

#endif


void PS()
{

// Вычисляем расстояние до глифа по горизонтали.
#ifdef HORIZONTAL
    
    // Стартовый пиксель принадлежит глифу, расстояние до глифа нулевое.
    if (IsGlyph(vTexCoord))
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
        return;
    }
    
    // Ищем пиксели глифа по горизонтали вправо и влево.
    for (int i = 1; i <= cRadius; i++)
    {
        if (IsGlyph(vTexCoord + vec2(i, 0.0) * cTextureInvSize))
        {
            // Нашли ближайший пиксель. Сохраняем расстояние в альфу.
            // Расстояние приводится к диапазону [0.0, 1.0].
            gl_FragColor = vec4(1.0, 1.0, 1.0, float(i) / cRadius);
            return;
        }
        
        if (IsGlyph(vTexCoord - vec2(i, 0.0) * cTextureInvSize))
        {
            gl_FragColor = vec4(1.0, 1.0, 1.0, float(i) / cRadius);
            return;
        }
    }
    
    // Глиф в пределах заданного радиуса не найден. Устанавливаем расстояние в максимум.
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);

#endif

// Вычисляем итоговое расстояние от пикселя до глифа.
// На вход передается текстура с расстояниями до глифа по горизонтали.
#ifdef VERTICAL

    // Для начала примем смещение по вертикали равным нулю.
    // В этом случае гипотенузу вычислять не нужно,
    // так как она совпадает с ненулевым катетом.
    float dist = GetHDist(0);
    
    // Каждая гипотенуза - это расстояние от текущего пикселя до
    // крайних точек глифа в радиусе. Гипотенузу легко вычислить,
    // зная расстояния по горизонтали и по вертикали.
    // Выбираем минимальную гипотенузу.
    for (int i = 1; i <= cRadius; ++i)
    {
        float vDist = float(i) / cRadius;
        
        dist = min(dist, CalcC(GetHDist(i), vDist));
        dist = min(dist, CalcC(GetHDist(-i), vDist));
    }
    
    gl_FragColor = vec4(1.0, 1.0, 1.0, dist);

#endif


// Комбинируем две карты расстояний в одну.
#ifdef COMBINE
    
    // Диапазоны [0.0, 1.0] приводятся к диапазонам [0.0, 0.5].
    float toGlyphDist = texture2D(sDiffMap, vTexCoord).a * 0.5;
    float toBackgroundDist = texture2D(sNormalMap, vTexCoord).a * 0.5;
    
    // Для глифа альфа >= 0.5, для фона альфа < 0.5.
    float result = 0.5 - toGlyphDist + toBackgroundDist;
    
    gl_FragColor = vec4(1.0, 1.0, 1.0, result);

#endif
    
}
