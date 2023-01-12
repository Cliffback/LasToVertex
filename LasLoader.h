#pragma once
#include <string>
#include <vector>
// Created by Mathias Eek 2022

#include <cassert>
#include <iostream>
#include "glm/glm.hpp"

namespace mml {
    class vec3 {
    public:
        vec3(float xin, float yin, float zin);
        vec3(const float i = 0.f);

        const vec3& operator=(const vec3& vec);
        const vec3& operator+=(const vec3& vec);
        const vec3& operator-=(const vec3& vec);
        const vec3& operator+(const vec3& vec);
        const vec3& operator-(const vec3& vec);
        const vec3& operator*(const vec3& vec);
        const vec3& operator*(const float i);
        const vec3& operator/(const vec3& vec);

        float length() const;
        void normalize();

        vec3 cross(const vec3& vec1, const vec3& vec2);
        vec3 dot(const vec3& vec1, const vec3& vec2);

        float x;
        float y;
        float z;

    private:
    };

    struct vec2 {
        float x;
        float y;
        float z;
    };
}

namespace LAS {
#define LOG(msg) std::cout << msg
#ifndef NDEBUG
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif // !NDEBUG

   
    struct MeshVertex {
        mml::vec3 Pos{1.f};
        mml::vec3 Normal{};
        mml::vec2 UV{};
    };

    struct ColorVertex {
        mml::vec3 Pos{};
        mml::vec3 Color{};
    };

    struct ColorNormalVertex {
        mml::vec3 Pos{};
        mml::vec3 Color{};
        mml::vec3 Normal{};
    };

    struct Triangle {
        mml::vec3 A;
        mml::vec3 B;
        mml::vec3 C;
        mml::vec3 N;
    };

    class LasLoader {

    public:
        LasLoader(const std::string& path);
        std::vector<ColorVertex> GetPointData();
        std::pair<std::vector<MeshVertex>, std::vector<uint32_t>> GetIndexedData();
        std::vector<MeshVertex> GetVertexData();
        std::pair<std::vector<ColorNormalVertex>, std::vector<uint32_t>> GetIndexedColorNormalVertexData();
        std::vector<std::vector<std::pair<Triangle, Triangle>>> GetTerrainData();
        float GetMinY() { return -max.y; }
    private:
        std::vector<ColorVertex> PointData;
        std::vector<MeshVertex> VertexData;
        std::vector<ColorNormalVertex> ColorNormalVertexData;
        std::vector<uint32_t> IndexData;
        std::vector<MeshVertex> TriangulatedVertexData;
        std::vector<Triangle> triangles;

        void ReadTxt(const std::string& path);
        void ReadBin(const std::string& path);
        void ReadLas(const std::string& path);

        void CalcCenter();
        void FindMinMax();
        void UpdatePoints();
        void Triangulate();

        mml::vec3 min;
        mml::vec3 max;
        mml::vec3 middle;
        mml::vec3 offset;
        int xSquares{ 0 };
        int zSquares{ 0 };
    };

    struct HeightAndColor {
        int count{ 0 };
        float sum{ 0.f };
        mml::vec3 color{ 0.f };
    };

    // Can't use struct directly because of padding of the size of the struct
    struct lasHeader {
        char fileSignature[4];
        uint16_t sourceID;
        uint16_t globalEncoding;
        uint32_t GUID1;
        uint16_t GUID2;
        uint16_t GUID3;
        uint8_t GUID4[8];
        uint8_t versionMajor;
        uint8_t versionMinor;
        char systemIdentifier[32];
        char generatingSoftware[32];
        uint16_t creationDay;
        uint16_t creationYear;
        uint16_t headerSize;
        uint32_t offsetToPointData;
        uint32_t numberVariableLengthRecords;
        uint8_t pointDataRecordFormat;
        uint16_t pointDataRecordLength;
        int32_t legacyNumberPointsRecords;
        int32_t legacyNumberPointReturn[5];
        double xScaleFactor, yScaleFactor, zScaleFactor;
        double xOffset, yOffset, zOffset;
        double maxX, minX;
        double maxY, minY;
        double maxZ, minZ;
    };

    // Not needed
    struct lasVariableLengthRecords {
        // Variable Length Records
        uint16_t lasReserved;
        char UserID[16];
        uint16_t recordID;
        uint16_t recordLengthAfterHeader;
        char lasDescription[32];
    };

    // Can't use struct directly because of padding of the size of the struct
    struct lasPointData1 {
        int32_t xPos;
        int32_t yPos;
        int32_t zPos;
        uint16_t intensity;
        int8_t flags;
        uint8_t classificaton;
        int8_t scanAngle;
        uint8_t userData;
        uint16_t pointSourceID;
        double GPSTime;
    };

    // Can't use struct directly because of padding of the size of the struct
    struct lasPointData2 {
        int32_t xPos;
        int32_t yPos;
        int32_t zPos;
        uint16_t intensity;
        int8_t flags;
        uint8_t classificaton;
        int8_t scanAngle;
        uint8_t userData;
        uint16_t pointSourceID;
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };

}