#pragma once

class ParticleShaders
{
public:
    static TEString GetVertexShader()
    {
        return R"(#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;
out vec4 v_Color;

void main()
{
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";
    }

    static TEString GetFragmentShader()
    {
        return R"(#version 450 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;
uniform int u_UseTexture;

void main()
{
    vec4 texColor = vec4(1.0);
    if (u_UseTexture == 1)
    {
        texColor = texture(u_Texture, v_TexCoord);
    }
    else
    {
        // Smooth circular soft particle falloff if no texture
        vec2 uv = v_TexCoord * 2.0 - 1.0;
        float dist = length(uv);
        float alpha = clamp(1.0 - dist, 0.0, 1.0);
        alpha = smoothstep(0.0, 1.0, alpha);
        texColor = vec4(1.0, 1.0, 1.0, alpha);
    }

    FragColor = texColor * v_Color;
}
)";
    }
};
