#include <luabind/luabind.hpp>
#include <luabind/adopt_policy.hpp>
#include "libGlm.hpp"
#include "IGuiBox.hpp"
#include "IText.hpp"
#include "ICamera.hpp"


using namespace luabind;


namespace libIText
{

  class TextBox : public ITextBox
  {
  public:
    TextBox(const char* name, const glm::ivec4& box, const ICamera* camera)
      : name_(name), box_(box), camera_(camera)
    {
      this->resize();
    }
    virtual ~TextBox() 
    {
    };

    virtual const char* GetText() const { return str_.c_str(); }
    virtual void SetText(const char* str) { str_ = str; }
    virtual glm::ivec4 GetBox() const { return glm::ivec4(cbox_); }
    virtual const std::string& GetName() const { return name_; }

    virtual void resize()
    {
      glm::ivec4 vp = camera_->GetViewport();
      cbox_[0] = ((box_[0] < 0) ? vp[2] : vp[0]) + box_[0];
      cbox_[1] = ((box_[1] < 0) ? vp[3] : vp[1]) + box_[1];
      cbox_[2] = cbox_[0] + box_[2];
      cbox_[3] = cbox_[1] + box_[3];
    }

  private:
    std::string name_;
    glm::dvec4 box_, cbox_;
    const ICamera* camera_;
    std::string str_;
  };


  ITextBox* GetTextBox(IText* self, const char* name)
  {
    return self->get_text_box(name);
  }


  ITextBox* CreateTextBoxCpp(IText* self, const char* name, const glm::ivec4& box, const class ICamera* camera)
  {
    ITextBox* ptr = new TextBox(name, box, camera);
    self->set_text_box(ptr);
    return ptr;
  }


  ITextBox* CreateTextBox(IText* self, const char* name, const glm::ivec4& box, object camera)
  {
    return CreateTextBoxCpp(self, name, box, object_cast<const ICamera*>(camera["cc"]));
  }


  ITextBox* RemoveTextBox(IText* self, ITextBox* tb)
  {
    self->delete_text_box(static_cast<TextBox*>(tb)->GetName().c_str());
    return 0;
  }


}


void luaopen_libIText(lua_State* L)
{
  module(L)[
    class_<ITextBox>("__textbox")
      .def("SetText", &ITextBox::SetText),

    class_<IText>("__IText")
      .def("extent", (glm::ivec2 (IText::*)(const char*)) &IText::extent)
      .def("print", (void (IText::*)(const char*)) &IText::print)
      .def("setOrig", (void (IText::*)(const glm::ivec2&)) &IText::set_orig)
      .def("setColor", (void (IText::*)(const glm::vec4&)) &IText::set_color)
      .def("setCursor", (void (IText::*)(const glm::ivec2&)) &IText::set_cursor)
      .def("GetTextBox", &libIText::GetTextBox)
      .def("CreateTextBox", &libIText::CreateTextBox)
      .def("RemoveTextBox", &libIText::RemoveTextBox)
  ];
}

