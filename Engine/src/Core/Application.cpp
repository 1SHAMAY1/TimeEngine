#include "Application.h"

#include <glad/glad.h>

#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Window/IWindow.hpp"
#include "ImGUI/ImGuiLayer.hpp"
#include "Renderer/RenderCommand.hpp"

#define TE_USE_HARDCODE_TRIANGLE 0

namespace TE
{
    Application* Application::s_Instance = nullptr;

    Application::Application()
        : m_Running(true)
    {
        TE_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        TE::Log::Init(true, "TimeEngineLog.json");
        TE_CORE_INFO("Application Constructor called.");

        m_Window = std::unique_ptr<IWindow>(IWindow::Create());

        if (!gladLoadGLLoader((GLADloadproc)m_Window->GetGLLoaderFunction()))
        {
            TE_CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        TE_CORE_INFO("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));

        m_Window->SetEventCallback([this](Event& e) {
            EventDispatcher dispatcher(e);

            dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
                TE_CLIENT_INFO("Window close event received.");
                m_Running = false;
                return true;
            });

            dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& event) {
                TE_CLIENT_INFO("Window resized: " + event.ToString());
                return false;
            });

            dispatcher.Dispatch<WindowFocusEvent>([](WindowFocusEvent& event) {
                TE_CLIENT_INFO("Window gained focus");
                return false;
            });

            dispatcher.Dispatch<WindowLostFocusEvent>([](WindowLostFocusEvent& event) {
                TE_CLIENT_INFO("Window lost focus");
                return false;
            });
        });

        

        // === ImGui Layer Setup ===
        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

#if TE_USE_HARDCODE_TRIANGLE

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // === VAO ===
    glGenVertexArrays(1, &I_VertexArray);
    glBindVertexArray(I_VertexArray);

    // === VBO ===
    float vertices[] = {
        0.0f,  0.5f, 0.0f,   // Top
       -0.5f, -0.5f, 0.0f,   // Bottom Left
        0.5f, -0.5f, 0.0f    // Bottom Right
    };

    glCreateBuffers(1, &I_VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, I_VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0
    );

    // === IBO ===
    unsigned int indices[] = { 0, 1, 2 };
    glGenBuffers(1, &I_IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, I_IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // === Shader ===
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;

        out vec3 v_Position;

        void main() {
            v_Position = a_Position;
            gl_Position = vec4(a_Position, 1.0);
        }
    )";
        
    const char* fragmentSrc = R"(
        #version 330 core
        in vec3 v_Position;
        out vec4 FragColor;

        void main() {
            // Convert from [-0.5, 0.5] to [0, 1]
            vec2 coord = v_Position.xy + vec2(0.5);
            float dist = length(coord - vec2(0.5));

            // Low alpha at center (dist = 0), high at edge (dist ≈ 0.7)
            float alpha = smoothstep(0.1, 0.5, dist); // tweak 0.1 and 0.5 for effect

            vec3 color = vec3(coord.x, 1.0 - coord.y, 0.6); // soft gradient color
            FragColor = vec4(color, alpha);
        }
    )";

    // Compile Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // Compile Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // Link Shader Program
    I_ShaderProgram = glCreateProgram();
    glAttachShader(I_ShaderProgram, vertexShader);
    glAttachShader(I_ShaderProgram, fragmentShader);
    glLinkProgram(I_ShaderProgram);

    glGetProgramiv(I_ShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(I_ShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

#else

        // Initialize RenderCommand system
        RenderCommand::Init();

        // Create Vertex Array
        m_VertexArray.reset(VertexArray::Create());

        // Vertex data: 3 vertices, each with x, y, z
        float vertices[] = {
            0.0f,  0.5f, 0.0f,  // Top
           -0.5f, -0.5f, 0.0f,  // Bottom Left
            0.5f, -0.5f, 0.0f   // Bottom Right
       };

        // Create Vertex Buffer
        m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

        // Bind VAO and set vertex attributes
        m_VertexArray->Bind();
        m_VertexBuffer->Bind();

        m_VertexArray->AddVertexBuffer(m_VertexBuffer.get());

        // Create Index Buffer
        unsigned int indices[] = { 0, 1, 2 };
        m_IndexBuffer.reset(IndexBuffer::Create(indices, 3));

        // Attach IBO to VAO
        m_VertexArray->Bind();          // Must bind VAO first
        m_IndexBuffer->Bind();          // Then bind IBO while VAO is active
        m_VertexArray->SetIndexBuffer(m_IndexBuffer.get());

        // Create Shader
        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            out vec3 v_Position;
            void main() {
                v_Position = a_Position;
                gl_Position = vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            in vec3 v_Position;
            out vec4 FragColor;
            void main() {
                // Convert from [-0.5, 0.5] to [0, 1]
                vec2 coord = v_Position.xy + vec2(0.5);
                float dist = length(coord - vec2(0.5));
                
                // Low alpha at center (dist = 0), high at edge (dist ≈ 0.7)
                float alpha = smoothstep(0.1, 0.5, dist);
                
                vec3 color = vec3(coord.x, 1.0 - coord.y, 0.6);
                FragColor = vec4(color, alpha);
            }
        )";

        m_Shader.reset(Shader::Create(vertexSrc, fragmentSrc));

#endif

        
    }

    Application::~Application()
    {
        TE_CORE_INFO("Application Destructor called.");
    }

    void Application::Run()
    {
        TE_CORE_INFO("Application Run started.");

        float time = 0.0f;

        while (m_Running)
        {
            glClearColor(0, 0, 0, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

#if TE_USE_HARDCODE_TRIANGLE
            glUseProgram(I_ShaderProgram);
            glBindVertexArray(I_VertexArray);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
#else
            // Use abstract rendering system
            m_Shader->Bind();
            m_VertexArray->Bind();
            RenderCommand::DrawIndexed(m_VertexArray->GetRendererID(), m_IndexBuffer->GetCount());
#endif


            // Logic update
            for (Layer* layer : m_LayerStack)
                if (layer)
                    layer->OnUpdate();

            // ImGui Rendering
            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                if (layer)
                    layer->OnImGuiRender();
            m_ImGuiLayer->End();

            // Process any deferred layer removals after all layer operations are complete
            m_LayerStack.ProcessDeferredRemovals();

            m_Window->OnUpdate();
        }

        TE_CORE_INFO("Application Run ended.");
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        //layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
        //overlay->OnAttach();
    }

    void Application::PopLayer(Layer* layer)
    {
        m_LayerStack.PopLayer(layer);
        //layer->OnDetach();
    }

    void Application::PopOverlay(Layer* overlay)
    {
        m_LayerStack.PopOverlay(overlay);
        //overlay->OnDetach();
    }

    void Application::MarkLayerForRemoval(Layer* layer)
    {
        m_LayerStack.MarkLayerForRemoval(layer);
    }


    void Application::MarkOverlayForRemoval(Layer* overlay)
    {
        m_LayerStack.MarkOverlayForRemoval(overlay);
    }
}
