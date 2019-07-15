#include "BsFPSCamera.h"
#include <REGothEngine.hpp>
#include <Components/BsCCamera.h>
#include <Scene/BsPrefab.h>
#include <Scene/BsSceneObject.h>
#include <components/Character.hpp>
#include <components/CharacterAI.hpp>
#include <components/CharacterEventQueue.hpp>
#include <components/CharacterKeyboardInput.hpp>
#include <components/GameplayUI.hpp>
#include <components/GameWorld.hpp>
#include <components/ThirdPersonCamera.hpp>
#include <exception/Throw.hpp>
#include <original-content/OriginalGameFiles.hpp>
#include <original-content/VirtualFileSystem.hpp>

class REGothWorldViewer : public REGoth::REGothEngine
{
public:
  void loadModPackages(const REGoth::OriginalGameFiles& files) override
  {
    // using namespace REGoth;

    // for (auto p : files.allModPackages())
    // {
    //   bs::gDebug().logDebug("[WorldViewer] Loading Mod: " + p.toString());
    //   gVirtualFileSystem().loadPackage(p);
    // }

    // for (auto zen : gVirtualFileSystem().listByExtension(".ZEN"))
    // {
    //   bs::gDebug().logDebug("[WorldViewer] Found ZEN: " + zen);
    // }
  }

  void setupMainCamera() override
  {
    REGoth::REGothEngine::setupMainCamera();

    mThirdPersonCamera = mMainCamera->SO()->addComponent<REGoth::ThirdPersonCamera>();
  }

  void setupScene() override
  {
    using namespace REGoth;

    if (mWorld.empty())
    {
      REGOTH_THROW(InvalidStateException, "World cannot be empty.");
    }

    bs::StringUtil::toUpperCase(mWorld);
    if (not bs::StringUtil::endsWith(mWorld, ".ZEN"))
    {
      mWorld += ".ZEN";
    }

    const bs::String SAVEGAME = "WorldViewer-" + mWorld;

    bs::HPrefab worldPrefab = GameWorld::load(SAVEGAME);
    HGameWorld world;

    if (!worldPrefab)
    {
      world = GameWorld::importZEN(mWorld);

      HCharacter hero = world->insertCharacter("PC_HERO", "START");
      hero->useAsHero();
      hero->SO()->addComponent<CharacterKeyboardInput>(world);

      // bs::HSceneObject diegoSO = world->insertCharacter("PC_THIEF", "WP_INTRO_FALL3")->SO();

      world->runInitScripts();

      world->save(SAVEGAME);
    }
    else
    {
      bs::HSceneObject worldSO = worldPrefab->instantiate();

      world = worldSO->getComponent<GameWorld>();
    }

    bs::HSceneObject heroSO = world->SO()->findChild("PC_HERO");

    if (!heroSO)
    {
      REGOTH_THROW(InvalidStateException, "Expected PC_HERO in world");
    }

    HCharacter hero = heroSO->getComponent<Character>();

    mThirdPersonCamera->follow(hero);

    REGoth::GameplayUI::createGlobal(mMainCamera);
  }

  virtual void registerArguments(cxxopts::Options& opts) override
  {
    opts.add_options()
      ("w,world", "World name to load", cxxopts::value<bs::String>(mWorld), "[NAME]")
      ;
  }

protected:
  REGoth::HThirdPersonCamera mThirdPersonCamera;
  bs::String mWorld;
};

int main(int argc, char** argv)
{
  REGothWorldViewer regoth;

  return REGoth::main(regoth, argc, argv);
}
