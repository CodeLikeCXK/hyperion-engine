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
    Assert(m_face != nullptr);

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

    FT_GlyphSlot glyphSlot = m_face->GetFace()->glyph;
    Assert(glyphSlot != nullptr);

    FT_Bitmap& ftBitmap = glyphSlot->bitmap;

    m_metrics.width = uint16(glyphSlot->bitmap.width);
    m_metrics.height = uint16(glyphSlot->bitmap.rows);
    m_metrics.bearingX = int16(glyphSlot->bitmap_left);
    m_metrics.bearingY = int16(glyphSlot->bitmap_top);
    m_metrics.advance = uint32(glyphSlot->advance.x);
#else
    HYP_LOG(Font, Error, "Glyph::LoadMetrics called without FreeType support!");
#endif
}

TResult<UniquePtr<GlyphBitmap>> Glyph::Rasterize()
{
    Assert(m_face != nullptr);

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

    FT_GlyphSlot glyphSlot = m_face->GetFace()->glyph;
    Assert(glyphSlot != nullptr);

    FT_Bitmap& ftBitmap = glyphSlot->bitmap;

    Assert(glyphSlot->format == FT_GLYPH_FORMAT_BITMAP);
#endif

    m_metrics.imagePosition = { 0, 0 };

    AssertDebug(m_metrics.width != 0 && m_metrics.height != 0);

#ifdef HYP_FREETYPE
    const Vec2i dimensions = GetMax();

    ByteBuffer byteBuffer;
    byteBuffer.SetSize(dimensions.Volume() * BytesPerComponent(g_glyphTextureFormat) * NumComponents(g_glyphTextureFormat));

    if (ftBitmap.buffer != nullptr)
    {
        for (uint32 row = 0; row < ftBitmap.rows; ++row)
        {
            for (uint32 col = 0; col < ftBitmap.width; ++col)
            {
                ubyte* dstOffset = byteBuffer.Data() + (row * dimensions.x + col) * BytesPerComponent(g_glyphTextureFormat) * NumComponents(g_glyphTextureFormat);

                // Copy the grayscale value into the RGBA channels
                for (uint32 componentIndex = 0; componentIndex < NumComponents(g_glyphTextureFormat); componentIndex++)
                {
                    for (uint32 byteIndex = 0; byteIndex < BytesPerComponent(g_glyphTextureFormat); byteIndex++)
                    {
                        dstOffset[componentIndex * BytesPerComponent(g_glyphTextureFormat) + byteIndex] = ftBitmap.buffer[row * ftBitmap.pitch + col];
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
    Assert(m_face != nullptr);

#ifdef HYP_FREETYPE
    return MathUtil::Max({ MathUtil::Abs(m_face->GetFace()->glyph->bitmap.pitch), int(m_face->GetFace()->glyph->bitmap.rows) }, Vec2i::One());
#else
    return { 0, 0 };
#endif
}

Vec2i Glyph::GetMin()
{
    Assert(m_face != nullptr);

#ifdef HYP_FREETYPE
    auto& box = m_face->GetFace()->bbox;
    return { int(box.xMin), int(box.yMin) };
#else
    return { 0, 0 };
#endif
}

#pragma endregion Glyph

} // namespace hyperion
