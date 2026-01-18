// font_manager.hpp

#pragma once

#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../thirdparty/glm/glm/glm.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class iGraphicsAPI;
    class cApplication;
    class cTexture;
    class cContext;

    struct sGlyph
    {
        types::u8 _character = 0;
        types::s32 _width = 0;
        types::s32 _height = 0;
        types::s32 _left = 0;
        types::s32 _top = 0;
        types::f32 _advanceX = 0.0f;
        types::f32 _advanceY = 0.0f;
        types::s32 _atlasXOffset = 0;
        types::s32 _atlasYOffset = 0;
        void* _bitmapData = nullptr;
    };

    class cFontFace : public cObject
    {
        REALWARE_OBJECT(cFontFace)

    public:
        explicit cFontFace(cContext* context);
        virtual ~cFontFace() override final;

        void FillAlphabetAndFindAtlasSize(types::usize& xOffset, types::usize& atlasWidth, types::usize& atlasHeight);
        void FillAtlasWithGlyphs(types::usize& atlasWidth, types::usize& atlasHeight);

        inline FT_Face GetFont() const { return _font; }
        inline types::usize GetGlyphSize() const { return _glyphSize; }
        inline types::usize GetOffsetNewline() const { return _offsetNewline; }
        inline types::usize GetOffsetSpace() const { return _offsetSpace; }
        inline types::usize GetOffsetTab() const { return _offsetTab; }
        inline std::unordered_map<types::u8, sGlyph>& GetAlphabet() const { return _alphabet; }
        inline cTexture* GetAtlas() const { return _atlas; }
        inline void SetGlyphSize(types::usize size) { _glyphSize = size; }
        inline void SetOffsetNewline(types::usize offset) { _offsetNewline = offset; }
        inline void SetOffsetSpace(types::usize offset) { _offsetSpace = offset; }
        inline void SetOffsetTab(types::usize offset) { _offsetTab = offset; }

    private:
        FT_Face _font = {};
        types::usize _glyphCount = 0;
        types::usize _glyphSize = 0;
        types::usize _offsetNewline = 0;
        types::usize _offsetSpace = 0;
        types::usize _offsetTab = 0;
        mutable std::unordered_map<types::u8, sGlyph> _alphabet = {};
        cTexture* _atlas = nullptr;
    };

    class cText : public cObject
    {
        REALWARE_OBJECT(cText)

    public:
        explicit cText(cContext* context);
        virtual ~cText() override final;

        inline void SetFont(cFontFace* font) { _font = font; }
        inline void SetText(const std::string& text) { _text = text; }

    private:
        cFontFace* _font = nullptr;
        std::string _text = "";
    };

    class cFont : public cObject
    {
        REALWARE_OBJECT(cFont)

    public:
        explicit cFont(cContext* context);
        virtual ~cFont() override final;

        cFontFace* CreateFontTTF(const std::string& filename, types::usize glyphSize);
        cText* CreateText(cFontFace* font, const std::string& text);
        void DestroyFontTTF(cFontFace* font);
        void DestroyText(cText* text);
            
        types::f32 GetTextWidth(cFontFace* font, const std::string& text) const;
        types::f32 GetTextHeight(cFontFace* font, const std::string& text) const;
        types::usize GetCharacterCount(const std::string& text) const;
        types::usize GetNewlineCount(const std::string& text) const;

        static constexpr types::usize K_MAX_ATLAS_WIDTH = 2048;

    private:
        types::boolean _initialized = types::K_FALSE;
        iGraphicsAPI* _gfx = nullptr;
        FT_Library _lib = {};
    };
}