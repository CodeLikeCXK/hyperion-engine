
#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_separate_shader_objects : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texcoord;

layout(location = 0) out vec4 color_output;

#define HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS
#include "../include/shared.inc"
#include "../include/packing.inc"

#include "../include/scene.inc"

#undef HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS

HYP_DESCRIPTOR_SRV(CombineShadowMapsDescriptorSet, Src0) uniform texture2D src0;
HYP_DESCRIPTOR_SRV(CombineShadowMapsDescriptorSet, Src1) uniform texture2D src1;
HYP_DESCRIPTOR_SAMPLER(Global, SamplerNearest) uniform sampler sampler_nearest;

void main()
{
    vec4 color0 = Texture2D(sampler_nearest, src0, v_texcoord);
    vec4 color1 = Texture2D(sampler_nearest, src1, v_texcoord);

#ifdef VSM
    color_output = color0.r < color1.r ? color0 : color1;
#else
    float unpackedDepth0 = UnpackDepth(color0);
    float unpackedDepth1 = UnpackDepth(color1);

    float depth = min(unpackedDepth0, unpackedDepth1);

    color_output = vec4(depth);
#endif
}
