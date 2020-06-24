////////////////////////////////////////////////////////////////////////////////////////////////////
// quadDebug10.fx (HLSL)
// Brief: Debugging operations
// Copyright: 2020 Artineering and/or its licensors
// License: MIT
////////////////////////////////////////////////////////////////////////////////////////////////////
// COMMON MAYA VARIABLES
float4x4 gWVP : WorldViewProjection;

// TEXTURES
Texture2D gInputTex;

// VARIABLES
float4 gColorChannels = float4( 1.0, 1.0, 1.0, 0.0 );

// VERTEX SHADER
struct appData {
	float3 vertex : POSITION;
};

struct vertexOutput {
	float4 pos : SV_POSITION;
};

vertexOutput quadVert(appData v) {
	vertexOutput o;
	o.pos = mul(float4(v.vertex, 1.0f), gWVP);
	return o;
}


// PIXEL SHADER
float4 debugPix(vertexOutput i) : SV_Target {
    int3 loc = int3(i.pos.xy, 0);
    float4 tex = gInputTex.Load(loc);

    // channel debugger
    if (gColorChannels.a > 0) {
        return float4(tex.a, tex.a, tex.a, tex.a);
    } else {
        return float4(gColorChannels.r * tex.r, gColorChannels.g * tex.g, gColorChannels.b * tex.b, tex.a);
    }
}

// TECHNIQUES
technique11 debug {
    pass p0 {
        SetVertexShader(CompileShader(vs_5_0, quadVert()));
        SetPixelShader(CompileShader(ps_5_0, debugPix()));
    }
}
