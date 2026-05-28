/**********************************************************************************
// Geometry (Arquivo de Cabeçalho)
//
// Criação:     03 Fev 2013
// Atualização: 05 Mar 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Define vértices e índices para várias geometrias
//
**********************************************************************************/

#ifndef DXUT_GEOMETRY_H_
#define DXUT_GEOMETRY_H_

// -------------------------------------------------------------------------------

#include "Types.h"
#include <vector>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "Colors.h"
using namespace DirectX;
using std::vector;

// -------------------------------------------------------------------------------

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

// -------------------------------------------------------------------------------
// Geometry
// -------------------------------------------------------------------------------

class Geometry
{
protected:
    void Subdivide();                       // subdivide triângulos

public:
    vector<Vertex> vertices;                // vértices da geometria
    vector<uint> indices;                   // índices da geometria

    // métodos inline
    const Vertex* VertexData() const        // retorna vértices da geometria
    { return vertices.data(); }

    const uint* IndexData() const           // retorna índices da geometria
    { return indices.data(); }

    uint VertexCount() const                // retorna número de vértices
    { return uint(vertices.size()); }

    uint IndexCount() const                 // retorna número de índices
    { return uint(indices.size()); }
};

// -------------------------------------------------------------------------------
// Box
// -------------------------------------------------------------------------------

struct Box : public Geometry
{
    Box(float width,                        // largura
        float height,                       // altura
        float depth,                        // profundidade
        XMFLOAT4 color = Wood);             // cor
};

// -------------------------------------------------------------------------------
// Cylinder
// -------------------------------------------------------------------------------

struct Cylinder : public Geometry
{
    Cylinder(float bottom,                  // raio inferior
        float top,                          // raio superior
        float height,                       // altura
        uint sliceCount,                    // número de fatias
        uint stackCount,                    // número de camadas
        XMFLOAT4 color = Yellow);           // cor
};

// -------------------------------------------------------------------------------
// Sphere
// -------------------------------------------------------------------------------

struct Sphere : public Geometry
{
    Sphere(float radius,                    // raio da esfera
        uint sliceCount,                    // número de fatias
        uint stackCount,                    // número de camadas
        XMFLOAT4 color = Crimson);          // cor
};

// -------------------------------------------------------------------------------
// GeoSphere
// -------------------------------------------------------------------------------

struct GeoSphere : public Geometry
{
    GeoSphere(float radius,                 // raio da geoesfera
        uint subdivisions,                  // número de subdivisões (máximo 6)
        XMFLOAT4 color = Orange);           // cor
};


// -------------------------------------------------------------------------------
// Grid
// -------------------------------------------------------------------------------

struct Grid : public Geometry
{
    Grid(float width,                       // largura
        float depth,                        // profundidade
        uint m,                             // número de linhas
        uint n,                             // número de colunas
        XMFLOAT4 color = Gray);             // cor
};

// -------------------------------------------------------------------------------
// Quad
// -------------------------------------------------------------------------------

struct Quad : public Geometry
{
    Quad(float width,                       // largura
        float height,                       // altura
        XMFLOAT4 color = Green);            // cor
};

// -------------------------------------------------------------------------------

#endif
