#include "BsFPSCamera.h"
#include "REGothEngine.hpp"
#include <GUI/BsGUILayoutY.h>
#include <BsZenLib/ImportFont.hpp>
#include <BsZenLib/ImportTexture.hpp>
#include <Components/BsCCamera.h>
#include <GUI/BsCGUIWidget.h>
#include <Image/BsSpriteTexture.h>
#include <GUI/BsGUILabel.h>
#include <GUI/BsGUIPanel.h>
#include <GUI/BsGUISkin.h>
#include <GUI/BsGUITexture.h>
#include <Resources/BsBuiltinResources.h>
#include <Scene/BsSceneObject.h>
#include <Text/BsFont.h>
#include <original-content/VirtualFileSystem.hpp>
#include <gui/skin_gothic.hpp>

class REGothFontViewer : public REGoth::REGothEngine
{
public:
  void setupMainCamera() override
  {
    REGoth::REGothEngine::setupMainCamera();

    mFPSCamera = mMainCamera->SO()->addComponent<bs::FPSCamera>();
    mMainCamera->getViewport()->setClearColorValue(bs::Color(0.0f, 0.0f, 0.0f, 1.0f));
  }

  void setupScene() override
  {
    // Add GUI
    bs::HSceneObject guiSO = bs::SceneObject::create("GUI");
    bs::HGUIWidget gui = guiSO->addComponent<bs::CGUIWidget>(mMainCamera);

    // gui->setSkin(bs::BuiltinResources::instance().getGUISkin());
    gui->setSkin(REGoth::GUI::getGothicStyleSkin());

    bs::GUIPanel* mainPanel = gui->getPanel();
    bs::GUILayoutY* layoutY = mainPanel->addNewElement<bs::GUILayoutY>();

    const char* TEXT =
        "This is some bullshit text for testing. Does this thing handle newlines?\n"
        "I guess we'll see. What about umlauts like ä, ö, ü and ß? UPPERCASE ÄÖÜ?\n"
        "Do special characters work? Something like %, #, / or _?";

    layoutY->addNewElement<bs::GUILabel>(bs::HString(TEXT), "Label");
    layoutY->addNewElement<bs::GUILabel>(bs::HString(TEXT), "GothicLabel");
    layoutY->addNewElement<bs::GUILabel>(bs::HString(TEXT), "GothicLabelLarge");
    layoutY->addNewElement<bs::GUILabel>(bs::HString(TEXT), "GothicLabelHighlighted");
    layoutY->addNewElement<bs::GUILabel>(bs::HString(TEXT), "GothicLabelLargeHighlighted");
  }

protected:
  bs::HFPSCamera mFPSCamera;
};

int main(int argc, char** argv)
{
  REGothFontViewer regoth;

  return REGoth::main(regoth, argc, argv);
}
