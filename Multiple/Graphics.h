/**********************************************************************************
// Graphics (Arquivo de Cabeçalho)
// 
// Criação:     06 Abr 2011
// Atualização: 19 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Usa funções do Direct3D 12 para acessar a GPU
//
**********************************************************************************/

#ifndef DXUT_GRAPHICS_H
#define DXUT_GRAPHICS_H

// --------------------------------------------------------------------------------
// Inclusões

#include <dxgi1_6.h>             // infraestrutura gráfica do DirectX
#include <d3d12.h>               // principais funções do Direct3D
#include "Window.h"              // cria e configura uma janela do Windows
#include "Types.h"               // tipos específicos da engine
#include <D3DCompiler.h>         // fornece D3DBlob

enum AllocationType { GPU, UPLOAD, CBUFFER };

// --------------------------------------------------------------------------------

class Graphics
{
private:
    // configuração
    static const uint            bufferCount = 2;           // número de buffers na swap chain (double, triple, etc.)
    uint                         bufferIndex;               // índice do buffer atual
    uint                         antialiasing;              // número de amostras para cada pixel na tela
    uint                         quality;                   // qualidade da amostragem de antialiasing
    bool                         vSync;                     // vertical sync 
    float                        bgColor[4];                // cor de fundo do backbuffer

    // pipeline
    ID3D12Device8              * device;                    // dispositivo gráfico
    IDXGIFactory7              * factory;                   // interface principal da DXGI
    IDXGISwapChain1            * swapChain;                 // swap chain
    
    ID3D12CommandQueue         * commandQueue;              // fila de comandos da GPU
    ID3D12GraphicsCommandList  * commandList;               // lista de comandos a submeter para GPU
    ID3D12CommandAllocator     * commandAlloc[bufferCount]; // alocadores da lista de comandos
     
    ID3D12Resource            ** renderTargets;             // buffers para renderização (front e back)
    ID3D12Resource             * depthStencil;              // buffer de profundidade e estampa            
    ID3D12DescriptorHeap       * renderTargetHeap;          // descriptor heap para os Render Targets
    ID3D12DescriptorHeap       * depthStencilHeap;          // descriptor heap para o Depth Stencil
    uint                         rtDescriptorSize;          // tamanho de cada descritor de Render Target
    D3D12_VIEWPORT               viewport;                  // viewport
    D3D12_RECT                   scissorRect;               // retângulo de corte

    // sincronização                         
    ID3D12Fence                * fence;                     // cerca para sincronizar CPU/GPU
    HANDLE                       fenceEvent;                // sinalizador de eventos
    ullong                       fenceValue[bufferCount];   // número atual da cerca

    // métodos privados
    void LogHardwareInfo();                                 // mostra informações do hardware

public:
    Graphics();                                             // constructor
    ~Graphics();                                            // destructor

    void VSync(bool state);                                 // liga/desliga vertical sync
    void Initialize(Window * window);                       // inicializa o Direct3D    
    void Clear();                                           // limpa o backbuffer com a cor de fundo
    void Present();                                         // apresenta desenho na tela
    
    void WaitForGpu();								        // espera execução da fila de comandos da GPU
    void SyncToGpu();							            // espera a liberação de um alocador de comandos

    void Allocate(uint type,
                  uint sizeInBytes, 
                  ID3D12Resource** resource);               // aloca recursos na memória de vídeo

    void Copy(const void* vertices, 
              uint sizeInBytes,
              ID3D12Resource* bufferUpload,
              ID3D12Resource* bufferGPU);                   // copia vértices para memória de vídeo

    ID3D12Device8* Device();                                // retorna dispositivo Direct3D
    ID3D12GraphicsCommandList* CommandList();               // retorna lista de comandos
    uint Antialiasing();                                    // retorna número de amostras por pixel
    uint Quality();                                         // retorna qualidade das amostras
};

// --------------------------------------------------------------------------------
// Métodos Inline

// liga/desliga vertical sync
inline void Graphics::VSync(bool state)
{ vSync = state; }

// retorna dispositivo Direct3D
inline ID3D12Device8* Graphics::Device()
{ return device; }

// retorna lista de comandos do dispositivo
inline ID3D12GraphicsCommandList* Graphics::CommandList()
{ return commandList; }

// retorna número de amostras por pixel
inline uint Graphics::Antialiasing()
{ return antialiasing; }

// retorna qualidade das amostras
inline uint Graphics::Quality()
{ return quality; }

// --------------------------------------------------------------------------------

#endif