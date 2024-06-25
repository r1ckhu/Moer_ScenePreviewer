#include "Mesh.h"

#include <iostream>

void components_to_vec3f(const std::vector<tinyobj::real_t>& components,
                         std::vector<Vector3f>& dst);
void components_to_vec2f(const std::vector<tinyobj::real_t>& components,
                         std::vector<Vector2f>& dst);
void load_triangle_faces(const tinyobj::shape_t& shape,
                         std::vector<std::vector<Vector3i>>& faces);

Mesh::Mesh() {}

Mesh::Mesh(const std::string& file_path) {
   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> objmaterials;  // not used
   std::string warn, err;
   bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, &warn, &err,
                                   file_path.c_str());
   if (!err.empty()) {
      std::cerr << err << std::endl;
   } else if (!warn.empty()) {
      std::cerr << warn << std::endl;
   }
   if (!success) {
      std::cerr << "TinyObj load mesh failed." << std::endl;
      exit(1);
   }
   components_to_vec3f(attrib.vertices, this->vertices);
   components_to_vec3f(attrib.normals, this->normals);
   components_to_vec2f(attrib.texcoords, this->uvs);

   for (auto shape = shapes.begin(); shape != shapes.end(); shape++) {
      createOpenGLData(*shape);
      load_triangle_faces(*shape, this->faces);
   }
   std::cout << "Load model: " << file_path << " sucessfully." << std::endl;
}

size_t Mesh::faces_nr() { return faces.size(); }

size_t Mesh::vertices_nr() { return vertices.size(); }

Vector3f Mesh::normal(size_t iface, size_t nth_vertex) {
   return normals[faces[iface][nth_vertex][2]].normalized();
}

Vector3f Mesh::vertex(size_t iface, size_t nth_vertex) {
   return vertices[faces[iface][nth_vertex][0]];
}

Vector3f Mesh::vertex(size_t ivertex) { return vertices[ivertex]; }

void components_to_vec3f(const std::vector<tinyobj::real_t>& components,
                         std::vector<Vector3f>& dst) {
   if (components.size() % 3 != 0) {
      throw "Load Error";
   }
   size_t component_nr = components.size() / 3;
   dst.reserve(component_nr);
   for (size_t i = 0; i < component_nr; i++) {
      dst.push_back(Vector3f(components[i * 3], components[i * 3 + 1],
                             components[i * 3 + 2]));
   }
}

void Mesh::apply(Matrix4f transform) {
   for (size_t i = 0; i < openglVertices.size(); i++) {
      Vector4f v = (transform * openglVertices[i].homogeneous());
      openglVertices[i] = v.hnormalized();
   }
   Matrix4f normalTransform = transform.inverse().transpose();
   for (size_t i = 0; i < openglNormals.size(); i++) {
      Vector4f v = (normalTransform * openglNormals[i].homogeneous());
      openglNormals[i] = v.head<3>().normalized();
   }
}

void components_to_vec2f(const std::vector<tinyobj::real_t>& components,
                         std::vector<Vector2f>& dst) {
   if (components.size() % 2 != 0) {
      throw "Load Error";
   }
   size_t component_nr = components.size() / 2;
   dst.reserve(component_nr);
   for (size_t i = 0; i < component_nr; i++) {
      dst.push_back(Vector2f(components[i * 2], components[i * 2 + 1]));
   }
}

void Mesh::createOpenGLData(const tinyobj::shape_t& shape) {
   const std::vector<tinyobj::index_t>& indices = shape.mesh.indices;
   if (indices.size() % 3 != 0) {
      assert(0);
   }
   size_t indices_nr = indices.size() / 3;
   for (size_t i = 0; i < indices_nr; i++) {
      for (size_t j = 0; j < 3; j++) {
         int vertex_index = indices[3 * i + j].vertex_index;
         int normal_index = indices[3 * i + j].normal_index;
         openglVertices.push_back(vertices[vertex_index]);
         openglNormals.push_back(normals[normal_index]);
         openglIndex.push_back(3 * i + j);
      }
   }
}

void Mesh::createOpenGLDataFromFaces() {
   size_t faces_nr = faces.size();
   for (size_t i = 0; i < faces_nr; i++) {
      assert(faces[i].size() == 3);
      for (size_t j = 0; j < 3; j++) {
         int vertex_index = faces[i][j].x();
         int normal_index = faces[i][j].z();
         openglVertices.push_back(vertices[vertex_index]);
         openglNormals.push_back(normals[normal_index]);
         openglIndex.push_back(3 * i + j);
      }
   }
}

void load_triangle_faces(const tinyobj::shape_t& shape,
                         std::vector<std::vector<Vector3i>>& faces) {
   const std::vector<tinyobj::index_t>& indices = shape.mesh.indices;
   if (indices.size() % 3 != 0) {
      throw "Load Error";
   }
   size_t indices_nr = indices.size() / 3;
   for (size_t i = 0; i < indices_nr; i++) {
      std::vector<Vector3i> vertices;
      vertices.reserve(3);
      for (size_t j = 0; j < 3; j++) {
         vertices.push_back(Vector3i(indices[3 * i + j].vertex_index,
                                     indices[3 * i + j].texcoord_index,
                                     indices[3 * i + j].normal_index));
      }
      faces.push_back(vertices);
   }
}

Quad::Quad() : Mesh() {
   /*
   d(-0.5,0,-0.5)-----c(0.5,0,-0.5)
      |                    |
      |                    |
      |                    |
      |                    |
   a(-0.5,0,0.5)-----b(0.5,0,0.5)
   */
   vertices.push_back(Vector3f(-0.5f, 0, 0.5f));  // a
   vertices.push_back(Vector3f(0.5, 0, 0.5));
   vertices.push_back(Vector3f(0.5f, 0, -0.5f));
   vertices.push_back(Vector3f(-0.5f, 0, -0.5f));  // d

   normals.push_back(Vector3f(0, 1, 0));
   // no uv for now
   Vector3i a(0, 0, 0), b(1, 0, 0), c(2, 0, 0), d(3, 0, 0);
   faces.resize(2);  // two faces for a Quad
   faces[0].push_back(a);
   faces[0].push_back(b);
   faces[0].push_back(d);

   faces[1].push_back(b);
   faces[1].push_back(c);
   faces[1].push_back(d);
   createOpenGLDataFromFaces();
}

Cube::Cube() : Mesh() {
   /*
               h--------g
              /        /|
             /        / |
            e--------f  c
            |        | /
            |        |/
            a--------b
   (-0,5,0,0.5) (0.5,0,0.5)
   */
   // add a~h to vertices
   vertices.emplace_back(-0.5f, 0, 0.5f);   // a = 0
   vertices.emplace_back(0.5, 0, 0.5);      // b = 1
   vertices.emplace_back(0.5f, 0, -0.5f);   // c = 2
   vertices.emplace_back(-0.5f, 0, -0.5f);  // d = 3

   vertices.emplace_back(-0.5f, 1, 0.5f);   // e = 4
   vertices.emplace_back(0.5, 1, 0.5);      // f = 5
   vertices.emplace_back(0.5f, 1, -0.5f);   // g = 6
   vertices.emplace_back(-0.5f, 1, -0.5f);  // h = 7

   normals.emplace_back(0, 0, 1);   // n = 0
   normals.emplace_back(0, 0, -1);  // f = 1
   normals.emplace_back(-1, 0, 0);  // l = 2
   normals.emplace_back(1, 0, 0);   // r = 3
   normals.emplace_back(0, 1, 0);   // t = 4
   normals.emplace_back(0, -1, 0);  // b = 5

   // no nv for now, all uv_index = 0
   faces.resize(12);
   // create near plane
   // a, e, b
   faces[0].emplace_back(0, 0, 0);
   faces[0].emplace_back(4, 0, 0);
   faces[0].emplace_back(1, 0, 0);
   // e, f, b
   faces[1].emplace_back(4, 0, 0);
   faces[1].emplace_back(5, 0, 0);
   faces[1].emplace_back(1, 0, 0);

   // create far plane
   // d, c, h
   faces[2].emplace_back(3, 0, 1);
   faces[2].emplace_back(2, 0, 1);
   faces[2].emplace_back(7, 0, 1);
   // c, g, h
   faces[3].emplace_back(2, 0, 1);
   faces[3].emplace_back(6, 0, 1);
   faces[3].emplace_back(7, 0, 1);

   // create left plane
   // a, e, d
   faces[4].emplace_back(0, 0, 2);
   faces[4].emplace_back(4, 0, 2);
   faces[4].emplace_back(3, 0, 2);
   // e, h, d
   faces[5].emplace_back(4, 0, 2);
   faces[5].emplace_back(7, 0, 2);
   faces[5].emplace_back(3, 0, 2);

   // create right plane
   // b, f, c
   faces[6].emplace_back(1, 0, 3);
   faces[6].emplace_back(5, 0, 3);
   faces[6].emplace_back(2, 0, 3);
   // f, g, c
   faces[7].emplace_back(5, 0, 3);
   faces[7].emplace_back(6, 0, 3);
   faces[7].emplace_back(2, 0, 3);

   // create top plane
   // e, f, g
   faces[8].emplace_back(4, 0, 4);
   faces[8].emplace_back(5, 0, 4);
   faces[8].emplace_back(6, 0, 4);
   // e, g, h
   faces[9].emplace_back(4, 0, 4);
   faces[9].emplace_back(6, 0, 4);
   faces[9].emplace_back(7, 0, 4);

   // create bottom plane
   // a, b, c
   faces[10].emplace_back(0, 0, 5);
   faces[10].emplace_back(1, 0, 5);
   faces[10].emplace_back(2, 0, 5);
   // a, c, d
   faces[11].emplace_back(0, 0, 5);
   faces[11].emplace_back(2, 0, 5);
   faces[11].emplace_back(3, 0, 5);
   createOpenGLDataFromFaces();
}