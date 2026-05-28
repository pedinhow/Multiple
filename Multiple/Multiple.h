/**********************************************************************************
// Multiple (Arquivo de Cabeçalho)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "DXUT.h"

// ------------------------------------------------------------------------------

struct Constants
{
    XMFLOAT4X4 WorldViewProj =
    { 1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f };

    // Vetor para controle de cor de destaque (Se w > 0.5, substitui a cor original)
    XMFLOAT4 ColorOverride = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct Object
{
    XMFLOAT4X4 world = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    Mesh* mesh = nullptr;
    VertexBuffer<Vertex>* vbuffer = nullptr;
    IndexBuffer<uint>* ibuffer = nullptr;
    ConstantBuffer<Constants>* cbuffer = nullptr;

    // Atributos de controle vetorial de transformações
    XMFLOAT3 translation = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

    // Sincroniza vetores individuais com a matriz final de mundo
    void UpdateWorldMatrix()
    {
        XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
        XMMATRIX matRot = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        XMMATRIX matTrans = XMMatrixTranslation(translation.x, translation.y, translation.z);

        XMStoreFloat4x4(&world, matScale * matRot * matTrans);
    }
};

// ------------------------------------------------------------------------------

class Multiple : public App
{
private:
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    
    OrbitCamera camera;
    XMFLOAT4X4 Proj;
    static Timer timer;

    std::vector<Object> scene;
    int selectedIndex = -1;       // Índice do elemento focado (-1 para vazio)
    bool fourViewsMode = false;   // Alternância do modo de exibição de viewports

    // Recursos exclusivos para as linhas de demarcação tridimensionais/bidimensionais
    VertexBuffer<Vertex>* lineVBuffer = nullptr;
    IndexBuffer<uint>* lineIBuffer = nullptr;
    ConstantBuffer<Constants>* lineCBuffer = nullptr;

    // Funções auxiliares internas de controle
    void AddPrimitive(Geometry* geo);

public:
    void Init();
    void Update();
    void Draw();
    void Finalize();

    static void Pause()  { timer.Stop();  }
    static void Resume() { timer.Start(); }

    void BuildRootSignature();
    void BuildPipelineState();
};

// ------------------------------------------------------------------------------
