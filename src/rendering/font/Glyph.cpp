/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <rendering/font/Glyph.hpp>
#include <rendering/font/FontEngine.hpp>
#include <rendering/font/FontFace.hpp>

#include <streaming/StreamedTextureData.hpp>

#include <scene/Texture.hpp>

#include <core/logging/Logger.hpp>

#ifdef HYP_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#endif

namespace hyperion {

HYP_DECLARE_LOG_CHANNEL(Font);

static constexpr TextureFormat g_glyphTextureFormat = TF_RGBA8;

#pragma region GlyphImageData

UniquePtr<GlyphBitmap> GlyphImageData::CreateBitmap() const
{
    return MakeUnique<GlyphBitmap>(byteBuffer.ToByteView(), uint32(dimensions.x), uint32(dimensions.y));
}

#pragma endregion GlyphImageData

#pragma region Glyph

Glyph::Glyph(RC<FontFace> face, FontFace::GlyphIndex index, float scale)
    : m_face(std::move(face)),
      m_index(index),
      m_scale(scale)
{
}

void Glyph::LoadMetrics()
{
    AssertThrow(m_face != nullptr);

#ifdef HYP_FREETYPE
    if (FT_Set_Pixel_Sizes(m_face->GetFace(), 0, MathUtil::Floor<float, uint32>(64.0f * m_scale)))
    {
        HYP_LOG(Font, Error, "Error setting pixel size for font face!");

        return;
    }

    if (FT_Load_Glyph(m_face->GetFace(), m_index, FT_LOAD_DEFAULT | FT_LOAD_COLOR | FT_PIXEL_MODE_GRAY))
    {
        HYP_LOG(Font, Error, "Error loading glyph from font face!");

        return;
    }
#endif
}

TResult<UniquePtr<GlyphBitmap>> Glyph::Rasterize()
{
    AssertThrow(m_face != nullptr);

    FontEngine::Glyph glyph {};

#ifdef HYP_FREETYPE
    if (FT_Set_Pixel_Sizes(m_face->GetFace(), 0, MathUtil::Floor<float, uint32>(64.0f * m_scale)))
    {
        return HYP_MAKE_ERROR(Error, "Error setting pixel size for font face!");
    }

    if (FT_Load_Glyph(m_face->GetFace(), m_index, FT_LOAD_DEFAULT | FT_LOAD_COLOR | FT_LOAD_RENDER | FT_PIXEL_MODE_GRAY))
    {
        return HYP_MAKE_ERROR(Error, "Error loading glyph from font face!");
    }

    glyph = m_face->GetFace()->glyph;
    FT_Bitmap& ftBitmap = glyph->bitmap;

    AssertThrow(glyph->format == FT_GLYPH_FORMAT_BITMAP);

    m_metrics.width = uint16(glyph->bitmap.width);
    m_metrics.height = uint16(glyph->bitmap.rows);
    m_metrics.bearingX = int16(glyph->bitmap_left);
    m_metrics.bearingY = int16(glyph->bitmap_top);
    m_metrics.advance = uint32(glyph->advance.x);
#endif

    m_metrics.imagePosition = { 0, 0 };

    AssertDebug(m_metrics.width != 0 && m_metrics.height != 0);

#ifdef HYP_FREETYPE
    const Vec2i dimensions = GetMax();

    ByteBuffer byteBuffer;
    byteBuffer.SetSize(dimensions.Volume() * NumComponents(g_glyphTextureFormat) * NumBytes(g_glyphTextureFormat));

    if (ftBitmap.buffer != nullptr)
    {
        for (uint32 row = 0; row < ftBitmap.rows; ++row)
        {
            for (uint32 col = 0; col < ftBitmap.width; ++col)
            {
                ubyte* dstOffset = byteBuffer.Data() + (row * dimensions.x + col) * NumComponents(g_glyphTextureFormat) * NumBytes(g_glyphTextureFormat);

                // Copy the grayscale value into the RGBA channels
                for (uint32 componentIndex = 0; componentIndex < NumComponents(g_glyphTextureFormat); componentIndex++)
                {
                    for (uint32 byteIndex = 0; byteIndex < NumBytes(g_glyphTextureFormat); byteIndex++)
                    {
                        dstOffset[componentIndex * NumBytes(g_glyphTextureFormat) + byteIndex] = ftBitmap.buffer[row * ftBitmap.pitch + col];
                    }
                }
                // byteBuffer.Data()[row * dimensions.x + col] = ftBitmap.buffer[row * ftBitmap.pitch + col];
            }
        }
    }

    m_glyphImageData = GlyphImageData { dimensions, std::move(byteBuffer) };
#endif

    if (m_glyphImageData.byteBuffer.Empty())
    {
        return HYP_MAKE_ERROR(Error, "Failed to rasterize glyph, no font data in buffer");
    }

    return m_glyphImageData.CreateBitmap();
}

Vec2i Glyph::GetMax()
{
    AssertThrow(m_face != nullptr);

#ifdef HYP_FREETYPE
    return MathUtil::Max({ MathUtil::Abs(m_face->GetFace()->glyph->bitmap.pitch), int(m_face->GetFace()->glyph->bitmap.rows) }, Vec2i::One());
#else
    return { 0, 0 };
#endif
}

Vec2i Glyph::GetMin()
{
    AssertThrow(m_face != nullptr);

#ifdef HYP_FREETYPE
    auto& box = m_face->GetFace()->bbox;
    return { int(box.xMin), int(box.yMin) };
#else
    return { 0, 0 };
#endif
}

#pragma endregion Glyph

} // namespace hyperion
