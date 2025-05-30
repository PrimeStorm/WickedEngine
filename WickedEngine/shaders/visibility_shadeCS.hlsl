#define ENTITY_TILE_UNIFORM
#define SHADOW_MASK_ENABLED
#define DISABLE_DECALS // decals were applied in surface shader
//#define DISABLE_VOXELGI
//#define DISABLE_ENVMAPS
//#define DISABLE_SOFT_SHADOWMAP
//#define DISABLE_TRANSPARENT_SHADOWMAP

#ifdef PLANARREFLECTION
#define DISABLE_ENVMAPS
#define DISABLE_VOXELGI
#endif // PLANARREFLECTION

#include "globals.hlsli"
#include "ShaderInterop_Renderer.h"
#include "raytracingHF.hlsli"
#include "brdf.hlsli"
#include "shadingHF.hlsli"

struct VisibilityPushConstants
{
	uint global_tile_offset;
};
PUSHCONSTANT(push, VisibilityPushConstants);

StructuredBuffer<VisibilityTile> binned_tiles : register(t0);
Texture2D<uint4> input_payload_0 : register(t2);
Texture2D<uint4> input_payload_1 : register(t3);

RWTexture2D<float4> output : register(u0);

[numthreads(VISIBILITY_BLOCKSIZE, VISIBILITY_BLOCKSIZE, 1)]
void main(uint Gid : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	const uint tile_offset = push.global_tile_offset + Gid.x;
	VisibilityTile tile = binned_tiles[tile_offset];
	[branch] if (!tile.check_thread_valid(groupIndex)) return;
	const uint2 GTid = remap_lane_8x8(groupIndex);
	const uint2 pixel = unpack_pixel(tile.visibility_tile_id) * VISIBILITY_BLOCKSIZE + GTid;

	const float2 uv = ((float2)pixel + 0.5) * GetCamera().internal_resolution_rcp;
	RayDesc ray = CreateCameraRay(pixel);

	uint primitiveID = texture_primitiveID[pixel];
	PrimitiveID prim;
	prim.init();
	prim.unpack(primitiveID);

	Surface surface;
	surface.init();

	[branch]
	if (!surface.load(prim, ray.Origin, ray.Direction))
	{
		return;
	}
	surface.pixel = pixel.xy;
	surface.screenUV = uv;

	// Unpack primary payload:
	uint4 payload_0 = input_payload_0[pixel];
	half4 data0 = unpack_rgba(payload_0.x);
	surface.albedo = RemoveSRGBCurve_Fast(data0.rgb);
	surface.occlusion = data0.a;
	half4 data1 = unpack_rgba(payload_0.y);
	surface.f0 = RemoveSRGBCurve_Fast(data1.rgb);
	surface.roughness = data1.a;
	surface.N = decode_oct(unpack_half2(payload_0.z));
	surface.emissiveColor = Unpack_R11G11B10_FLOAT(payload_0.w);

	surface.opacity = 1;
	surface.baseColor = half4(surface.albedo, surface.opacity);

#ifdef ANISOTROPIC
	surface.T = unpack_half4(input_payload_1[pixel].xy);
#endif // ANISOTROPIC

#ifdef SHEEN
	float4 data_sheen = unpack_rgba(input_payload_1[pixel].x);
	surface.sheen.color = data_sheen.rgb;
	surface.sheen.roughness = data_sheen.a;
#endif // SHEEN

#ifdef CLEARCOAT
	surface.clearcoat.N = decode_oct(unpack_half2(input_payload_1[pixel].y));
	surface.clearcoat.roughness = unpack_rgba(input_payload_1[pixel].z).r;
#endif // CLEARCOAT

	surface.update();

	if (!surface.IsGIApplied())
	{
		half3 ambient = GetAmbient(surface.N);
		surface.gi = lerp(ambient, ambient * surface.sss.rgb, saturate(surface.sss.a));
	}

	Lighting lighting;
	lighting.create(0, 0, surface.gi, 0);


#ifdef PLANARREFLECTION
	float2 bumpColor = unpack_half2(input_payload_1[pixel].x);
	lighting.indirect.specular += PlanarReflection(surface, bumpColor) * surface.F;
#endif // PLANARREFLECTION

	TiledLighting(surface, lighting, tile.entity_flat_tile_index);

#ifndef CARTOON
	[branch]
	if (GetCamera().texture_ssr_index >= 0)
	{
		half4 ssr = bindless_textures_half4[descriptor_index(GetCamera().texture_ssr_index)].SampleLevel(sampler_linear_clamp, surface.screenUV, 0);
		lighting.indirect.specular = lerp(lighting.indirect.specular, ssr.rgb * surface.F, ssr.a);
	}
	[branch]
	if (GetCamera().texture_ssgi_index >= 0)
	{
		surface.ssgi = bindless_textures_half4[descriptor_index(GetCamera().texture_ssgi_index)].SampleLevel(sampler_linear_clamp, surface.screenUV, 0).rgb;
	}
	[branch]
	if (GetCamera().texture_ao_index >= 0)
	{
		surface.occlusion *= bindless_textures_half4[descriptor_index(GetCamera().texture_ao_index)].SampleLevel(sampler_linear_clamp, surface.screenUV, 0).r;
	}
#endif // CARTOON

	half4 color = 0;

	ApplyLighting(surface, lighting, color);

	half4 rimHighlight = surface.inst.GetRimHighlight();
	color.rgb += rimHighlight.rgb * pow(1 - surface.NdotV, rimHighlight.w);

	ApplyFog(surface.hit_depth, surface.V, color);

	color = saturateMediump(color);

	output[pixel] = half4(color.rgb, 1);

}
