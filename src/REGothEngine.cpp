#include "REGothEngine.hpp"
#include <BsApplication.h>
#include <cxxopts.hpp>
#include <assert.h>
#include <BsZenLib/ImportMaterial.hpp>
#include <BsZenLib/ImportPath.hpp>
#include <BsZenLib/ResourceManifest.hpp>
#include <Components/BsCCamera.h>
#include <FileSystem/BsFileSystem.h>
#include <Importer/BsImporter.h>
#include <Input/BsVirtualInput.h>
#include <Scene/BsSceneObject.h>
#include <engine-content/EngineContent.hpp>
#include <exception/Throw.hpp>
#include <original-content/OriginalGameFiles.hpp>
#include <original-content/VirtualFileSystem.hpp>

using namespace REGoth;

std::stringstream& operator>>(std::stringstream& str, bs::Path& path)
{
  path.assign(bs::Path{str.str().c_str()});
  return str;
}

/**
 * Name of REGoth's own content directory
 */
const bs::String REGOTH_CONTENT_DIR_NAME = "content";

REGothEngine::REGothEngine(const EngineConfig& config) :
  mConfig{config}
{
  // pass
}

REGothEngine::~REGothEngine()
{
}

void REGothEngine::loadGamePackages()
{
  OriginalGameFiles files = OriginalGameFiles(config().originalAssetsPath);

  gVirtualFileSystem().setPathToEngineExecutable(config().engineExecutablePath.toString());

  bs::gDebug().logDebug("[VDFS] Indexing packages: ");

  for (auto p : files.allVdfsPackages())
  {
    bs::gDebug().logDebug("[VDFS]  - " + p.getFilename());
    gVirtualFileSystem().loadPackage(p);
  }

  gVirtualFileSystem().mountDirectory(files.vdfsFileEntryPoint());

  loadModPackages(files);
}

void REGothEngine::loadModPackages(const OriginalGameFiles& files)
{
  // Don't load mod-files by defaults
}

void REGothEngine::saveCachedResourceManifests()
{
  bs::gDebug().logDebug("[REGothEngine] Saving resource manifests:");

  bs::gDebug().logDebug("[REGothEngine]   - Gothic Cache");
  BsZenLib::SaveResourceManifest();

  // REGothEngine-Content manifest is saved after every resource load since
  // there are only a few resources to handle. If that ever takes too long
  // the manifest should be saved here.
}

bool REGothEngine::hasFoundGameFiles()
{
  return gVirtualFileSystem().hasFoundGameFiles();
}

void REGothEngine::findEngineContent()
{
  mEngineContent = bs::bs_shared_ptr_new<EngineContent>(config().engineExecutablePath);

  if (!mEngineContent->hasFoundContentDirectory())
  {
    REGOTH_THROW(InvalidStateException, "Did not find REGoth content directory!");
  }

  bs::gDebug().logDebug("[REGothEngine] Found REGoth-content directory at: " +
                        mEngineContent->contentPath().toString());
}

void REGothEngine::initializeBsf()
{
  using namespace bs;

  VideoMode videoMode{config().resolutionX, config().resolutionY};
  Application::startUp(videoMode, "REGoth", config().isFullscreen);
}

void REGothEngine::loadCachedResourceManifests()
{
  using namespace bs;

  gDebug().logDebug("[REGothEngine] Loading cached resource manifests");

  if (!mEngineContent)
  {
    REGOTH_THROW(InvalidStateException,
                 "Engine Content not initialized, has findEngineContent() been called?");
  }

  gDebug().logDebug("[REGothEngine]   - REGoth Assets");
  mEngineContent->loadResourceManifest();

  gDebug().logDebug("[REGothEngine]   - Original Gothic Assets");
  BsZenLib::LoadResourceManifest();
}

void REGothEngine::setupInput()
{
  using namespace bs;

  auto inputConfig = gVirtualInput().getConfiguration();

  // Camera controls for buttons (digital 0-1 input, e.g. keyboard or gamepad button)
  inputConfig->registerButton("Forward", BC_W);
  inputConfig->registerButton("Back", BC_S);
  inputConfig->registerButton("Left", BC_A);
  inputConfig->registerButton("Right", BC_D);
  inputConfig->registerButton("Forward", BC_UP);
  inputConfig->registerButton("Back", BC_DOWN);
  inputConfig->registerButton("RotateLeft", BC_LEFT);
  inputConfig->registerButton("RotateRight", BC_RIGHT);
  inputConfig->registerButton("FastMove", BC_LSHIFT);
  inputConfig->registerButton("Rotate", BC_MOUSE_LEFT);
  inputConfig->registerButton("ToggleMeleeWeapon", BC_1);
  inputConfig->registerButton("Action", BC_LCONTROL);
  inputConfig->registerButton("QuickSave", BC_F5);

  // Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
  // These return values in [-1.0, 1.0] range.
  inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
  inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));
}

void REGothEngine::setupMainCamera()
{
  using namespace bs;

  // Add a scene object containing a camera component
  HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");
  HCamera sceneCamera        = sceneCameraSO->addComponent<CCamera>();
  sceneCamera->setMain(true);
  sceneCamera->setMSAACount(1);

  // Disable some fancy rendering
  auto rs = sceneCamera->getRenderSettings();

  rs->screenSpaceReflections.enabled = false;
  rs->ambientOcclusion.enabled       = false;
  rs->enableIndirectLighting         = false;
  rs->enableFXAA                     = false;
  rs->enableHDR                      = false;
  rs->enableTonemapping              = false;
  rs->enableAutoExposure             = false;
  rs->enableSkybox                   = false;
  rs->exposureScale                  = 0.f;
  rs->gamma                          = 2.f;
  rs->cullDistance                   = 100.0f;

  sceneCamera->setRenderSettings(rs);

  mMainCamera = sceneCamera;
}

void REGothEngine::setupScene()
{
  bs::gDebug().logDebug("[REGothEngine] Setting up scene");
}

void REGothEngine::setShaders()
{
  if (!mEngineContent)
  {
    REGOTH_THROW(InvalidStateException,
                 "Has not found REGoth content yet, has findEngineContent() been called?");
  }

  EngineContent::Shaders shaders = mEngineContent->loadShaders();

  BsZenLib::SetShaderFor(BsZenLib::ShaderKind::Opaque, shaders.opaque);

  // FIXME: Use correct shader
  BsZenLib::SetShaderFor(BsZenLib::ShaderKind::AlphaMasked, shaders.opaque);

  // FIXME: Use correct shader
  BsZenLib::SetShaderFor(BsZenLib::ShaderKind::Transparent, shaders.opaque);
}

void REGothEngine::run()
{
  // FIXME: This is a workaround for the camera not being able to move caused by
  //        making bs::SceneManager::findComponent work in the zen loader.
  mMainCamera->SO()->setActive(false);
  mMainCamera->SO()->setActive(true);

  bs::gDebug().logDebug("[REGothEngine] Running mainloop now!");

  bs::Application::instance().runMainLoop();
}

void REGothEngine::shutdown()
{
  if (bs::Application::isStarted())
  {
    bs::gDebug().logDebug("[REGothEngine] Shutting down bs::f");

    bs::Application::shutDown();
  }
  else
  {
    bs::gDebug().logWarning("[REGothEngine] Received shutdown request, but bs::f is not running!");
  }
}

const EngineConfig& REGothEngine::config() const
{
  return mConfig;
}

void ::REGoth::parseArguments(int argc, char** argv, EngineConfig& config)
{
  bool help;
  bool version;

  cxxopts::Options options{argv[0], "REGoth - zEngine Reimplementation."};

  // Add general options.
  options.add_options()
    ("h,help", "Print this help message", cxxopts::value<bool>(help))
    ("version", "Print the REGoth version", cxxopts::value<bool>(version))
    ("v,verbosity", "Verbosity level", cxxopts::value<bool>())
    ;

  // Add options (engine options and specialised ones).
  config.registerCLIEngineOptions(options);
  config.registerCLIOptions(options);

  // Parse argv.
  cxxopts::ParseResult result = options.parse(argc, argv);

  // Print help if `-h` or `--help` is passed and exit.
  if (help)
  {
    std::cout << options.help() << std::endl;
    std::exit(EXIT_SUCCESS);
  }

  // Print REGoth version if `--version` is passed and exit.
  if (version)
  {
    std::cout << "Not yet implemented" << std::endl;
    std::exit(EXIT_SUCCESS);
  }

  // Set verbosity level.
  config.verbosity = static_cast<unsigned int>(result.count("verbosity"));

  // Game executable path must be set manually here.
  config.engineExecutablePath = bs::Path{argv[0]};

  // Verify configuration.
  config.verifyCLIEngineOptions();
  config.verifyCLIOptions();
}

int ::REGoth::runEngine(REGothEngine& engine)
{
  engine.initializeBsf();

  bs::gDebug().logDebug("[Main] Running REGothEngine");
  //bs::gDebug().logDebug("[Main]  - Engine executable: " + engineExecutablePath.toString());
  //bs::gDebug().logDebug("[Main]  - Game directory:    " + gameDirectory.toString());

  bs::gDebug().logDebug("[Main] Finding REGoth content-directory");
  engine.findEngineContent();

  bs::gDebug().logDebug("[Main] Loading original game packages");
  engine.loadGamePackages();

  if (!engine.hasFoundGameFiles())
  {
    std::cerr << "No files loaded into the VDFS - is the game assets path correct?" << std::endl;
    return EXIT_FAILURE;
  }

  bs::gDebug().logDebug("[REGothEngine] Load cached resource manifests");
  engine.loadCachedResourceManifests();

  bs::gDebug().logDebug("[REGothEngine] Loading Shaders");
  engine.setShaders();
  engine.setupInput();

  bs::gDebug().logDebug("[REGothEngine] Setting up Main Camera");
  engine.setupMainCamera();

  bs::gDebug().logDebug("[REGothEngine] Setting up Scene");
  engine.setupScene();

  bs::gDebug().logDebug("[REGothEngine] Save cached resource manifests");
  engine.saveCachedResourceManifests();

  bs::gDebug().logDebug("[REGothEngine] Run");
  engine.run();

  bs::gDebug().logDebug("[REGothEngine] Save cached resource manifests");
  engine.saveCachedResourceManifests();

  bs::gDebug().logDebug("[REGothEngine] Shutdown");
  engine.shutdown();

  return EXIT_SUCCESS;
}
