#pragma once
#include <glad/glad.h>

#include <memory>

#include "Mesh.h"

class VertexArrayObject {
  protected:
   GLuint VBO, VAO, EBO;
   int indexCnt;

  public:
   void create_buffers(std::shared_ptr<Mesh> mesh);
   void delete_buffers();
   void bind();
   void unbind();
   void draw();
};

class QuadVertexArrayObject : public VertexArrayObject {
  public:
   QuadVertexArrayObject();
   void draw();
};