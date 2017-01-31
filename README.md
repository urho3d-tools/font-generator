# Urho3D Bitmap Font Generator

License: Public Domain

## SDF

1) SDF generator uses true brute force on GPU for maximum quality. Currently exists only GLSL shader for it:
https://github.com/1vanK/Urho3DBitmapFontGenerator/blob/master/Bin/MyData/Shaders/GLSL/SdfCalculator.glsl

2) SDF fonts render with large scale and after it downscaled to required size. You can decrease "Scale" parameter to fast generation or increase to improve quality.

3) Do not use large distance for thin fonts: stroke effect will look bad.

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/SDF.png)

## Bitmap Fonts

Bitmap fonts are best used with https://github.com/1vanK/Urho3DSpriteBatch (but not necessarily).

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/Contour.png)

With blurring:

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/OutlinedGlow.png)
![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/OutlinedGlowView.png)

Without blurring:

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/OutlinedYellow.png)

Special case: when the inner and outer colors are identical, the internal character is not rendered.

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/BlurredShadowSettings.png)
![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/WithBlurredShadow.png)

It can be useful to create shadows:
```
spriteBatch_->DrawString(String("Hello!"), CACHE->GetResource<Font>("Fonts/BigFontShadow.fnt"), 40, Vector2(50.0f + 10.0f, 50.0f + 10.0f), Color(1.0f, 1.0f, 1.0f, 0.5f));
spriteBatch_->DrawString(String("Hello!"), CACHE->GetResource<Font>("Fonts/BigFont.fnt"), 40, Vector2(50.0f, 50.0f));
```

![Screenshot](https://raw.githubusercontent.com/1vanK/Urho3DBitmapFontGenerator/master/BlurredShadow.png)
