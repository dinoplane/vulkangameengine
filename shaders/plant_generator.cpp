#include "plant_generator.hpp"


PlantGenerator::PlantGenerator(std::string stringrepr, size_t generations, size_t angle, std::unordered_map<std::string, std::string> rules)
    : stringRepresentation{stringrepr}, generations{generations}, angle{angle}, rules{rules}{

}

~PlantGenerator::PlantGenerator(){

}

void PlantGenerator::generatePlant(){

}
