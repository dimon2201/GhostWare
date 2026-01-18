// font_manager.cpp

#include <iostream>
#include "font_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "engine.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
{
    usize CalculateNewlineOffset(cFontFace* font)
    {
        return font->GetFont()->size->metrics.height >> 6;
    }

    usize CalculateSpaceOffset(cFontFace* font)
    {
        const FT_Face ftFont = font->GetFont();
        const FT_UInt spaceIndex = FT_Get_Char_Index(ftFont, ' ');
        if (FT_Load_Glyph(ftFont, spaceIndex, FT_LOAD_DEFAULT) == 0)
            return ftFont->glyph->advance.x >> 6;
        else
            return 0;
    }

    cFontFace::cFontFace(cContext* context) : cObject(context) {}

    cFontFace::~cFontFace()
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();

        for (const auto& glyph : _alphabet)
            memoryAllocator->Deallocate(glyph.second._bitmapData);
        _alphabet.clear();

        gfx->DestroyTexture(_atlas);

        FT_Done_Face(_font);
    }

    void cFontFace::FillAlphabetAndFindAtlasSize(usize& xOffset, usize& atlasWidth, usize& atlasHeight)
    {
        const sEngineCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

        const FT_Face ftFont = _font;
        usize maxGlyphHeight = 0;

        for (usize c = 0; c < 256; c++)
        {
            if (c == '\n' || c == ' ' || c == '\t')
                continue;

            const FT_Int ci = FT_Get_Char_Index(ftFont, c);
            if (FT_Load_Glyph(ftFont, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
            {
                _glyphCount += 1;

                FT_Render_Glyph(ftFont->glyph, FT_RENDER_MODE_NORMAL);

                sGlyph glyph = {};
                glyph._character = (u8)c;
                glyph._width = ftFont->glyph->bitmap.width;
                glyph._height = ftFont->glyph->bitmap.rows;
                glyph._left = ftFont->glyph->bitmap_left;
                glyph._top = ftFont->glyph->bitmap_top;
                glyph._advanceX = ftFont->glyph->advance.x >> 6;
                glyph._advanceY = ftFont->glyph->advance.y >> 6;
                glyph._bitmapData = memoryAllocator->Allocate(glyph._width * glyph._height, caps->memoryAlignment);

                if (ftFont->glyph->bitmap.buffer)
                    memcpy(glyph._bitmapData, ftFont->glyph->bitmap.buffer, glyph._width * glyph._height);

                _alphabet.insert({ (u8)c, glyph });

                xOffset += glyph._width + 1;

                if (atlasWidth < cFont::K_MAX_ATLAS_WIDTH - (glyph._width + 1))
                    atlasWidth += glyph._width + 1;

                if (glyph._height > maxGlyphHeight)
                    maxGlyphHeight = glyph._height;

                if (xOffset >= cFont::K_MAX_ATLAS_WIDTH)
                {
                    atlasHeight += maxGlyphHeight + 1;
                    xOffset = 0;
                    maxGlyphHeight = 0;
                }
            }
        }

        if (atlasHeight < maxGlyphHeight + 1)
            atlasHeight += maxGlyphHeight + 1;
    }

    void cFontFace::FillAtlasWithGlyphs(usize& atlasWidth, usize& atlasHeight)
    {
        const sEngineCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();

        usize maxGlyphHeight = 0;

        void* atlasPixels = memoryAllocator->Allocate(atlasWidth * atlasHeight, caps->memoryAlignment);
        memset(atlasPixels, 0, atlasWidth * atlasHeight);

        usize xOffset = 0;
        usize yOffset = 0;
        u8* pixelsU8 = (u8*)atlasPixels;

        for (auto& glyph : _alphabet)
        {
            glyph.second._atlasXOffset = xOffset;
            glyph.second._atlasYOffset = yOffset;

            for (usize y = 0; y < glyph.second._height; y++)
            {
                for (usize x = 0; x < glyph.second._width; x++)
                {
                    const usize glyphPixelIndex = x + (y * glyph.second._width);
                    const usize pixelIndex = (xOffset + x) + ((yOffset + y) * atlasWidth);

                    if (glyphPixelIndex < glyph.second._width * glyph.second._height &&
                        pixelIndex < atlasWidth * atlasHeight)
                        pixelsU8[pixelIndex] = ((u8*)glyph.second._bitmapData)[glyphPixelIndex];
                }
            }

            xOffset += glyph.second._width + 1;
            if (glyph.second._height > maxGlyphHeight)
                maxGlyphHeight = glyph.second._height;

            if (xOffset >= cFont::K_MAX_ATLAS_WIDTH)
            {
                yOffset += maxGlyphHeight + 1;
                xOffset = 0;
                maxGlyphHeight = 0;
            }
        }

        _atlas = gfx->CreateTexture(
            atlasWidth,
            atlasHeight,
            0,
            cTexture::eDimension::TEXTURE_2D,
            cTexture::eFormat::R8,
            atlasPixels
        );

        memoryAllocator->Deallocate(atlasPixels);
    }

    cText::cText(cContext* context) : cObject(context) {}

    cText::~cText() {}

    cFont::cFont(cContext* context) : cObject(context), _gfx(context->GetSubsystem<cGraphics>()->GetAPI())
    {
        if (FT_Init_FreeType(&_lib))
        {
            Print("Failed to initialize FreeType library!");
            return;
        }

        _initialized = K_TRUE;
    }

    cFont::~cFont()
    {
        if (_initialized)
            FT_Done_FreeType(_lib);
    }

    usize NextPowerOfTwo(usize n)
    {
        if (n <= 0)
            return 1;

        usize power = 1;
        while (power < n)
        {
            if (power >= 0x80000000)
                return 1;

            power <<= 1;
        }

        return power;
    }

    void MakeAtlasSizePowerOf2(usize& atlasWidth, usize& atlasHeight)
    {
        atlasWidth = NextPowerOfTwo(atlasWidth);
        atlasHeight = NextPowerOfTwo(atlasHeight);
    }

    cFontFace* cFont::CreateFontTTF(const std::string& filename, usize glyphSize)
    {
        cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
        iGraphicsAPI* gfx = _context->GetSubsystem<cGraphics>()->GetAPI();
        cFontFace* font = _context->Create<cFontFace>(_context);

        FT_Face ftFont = font->GetFont();

        if (FT_New_Face(_lib, filename.c_str(), 0, &ftFont) == 0)
        {
            FT_Select_Charmap(ftFont, FT_ENCODING_UNICODE);

            if (FT_Set_Pixel_Sizes(ftFont, glyphSize, glyphSize) == 0)
            {
                font->SetGlyphSize(glyphSize);
                font->SetOffsetNewline(CalculateNewlineOffset(font));
                font->SetOffsetSpace(CalculateSpaceOffset(font));
                font->SetOffsetTab(font->GetOffsetSpace() * 4);

                usize atlasWidth = 0;
                usize atlasHeight = 0;
                usize xOffset = 0;

                font->FillAlphabetAndFindAtlasSize(xOffset, atlasWidth, atlasHeight);
                MakeAtlasSizePowerOf2(atlasWidth, atlasHeight);
                font->FillAtlasWithGlyphs(atlasWidth, atlasHeight);
            }
            else
            {
                _context->Destroy<cFontFace>(font);
                    
                return nullptr;
            }
        }
        else
        {
            Print("Error creating FreeType font face!");

            _context->Destroy<cFontFace>(font);
                
            return nullptr;
        }

        return font;
    }

    cText* cFont::CreateText(cFontFace* font, const std::string& text)
    {
        cText* textObject = (cText*)_context->Create<cText>(_context);

        textObject->SetFont(font);
        textObject->SetText(text);

        return textObject;
    }

    void cFont::DestroyFontTTF(cFontFace* font)
    {
        _context->Destroy<cFontFace>(font);
    }

    void cFont::DestroyText(cText* text)
    {
        _context->Destroy<cText>(text);
    }

    f32 cFont::GetTextWidth(cFontFace* font, const std::string& text) const
    {
        cInput* input = _context->GetSubsystem<cInput>();

        f32 textWidth = 0.0f;
        f32 maxTextWidth = 0.0f;
        const usize textByteSize = strlen(text.c_str());
        const glm::vec2 windowSize = input->GetWindow()->GetSize();

        for (usize i = 0; i < textByteSize; i++)
        {
            const sGlyph& glyph = font->GetAlphabet().find(text[i])->second;

            if (text[i] == '\t')
            {
                textWidth += font->GetOffsetTab();
            }
            else if (text[i] == ' ')
            {
                textWidth += font->GetOffsetSpace();
            }
            else if (text[i] == '\n')
            {
                if (maxTextWidth < textWidth)
                    maxTextWidth = textWidth;
                textWidth = 0.0f;
            }
            else
            {
                textWidth += ((f32)glyph._width / windowSize.x);
            }
        }

        if (maxTextWidth < textWidth)
        {
            maxTextWidth = textWidth;
            textWidth = 0.0f;
        }

        return maxTextWidth;
    }

    f32 cFont::GetTextHeight(cFontFace* font, const std::string& text) const
    {
        cInput* input = _context->GetSubsystem<cInput>();

        f32 textHeight = 0.0f;
        f32 maxHeight = 0.0f;
        const usize textByteSize = strlen(text.c_str());
        const glm::vec2 windowSize = input->GetWindow()->GetSize();

        for (usize i = 0; i < textByteSize; i++)
        {
            const sGlyph& glyph = font->GetAlphabet().find(text[i])->second;

            if (text[i] == '\n')
            {
                textHeight += font->GetOffsetNewline();
            }
            else
            {
                f32 glyphHeight = ((f32)glyph._height / windowSize.y);
                if (glyphHeight > maxHeight) {
                    maxHeight = glyphHeight;
                }
            }

            if (i == textByteSize - 1)
            {
                textHeight += maxHeight;
                maxHeight = 0.0f;
            }
        }

        return textHeight;
    }

    usize cFont::GetCharacterCount(const std::string& text) const
    {
        return strlen(text.c_str());
    }

    usize cFont::GetNewlineCount(const std::string& text) const
    {
        usize newlineCount = 0;
        const usize charCount = strlen(text.c_str());
        for (usize i = 0; i < charCount; i++)
        {
            if (text[i] == '\n')
                newlineCount++;
        }

        return newlineCount;
    }
}