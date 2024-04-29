#pragma once

// nightfire RenderColor class

#define BGRA_R 2u
#define BGRA_G 1u
#define BGRA_B 0u
#define BGRA_A 3u

#define RGBA_R 0u
#define RGBA_G 1u
#define RGBA_B 2u
#define RGBA_A 3u

template <typename T, unsigned char positionRed, unsigned char positionGreen, unsigned char positionBlue, unsigned char positionAlpha>
struct RenderColor {
    T color[4];

    RenderColor(T red, T green, T blue, T alpha) {
        color[positionRed] = red;
        color[positionGreen] = green;
        color[positionBlue] = blue;
        color[positionAlpha] = alpha;
    }

    /*D3DCOLOR*/ unsigned int AsInteger() { return (colors[3] << 24) | (colors[2] << 16) | (colors[1] << 8) | colors[0]; }

    // Constructor for RGBA format
    static RenderColor<T, RGBA_R, RGBA_G, RGBA_B, RGBA_A> FromRGBA(T red, T green, T blue, T alpha) {
        return RenderColor<T, RGBA_R, RGBA_G, RGBA_B, RGBA_A>(red, green, blue, alpha);
    }

    // Constructor for BGRA format
    static RenderColor<T, BGRA_R, BGRA_G, BGRA_B, BGRA_A> FromBGRA(T blue, T green, T red, T alpha) {
        return RenderColor<T, BGRA_R, BGRA_G, BGRA_B, BGRA_A>(red, green, blue, alpha);
    }
};


// WARNING: arguments R G B A!!
typedef RenderColor<unsigned char, BGRA_R, BGRA_G, BGRA_B, BGRA_A> ColorBGRA;

// arguments R G B A
typedef RenderColor<unsigned char, RGBA_R, RGBA_G, RGBA_B, RGBA_A> ColorRGBA;


// arguments B G R A
template <typename T>
struct RenderColorBGRA : public RenderColor<T, BGRA_R, BGRA_G, BGRA_B, BGRA_A> {
    RenderColorBGRA(T blue, T green, T red, T alpha) : RenderColor<T, BGRA_R, BGRA_G, BGRA_B, BGRA_A>(red, green, blue, alpha) {}

    T& Red() { return this->colors[BGRA_R]; }
    T& Green() { return this->colors[BGRA_G]; }
    T& Blue() { return this->colors[BGRA_B]; }
    T& Alpha() { return this->colors[BGRA_A]; }

    ColorBGRA& AsColorBGRA() { return *this; }
};

// arguments R G B A
template <typename T>
struct RenderColorRGBA : public RenderColor<T, RGBA_R, RGBA_G, RGBA_B, RGBA_A> {
    RenderColorRGBA(T red, T green, T blue, T alpha) : RenderColor<T, RGBA_R, RGBA_G, RGBA_B, RGBA_A>(red, green, blue, alpha) {}

    T& Red() { return this->colors[RGBA_R]; }
    T& Green() { return this->colors[RGBA_G]; }
    T& Blue() { return this->colors[RGBA_B]; }
    T& Alpha() { return this->colors[RGBA_A]; }

    ColorRGBA& AsColorRGBA() { return *this; }
};

#if 0
#include <iostream>
inline void TestRenderColor()
{
    RenderColorBGRA<unsigned char> clearColorBGRATest(255, 0, 0, 255); // Blue with full opacity
    std::cout << "RenderColorBGRA red, green, blue: (" << (int)clearColorBGRATest.colors[BGRA_R] << ", " << (int)clearColorBGRATest.colors[BGRA_G] << ", " << (int)clearColorBGRATest.colors[BGRA_B] << ", " << (int)clearColorBGRATest.colors[BGRA_A] << ")" << std::endl;

    RenderColorRGBA<unsigned char> clearColorRGBATest(0, 0, 255, 255); // Blue with full opacity
    std::cout << "RenderColorRGBA red, green, blue: (" << (int)clearColorRGBATest.colors[RGBA_R] << ", " << (int)clearColorRGBATest.colors[RGBA_G] << ", " << (int)clearColorRGBATest.colors[RGBA_B] << ", " << (int)clearColorRGBATest.colors[RGBA_A] << ")" << std::endl;

    // Example usage with BGRA colors format
    ColorBGRA clearColorBGRA = ColorBGRA::FromBGRA(255, 0, 0, 255); // Blue with full opacity
    std::cout << "BGRA red, green, blue: (" << (int)clearColorBGRA.colors[BGRA_R] << ", " << (int)clearColorBGRA.colors[BGRA_G] << ", " << (int)clearColorBGRA.colors[BGRA_B] << ", " << (int)clearColorBGRA.colors[BGRA_A] << ")" << std::endl;

    // Example usage with RGBA colors format
    ColorRGBA clearColorRGBA = ColorRGBA::FromRGBA(0, 0, 255, 255); // Blue with full opacity
    std::cout << "RGBA red, green, blue: (" << (int)clearColorRGBA.colors[RGBA_R] << ", " << (int)clearColorRGBA.colors[RGBA_G] << ", " << (int)clearColorRGBA.colors[RGBA_B] << ", " << (int)clearColorRGBA.colors[RGBA_A] << ")" << std::endl;


    // ClearBuffers test
    ColorBGRA* pColor = &clearColorBGRA;
    unsigned int combined = (pColor->colors[3] << 24) | (pColor->colors[2] << 16) | (pColor->colors[1] << 8) | pColor->colors[0];
    RenderColorBGRA<unsigned char>* pCombined = (RenderColorBGRA<unsigned char>*) & combined;
    std::cout << "ClearBuffers red, green, blue: (" << (int)pCombined->Red() << ", " << (int)pCombined->Green() << ", " << (int)pCombined->Blue() << ", " << (int)pCombined->Alpha() << ")" << std::endl;
    std::cout << "Unsigned int value: " << clearColorBGRA.AsInteger() << std::endl;
}
#endif