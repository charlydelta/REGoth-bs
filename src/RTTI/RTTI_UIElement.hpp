#pragma once
#include "RTTIUtil.hpp"
#include <components/UIElement.hpp>

namespace REGoth
{
  class RTTI_UIElement
      : public bs::RTTIType<UIElement, bs::Component, RTTI_UIElement>
  {
    BS_BEGIN_RTTI_MEMBERS
    // This class should not be serialized
    BS_END_RTTI_MEMBERS

  public:
    RTTI_UIElement()
    {
    }

    REGOTH_IMPLEMENT_RTTI_CLASS_FOR_COMPONENT(UIElement)
  };
}  // namespace REGoth
