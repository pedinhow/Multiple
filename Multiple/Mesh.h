/**********************************************************************************
// Mesh (Arquivo de Cabeçalho)
//
// Criação:     28 Abr 2016
// Atualização: 21 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Representa uma malha 3D
//
**********************************************************************************/

#ifndef DXUT_MESH_H_
#define DXUT_MESH_H_

// -------------------------------------------------------------------------------

#include "Types.h"
#include "Graphics.h"
#include "Geometry.h"

// -------------------------------------------------------------------------------

struct Mesh
{
    uint indexCount = 0;
    uint startIndex = 0;
    uint baseVertex = 0;

    Mesh() {}
    Mesh(const Geometry& geo) : indexCount(geo.IndexCount()) {}
};

// -------------------------------------------------------------------------------

#endif

