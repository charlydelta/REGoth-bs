#include "BsFPSCamera.h"
#include "REGothEngine.hpp"
#include <Components/BsCCamera.h>
#include <Scene/BsSceneObject.h>
#include <world/internals/ConstructFromZEN.hpp>

class REGothWorldMeshViewer : public REGoth::REGothEngine
{
public:

  using REGoth::REGothEngine::REGothEngine;

  void setupMainCamera() override
  {
    REGoth::REGothEngine::setupMainCamera();

    mFPSCamera = mMainCamera->SO()->addComponent<bs::FPSCamera>();
  }

  void setupScene() override
  {
    REGoth::Internals::loadWorldMeshFromZEN("ADDONWORLD.ZEN");
  }

protected:
  bs::HFPSCamera mFPSCamera;
};

int main(int argc, char** argv)
{
  REGoth::EngineConfig config;
  REGoth::parseArguments(argc, argv, config);
  REGothWorldMeshViewer engine{config};

  return REGoth::runEngine(engine);
}
