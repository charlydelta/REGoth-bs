#include "NodeVisuals.hpp"
#include <Animation/BsSkeleton.h>
#include <Components/BsCBone.h>
#include <Components/BsCRenderable.h>
#include <Mesh/BsMesh.h>
#include <RTTI/RTTI_NodeVisuals.hpp>
#include <Scene/BsSceneObject.h>
#include <components/Visual.hpp>
#include <components/VisualStaticMesh.hpp>
#include <log/logging.hpp>

namespace REGoth
{
  NodeVisuals::NodeVisuals(const bs::HSceneObject& parent)
      : bs::Component(parent)
  {
  }

  bool NodeVisuals::hasNode(const bs::String& name) const
  {
    bs::SPtr<bs::Skeleton> skeleton = getSkeleton();

    if (!skeleton) return false;

    for (bs::UINT32 i = 0; i < skeleton->getNumBones(); i++)
    {
      if (skeleton->getBoneInfo(i).name == name) return true;
    }

    return false;
  }

  void NodeVisuals::attachVisualToNode(const bs::String& node, const bs::String& visual)
  {
    clearNodeAttachment(node);

    bs::HSceneObject boneSO = bs::SceneObject::create(node);

    enum
    {
      KeepWorldTransform   = true,
      MoveRelativeToParent = false,
    };

    boneSO->setParent(SO(), MoveRelativeToParent);

    bs::HBone bone = boneSO->addComponent<bs::CBone>();
    bone->setBoneName(node);

    bool hasCreated = Visual::addToSceneObject(boneSO, visual);

    if (!hasCreated)
    {
      clearNodeAttachment(node);

      REGOTH_LOG(Warning, Uncategorized, "[NodeVisuals] Failed to attach visual '{0}' to node '{1}'",
                 visual, node);
    }
  }

  void NodeVisuals::attachMeshToNode(const bs::String& node, BsZenLib::Res::HMeshWithMaterials mesh)
  {
    clearNodeAttachment(node);

    bs::HSceneObject boneSO = bs::SceneObject::create(node);

    enum
    {
      KeepWorldTransform   = true,
      MoveRelativeToParent = false,
    };

    boneSO->setParent(SO(), MoveRelativeToParent);

    bs::HBone bone = boneSO->addComponent<bs::CBone>();
    bone->setBoneName(node);

    bs::HRenderable renderable = boneSO->addComponent<bs::CRenderable>();
    renderable->setMesh(mesh->getMesh());
    renderable->setMaterials(mesh->getMaterials());
  }

  void NodeVisuals::clearNodeAttachment(const bs::String& node)
  {
    bs::HSceneObject bone = SO()->findChild(node);

    if (!bone.isDestroyed())
    {
      bone->destroy();
    }
  }

  bs::SPtr<bs::Skeleton> NodeVisuals::getSkeleton() const
  {
    bs::HRenderable renderable = SO()->getComponent<bs::CRenderable>();

    if (!renderable) return nullptr;

    if (!renderable->getMesh()) return nullptr;

    return renderable->getMesh()->getSkeleton();
  }

  REGOTH_DEFINE_RTTI(NodeVisuals)
}  // namespace REGoth
