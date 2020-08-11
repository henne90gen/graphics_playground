#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

/* https://www.esri.com/library/whitepapers/pdfs/shapefile.pdf */
struct ShpHeader {
    int32_t fileCode;   // big endian
    int32_t unused[5];  // big endian
    int32_t fileLength; // big endian
    int32_t version;    // little endian
    int32_t shapeType;  // little endian
    double minX;        // little endian
    double minY;        // little endian
    double maxX;        // little endian
    double maxY;        // little endian
    double minZ;        // little endian, optional
    double maxZ;        // little endian, optional
    double minM;        // little endian, optional
    double maxM;        // little endian, optional
};

int32_t swapOrder(int32_t in) {
    unsigned char buffer[4];
    for (int i = 0; i < 4; i++) {
        buffer[3 - i] = (in >> (i * 8));
    }
    int32_t num = (int)buffer[0] | (int)buffer[1] << 8 | (int)buffer[2] << 16 | (int)buffer[3] << 24;
    return num;
}

std::istream &operator>>(std::istream &is, ShpHeader &header) {
    is.read(reinterpret_cast<char *>(&header), sizeof(header) - 4 * sizeof(double));
    if (is.fail()) {
        std::cout << "Failed to parse fileCode" << std::endl;
        return is;
    }
    header.fileCode = swapOrder(header.fileCode);
    header.fileLength = swapOrder(header.fileLength);

    if (header.shapeType == 11 || header.shapeType == 13 || header.shapeType == 15 || header.shapeType == 18) {
        is.read(reinterpret_cast<char *>(&header.minZ), 2 * sizeof(double));
    }
    if (header.shapeType == 21 || header.shapeType == 23 || header.shapeType == 25 || header.shapeType == 28) {
        is.read(reinterpret_cast<char *>(&header.minM), 2 * sizeof(double));
    }

    std::cout << header.fileCode << ", " << header.fileLength << ", " << header.version << ", " << header.shapeType
              << std::endl;

    return is;
}

struct ShiHeader {};

/*
    https://www.dbase.com/Knowledgebase/INT/db7_file_fmt.htm
    http://www.dbfree.org/webdocs/1-documentation/b-dbf_header_specifications.htm
*/
struct DbfHeader {
    u_int8_t fileType;
};

void loadShpFile(const std::string &fileName) {
    std::ifstream shpFile;
    shpFile.open(fileName, std::ios::in | std::ios::binary);
    if (!shpFile.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return;
    }

    ShpHeader header = {};
    shpFile >> header;
    if (shpFile.rdstate() != std::ios_base::goodbit) {
        std::cout << "Failed to parse header" << std::endl;
    }
    shpFile.close();
}
