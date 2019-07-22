/** \file
 * Entry point for REGoth
 */

#include "REGothEngine.hpp"

int main(int argc, char** argv)
{
  ::REGoth::EngineConfig config;
  ::REGoth::parseArguments(argc, argv, config);
  ::REGoth::REGothEngine engine{config};

  return ::REGoth::runEngine(engine);
}
