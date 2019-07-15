/** \file
 */

#pragma once

#include <BsPrerequisites.h>
#include <cxxopts.hpp>

namespace REGoth
{
  class EngineContent;
  class OriginalGameFiles;

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
    REGothEngine() = default;
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
    void loadGamePackages(const bs::Path& executablePath, const bs::Path& gameDirectory);

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
    void findEngineContent(const bs::Path& executablePath);

    /**
     * Run the main-loop
     */
    void run();

    /**
     * Shutdown bsf
     */
    void shutdown();

    /**
     * @brief Registers command line options.
     * @param opts cxxopts Options object.
     */
    virtual void registerArguments(cxxopts::Options& opts);

    /**
     * @brief Verifies the parsed command line options results.
     * @param result cxxopts Results object.
     * @return true if options are valid, false otherwise.
     */
    virtual bool checkArguments(cxxopts::ParseResult& result);

  protected:

    /**
     * Main camera this engines renders with
     */
    bs::HCamera mMainCamera;

    /**
     * Path to REGoth's own `content`-directory and resource loader.
     */
    bs::SPtr<EngineContent> mEngineContent;
  };

  /**
   * Boilerplate-code to setup and run the given engine
   *
   * @param  regoth  Untouched fresh instance of the engine class you want to run
   * @param    argc  `argc` as in `main`
   * @param    argv  `argv` as in `main`
   *
   * @return Return value as in `main`.
   */
  int main(REGothEngine& regoth, int argc, char** argv);
}  // namespace REGoth
