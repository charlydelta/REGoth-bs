#include "components/AnchoredTextLabels.h"
#include <GUI/BsCGUIWidget.h>
#include <components/GameWorld.hpp>
#include "BsFPSCamera.h"
#include "REGothEngine.hpp"
#include <Components/BsCCamera.h>
#include <Scene/BsSceneObject.h>
#include <components/Waynet.hpp>
#include <components/Waypoint.hpp>
#include <exception/Throw.hpp>
#include <original-content/VirtualFileSystem.hpp>

class REGothWaynetTester : public REGoth::REGothEngine
{
public:

  using REGoth::REGothEngine::REGothEngine;

  void setupMainCamera() override
  {
    REGoth::REGothEngine::setupMainCamera();

    mMainCamera->SO()->addComponent<bs::FPSCamera>();

    auto guiSO = bs::SceneObject::create("GUI");
    auto guiWidget = guiSO->addComponent<bs::CGUIWidget>(mMainCamera);

    auto debugOverlaySO = bs::SceneObject::create("DebugOverlay");
    mTextLabels         = debugOverlaySO->addComponent<REGoth::AnchoredTextLabels>(guiWidget);
    mTextLabels->setMaximumDistance(50.f);
  }

  void setupScene() override
  {
    using namespace REGoth;

    HGameWorld world = GameWorld::importZEN("OLDWORLD.ZEN");

    world->waynet()->debugDraw(mTextLabels);
  }

protected:
  REGoth::HAnchoredTextLabels mTextLabels;
};

int main(int argc, char** argv)
{
  REGoth::EngineConfig config;
  REGoth::parseArguments(argc, argv, config);
  REGothWaynetTester engine{config};

  return REGoth::runEngine(engine);
}
