#ifndef GLSLTEXT_HPP_INCLUDED
#define GLSLTEXT_HPP_INCLUDED

#include <memory>
#include <unordered_map>
#include <glsl/bmfont.hpp>
#include <glsl/buffer.hpp>
#include <glsl/vertexarray.hpp>
#include "IText.hpp"


namespace bmf = glsl::bmfont;


class GLSLText : public IText
{
public:
  GLSLText();
  virtual ~GLSLText();

  int font_size() const { return text2D_->FontSize(); }
  int line_height() const { return text2D_->LineHeight(); }
  int base() const { return text2D_->Base(); }
  glm::ivec2 cursor() const { return text2D_->GetCursor(); };
  glm::ivec2 orig() const { return text2D_->GetOrig(); };
  
  glm::ivec2 extent(const char* str);
  void print(const char* str);
  void set_color(const glm::vec4& color) { *text2D_ << bmf::color_manip(color); }
  void set_orig(const glm::ivec2& orig) { *text2D_ << bmf::orig_manip(orig); }
  void set_cursor(const glm::ivec2& cursor) { *text2D_ << bmf::cursor_manip(cursor); }
  void set_tabsize(int tab) { *text2D_ << bmf::tab_manip(tab); }
  
  ITextBox* get_text_box(const char* name);
  void set_text_box(ITextBox* ptr);
  void delete_text_box(const char* name);

  void Flush();
  void Render();

private:
  std::unordered_map<std::string, std::shared_ptr<ITextBox> > TextBoxes_;
  std::unordered_map<std::string, std::shared_ptr<bmf::Font> > Faces_;
  std::unique_ptr<bmf::Text2D>  text2D_;

  bool match_command(bmf::Text2D& txt, const char* command, const char* argf, const char* argl);
  const char* parse_commands(bmf::Text2D& txt, const char* first, const char* last);
  
  unsigned totVertexCount_;
  std::unique_ptr<unsigned[]> vc_;
  glsl::buffer buffer_;
  glsl::vertexarray vao_;
};


#undef MAX_FACES

#endif
