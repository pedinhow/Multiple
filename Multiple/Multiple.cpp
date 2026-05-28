/**********************************************************************************
// Multiple (Código Fonte)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "Multiple.h"

// ------------------------------------------------------------------------------

Timer Multiple::timer;

// ------------------------------------------------------------------------------

void Multiple::Init()
{
    // --------------------------------------
    // Transformação, Visualização e Projeção
    // --------------------------------------

    // posição inicial da câmera
    camera = { XM_PIDIV2, XM_PIDIV4, 5.0f };

    // inicializa a matriz de projeção
    XMStoreFloat4x4(&Proj, XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f), 
        window->AspectRatio(), 
        1.0f, 100.0f));

    // -------------------------------------------
    // Criação das Geometrias: Vértices e Índices
    // -------------------------------------------

    Box box(2.0f, 2.0f, 2.0f, Orange);
    Cylinder cylinder(1.0f, 0.5f, 3.0f, 20, 20, Yellow);
    Sphere sphere(1.0f, 20, 20, Crimson);
    Grid grid(3.0f, 3.0f, 20, 20, Gray);

    // -------------------------
    // Definição dos Objetos 3D
    // -------------------------

    // box
    Object boxObj;
    XMStoreFloat4x4(&boxObj.world,
        XMMatrixScaling(0.4f, 0.4f, 0.4f) *
        XMMatrixTranslation(-1.0f, 0.41f, 1.0f));
    boxObj.mesh = new Mesh(box);
    boxObj.vbuffer = new VertexBuffer<Vertex>(box);
    boxObj.ibuffer = new IndexBuffer<uint>(box);
    boxObj.cbuffer = new ConstantBuffer<Constants>();
    scene.push_back(boxObj);

    // cylinder
    Object cylinderObj;
    XMStoreFloat4x4(&cylinderObj.world,
        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        XMMatrixTranslation(1.0f, 0.75f, -1.0f));
    cylinderObj.mesh = new Mesh(cylinder);
    cylinderObj.vbuffer = new VertexBuffer<Vertex>(cylinder);
    cylinderObj.ibuffer = new IndexBuffer<uint>(cylinder);
    cylinderObj.cbuffer = new ConstantBuffer<Constants>();
    scene.push_back(cylinderObj);

    // sphere
    Object sphereObj;
    XMStoreFloat4x4(&sphereObj.world,
        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        XMMatrixTranslation(0.0f, 0.5f, 0.0f));
    sphereObj.mesh = new Mesh(sphere);
    sphereObj.vbuffer = new VertexBuffer<Vertex>(sphere);
    sphereObj.ibuffer = new IndexBuffer<uint>(sphere);
    sphereObj.cbuffer = new ConstantBuffer<Constants>();
    scene.push_back(sphereObj);

    // grid
    Object gridObj;
    gridObj.mesh = new Mesh(grid);
    XMStoreFloat4x4(&gridObj.world,
        XMMatrixIdentity());
    gridObj.vbuffer = new VertexBuffer<Vertex>(grid);
    gridObj.ibuffer = new IndexBuffer<uint>(grid);
    gridObj.cbuffer = new ConstantBuffer<Constants>();
    scene.push_back(gridObj);
 
    // ---------------------

    BuildRootSignature();
    BuildPipelineState();    

    // ----------------------

    timer.Start();
}

// ------------------------------------------------------------------------------

void Multiple::Update()
{
    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // ativa ou desativa o giro do objeto
    if (input->KeyPress('S'))
    {
        spinning = !spinning;

        if (spinning)
            timer.Start();
        else
            timer.Stop();
    }

    // atualiza posição da câmera
    camera.Update();

    // constrói a matriz de visualização
    XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);

    // carrega matriz de projeção
    XMMATRIX proj = XMLoadFloat4x4(&Proj);

    // modifica matriz de mundo da esfera
    XMStoreFloat4x4(&scene[2].world,
        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        XMMatrixRotationY(float(timer.Elapsed())) *
        XMMatrixTranslation(0.0f, 0.5f, 0.0f));

    // ajusta o constant buffer de cada objeto
    for (auto & obj : scene)
    {
        // carrega matriz de mundo
        XMMATRIX world = XMLoadFloat4x4(&obj.world);      

        // constrói matriz combinada
        XMMATRIX WorldViewProj = world * view * proj;        

        // atualiza o buffer constante com a matriz combinada
        Constants constants;
        XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(WorldViewProj));
        obj.cbuffer->Copy(&constants);
    }
}

// ------------------------------------------------------------------------------

void Multiple::Draw()
{
    // limpa o backbuffer
    graphics->Clear();

    // comandos de configuração comuns a todos os objetos
    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // desenha objetos da cena
    for (auto& obj : scene)
    {
        // comandos de configuração específicos a cada objeto
        graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer->View());
        graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
        graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());

        // desenha objeto
        graphics->CommandList()->DrawIndexedInstanced(
            obj.mesh->indexCount, 1,
            obj.mesh->startIndex,
            obj.mesh->baseVertex,
            0);
    }
 
    // apresenta o backbuffer na tela
    graphics->Present();    
}

// ------------------------------------------------------------------------------

void Multiple::Finalize()
{
    // espera GPU finalizar comandos pendentes
    graphics->WaitForGpu();

    // libera memória alocada
    rootSignature->Release();
    pipelineState->Release();

    for (auto& obj : scene)
    {
        delete obj.mesh;
        delete obj.vbuffer;
        delete obj.ibuffer;
        delete obj.cbuffer;
    }
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    try
    {
        // cria motor e configura a janela
        Engine* engine = new Engine();
        engine->window->Mode(ASPECTRATIO);
        engine->window->Size(1024, 720);
        engine->window->Color(25, 25, 25);
        engine->window->Title("Multiple");
        engine->window->Icon("Icon");
        engine->window->LostFocus(Multiple::Pause);
        engine->window->InFocus(Multiple::Resume);

        // cria e executa a aplicação
        engine->Start(new Multiple());

        // finaliza execução
        delete engine;
    }
    catch (Error & e)
    {
        // exibe mensagem em caso de erro
        MessageBox(nullptr, e.ToString().data(), "Multiple", MB_OK);
    }

    return 0;
}

// ----------------------------------------------------------------------------
