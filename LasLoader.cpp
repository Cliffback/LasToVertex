// Created by Mathias Eek 2022

#include "LasLoader.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <stdio.h>
#include <iostream>

namespace mml {
    vec3::vec3(float xin, float yin, float zin) : x{ xin }, y{ yin }, z{ zin } {}
    vec3::vec3(const float i) : x{ i }, y{ i }, z{ i } {}

    const vec3& vec3::operator=(const vec3& vec) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        return *this;
    }
    const vec3& vec3::operator+=(const vec3& vec) {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        return *this;
    }
    const vec3& vec3::operator-=(const vec3& vec) {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        return *this;
    }
    /*const vec3& vec3::operator+(const vec3& vec) {
        return { x + vec.x, y + vec.y, z + vec.z };
    }*/
    const vec3& vec3::operator-(const vec3& vec) {
        return { x - vec.x, y - vec.y, z - vec.z };
    }
    const vec3& vec3::operator*(const vec3& vec) {
        return { x * vec.x, y * vec.y, z * vec.z };
    }
    const vec3& vec3::operator*(const float i) {
        return { x * i, y * i, z * i };
    }
    const vec3& vec3::operator/(const vec3& vec) {
        return { x / vec.x, y / vec.y, z / vec.z };
    }
    const vec3& vec3::operator/(const float i) {
        return { x / i, y / i, z / i };
    }
    const bool& vec3::operator!=(const vec3& vec) {
        return !(x == vec.x || y == vec.y || z == vec.z);
    }
    const bool& vec3::operator==(const vec3& vec) {
        return !(x != vec.x || y != vec.y || z != vec.z);
    }
    float vec3::length() const {
        return std::sqrt(std::pow(x, 2.f) + std::pow(y, 2.f) + std::pow(z, 2.f));
    }
    vec3 normalize(const vec3& vec) {
        vec3 temp;
        float mylength = vec.length();
        if (mylength > 0.f) {
            temp.x = vec.x / mylength;
            temp.y = vec.y / mylength;
            temp.z = vec.z / mylength;
        }
        return temp;
    }
    vec3 cross(const vec3& vec1, const vec3& vec2) {
        return {
            (vec1.y * vec2.z) - (vec1.z * vec2.y), 
            (vec1.z * vec2.x) - (vec1.x * vec2.z),
            (vec1.x * vec2.y) - (vec1.y * vec2.x)
        };
    }
    vec3 dot(const vec3& vec1, const vec3& vec2) {
        return {
            (vec1.x * vec2.x) +
            (vec1.y * vec2.y) +
            (vec1.z * vec2.z)
        };
    }
    const vec3& operator+(const vec3& vec1, const vec3& vec2) {
        return { vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z };
    }
}

namespace LAS {

    LasLoader::LasLoader(const std::string& path) : PointData{} {

        std::string txt(".txt");
        std::string lasbin(".lasbin");
        std::string las(".las");

        if (path.find(txt) != std::string::npos)
            ReadTxt(path);
        else if (path.find(lasbin) != std::string::npos)
            ReadBin(path);
        else if (path.find(las) != std::string::npos)
            ReadLas(path);
        // TODO: Dont calc center when reading .las (already in header)

        CalcCenter();
        UpdatePoints();
        Triangulate();
    }

    std::vector<ColorVertex> LasLoader::GetPointData() {
        return LasLoader::PointData;
    }


    void LasLoader::FindMinMax() {

        // Check if min/max already found
        if (max != mml::vec3(0.f)) {
            return;
        }
        min = mml::vec3(std::numeric_limits<float>::max());
        max = mml::vec3(std::numeric_limits<float>::min());

        for (auto& vertex : PointData) {
            if (min.x > vertex.Pos.x) { min.x = vertex.Pos.x; }
            else if (max.x < vertex.Pos.x) { max.x = vertex.Pos.x; }
            if (min.z > vertex.Pos.z) { min.z = vertex.Pos.z; }
            else if (max.z < vertex.Pos.z) { max.z = vertex.Pos.z; }
            if (min.y > vertex.Pos.y) { min.y = vertex.Pos.y; }
            else if (max.y < vertex.Pos.y) { max.y = vertex.Pos.y; }
        }
    }

    void LasLoader::CalcCenter() {

        FindMinMax();

        // Find middle

        mml::vec3 size = (min + (max - min));
        middle = size / 2.f;
        offset = min;

        // Update min/max
        min -= offset;
        max -= offset;
    }

    void LasLoader::UpdatePoints() {

        for (auto& vertex : PointData) {
            if (middle != mml::vec3(0.f))
                vertex.Pos -= offset;
        }
    }

    void LasLoader::Triangulate() {

        // width and height
        xSquares = (max.x - min.x);
        zSquares = (max.z - min.z);

        // Save all height data for each vertex
        std::vector<std::vector<HeightAndColor>> heightmap(zSquares, std::vector<HeightAndColor>(xSquares));
        for (auto& vertex : PointData) {
            int xPos = vertex.Pos.x;
            int zPos = vertex.Pos.z;

            if (xPos < 0.f || xPos > xSquares - 1
                || zPos < 0.f || zPos > zSquares - 1) {
                continue;
            }

            // Instead of push back, add height and increment
            heightmap[zPos][xPos].count++;
            heightmap[zPos][xPos].sum += vertex.Pos.y;
            heightmap[zPos][xPos].color += vertex.Color;
        }

        std::vector<std::pair<int, int>> noHeight;

        // Calculate average height for each vertex and push
        for (int z = 0; z < zSquares; ++z) {
            for (int x = 0; x < xSquares; ++x) {
                float y;
                mml::vec3 color{};
                if (heightmap[z][x].count == 0) {
                    y = -max.y;
                    noHeight.push_back(std::make_pair(x, z));
                    color = mml::vec3(1.f);
                }
                else {
                    //y = (average / count) - max.y;
                    y = heightmap[z][x].sum / heightmap[z][x].count - max.y;
                    color = heightmap[z][x].color / mml::vec3(heightmap[z][x].count);
                }
                MeshVertex temp{};
                ColorNormalVertex temp2{};
                temp.Pos = mml::vec3(x, y, z);
                temp2.Pos = temp.Pos;
                temp2.Color = color;
                VertexData.push_back(temp);
                ColorNormalVertexData.push_back(temp2);
            }
        }

        // Calculate average height if no height
        for (auto& vertex : noHeight) {
            int x = vertex.first;
            int z = vertex.second;
            if (x <= 0 || x >= xSquares - 1 || z <= 0 || z >= zSquares - 1) {
                continue;
            }
            else {
                float averageHeight{ 0.f };
                averageHeight += VertexData[(x - 1) + (z * xSquares)].Pos.y;
                averageHeight += VertexData[(x - 1) + ((z - 1) * xSquares)].Pos.y;
                averageHeight += VertexData[x + ((z - 1) * xSquares)].Pos.y;
                averageHeight += VertexData[(x + 1) + ((z - 1) * xSquares)].Pos.y;
                averageHeight += VertexData[(x + 1) + (z * xSquares)].Pos.y;
                averageHeight += VertexData[(x + 1) + ((z + 1) * xSquares)].Pos.y;
                averageHeight += VertexData[x + ((z + 1) * xSquares)].Pos.y;
                averageHeight += VertexData[(x - 1) + ((z + 1) * xSquares)].Pos.y;

                mml::vec3 averageColor{};
                averageColor += ColorNormalVertexData[(x - 1) + (z * xSquares)].Color;
                averageColor += ColorNormalVertexData[(x - 1) + ((z - 1) * xSquares)].Color;
                averageColor += ColorNormalVertexData[x + ((z - 1) * xSquares)].Color;
                averageColor += ColorNormalVertexData[(x + 1) + ((z - 1) * xSquares)].Color;
                averageColor += ColorNormalVertexData[(x + 1) + (z * xSquares)].Color;
                averageColor += ColorNormalVertexData[(x + 1) + ((z + 1) * xSquares)].Color;
                averageColor += ColorNormalVertexData[x + ((z + 1) * xSquares)].Color;
                averageColor += ColorNormalVertexData[(x - 1) + ((z + 1) * xSquares)].Color;

                VertexData[x + (z * xSquares)].Pos.y = averageHeight / 8.f;
                ColorNormalVertexData[x + (z * xSquares)].Pos.y = averageHeight / 8.f;
                ColorNormalVertexData[x + (z * xSquares)].Color = averageColor / 8.f;
            }
        }

        // Create Index
        for (int z = 0; z < zSquares - 1; ++z) {
            for (int x = 0; x < xSquares - 1; ++x) {
                IndexData.emplace_back(x + (xSquares * z));
                IndexData.emplace_back(x + 1 + (xSquares * (z + 1)));
                IndexData.emplace_back(x + 1 + (xSquares * z));

                IndexData.emplace_back(x + (xSquares * z));
                IndexData.emplace_back(x + (xSquares * (z + 1)));
                IndexData.emplace_back(x + 1 + (xSquares * (z + 1)));
            }
        }

        // Calculate smooth normals
        int xmin = 0, xmax = xSquares, zmin = 0, zmax = zSquares; // The size to draw

        // Normals for rest
        for (int z = zmin; z < zmax; z++) {
            for (int x = xmin; x < xmax; x++) {
                if (z == zmin || z == zmax - 1 || x == xmin || x == xmax - 1) {
                    VertexData[x + (xmax * z)].Normal = mml::vec3(0.f, 1.f, 0.f);
                }
                else {
                    mml::vec3 a(VertexData[x + (xmax * z)].Pos);
                    mml::vec3 b(VertexData[x + 1 + (xmax * z)].Pos);
                    mml::vec3 c(VertexData[x + 1 + (xmax * (z + 1))].Pos);
                    mml::vec3 d(VertexData[x + (xmax * (z + 1))].Pos);
                    mml::vec3 e(VertexData[x - 1 + (xmax * z)].Pos);
                    mml::vec3 f(VertexData[x - 1 + (xmax * (z - 1))].Pos);
                    mml::vec3 g(VertexData[x + (xmax * (z - 1))].Pos);

                    auto n0 = mml::cross(c - a, b - a);
                    auto n1 = mml::cross(d - a, c - a);
                    auto n2 = mml::cross(e - a, d - a);
                    auto n3 = mml::cross(f - a, e - a);
                    auto n4 = mml::cross(g - a, f - a);
                    auto n5 = mml::cross(b - a, g - a);

                    mml::vec3 normal = n0 + n1 + n2 + n3 + n4 + n5;
                    normal = mml::normalize(normal);

                    VertexData[x + (xmax * z)].Normal = normal;
                    ColorNormalVertexData[x + (xmax * z)].Normal = normal;
                }
            }
        }
    }


    std::pair<std::vector<MeshVertex>, std::vector<uint32_t>> LasLoader::GetIndexedData() {
        return { LasLoader::VertexData, LasLoader::IndexData };
    }

    std::vector<MeshVertex> LasLoader::GetVertexData() {
        std::vector<MeshVertex> out;
        int i = 0;
        while (i != IndexData.size()) {
            MeshVertex tempA;
            tempA = VertexData[IndexData[i]];
            i++;

            MeshVertex tempB;
            tempB = VertexData[IndexData[i]];
            i++;

            MeshVertex tempC;
            tempC = VertexData[IndexData[i]];
            i++;

            auto ab = tempB.Pos - tempA.Pos;
            auto ac = tempC.Pos - tempA.Pos;

            mml::vec3 normal = mml::normalize(mml::cross(ab, ac));
            tempA.Normal = normal;
            tempB.Normal = normal;
            tempC.Normal = normal;

            tempA.UV.x = tempA.Pos.x;
            tempA.UV.y = tempA.Pos.y;
            tempB.UV.x = tempB.Pos.x;
            tempB.UV.y = tempB.Pos.y;
            tempC.UV.x = tempC.Pos.x;
            tempC.UV.y = tempC.Pos.y;

            out.push_back(tempA);
            out.push_back(tempB);
            out.push_back(tempC);
        }
        return out;
    }

    std::pair<std::vector<ColorNormalVertex>, std::vector<uint32_t>> LasLoader::GetIndexedColorNormalVertexData() {
        return { ColorNormalVertexData, IndexData };
    }

    std::vector<std::vector<std::pair<Triangle, Triangle>>> LasLoader::GetTerrainData() {

        int width = (max.x - min.x);
        int height = (max.z - min.z);

        std::vector<std::vector<std::pair<Triangle, Triangle>>> out(height - 1,
            std::vector<std::pair<Triangle, Triangle>>(width - 1));

        for (int z = 0; z < height - 1; z++) {
            for (int x = 0; x < width - 1; x++) {
                Triangle bottom;
                Triangle top;

                int aIndex = (x + (z * width));
                int bIndex = ((x + 1) + (z * width));
                int cIndex = ((x + 1) + ((z + 1) * width));
                int dIndex = (x + ((z + 1) * width));

                bottom.A = VertexData[aIndex].Pos;
                bottom.B = VertexData[cIndex].Pos;
                bottom.C = VertexData[bIndex].Pos;

                top.A = VertexData[aIndex].Pos;
                top.B = VertexData[dIndex].Pos;
                top.C = VertexData[cIndex].Pos;

                bottom.N = mml::normalize(mml::cross(bottom.B - bottom.A, bottom.C - bottom.A));
                top.N = mml::normalize(mml::cross(top.B - top.A, top.C - top.A));

                out[z][x] = std::make_pair(bottom, top);
            }
        }
        return out;
    }

    void LasLoader::ReadTxt(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cout << "Cant open file: " << path << std::endl;
            return;
        }
        std::string line;
        ColorVertex tempVertex{};
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            ss >> tempVertex.Pos.x;
            ss >> tempVertex.Pos.z;
            ss >> tempVertex.Pos.y;
            tempVertex.Color = mml::vec3(0.f, 1.f, 0.f);
            PointData.push_back(tempVertex);
        }
    }

    void LasLoader::ReadBin(const std::string& path) {

        std::ifstream is(path, std::ios::binary | std::ios::ate);
        auto size = is.tellg();
        std::vector<mml::vec3> lasDataPoints(size / sizeof(mml::vec3));
        is.seekg(0);
        is.read(reinterpret_cast<char*>(lasDataPoints.data()), size);

        for (auto& point : lasDataPoints) {
            ColorVertex tempVertex{};
            tempVertex.Pos.x = point.x;
            tempVertex.Pos.y = point.z;
            tempVertex.Pos.z = point.y;
            tempVertex.Color = mml::vec3(1.f, 1.f, 1.f);
            PointData.push_back(tempVertex);
        }
    }

    void LasLoader::ReadLas(const std::string& path) {
        lasHeader header;
        std::ifstream inf(path, std::ios::binary | std::ios::ate);
        if (inf.is_open()) {

            // Read Header
            inf.seekg(0);
            inf.read((char*)&header.fileSignature, sizeof(header.fileSignature));
            inf.read((char*)&header.sourceID, sizeof(header.sourceID));
            inf.read((char*)&header.globalEncoding, sizeof(header.globalEncoding));
            inf.read((char*)&header.GUID1, sizeof(header.GUID1));
            inf.read((char*)&header.GUID2, sizeof(header.GUID2));
            inf.read((char*)&header.GUID3, sizeof(header.GUID3));
            inf.read((char*)&header.GUID4, sizeof(header.GUID4));
            inf.read((char*)&header.versionMajor, sizeof(header.versionMajor));
            inf.read((char*)&header.versionMinor, sizeof(header.versionMinor));
            inf.read((char*)&header.systemIdentifier, sizeof(header.systemIdentifier));
            inf.read((char*)&header.generatingSoftware, sizeof(header.generatingSoftware));
            inf.read((char*)&header.creationDay, sizeof(header.creationDay));
            inf.read((char*)&header.creationYear, sizeof(header.creationYear));
            inf.read((char*)&header.headerSize, sizeof(header.headerSize));
            inf.read((char*)&header.offsetToPointData, sizeof(header.offsetToPointData));
            inf.read((char*)&header.numberVariableLengthRecords, sizeof(header.numberVariableLengthRecords));
            inf.read((char*)&header.pointDataRecordFormat, sizeof(header.pointDataRecordFormat));
            inf.read((char*)&header.pointDataRecordLength, sizeof(header.pointDataRecordLength));
            inf.read((char*)&header.legacyNumberPointsRecords, sizeof(header.legacyNumberPointsRecords));
            inf.read((char*)&header.legacyNumberPointReturn, sizeof(header.legacyNumberPointReturn));
            inf.read((char*)&header.xScaleFactor, sizeof(header.xScaleFactor));
            inf.read((char*)&header.yScaleFactor, sizeof(header.yScaleFactor));
            inf.read((char*)&header.zScaleFactor, sizeof(header.zScaleFactor));
            inf.read((char*)&header.xOffset, sizeof(header.xOffset));
            inf.read((char*)&header.yOffset, sizeof(header.yOffset));
            inf.read((char*)&header.zOffset, sizeof(header.zOffset));
            inf.read((char*)&header.maxX, sizeof(header.maxX));
            inf.read((char*)&header.minX, sizeof(header.minX));
            inf.read((char*)&header.maxY, sizeof(header.maxY));
            inf.read((char*)&header.minY, sizeof(header.minY));
            inf.read((char*)&header.maxZ, sizeof(header.maxZ));
            inf.read((char*)&header.minZ, sizeof(header.minZ));

            // Save max and min from header (so that we don't need to calculate it later)
            min.x = header.minX;
            min.y = header.minZ;
            min.z = header.minY;
            max.x = header.maxX;
            max.y = header.maxZ;
            max.z = header.maxY;

            // Read pointdata
            inf.seekg(header.offsetToPointData);

            // Only format 1 and 2 supported
            ASSERT(header.pointDataRecordFormat == 1 || header.pointDataRecordFormat == 2);

            // Read format 1
            if (header.pointDataRecordFormat == 1) {
                for (int i = 0; i < header.legacyNumberPointsRecords; ++i) {
                    lasPointData1 temp;
                    inf.seekg(header.offsetToPointData + (header.pointDataRecordLength * i));
                    inf.read((char*)&temp.xPos, sizeof(temp.xPos));
                    inf.read((char*)&temp.yPos, sizeof(temp.yPos));
                    inf.read((char*)&temp.zPos, sizeof(temp.zPos));

                    ColorVertex tempVertex{};
                    // Final position = (pos * scale factor) + offset
                    tempVertex.Pos.x = (temp.xPos * header.xScaleFactor) + header.xOffset;
                    tempVertex.Pos.y = (temp.zPos * header.zScaleFactor) + header.zOffset;
                    tempVertex.Pos.z = (temp.yPos * header.yScaleFactor) + header.yOffset;
                    tempVertex.Color = mml::vec3(0.f, 1.f, 0.f);
                    PointData.push_back(tempVertex);
                }
            }
            // Read format 2
            else if (header.pointDataRecordFormat == 2) {
                for (int i = 0; i < header.legacyNumberPointsRecords; ++i) {
                    lasPointData2 temp;
                    inf.read((char*)&temp.xPos, sizeof(temp.xPos));
                    inf.read((char*)&temp.yPos, sizeof(temp.yPos));
                    inf.read((char*)&temp.zPos, sizeof(temp.zPos));
                    inf.read((char*)&temp.intensity, sizeof(temp.intensity));
                    inf.read((char*)&temp.flags, sizeof(temp.flags));
                    inf.read((char*)&temp.classificaton, sizeof(temp.classificaton));
                    inf.read((char*)&temp.scanAngle, sizeof(temp.scanAngle));
                    inf.read((char*)&temp.userData, sizeof(temp.userData));
                    inf.read((char*)&temp.pointSourceID, sizeof(temp.pointSourceID));
                    inf.read((char*)&temp.red, sizeof(temp.red));
                    inf.read((char*)&temp.green, sizeof(temp.green));
                    inf.read((char*)&temp.blue, sizeof(temp.blue));

                    ColorVertex tempVertex{};
                    tempVertex.Pos.x = (temp.xPos * header.xScaleFactor) + header.xOffset;
                    tempVertex.Pos.y = (temp.zPos * header.zScaleFactor) + header.zOffset;
                    tempVertex.Pos.z = (temp.yPos * header.yScaleFactor) + header.yOffset;
                    tempVertex.Color = mml::vec3(temp.red * 0.00001, temp.green * 0.00001, temp.blue * 0.00001);
                    PointData.push_back(tempVertex);
                }
            }
        }
    }
}