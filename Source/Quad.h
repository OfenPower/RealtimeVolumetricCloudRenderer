#pragma once

struct QuadVertexData;

struct Quad
{
    Quad();
    void Initialize(QuadVertexData* vertexData);

    // renders a 2D Quad
    void Draw();


public:
    unsigned int VAO;
    QuadVertexData* quadVertexData;
};