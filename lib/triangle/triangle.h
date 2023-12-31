#ifndef TRIANGLE_H
#define TRIANGLE_H
#include <stevesch-MathBase.h>
#include <stevesch-MathVec.h>
#include "fragment.h"
#include "vertex3.h"
#include "FastNoise.h"
using stevesch::vector3;

void triangle(Vertex a, Vertex b, Vertex c, void (*func)(const Fragment &), bool (*check)(const vector3 &), std::vector<FastNoiseLite> noises);
vector3 barycentricCoordinates(const vector3& P, const vector3& A, const vector3& B, const vector3& C);

#endif