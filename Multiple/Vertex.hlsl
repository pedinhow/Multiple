/**********************************************************************************
// Vertex (Arquivo de Sombreamento)
//
// Criação:     22 Jul 2020
// Atualização: 08 Set 2023
// Compilador:  Direct3D Shader Compiler (FXC)
//
// Descrição:   Um vertex shader que faz a transformação de vértices
//              a partir de uma matriz combinada WorldViewProj fornecida
//
**********************************************************************************/

cbuffer Object
{
    float4x4 WorldViewProj;
};

struct VertexIn
{
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOutput output;
    output.pos = mul(float4(input.pos, 1.0f), WorldViewProj);
    
    // Se a propriedade alpha do override estiver preenchida, substitui a cor original
    if (ColorOverride.a > 0.5f)
        output.color = float4(ColorOverride.rgb, 1.0f);
    else
        output.color = input.color;
        
    return output;
}