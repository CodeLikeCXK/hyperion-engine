/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <rendering/DOFBlur.hpp>
#include <rendering/ShaderManager.hpp>

#include <rendering/backend/RendererFrame.hpp>

#include <core/logging/Logger.hpp>
#include <core/logging/LogChannels.hpp>

namespace hyperion {

DOFBlur::DOFBlur(const Vec2u& extent, GBuffer* gbuffer)
    : m_gbuffer(gbuffer),
      m_extent(extent)
{
}

DOFBlur::~DOFBlur() = default;

void DOFBlur::Create()
{
    ShaderRef blurHorizontalShader = ShaderManager::GetInstance()->GetOrCreate(NAME("DOFBlurDirection"), ShaderProperties({ "DIRECTION_HORIZONTAL" }));
    Assert(blurHorizontalShader.IsValid());

    m_blurHorizontalPass = MakeUnique<FullScreenPass>(blurHorizontalShader, TF_RGBA8, m_extent, m_gbuffer);
    m_blurHorizontalPass->Create();

    ShaderRef blurVerticalShader = ShaderManager::GetInstance()->GetOrCreate(NAME("DOFBlurDirection"), ShaderProperties({ "DIRECTION_VERTICAL" }));
    Assert(blurVerticalShader.IsValid());

    m_blurVerticalPass = MakeUnique<FullScreenPass>(blurVerticalShader, TF_RGBA8, m_extent, m_gbuffer);
    m_blurVerticalPass->Create();

    ShaderRef blurMixShader = ShaderManager::GetInstance()->GetOrCreate(NAME("DOFBlurMix"));
    Assert(blurMixShader.IsValid());

    m_blurMixPass = MakeUnique<FullScreenPass>(blurMixShader, TF_RGBA8, m_extent, m_gbuffer);
    m_blurMixPass->Create();
}

void DOFBlur::Destroy()
{
    m_blurHorizontalPass.Reset();
    m_blurVerticalPass.Reset();
    m_blurMixPass.Reset();
}

void DOFBlur::Render(FrameBase* frame, const RenderSetup& renderSetup)
{
    struct
    {
        Vec2u dimension;
    } pushConstants;

    pushConstants.dimension = m_extent;

    const uint32 frameIndex = frame->GetFrameIndex();

    FixedArray<FullScreenPass*, 2> directionalPasses {
        m_blurHorizontalPass.Get(),
        m_blurVerticalPass.Get()
    };

    for (FullScreenPass* pass : directionalPasses)
    {
        pass->SetPushConstants(&pushConstants, sizeof(pushConstants));
        pass->Render(frame, renderSetup);
    }

    m_blurMixPass->SetPushConstants(&pushConstants, sizeof(pushConstants));
    m_blurMixPass->Render(frame, renderSetup);
}

} // namespace hyperion