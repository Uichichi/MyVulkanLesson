#pragma once



#include "vkEngine/model.hpp"

#include <memory>

namespace vkEG {
    class Object{

        struct TransForm2dComponent{
            glm::vec2 transLation{};
            glm::vec2 scale{1.f,1.f};
            float rotation;
            
            //以列为参数来初始化
                /*
                mat2{{1,2},{3,4}} == [1 3
                                      2 4]
                */ 
            glm::mat2 mat2(){
                const float s =glm::sin(rotation);
                const float c =glm::cos(rotation);
                //以列为参数来初始化
                glm::mat2 roMatrix{{c,s},{-s,c}};
                glm::mat2 scaleMat{{scale.x,.0f},{.0f,scale.y}};

                return roMatrix * scaleMat;
            }
        };

        public:
            using id_t = unsigned int;

            static Object createObject(){
                static id_t currentId = 0;
                return Object{currentId++};
            };

            Object(const Object&) = delete;
            Object& operator=(const Object&) = delete;
            Object(Object&&) = default;
            Object& operator=(Object&&) = default;

            id_t getId(){return id;}

            std::shared_ptr<Model> model{};
            glm::vec3 color{};
            TransForm2dComponent transform2d{};
        private:
        Object(id_t objId) : id{objId}{

        };    
        
        id_t id;

    };
}