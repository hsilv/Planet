#include "vertexShader.h"

void printVector(const vector3 &v, const std::string &name);

Vertex vertexShader(const Vertex &vertex, const Uniforms &uniforms)
{
    vector4 r = uniforms.viewport * uniforms.projection * uniforms.view * uniforms.model * vector4(vertex.position.x, vertex.position.y, vertex.position.z, 1);
    vector4 n = uniforms.model * vector4(vertex.normal.x, vertex.normal.y, vertex.normal.z, 1);
    vector3 normal = vector3(n.x, n.y, n.z);
    normal = normal.normalize();
    return Vertex{
        vector3(r.x / r.w, r.y / r.w, r.z * r.x),
        normal,
        vertex.color};
}

void printVector(const vector3 &v, const std::string &name){
    Serial.printf("Vector %s: x: %lf, y: %lf, z: %lf \n", name.c_str(), v.x, v.y, v.z);
}