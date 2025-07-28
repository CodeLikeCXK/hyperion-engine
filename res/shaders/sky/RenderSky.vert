#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_separate_shader_objects : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_multiview : require

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_texcoord0;
layout(location = 4) out vec3 v_tangent;
layout(location = 5) out vec3 v_bitangent;
layout(location = 7) out flat vec3 v_camera_position;
layout(location = 8) out mat3 v_tbn_matrix;
layout(location = 11) out flat uint v_object_index;
layout(location = 13) out flat uint v_cube_face_index;
layout(location = 14) out vec2 v_cube_face_uv;

HYP_ATTRIBUTE(0) vec3 a_position;
HYP_ATTRIBUTE(1) vec3 a_normal;
HYP_ATTRIBUTE(2) vec2 a_texcoord0;
HYP_ATTRIBUTE(3) vec2 a_texcoord1;
HYP_ATTRIBUTE(4) vec3 a_tangent;
HYP_ATTRIBUTE(5) vec3 a_bitangent;

#define HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS

#include "../include/scene.inc"

#include "../include/object.inc"
#include "../include/env_probe.inc"

#undef HYP_DO_NOT_DEFINE_DESCRIPTOR_SETS

HYP_DESCRIPTOR_CBUFF_DYNAMIC(Global, CamerasBuffer) uniform CameraShaderData
{
    Camera camera;
};

HYP_DESCRIPTOR_SSBO_DYNAMIC(Global, CurrentEnvProbe) readonly buffer CurrentEnvProbe
{
    EnvProbe current_env_probe;
};

#ifdef INSTANCING
HYP_DESCRIPTOR_SSBO(Global, ObjectsBuffer) readonly buffer ObjectsBuffer
{
    Object objects[];
};

HYP_DESCRIPTOR_SSBO_DYNAMIC(Instancing, EntityInstanceBatchesBuffer) readonly buffer EntityInstanceBatchesBuffer
{
    EntityInstanceBatch entity_instance_batch;
};
#else
HYP_DESCRIPTOR_SSBO_DYNAMIC(Object, CurrentObject) readonly buffer ObjectsBuffer
{
    Object object;
};
#endif

void main()
{
    vec4 position;
    mat4 normal_matrix;

    // if (object.bucket == HYP_OBJECT_BUCKET_SKYBOX) {
    //     position = vec4((a_position * 150.0) + camera.position.xyz, 1.0);
    //     normal_matrix = transpose(inverse(object.model_matrix));
    // } else {
    position = object.model_matrix * vec4(a_position, 1.0);
    normal_matrix = transpose(inverse(object.model_matrix));
    // }

    v_position = position.xyz;
    v_normal = (normal_matrix * vec4(a_normal, 0.0)).xyz;
    v_texcoord0 = vec2(a_texcoord0.x, 1.0 - a_texcoord0.y);
    v_camera_position = camera.position.xyz;

    v_tangent = normalize(normal_matrix * vec4(a_tangent, 0.0)).xyz;
    v_bitangent = normalize(normal_matrix * vec4(a_bitangent, 0.0)).xyz;
    v_tbn_matrix = mat3(v_tangent, v_bitangent, v_normal);

    mat4 projection_matrix = camera.projection;
    mat4 view_matrix = current_env_probe.face_view_matrices[gl_ViewIndex];

#ifdef INSTANCING
    v_object_index = OBJECT_INDEX;
#endif

    gl_Position = projection_matrix * view_matrix * position;

    v_cube_face_index = gl_ViewIndex;
    v_cube_face_uv = gl_Position.xy * 0.5 + 0.5;
}
