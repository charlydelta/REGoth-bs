#include "BsFPSCamera.h"
#include <REGothEngine.hpp>
#include <Components/BsCCamera.h>
#include <Scene/BsPrefab.h>
#include <Scene/BsSceneObject.h>
#include <components/Character.hpp>
#include <components/CharacterAI.hpp>
#include <components/CharacterEventQueue.hpp>
#include <components/CharacterKeyboardInput.hpp>
#include <components/GameWorld.hpp>
#include <components/GameplayUI.hpp>
#include <components/ThirdPersonCamera.hpp>
#include <exception/Throw.hpp>
#include <log/logging.hpp>
#include <original-content/OriginalGameFiles.hpp>
#include <original-content/VirtualFileSystem.hpp>

class Config : public REGoth::EngineConfig
{

public:

  virtual void registerCLIOptions(cxxopts::Options& opts) override
  {
    opts.add_options()
      ("w,world", "Name of the world to load", cxxopts::value<bs::String>(world), "[NAME]")
      ;
  }

  virtual void verifyCLIOptions() override
  {
    // Make sure that world is set.
    if (world.empty())
    {
      REGOTH_THROW(InvalidStateException, "World cannot be empty.");
    }

    // Unify possible inputs.
    bs::StringUtil::toUpperCase(world);
    if (!bs::StringUtil::endsWith(world, ".ZEN"))
    {
      world += ".ZEN";
    }
  }

  bs::String world;

};

class REGothWorldViewer : public REGoth::REGothEngine
{
public:
  REGothWorldViewer(const Config& config) : REGoth::REGothEngine(config) {}

  const Config& config() const override
  {
    return static_cast<const Config&>(mConfig);
  }

  void loadModPackages(const REGoth::OriginalGameFiles& files) override
  {
    // using namespace REGoth;

    // for (auto p : files.allModPackages())
    // {
    //   REGOTH_LOG(Info, Uncategorized, "[WorldViewer] Loading Mod: " + p.toString());
    //   gVirtualFileSystem().loadPackage(p);
    // }

    // for (auto zen : gVirtualFileSystem().listByExtension(".ZEN"))
    // {
    //   REGOTH_LOG(Info, Uncategorized, "[WorldViewer] Found ZEN: " + zen);
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

    const bs::String SAVEGAME = "WorldViewer-" + config().world;

    bs::HPrefab worldPrefab = GameWorld::load(SAVEGAME);
    HGameWorld world;

    if (!worldPrefab)
    {
      world = GameWorld::importZEN(config().world);

      HCharacter hero = world->insertCharacter("PC_HERO", WORLD_STARTPOINT);
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

protected:

  REGoth::HThirdPersonCamera mThirdPersonCamera;

};

int main(int argc, char** argv)
{
  Config config;
  REGoth::parseArguments(argc, argv, config);
  REGothWorldViewer engine{config};

  return REGoth::runEngine(engine);
}
