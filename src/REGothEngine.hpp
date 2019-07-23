/** \file
 */

#pragma once

#include <iostream>

#include <BsPrerequisites.h>
#include <FileSystem/BsFileSystem.h>
#include <cxxopts.hpp>

/**
 * Allows using the bs::Path data type together with cxxopts.
 *
 * @param str Input stringstream.
 * @param path Path to write data to.
 * @return stringstream.
 */
std::stringstream& operator>>(std::stringstream& str, bs::Path& path);

namespace REGoth
{
  class EngineContent;
  class OriginalGameFiles;

  /**
   * The base configuration of the engine.
   */
  class EngineConfig
  {

  public:

    virtual ~EngineConfig()
    {
      // pass
    }

    void registerCLIEngineOptions(cxxopts::Options& options)
    {
      // Configure positional handling
      options.positional_help("[GAME ASSETS PATH]");
      options.show_positional_help();

      // Define engine options
      options.add_options()
        ("a,game-assets", "Path to a Gothic or Gothic 2 installation", cxxopts::value<bs::Path>(originalAssetsPath), "[PATH]")
        ("video-x-res", "X resolution", cxxopts::value<unsigned int>(resolutionX), "[PX]")
        ("video-y-res", "Y resolution", cxxopts::value<unsigned int>(resolutionY), "[PX]")
        ("video-fullscreen", "Run in fullscreen mode", cxxopts::value<bool>(isFullscreen))
        ;

      // Allow game-assets to also be a positional
      options.parse_positional({"game-assets"});
    }

    void verifyCLIEngineOptions()
    {
      // Resolve paths.
      engineExecutablePath.makeAbsolute(bs::FileSystem::getWorkingDirectoryPath());
      originalAssetsPath.makeAbsolute(bs::FileSystem::getWorkingDirectoryPath());

      // TODO: Verify that assetsPath is a valid Gothic or Gothic 2 installation.
      // TODO: Info whether assetsPath points to Gothic or Gothic 2 could be helpful. How to determine?
    }

    virtual void registerCLIOptions(cxxopts::Options& /* options */)
    {
      // pass
    }

    virtual void verifyCLIOptions()
    {
      // pass
    }

    unsigned int verbosity = 0;
    bs::Path engineExecutablePath;
    bs::Path originalAssetsPath;
    unsigned int resolutionX = 1280;
    unsigned int resolutionY = 768;
    bool isFullscreen = false;

  };

  /**
   * This is the REGoth-Core-Class, which initializes the engine, sets the
   * input and the scene.
   *
   * To handle more use cases, the REGothEngine-class can be extended.
   * The base class will not load any world and start on an empty scene but with
   * most of the utilities set up to load original content and game mechanics.
   *
   * Therefore, the REGothEngine-class can be used to implement viewers and other tools
   * as well as the actual reimplementation of the Gothic games.
   *
   *
   * Some important virtual functions exist which can be overridden:
   *
   *  - getVdfsPackagesToLoad()
   *  - setupInput()
   *  - setupScene()
   *
   * To actually run an instance of the engine, see the `main`-wrapper below.
   */
  class REGothEngine
  {

  public:

    REGothEngine(const EngineConfig& config);
    virtual ~REGothEngine();

    /**
     * Load VDFS packages from the original game. Will load data in the following order:
     *
     *  1. Data/ (*.vdf)
     *  2. _world/ (Recursive)
     *  3. Data/modvdf/ (*.mod, recursive)
     *
     * @param  executablePath  Path to the currently running executable (argv[0])
     * @param  gameDirectory   Location where Gothics game files can be found.
     */
    void loadGamePackages();

    /**
     * Called by loadOriginalGamePackages(). Can be overriden by the user to load specific
     * MOD-packages.
     *
     * To load a MOD-package, use `gVirtualFileSystem().loadPackage(p)`.
     *
     * @param  files  Reference to OriginalGameFiles-object, which has some utility methods to
     *                access files in the original game directory.
     */
    virtual void loadModPackages(const OriginalGameFiles& files);

    /**
     * When called after loadOriginalGamePackages(), this will check whether Gothics game files were
     * found at the location given to loadOriginalGamePackages().
     *
     * @return Whether game files were found.
     */
    bool hasFoundGameFiles();

    /**
     * Initializes bsf and opens the window
     */
    void initializeBsf();

    /**
     * Load all resource manifests written by previous runs of REGoth.
     */
    void loadCachedResourceManifests();

    /**
     * Save resource manifests containing resources loaded during this run.
     */
    void saveCachedResourceManifests();

    /**
     * Assign buttons and axis to control the game
     */
    virtual void setupInput();

    /**
     * Sets up the main camera of this engine.
     */
    virtual void setupMainCamera();

    /**
     * Load scenes and other objects and add them to the scene
     */
    virtual void setupScene();

    /**
     * Set shaders to be used when re-caching the materials.
     */
    virtual void setShaders();

    /**
     * Find the location of REGoths own `content`-directory.
     *
     * @param  executablePath  Path to the currently running executable.
     */
    void findEngineContent();

    /**
     * Run the main-loop
     */
    void run();

    /**
     * Shutdown bsf
     */
    void shutdown();

    /**
     * Gets the engine's configuration.
     *
     * @return Engine configuration data structure.
     */
    virtual const EngineConfig& config();

  protected:

    /**
     * Main camera this engines renders with
     */
    bs::HCamera mMainCamera;

    /**
     * Path to REGoth's own `content`-directory and resource loader.
     */
    bs::SPtr<EngineContent> mEngineContent;

    /**
     * Engine base configuration
     */
    const EngineConfig mConfig;

  };

  /**
   * Parses the given command line arguments in argv to populate the given configuration object config.
   *
   * @param argc Main's argc.
   * @param argv Main's argv.
   * @param config Output parameter to populate with parsed options.
   */
  void parseArguments(int argc, char** argv, EngineConfig& config);

  /**
   * Bootstrap and run the given engine.
   *
   * @param Engine object to run.
   * @return Windows errorlevel / POSIX status code.
   */
  int runEngine(REGothEngine& engine);

}  // namespace REGoth
