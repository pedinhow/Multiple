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
#include <fstream>
#include <sstream>

// ------------------------------------------------------------------------------

Timer Multiple::timer;

// ------------------------------------------------------------------------------

struct LineGeometry : public Geometry
{
    LineGeometry()
    {
        // Define cruzamento perfeito de linhas ocupando o espaço homogêneo de projeção (-1 a 1)
        Vertex v1{ { -1.0f,  0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        Vertex v2{ {  1.0f,  0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        Vertex v3{ {  0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
        Vertex v4{ {  0.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

        vertices = { v1, v2, v3, v4 };
        indices = { 0, 1, 2, 3 };
    }
};

struct ObjGeometry : public Geometry
{
    ObjGeometry(const std::string& filename, XMFLOAT4 defaultColor = Crimson)
    {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::vector<XMFLOAT3> positions;
        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            if (prefix == "v")
            {
                float x, y, z;
                ss >> x >> y >> z;
                positions.push_back(XMFLOAT3(x, y, z));
            }
            else if (prefix == "f")
            {
                std::vector<uint> faceIndices;
                std::string token;
                while (ss >> token)
                {
                    std::stringstream tokenStream(token);
                    std::string indexStr;
                    std::getline(tokenStream, indexStr, '/');
                    if (!indexStr.empty())
                    {
                        int idx = std::stoi(indexStr);
                        if (idx > 0) faceIndices.push_back(static_cast<uint>(idx - 1));
                        else if (idx < 0) faceIndices.push_back(static_cast<uint>(positions.size() + idx));
                    }
                }

                // Triangulação em leque (Fan Triangulation) para polígonos complexos
                for (size_t i = 1; i < faceIndices.size() - 1; ++i)
                {
                    indices.push_back(faceIndices[0]);
                    indices.push_back(faceIndices[i]);
                    indices.push_back(faceIndices[i + 1]);
                }
            }
        }

        vertices.resize(positions.size());
        for (size_t i = 0; i < positions.size(); ++i)
        {
            vertices[i].pos = positions[i];
            vertices[i].color = defaultColor;
        }
    }
};

void Multiple::AddPrimitive(Geometry* geo)
{
    if (!geo || geo->VertexCount() == 0) return;

    Object obj;
    obj.mesh = new Mesh(*geo);
    obj.vbuffer = new VertexBuffer<Vertex>(*geo);
    obj.ibuffer = new IndexBuffer<uint>(*geo);
    obj.cbuffer = new ConstantBuffer<Constants>();

    obj.translation = { 0.0f, 0.0f, 0.0f };
    obj.rotation = { 0.0f, 0.0f, 0.0f };
    obj.scale = { 1.0f, 1.0f, 1.0f };
    obj.UpdateWorldMatrix();

    scene.push_back(obj);
    selectedIndex = (int)scene.size() - 1; // Foca automaticamente no objeto inserido
    delete geo;
}


// ------------------------------------------------------------------------------
void Multiple::Init()
{
    camera = { XM_PIDIV2, XM_PIDIV4, 5.0f };

    XMStoreFloat4x4(&Proj, XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f),
        window->AspectRatio(),
        1.0f, 100.0f));

    // Instanciação das Linhas Divisórias
    LineGeometry lineGeo;
    lineVBuffer = new VertexBuffer<Vertex>(lineGeo);
    lineIBuffer = new IndexBuffer<uint>(lineGeo);
    lineCBuffer = new ConstantBuffer<Constants>();

    // Carga de Primitiva Inicial (Grid de base estática)
    AddPrimitive(new Grid(6.0f, 6.0f, 30, 30, Gray));
    selectedIndex = -1; // Desmarca seleção inicial para fins estéticos

    BuildRootSignature();
    BuildPipelineState();

    timer.Start();
}

// ------------------------------------------------------------------------------

void Multiple::Update()
{
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // Chave de Alternância das Viewports (V)
    if (input->KeyPress('V'))
    {
        fourViewsMode = !fourViewsMode;
    }

    // Inserção de Primitivas Geométricas Padronizadas
    if (input->KeyPress('B')) AddPrimitive(new Box(1.0f, 1.0f, 1.0f, Orange));
    if (input->KeyPress('C')) AddPrimitive(new Cylinder(0.5f, 0.5f, 1.5f, 16, 16, Yellow));
    if (input->KeyPress('S')) AddPrimitive(new Sphere(0.6f, 16, 16, Crimson));
    if (input->KeyPress('G')) AddPrimitive(new GeoSphere(0.6f, 3, Pink));
    if (input->KeyPress('P')) AddPrimitive(new Grid(2.0f, 2.0f, 10, 10, Gray));
    if (input->KeyPress('Q')) AddPrimitive(new Quad(1.0f, 1.0f, Green));

    // Carregamento de Malhas OBJ Externas (Teclas 1 a 5)
    if (input->KeyPress('1')) AddPrimitive(new ObjGeometry("Ball.obj"));
    if (input->KeyPress('2')) AddPrimitive(new ObjGeometry("Capsule.obj"));
    if (input->KeyPress('3')) AddPrimitive(new ObjGeometry("House.obj"));
    if (input->KeyPress('4')) AddPrimitive(new ObjGeometry("Monkey.obj"));
    if (input->KeyPress('5')) AddPrimitive(new ObjGeometry("Thorus.obj"));

    // Controle do Mecanismo de Seleção de Atores (TAB)
    if (input->KeyPress(VK_TAB))
    {
        if (!scene.empty())
            selectedIndex = (selectedIndex + 1) % scene.size();
        else
            selectedIndex = -1;
    }

    // Remoção Destrutiva (DEL)
    if (input->KeyPress(VK_DELETE))
    {
        if (selectedIndex >= 0 && selectedIndex < (int)scene.size())
        {
            delete scene[selectedIndex].mesh;
            delete scene[selectedIndex].vbuffer;
            delete scene[selectedIndex].ibuffer;
            delete scene[selectedIndex].cbuffer;

            scene.erase(scene.begin() + selectedIndex);
            selectedIndex = scene.empty() ? -1 : 0;
        }
    }

    // Processamento de Câmera Esférica Perspectiva
    camera.Update();

    // Bloco de Manipulação do Elemento Focado (Teclado)
    if (selectedIndex >= 0 && selectedIndex < (int)scene.size())
    {
        float speed = 2.5f * (float)frameTime;
        float rotSpeed = 1.8f * (float)frameTime;

        // Controle de Rotação Espacial (Segurando a Tecla 'R')
        if (input->KeyDown('R'))
        {
            if (input->KeyDown(VK_LEFT))  scene[selectedIndex].rotation.y -= rotSpeed;
            if (input->KeyDown(VK_RIGHT)) scene[selectedIndex].rotation.y += rotSpeed;
            if (input->KeyDown(VK_UP))    scene[selectedIndex].rotation.x -= rotSpeed;
            if (input->KeyDown(VK_DOWN))  scene[selectedIndex].rotation.x += rotSpeed;
            if (input->KeyDown(VK_PRIOR)) scene[selectedIndex].rotation.z -= rotSpeed; // Page Up
            if (input->KeyDown(VK_NEXT))  scene[selectedIndex].rotation.z += rotSpeed; // Page Down
        }
        else // Controle de Translação Padrão
        {
            if (input->KeyDown(VK_LEFT))  scene[selectedIndex].translation.x -= speed;
            if (input->KeyDown(VK_RIGHT)) scene[selectedIndex].translation.x += speed;
            if (input->KeyDown(VK_UP))    scene[selectedIndex].translation.z += speed;
            if (input->KeyDown(VK_DOWN))  scene[selectedIndex].translation.z -= speed;
            if (input->KeyDown(VK_PRIOR)) scene[selectedIndex].translation.y += speed; // Page Up
            if (input->KeyDown(VK_NEXT))  scene[selectedIndex].translation.y -= speed; // Page Down
        }

        // Modificadores de Escala Absoluta Uniforme (Teclas U e J)
        if (input->KeyDown('U'))
        {
            scene[selectedIndex].scale.x += speed;
            scene[selectedIndex].scale.y += speed;
            scene[selectedIndex].scale.z += speed;
        }
        if (input->KeyDown('J'))
        {
            scene[selectedIndex].scale.x -= speed;
            scene[selectedIndex].scale.y -= speed;
            scene[selectedIndex].scale.z -= speed;
            // Impede inversão de normais por colapso de escala negativa
            if (scene[selectedIndex].scale.x < 0.01f)
                scene[selectedIndex].scale = { 0.01f, 0.01f, 0.01f };
        }

        scene[selectedIndex].UpdateWorldMatrix();
    }
}

// ------------------------------------------------------------------------------

void Multiple::Draw()
{
    graphics->Clear();

    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);

    float w = (float)window->Width();
    float h = (float)window->Height();
    int viewsCount = fourViewsMode ? 4 : 1;

    // Loop de renderização multicâmera baseada em Viewports
    for (int i = 0; i < viewsCount; ++i)
    {
        D3D12_VIEWPORT vp{};
        D3D12_RECT scissorRect{};
        XMMATRIX view, proj;

        if (!fourViewsMode)
        {
            // Viewport Integral Unificada (Perspectiva)
            vp = { 0.0f, 0.0f, w, h, 0.0f, 1.0f };
            scissorRect = { 0, 0, (LONG)w, (LONG)h };

            XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
            XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            view = viewMatrix;
            proj = XMLoadFloat4x4(&Proj);
        }
        else
        {
            float halfW = w / 2.0f;
            float halfH = h / 2.0f;
            float oScale = 5.0f; // Escala do volume de visualização ortogonal

            if (i == 0)      // Top-Left quadrant: Vista de Topo (TOP)
            {
                vp = { 0.0f, 0.0f, halfW, halfH, 0.0f, 1.0f };
                scissorRect = { 0, 0, (LONG)halfW, (LONG)halfH };
                view = XMMatrixLookAtLH(XMVectorSet(0.0f, 10.0f, 0.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
                proj = XMMatrixOrthographicLH(oScale * (halfW / halfH), oScale, 0.1f, 100.0f);
            }
            else if (i == 1) // Top-Right quadrant: Vista Frontal (FRONT)
            {
                vp = { halfW, 0.0f, halfW, halfH, 0.0f, 1.0f };
                scissorRect = { (LONG)halfW, 0, (LONG)w, (LONG)halfH };
                view = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
                proj = XMMatrixOrthographicLH(oScale * (halfW / halfH), oScale, 0.1f, 100.0f);
            }
            else if (i == 2) // Bottom-Left quadrant: Vista Lateral Direita (RIGHT)
            {
                vp = { 0.0f, halfH, halfW, halfH, 0.0f, 1.0f };
                scissorRect = { 0, (LONG)halfH, (LONG)halfW, (LONG)h };
                view = XMMatrixLookAtLH(XMVectorSet(10.0f, 0.0f, 0.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
                proj = XMMatrixOrthographicLH(oScale * (halfW / halfH), oScale, 0.1f, 100.0f);
            }
            else             // Bottom-Right quadrant: Perspectiva Dinâmica (PERSPECTIVE)
            {
                vp = { halfW, halfH, halfW, halfH, 0.0f, 1.0f };
                scissorRect = { (LONG)halfW, (LONG)halfH, (LONG)w, (LONG)h };
                XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
                view = XMMatrixLookAtLH(pos, XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
                proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), halfW / halfH, 1.0f, 100.0f);
            }
        }

        graphics->CommandList()->RSSetViewports(1, &vp);
        graphics->CommandList()->RSSetScissorRects(1, &scissorRect);

        // Renderização sequencial de objetos para a Viewport selecionada
        for (int j = 0; j < (int)scene.size(); ++j)
        {
            auto& obj = scene[j];
            XMMATRIX world = XMLoadFloat4x4(&obj.world);
            XMMATRIX wvp = world * view * proj;

            Constants constants;
            XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(wvp));

            // Atribuição de cor distinta (Ciano) se o objeto atual for o foco de seleção
            if (j == selectedIndex)
                constants.ColorOverride = { 0.0f, 1.0f, 1.0f, 1.0f };
            else
                constants.ColorOverride = { 0.0f, 0.0f, 0.0f, 0.0f };

            obj.cbuffer->Copy(&constants);

            graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer->View());
            graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
            graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());

            graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            graphics->CommandList()->DrawIndexedInstanced(
                obj.mesh->indexCount, 1,
                obj.mesh->startIndex,
                obj.mesh->baseVertex,
                0);
        }
    }

    // Desenho ortogonal das linhas divisórias de quadrantes
    if (fourViewsMode)
    {
        D3D12_VIEWPORT screenVp = { 0.0f, 0.0f, w, h, 0.0f, 1.0f };
        D3D12_RECT screenScissor = { 0, 0, (LONG)w, (LONG)h };
        graphics->CommandList()->RSSetViewports(1, &screenVp);
        graphics->CommandList()->RSSetScissorRects(1, &screenScissor);

        Constants lineConstants;
        XMStoreFloat4x4(&lineConstants.WorldViewProj, XMMatrixTranspose(XMMatrixIdentity()));
        lineConstants.ColorOverride = { 0.5f, 0.5f, 0.5f, 1.0f }; // Linhas cinzas separadoras
        lineCBuffer->Copy(&lineConstants);

        graphics->CommandList()->SetGraphicsRootConstantBufferView(0, lineCBuffer->View());
        graphics->CommandList()->IASetVertexBuffers(0, 1, lineVBuffer->View());
        graphics->CommandList()->IASetIndexBuffer(lineIBuffer->View());
        graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

        graphics->CommandList()->DrawIndexedInstanced(4, 1, 0, 0, 0);
    }

    graphics->Present();
}

// ------------------------------------------------------------------------------

void Multiple::Finalize()
{
    graphics->WaitForGpu();

    rootSignature->Release();
    pipelineState->Release();

    for (auto& obj : scene)
    {
        delete obj.mesh;
        delete obj.vbuffer;
        delete obj.ibuffer;
        delete obj.cbuffer;
    }

    // Liberação de memória das linhas divisórias
    delete lineVBuffer;
    delete lineIBuffer;
    delete lineCBuffer;
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
