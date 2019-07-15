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

/**
 * Name of REGoth's own content directory
 */
const bs::String REGOTH_CONTENT_DIR_NAME = "content";

/**
 * @brief Allows using the bs::Path data type together with cxxopts.
 * @param str Input stringstream.
 * @param path Path to write data to.
 * @return stringstream.
 */
std::stringstream& operator>>(std::stringstream& str, bs::Path& path)
{
  path.assign(bs::Path(str.str().c_str()));
  return str;
}

REGothEngine::~REGothEngine()
{
}

void REGothEngine::loadGamePackages(const bs::Path& executablePath, const bs::Path& gameDirectory)
{
  OriginalGameFiles files = OriginalGameFiles(gameDirectory);

  gVirtualFileSystem().setPathToEngineExecutable(executablePath.toString());

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

void REGothEngine::findEngineContent(const bs::Path& executablePath)
{
  mEngineContent = bs::bs_shared_ptr_new<EngineContent>(executablePath);

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

  // TODO: Make video mode configurable
  VideoMode videoMode(1280, 720);
  Application::startUp(videoMode, "REGoth", false);
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

void REGothEngine::registerArguments(cxxopts::Options& /* opts */)
{
  // pass
}

int ::REGoth::main(REGothEngine& regoth, int argc, char** argv)
{
  bool help = false;
  bool version = false;
  bs::Path engineExecutablePath{argv[0]};
  bs::Path gameDirectory;
  bs::Path world;

  cxxopts::Options options(argv[0], "REGoth - zEngine Reimplementation.");
  options.positional_help("[GAME ASSETS PATH]");
  options.show_positional_help();
  options.add_options()
    ("a,game-assets", "Path to a Gothic or Gothic 2 installation", cxxopts::value<bs::Path>(gameDirectory), "[PATH]")
    ("h,help", "Print this help message", cxxopts::value<bool>(help))
    ("v,version", "Print the REGoth version", cxxopts::value<bool>(version))
    ;

  regoth.registerArguments(options);

  options.parse_positional({"game-assets"});
  cxxopts::ParseResult result = options.parse(argc, argv);

  // Print help if `-h` or `--help` is passed and exit.
  if (help)
  {
    std::cout << options.help() << std::endl;
    return EXIT_SUCCESS;
  }

  // Print REGoth version if `-v` or `--version` is passed and exit.
  if (version)
  {
    std::cout << "Not yet implemented" << std::endl;
    return EXIT_SUCCESS;
  }

  // Assert that the game assets directory was specified.
  if (result.count("game-assets") == 0)
  {
    std::cerr << "No path to a Gothic or Gothic 2 installation was given. "
              << "You can specify the path via the first positional argument or "
              << "using the `--game-assets` argument." << std::endl;
    std::cerr << "Aborting." << std::endl;
    return EXIT_FAILURE;
  }

  engineExecutablePath.makeAbsolute(bs::FileSystem::getWorkingDirectoryPath());
  gameDirectory.makeAbsolute(bs::FileSystem::getWorkingDirectoryPath());

  regoth.initializeBsf();

  bs::gDebug().logDebug("[Main] Running REGothEngine");
  bs::gDebug().logDebug("[Main]  - Engine executable: " + engineExecutablePath.toString());
  bs::gDebug().logDebug("[Main]  - Game directory:    " + gameDirectory.toString());

  bs::gDebug().logDebug("[Main] Finding REGoth content-directory");
  regoth.findEngineContent(engineExecutablePath);

  bs::gDebug().logDebug("[Main] Loading original game packages");
  regoth.loadGamePackages(engineExecutablePath, gameDirectory);

  if (!regoth.hasFoundGameFiles())
  {
    std::cerr << "No files loaded into the VDFS - is the game assets path correct?" << std::endl;
    return EXIT_FAILURE;
  }

  regoth.loadCachedResourceManifests();

  bs::gDebug().logDebug("[REGothEngine] Loading Shaders");

  regoth.setShaders();
  regoth.setupInput();

  bs::gDebug().logDebug("[REGothEngine] Setting up Main Camera");

  regoth.setupMainCamera();

  bs::gDebug().logDebug("[REGothEngine] Setting up Scene");

  regoth.setupScene();

  regoth.saveCachedResourceManifests();

  regoth.run();

  regoth.saveCachedResourceManifests();

  regoth.shutdown();

  return EXIT_SUCCESS;
}
