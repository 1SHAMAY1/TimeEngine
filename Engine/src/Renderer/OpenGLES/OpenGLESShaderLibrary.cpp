#include "Renderer/OpenGLES/OpenGLESShaderLibrary.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace TE
{

// ---------------------------------------------------------------------------
// Shader Creation
// ---------------------------------------------------------------------------

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESBasicShader()
{
    // Minimal GLSL ES 3.0 vertex + fragment shaders
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
void main() {
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
out vec4 o_Color;
uniform vec4 u_Color;
void main() {
    o_Color = u_Color;
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESTextureShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
void main() {
    v_TexCoord  = a_TexCoord;
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Texture;
uniform vec4      u_Color;
void main() {
    o_Color = texture(u_Texture, v_TexCoord) * u_Color;
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESColorShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
out vec4 v_Color;
uniform mat4 u_ViewProjection;
void main() {
    v_Color     = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec4 v_Color;
out vec4 o_Color;
void main() {
    o_Color = v_Color;
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESStandardShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_TexCoord;
uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos     = vec3(worldPos);
    v_Normal      = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord    = a_TexCoord;
    gl_Position   = u_ViewProjection * worldPos;
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec3 v_Normal;
in  vec3 v_FragPos;
in  vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Texture;
uniform vec3  u_LightPos;
uniform vec3  u_LightColor;
uniform float u_AmbientStrength;
void main() {
    // Ambient
    vec3 ambient = u_AmbientStrength * u_LightColor;
    // Diffuse
    vec3 norm     = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * u_LightColor;
    vec4 texColor = texture(u_Texture, v_TexCoord);
    o_Color = vec4((ambient + diffuse) * texColor.rgb, texColor.a);
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESLightingShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
out vec3 v_Normal;
out vec3 v_FragPos;
uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos     = vec3(worldPos);
    v_Normal      = mat3(transpose(inverse(u_Model))) * a_Normal;
    gl_Position   = u_ViewProjection * worldPos;
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec3 v_Normal;
in  vec3 v_FragPos;
out vec4 o_Color;
uniform vec3  u_LightPos;
uniform vec3  u_LightColor;
uniform vec3  u_ObjectColor;
uniform float u_AmbientStrength;
uniform float u_SpecularStrength;
uniform float u_Shininess;
uniform vec3  u_ViewPos;
void main() {
    vec3 ambient  = u_AmbientStrength * u_LightColor;
    vec3 norm     = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * u_LightColor;
    vec3 viewDir  = normalize(u_ViewPos - v_FragPos);
    vec3 reflectD = reflect(-lightDir, norm);
    float spec    = pow(max(dot(viewDir, reflectD), 0.0), u_Shininess);
    vec3 specular = u_SpecularStrength * spec * u_LightColor;
    o_Color = vec4((ambient + diffuse + specular) * u_ObjectColor, 1.0);
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESParticleShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in float a_Size;
out vec4 v_Color;
uniform mat4 u_ViewProjection;
void main() {
    v_Color     = a_Color;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    gl_PointSize = a_Size;
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec4 v_Color;
out vec4 o_Color;
void main() {
    // Circular point sprite
    vec2  coord = gl_PointCoord - 0.5;
    if (dot(coord, coord) > 0.25)
        discard;
    o_Color = v_Color;
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESPostProcessShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
void main() {
    v_TexCoord  = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_ScreenTexture;
uniform float     u_Gamma;
void main() {
    vec4 color = texture(u_ScreenTexture, v_TexCoord);
    // Simple gamma correction pass
    o_Color = vec4(pow(color.rgb, vec3(1.0 / u_Gamma)), color.a);
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESUIShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;
out vec2 v_TexCoord;
out vec4 v_Color;
uniform mat4 u_Projection;
void main() {
    v_TexCoord  = a_TexCoord;
    v_Color     = a_Color;
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec2 v_TexCoord;
in  vec4 v_Color;
out vec4 o_Color;
uniform sampler2D u_Texture;
void main() {
    o_Color = v_Color * texture(u_Texture, v_TexCoord);
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

std::shared_ptr<OpenGLESShader> OpenGLESShaderLibrary::CreateOpenGLESLight2DShader()
{
    const std::string vert = R"(#version 300 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
out vec2 v_WorldPos;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
void main() {
    vec4 world  = u_Transform * vec4(a_Position, 1.0);
    v_WorldPos  = world.xy;
    v_TexCoord  = a_TexCoord;
    gl_Position = u_ViewProjection * world;
})";

    const std::string frag = R"(#version 300 es
precision mediump float;
in  vec2 v_TexCoord;
in  vec2 v_WorldPos;
out vec4 o_Color;
uniform sampler2D u_Texture;
uniform vec2      u_LightPos;
uniform vec4      u_LightColor;
uniform float     u_LightRadius;
void main() {
    float dist     = length(v_WorldPos - u_LightPos);
    float attenuation = clamp(1.0 - dist / u_LightRadius, 0.0, 1.0);
    attenuation    = attenuation * attenuation;
    vec4 texColor  = texture(u_Texture, v_TexCoord);
    o_Color        = texColor * u_LightColor * attenuation;
})";

    return std::make_shared<OpenGLESShader>(vert, frag);
}

// ---------------------------------------------------------------------------
// Uniform Setters
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::SetUniform1i(OpenGLESShader *shader, const std::string &name, int value)
{
    shader->Bind();
    shader->SetUniform1i(name, value);
}
void OpenGLESShaderLibrary::SetUniform1f(OpenGLESShader *shader, const std::string &name, float value)
{
    shader->Bind();
    shader->SetUniform1f(name, value);
}
void OpenGLESShaderLibrary::SetUniform2f(OpenGLESShader *shader, const std::string &name, const glm::vec2 &value)
{
    shader->Bind();
    shader->SetUniform2f(name, value);
}
void OpenGLESShaderLibrary::SetUniform3f(OpenGLESShader *shader, const std::string &name, const glm::vec3 &value)
{
    shader->Bind();
    shader->SetUniform3f(name, value);
}
void OpenGLESShaderLibrary::SetUniform4f(OpenGLESShader *shader, const std::string &name, const glm::vec4 &value)
{
    shader->Bind();
    shader->SetUniform4f(name, value);
}
void OpenGLESShaderLibrary::SetUniformMat3(OpenGLESShader *shader, const std::string &name, const glm::mat3 &value)
{
    shader->Bind();
    glUniformMatrix3fv(shader->GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}
void OpenGLESShaderLibrary::SetUniformMat4(OpenGLESShader *shader, const std::string &name, const glm::mat4 &value)
{
    shader->Bind();
    shader->SetUniformMat4(name, value);
}
void OpenGLESShaderLibrary::SetUniformMat4Array(OpenGLESShader *shader, const std::string &name,
                                                const std::vector<glm::mat4> &values)
{
    shader->Bind();
    glUniformMatrix4fv(shader->GetUniformLocation(name), static_cast<GLsizei>(values.size()), GL_FALSE,
                       glm::value_ptr(values[0]));
}

// ---------------------------------------------------------------------------
// State Management
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::EnableBlending() { glEnable(GL_BLEND); }
void OpenGLESShaderLibrary::DisableBlending() { glDisable(GL_BLEND); }
void OpenGLESShaderLibrary::SetBlendFunc(GLenum src, GLenum dst) { glBlendFunc(src, dst); }
void OpenGLESShaderLibrary::EnableDepthTest() { glEnable(GL_DEPTH_TEST); }
void OpenGLESShaderLibrary::DisableDepthTest() { glDisable(GL_DEPTH_TEST); }
void OpenGLESShaderLibrary::SetDepthFunc(GLenum func) { glDepthFunc(func); }
void OpenGLESShaderLibrary::EnableStencilTest() { glEnable(GL_STENCIL_TEST); }
void OpenGLESShaderLibrary::DisableStencilTest() { glDisable(GL_STENCIL_TEST); }
void OpenGLESShaderLibrary::SetStencilFunc(GLenum func, int ref, unsigned int mask) { glStencilFunc(func, ref, mask); }
void OpenGLESShaderLibrary::SetStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    glStencilOp(sfail, dpfail, dppass);
}
void OpenGLESShaderLibrary::EnableCullFace() { glEnable(GL_CULL_FACE); }
void OpenGLESShaderLibrary::DisableCullFace() { glDisable(GL_CULL_FACE); }
void OpenGLESShaderLibrary::SetCullFace(GLenum face) { glCullFace(face); }
void OpenGLESShaderLibrary::SetFrontFace(GLenum mode) { glFrontFace(mode); }

// ---------------------------------------------------------------------------
// Texture Helpers
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::BindTexture2D(unsigned int textureID, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}
void OpenGLESShaderLibrary::BindTextureCube(unsigned int textureID, int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}
void OpenGLESShaderLibrary::SetTextureFiltering(unsigned int textureID, GLenum minFilter, GLenum magFilter)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}
void OpenGLESShaderLibrary::SetTextureWrapping(unsigned int textureID, GLenum wrapS, GLenum wrapT)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}
void OpenGLESShaderLibrary::GenerateMipmaps(unsigned int textureID)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
}

// ---------------------------------------------------------------------------
// Framebuffer Helpers
// ---------------------------------------------------------------------------

unsigned int OpenGLESShaderLibrary::CreateFramebuffer()
{
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    return fbo;
}
void OpenGLESShaderLibrary::BindFramebuffer(unsigned int fbo) { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }
void OpenGLESShaderLibrary::AttachTexture2D(unsigned int fbo, unsigned int textureID, GLenum attachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
}
void OpenGLESShaderLibrary::AttachRenderbuffer(unsigned int fbo, unsigned int rbo, GLenum attachment)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);
}
bool OpenGLESShaderLibrary::CheckFramebufferStatus(unsigned int fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
void OpenGLESShaderLibrary::DeleteFramebuffer(unsigned int fbo) { glDeleteFramebuffers(1, &fbo); }

// ---------------------------------------------------------------------------
// Uniform Buffer Objects (ES 3.0+)
// ---------------------------------------------------------------------------

unsigned int OpenGLESShaderLibrary::CreateUniformBuffer(const void *data, size_t size)
{
    unsigned int ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return ubo;
}
void OpenGLESShaderLibrary::BindUniformBuffer(unsigned int ubo, unsigned int bindingPoint)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
}
void OpenGLESShaderLibrary::UpdateUniformBuffer(unsigned int ubo, const void *data, size_t offset, size_t size)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void OpenGLESShaderLibrary::BindUniformBlock(OpenGLESShader *shader, const std::string &blockName,
                                             unsigned int bindingPoint)
{
    unsigned int blockIndex = glGetUniformBlockIndex(shader->GetRendererID(), blockName.c_str());
    if (blockIndex != GL_INVALID_INDEX)
        glUniformBlockBinding(shader->GetRendererID(), blockIndex, bindingPoint);
}

// ---------------------------------------------------------------------------
// Vertex Attribute Helpers
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::SetVertexAttribPointer(unsigned int index, int size, GLenum type, bool normalized,
                                                   int stride, const void *pointer)
{
    glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE, stride, pointer);
}
void OpenGLESShaderLibrary::EnableVertexAttribArray(unsigned int index) { glEnableVertexAttribArray(index); }
void OpenGLESShaderLibrary::DisableVertexAttribArray(unsigned int index) { glDisableVertexAttribArray(index); }
void OpenGLESShaderLibrary::VertexAttribDivisor(unsigned int index, unsigned int divisor)
{
    glVertexAttribDivisor(index, divisor);
}

// ---------------------------------------------------------------------------
// Debug (KHR_debug extension on ES 3.2 / KHR_debug on ES 3.1-)
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::EnableDebugOutput()
{
    // Requires GL_KHR_debug or ES 3.2
    // glEnable(GL_DEBUG_OUTPUT);
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    std::cout << "[OpenGL ES] Debug output enable (requires KHR_debug extension)\n";
}
void OpenGLESShaderLibrary::SetDebugCallback()
{
    // glDebugMessageCallback(DebugCallback, nullptr);
    std::cout << "[OpenGL ES] Debug callback set (requires KHR_debug extension)\n";
}
void OpenGLESShaderLibrary::PushDebugGroup(const std::string &message)
{
    // glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0,
    //                  static_cast<GLsizei>(message.size()), message.c_str());
    (void)message;
}
void OpenGLESShaderLibrary::PopDebugGroup()
{
    // glPopDebugGroup();
}
void OpenGLESShaderLibrary::ObjectLabel(GLenum identifier, unsigned int name, const std::string &label)
{
    // glObjectLabel(identifier, name, static_cast<GLsizei>(label.size()), label.c_str());
    (void)identifier;
    (void)name;
    (void)label;
}
void GLAPIENTRY OpenGLESShaderLibrary::DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                                     GLsizei /*length*/, const char *message,
                                                     const void * /*userParam*/)
{
    std::cerr << "[OpenGL ES Debug] source=0x" << std::hex << source << " type=0x" << type << " id=" << std::dec << id
              << " severity=0x" << std::hex << severity << "\n  Message: " << message << "\n";
}

// ---------------------------------------------------------------------------
// Query Objects
// ---------------------------------------------------------------------------

void OpenGLESShaderLibrary::BeginQuery(GLenum target, unsigned int id) { glBeginQuery(target, id); }
void OpenGLESShaderLibrary::EndQuery(GLenum target) { glEndQuery(target); }
int OpenGLESShaderLibrary::GetQueryObjecti(unsigned int id, GLenum pname)
{
    GLuint result = 0;
    glGetQueryObjectuiv(id, pname, &result);
    return static_cast<int>(result);
}
void OpenGLESShaderLibrary::GetQueryObjectiv(unsigned int id, GLenum pname, int *params)
{
    if (params)
    {
        GLuint result = 0;
        glGetQueryObjectuiv(id, pname, &result);
        *params = static_cast<int>(result);
    }
}
void OpenGLESShaderLibrary::GetQueryObjectuiv(unsigned int id, GLenum pname, unsigned int *params)
{
    glGetQueryObjectuiv(id, pname, params);
}

// ---------------------------------------------------------------------------
// Sync / Fence (ES 3.0+)
// ---------------------------------------------------------------------------

GLsync OpenGLESShaderLibrary::FenceSync(GLenum condition, GLbitfield flags) { return glFenceSync(condition, flags); }
GLenum OpenGLESShaderLibrary::ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    return glClientWaitSync(sync, flags, timeout);
}
void OpenGLESShaderLibrary::DeleteSync(GLsync sync) { glDeleteSync(sync); }

} // namespace TE
