#pragma once

#include "ave_model.hpp"

#include <memory>

namespace ave
{
    struct Transform2dComponent{
        glm::vec2 translation;
        glm::mat2 transform(){ return glm::mat2{1.0f}; }
    }

    class AveGameObject {
        public:
        using id_t = unsigned int;

        // static int

        AveGameObject(const AveGameObject&) = delete;
        AveGameObject& operator=(const AveGameObject&) = delete;

        AveGameObject(AveGameObject&&) = default;
        AveGameObject& operator=(AveGameObject&&) = default;

        virtual ~AveGameObject() = default;

        static AveGameObject createGameObject(){
            static int currentId = 0;
            return AveGameObject{currentId++};
        }

        const id_t getId(){
            return id;
        }

        std::shared_ptr<AveModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d;

        private:
            AveGameObject(id_t obj_id) : id{obj_id} {};

        id_t id;
    }
} // namespace ave
