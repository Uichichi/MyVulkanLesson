#include "vkEngine/newapp.hpp"
#include "vkEngine/simple_rendersystem.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_Z_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>



namespace vkEG {
    NewApp::NewApp(){
        loadObjects();
    }

    NewApp::~NewApp(){}
    void NewApp::run(){
        SimpleRendersystem simpleRendersystem{appDevice,appRenderer.getSwapchainRenderPass()};
        while (!appWindow.shouldClose()) {
            glfwPollEvents();
            if(auto commandBuffer = appRenderer.beginFrame()){
                appRenderer.beginSwapchainRenderPass(commandBuffer);
                simpleRendersystem.renderObjects(commandBuffer, objects);
                appRenderer.endSwapchainRenderPass(commandBuffer);
                appRenderer.endFrame();
            }
        }
        appDevice.device().waitIdle();
    };

    void NewApp::loadObjects(){
        std::vector<Model::Vertex> vertices{
            {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f,0.5f},{0.0f,1.0f,0.0f}},
            {{-0.5f,0.5f},{0.0f,0.0f,1.0f}}
        };
        auto model = std::make_shared<Model>(appDevice,vertices);

        auto triangle = Object::createObject();
        triangle.model = model;
        triangle.color = {.1f,.8f,.1f};
        triangle.transform2d.transLation.x = .2f;
        triangle.transform2d.scale  ={2.f,.5f};
        //vulkan的y轴和常规数学相反，y在x轴上是负数
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        objects.push_back(std::move(triangle));
    };

}