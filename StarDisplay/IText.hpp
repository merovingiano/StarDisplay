#ifndef ITEXT_HPP_INCLUDED
#define ITEXT_HPP_INCLUDED

#include <memory>
#include "IGuiBox.hpp"


class __declspec(novtable) ITextBox : public IGuiBox
{
public:
  virtual ~ITextBox() {}
  virtual const char* GetText() const = 0;
  virtual void SetText(const char* str) = 0;
};


// Commands understood from IText::Print
// 
// \color{float float float}
// \color{float, float, float}
// \defcolor{}
// \orig{int int}
// \cursor{int int}
// \tabsize{int}
// \smallface{}
// \mediumface{}
// \bigface{}
// \monoface{}
// \t, \n
//
class __declspec(novtable) IText
{
public:
  virtual ~IText() {}
  virtual int font_size() const = 0;
  virtual int line_height() const = 0;
  virtual int base() const = 0;
  virtual glm::ivec2 orig() const = 0;
  virtual glm::ivec2 cursor() const = 0;

  virtual glm::ivec2 extent(const char* str) = 0;
  virtual void print(const char* str) = 0;
  virtual void set_color(const glm::vec4&) = 0;
  virtual void set_orig(const glm::ivec2&) = 0;
  virtual void set_cursor(const glm::ivec2&) = 0;
  virtual void set_tabsize(int) = 0;

  virtual ITextBox* get_text_box(const char* name) = 0;
  virtual void set_text_box(ITextBox* ptr) = 0;
  virtual void delete_text_box(const char* name) = 0;

  virtual void Flush();
  virtual void Render();
};


#endif
