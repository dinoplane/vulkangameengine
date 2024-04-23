#include "ave_constants.hpp"


#include <stack>
#include <unordered_map>

class PlantGenerator {
    struct TurtleState{
        glm::vec3 position;
        glm::vec3 direction; // Should i use quaternions?

        glm::vec3 color;
        float width;
    };




    private:
    std::string stringRepresentation;
    size_t angle;
    glm::mat4 rotationXMatrix;
    glm::mat4 rotationYMatrix;
    glm::mat4 rotationZMatrix;

    size_t generations;
    std::map<std::string, std::string> rules;

    TurtleState turtleState;
    std::stack<TurtleState> turtleStack;

    public:
    PlantGenerator(std::string stringrepr, size_t generations, size_t angle, std::unordered_map<std::string, std::string> rules);
    ~PlantGenerator();
    void generatePlant();

}
