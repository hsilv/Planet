#include "triangle.h"

vector3 L = {0.0f, 0.0f, -5.0f};
float persistence = 0.25;
float octaves = 4;
uint16_t gridWidth = 160;
uint16_t gridHeight = 120;

void triangle(Vertex a, Vertex b, Vertex c, void (*func)(const Fragment &), bool (*check)(const vector3 &), std::vector<FastNoiseLite> noises)
{
    vector3 A = a.position;
    vector3 B = b.position;
    vector3 C = c.position;
    vector3 TA = vector3(a.originals.x * gridWidth, a.originals.y * gridHeight, 0);
    vector3 TB = vector3(b.originals.x * gridWidth, b.originals.y * gridHeight, 0);
    vector3 TC = vector3(c.originals.x * gridWidth, c.originals.y * gridHeight, 0);

    int minX = std::min(std::min(A.x, B.x), C.x);
    int minY = std::min(std::min(A.y, B.y), C.y);
    int maxX = std::max(std::max(A.x, B.x), C.x);
    int maxY = std::max(std::max(A.y, B.y), C.y);

    L = L.normalize();

    for (float y = minY; y <= maxY; y++)
    {
        for (float x = minX; x <= maxX; x++)
        {
            vector3 P = vector3(x, y, 0);

            vector3 bar = barycentricCoordinates(P, A, B, C);

            if (
                bar.x <= 1 && bar.x >= 0 &&
                bar.y <= 1 && bar.y >= 0 &&
                bar.z <= 1 && bar.z >= 0)
            {
                float u = TA.x * bar.x + TB.x * bar.y + TC.x * bar.z;
                float v = bar.x * TA.y + bar.y * TB.y + bar.z * TC.y;

                vector3 normal = vector3(a.normal * bar.x + b.normal * bar.y + c.normal * bar.z);
                normal = normal.normalize();
                float intensity = normal.dot(L);
                P.z = a.position.z * bar.x + b.position.z * bar.y + c.position.z * bar.z;
                if (!(intensity < 0))
                {
                    /* Color color = a.color * bar.x + b.color * bar.y + c.color * bar.z; */
                    Color color;
                    /* Serial.printf("x: %lf, y: %lf, barx: %lf, bary: %lf, id: %i \n", TA.x, TB.x, TC.x, u, id); */
                    float noiseVal = noises[0].GetNoise(u * 35, v * 35);
                    if (noiseVal >= 0)
                    {
                        color = Color(0, 115, 0);
                        Color sand = Color(128, 99, 50);
                        float beachIndex = (1 + noises[1].GetNoise(u * 35, v * 35));
                        color = sand * beachIndex + color * (1.0 - beachIndex);
                    }
                    else
                    {
                        color = Color(0, 0, 128);
                    }
                    float cloudVal = (1 + noises[2].GetNoise(u*15, v*15)) * 1.2;
                    Color cloud = Color(76, 76, 76);
                    color = color + cloud * cloudVal;
                    color = color * intensity;
                    func(Fragment{P, color});
                }
            }
        }
    }

    return;
}

vector3 barycentricCoordinates(const vector3 &P, const vector3 &A, const vector3 &B, const vector3 &C)
{
    float w = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) /
              ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));

    float v = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) /
              ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));

    float u = 1.0f - w - v;

    return vector3(w, v, u);
}