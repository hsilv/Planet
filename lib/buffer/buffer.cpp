#pragma once
#include "buffer.h"

Color clearColor(0, 0, 0);
Color currentColor(0, 0, 0);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
void render(void *params);
SemaphoreHandle_t zbufferS;
std::vector<FastNoiseLite> noises;

struct RenderParameters
{
    const std::vector<vector3> &vertices;
    const std::vector<vector3> &normals;
    const std::vector<vector3> &textures;
    Uniforms &u;
    int startIndex;
    int endIndex;
    SemaphoreHandle_t semaphore;
};

bool checkFrag(const vector3 &f);
std::array<std::array<uint16_t, 128>, 160> zbuffer;
int calculateMiddleIndex(const std::vector<vector3> &vertices);

void clearBuffer()
{
    spr.fillSprite(clearColor.toHex());
}

void renderBuffer(const std::vector<vector3> &vertices, const std::vector<vector3>& textures, const std::vector<vector3> &normals, Uniforms &u, int wWidth, int wHeight)
{
    clearBuffer();
    int middleIndex = calculateMiddleIndex(vertices);
    SemaphoreHandle_t semaphoreTask1 = xSemaphoreCreateBinary();
    SemaphoreHandle_t semaphoreTask2 = xSemaphoreCreateBinary();

    RenderParameters r = {vertices, normals, textures, u, 0, middleIndex, semaphoreTask1};
    RenderParameters r2 = {vertices, normals, textures, u, middleIndex + 3, vertices.size(), semaphoreTask2};

    xTaskCreate(render, "Tarea 1", 4096, &r, 1, NULL);
    xTaskCreate(render, "Tarea 2", 4096, &r2, 1, NULL);

    xSemaphoreTake(semaphoreTask2, portMAX_DELAY);
    xSemaphoreTake(semaphoreTask1, portMAX_DELAY);

    spr.pushSprite(0, 0);

    for (int i = 0; i < zbuffer.size(); ++i)
    {
        for (int j = 0; j < zbuffer[i].size(); ++j)
        {
            zbuffer[i][j] = 65000;
        }
    }

    vSemaphoreDelete(semaphoreTask1);
    vSemaphoreDelete(semaphoreTask2);
}

void initBuffer()
{
    long terrainSeed = random(0, 2550);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    spr.createSprite(tft.width(), tft.height());
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Hybrid);
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    noise.SetCellularJitter(1.00);
    noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    noise.SetDomainWarpAmp(100.0);
    noise.SetFractalType(FastNoiseLite::FractalType_DomainWarpIndependent);
    noise.SetFractalOctaves(8);
    noise.SetFractalLacunarity(2.00);
    noise.SetFractalGain(0.50);
    noise.SetSeed(terrainSeed);
    noise.SetFrequency(0.005);
    noises.push_back(noise);
    FastNoiseLite noise2;
    noise2.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noise2.SetFrequency(0.005);
    noise2.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean);
    noise2.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    noise2.SetCellularJitter(1.00);
    noise2.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    noise2.SetDomainWarpAmp(100.0);
    noise2.SetFractalType(FastNoiseLite::FractalType_DomainWarpIndependent);
    noise2.SetFractalOctaves(8);
    noise2.SetFractalLacunarity(2.0);
    noise2.SetSeed(terrainSeed);
    noise2.SetFractalGain(0.5);
    noises.push_back(noise2);

    for (int i = 0; i < zbuffer.size(); ++i)
    {
        for (int j = 0; j < zbuffer[i].size(); ++j)
        {
            zbuffer[i][j] = 65000;
        }
    }
}

void pointBuffer(const Fragment &f)
{
    int x = static_cast<int>(f.position.x);
    int y = static_cast<int>(f.position.y);

    if (x >= 0 && x < static_cast<int>(spr.width()) && y >= 0 && y < static_cast<int>(spr.height()))
    {
        if (f.position.z * 10 <= zbuffer[x][y])
        {
            spr.drawPixel(x, y, f.color.toHex());
            zbuffer[x][y] = static_cast<uint16_t>(f.position.z * 10);
        };
    }
}

void setClearColorBuffer(Color newColor)
{
    clearColor = newColor;
}

void setCurrentColorBuffer(Color newColor)
{
    currentColor = newColor;
}

bool checkFrag(const vector3 &f)
{
    int x = static_cast<int>(f.x);
    int y = static_cast<int>(f.y);

    if (f.z * 10 <= zbuffer[x][y])
    {
        return true;
    }
    return false;
}

void render(void *params)
{
    RenderParameters *renderParams = static_cast<RenderParameters *>(params);
    int counter = 0;
    for (int z = renderParams->startIndex; z <= renderParams->endIndex; z += 3)
    {
        counter++;
        if (counter == 6)
        {
            delay(1);
            counter = 0;
        }
        std::vector<Vertex> transformedVertices;
        transformedVertices.reserve(3);
        std::vector<Vertex> origins;
        origins.reserve(3);
        for (int i = 0; i < 3; i += 1)
        {
            vector3 v = renderParams->vertices[z + i];
            vector3 n = renderParams->normals[z + i];
            vector3 t = renderParams->textures[z + i];
            vector3 c = vector3(0.6f, 0.6f, 0.6f);

            Vertex vertex = {v, n, t, Color(c.x, c.y, c.z)};
            origins.push_back(vertex);
            Vertex transformedVertex = vertexShader(vertex, renderParams->u);
            transformedVertices.push_back(transformedVertex);
        }
        std::vector<std::vector<Vertex>> triangles = primitiveAssembly(transformedVertices);

        transformedVertices.clear();
        transformedVertices.shrink_to_fit();
        rasterize(triangles, pointBuffer, checkFrag, noises);
        origins.clear();
        origins.shrink_to_fit();
        triangles.clear();
        triangles.shrink_to_fit();
    }
    xSemaphoreGive(renderParams->semaphore);
    vTaskDelete(NULL);
}

int calculateMiddleIndex(const std::vector<vector3> &vertices)
{
    int halfLength = vertices.size() / 2;
    int closestMultipleOfThree = (halfLength / 3) * 3;
    if (halfLength % 3 == 0)
    {
        return closestMultipleOfThree - 3;
    }
    return closestMultipleOfThree;
}
