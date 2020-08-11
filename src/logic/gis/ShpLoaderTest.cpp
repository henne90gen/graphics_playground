#include <iostream>

#include "ShpLoader.h"

int main() {
    ShpData shpData;
    bool success = loadShpFile("../../../gis_data/gravimetry/gravi.shp", shpData);
    if (!success) {
        std::cout << "Could not load shp file" << std::endl;
    } else {
        std::cout << "Successfully loaded shp file" << std::endl;
    }

    DbfData dbfData;
    success = loadDbfFile("../../../gis_data/gravimetry/gravi.dbf", dbfData);
    if (!success) {
        std::cout << "Could not load dbf file" << std::endl;
    } else {
        std::cout << "Successfully loaded dbf file" << std::endl;
    }
}
