#include <string.h>
#include <sstream>
#include "filesystem.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/bbox.hpp>
#include <glsl/shader_pool.hpp>
#include "GLSLText.hpp"
#include "GLSLState.hpp"
#include "Params.hpp"
#include "Globals.hpp"


GLSLText::GLSLText()
{
  filesystem::path media = luabind::object_cast<const char*>(Lua("MediaPath"));
  media /= "fonts";
  for (auto const & nfn : PARAMS.Fonts)
  {
    Faces_[nfn.first] = bmf::Font::Create((media / nfn.second).string().c_str());
  }
  text2D_.reset( new bmf::Text2D(Faces_.begin()->second) );
  vc_.reset(new unsigned[Faces_.size()]);

  GGl.use_program("Text")->uniform("FontTexture")->set1i(2);
  vao_.bind();
  buffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(bmf::TUS2_VS2_CUB4), (void*)(char*)(0));
  glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, sizeof(bmf::TUS2_VS2_CUB4), (void*)(char*)(4));
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(bmf::TUS2_VS2_CUB4), (void*)(char*)(8));
}


GLSLText::~GLSLText()
{
}


void GLSLText::Flush()
{
  // print text box content
  for (auto it = TextBoxes_.begin(); it != TextBoxes_.end(); ++it)
  {
    it->second->resize();
    glm::ivec4 box = it->second->GetBox();
    *text2D_ << bmf::orig_manip(box[0], box[1]);
    print(it->second->GetText());
    set_color(GGl.textColor());
  }
  totVertexCount_ = 0;
  for (auto const & bmf : Faces_)
  {
    totVertexCount_ += bmf.second->GetBuffer()->VertexCount(); 
  }
  if (totVertexCount_)
  {
    size_t i = 0;
    unsigned tvc = 0;
    unsigned stride = sizeof(bmf::TUS2_VS2_CUB4);
    buffer_.bind(GL_ARRAY_BUFFER);
    buffer_.data(stride*totVertexCount_, 0, GL_STREAM_DRAW);
    for (auto & bmf : Faces_)
    {
      unsigned vc = bmf.second->GetBuffer()->VertexCount();
      if (vc) 
      {
        buffer_.sub_data(stride*tvc, stride * vc, bmf.second->GetBuffer()->AttribPointer());
      }
      vc_[i++] = vc;
      tvc += vc;
      bmf.second->GetBuffer()->flush();
    }
  }
}


void GLSLText::Render()
{
  if (totVertexCount_)
  {
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    GGl.use_program("Text");
    vao_.bind();
    unsigned tvc = 0;
    size_t i = 0;
    for (auto & bmf : Faces_)
    {
      if (vc_[i]) { bmf.second->BindTexture(2); glDrawArrays(GL_LINES, tvc, vc_[i]); }
      tvc += vc_[i];
      ++i;
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
}


glm::ivec2 GLSLText::extent(const char* str)
{
  bmf::Text2D tmp(*text2D_);
  tmp << bmf::orig_manip(0, 0);
  tmp << bmf::cursor_manip(0, 0);
  glm::ivec2 tmpCursor = tmp.GetCursor();
  glmutils::tbbox<glm::ivec2> box(glm::ivec2(0));  
  const char* first(str);
  const char* last(first + strlen(str));
  for (; first != last; )
  {
    const char* pivot = first;
    for (; (first != last) && (*first != '\\'); ++first) {}
    glm::ivec2 ext = tmp.Extent(pivot, first, tmpCursor);
    glmutils::include(box, tmpCursor);
    glmutils::include(box, ext);
    first = parse_commands(tmp, first, last);
    if (*first == '\\') 
    {
      glm::ivec2 ext = tmp.Extent(first, ++first, tmpCursor);
      glmutils::include(box, tmpCursor);
      glmutils::include(box, ext);
    }
  }
  return glmutils::extent(box);
}


void GLSLText::print(const char* str)
{
  const char* first(str);
  const char* last(first + strlen(str));
  for (; first != last; )
  {
    const char* pivot = first;
    for (; (first != last) && (*first != '\\'); ++first) {}
    text2D_->Stream(pivot, first);
    first = parse_commands(*text2D_, first, last);
    if (*first == '\\') 
    {
      text2D_->Stream(first, ++first);
    }
  }
}


bool GLSLText::match_command(bmf::Text2D& txt, const char* command, const char* argf, const char* argl)
{
  char ch('\0');
  if (0 == strcmp(command, "color"))
  {
    std::istringstream ss(std::string(argf, argl));
    float r,g,b;
    ss >> r >> g >> b >> ch;
    if (ch == '}') 
    {
      *text2D_ << bmf::color_manip(r, g, b, 1.0f);
    }
    return ch == '}';
  }
  if (0 == strcmp(command, "defcolor"))
  {
    ch = *argf;
    if (ch == '}') 
    {
      glm::vec4 c = GGl.textColor();
      *text2D_ << bmf::color_manip(c.x, c.y, c.z, c.w);
    }
    return ch == '}';
  }
  else if (0 == strcmp(command, "orig"))
  {
    std::istringstream ss(std::string(argf, argl));
    int x, y;
    ss >> x >> y >> ch;
    if (ch == '}') 
    {
      *text2D_ << bmf::orig_manip(x, y);
    }
    return ch == '}';
  }
  else if (0 == strcmp(command, "cursor"))
  {
    std::istringstream ss(std::string(argf, argl));
    int x, y;
    ss >> x >> y >> ch;
    if (ch == '}') 
    {
      *text2D_ << bmf::cursor_manip(x, y);
    }
    return ch == '}';
  }
  else if (0 == strcmp(command, "tabsize"))
  {
    std::istringstream ss(std::string(argf, argl));
    int x;
    ss >> x >> ch;
    if (ch == '}') 
    {
      *text2D_ << bmf::tab_manip(x);
    }
    return ch == '}';
  }
  else 
  {
    auto it = Faces_.find(command);
    if (it != Faces_.end())
    {
      ch = *argf;
      if (ch == '}') 
      {
        txt.SetFont(it->second);
      }
      return ch == '}';
    }
  }
  return false;
}


const char* GLSLText::parse_commands(bmf::Text2D& txt, const char* first, const char* last)
{
parse_next_command:
  if (first != last && *first == '\\')
  {
    for (const char* cmd(first); (cmd != last); ++cmd)
    {
      if (*cmd == '{')
      {
        for (const char* args(cmd); args != last; ++args)
        {
          if (*args == '}')
          {
            std::string cmdstr(first + 1, cmd);
            if (match_command(txt, &cmdstr[0], cmd + 1, args + 1))
            {
              first = args + 1;
              goto parse_next_command;
            }
          }
        }
      }
    }
  }
  return first;
}


ITextBox* GLSLText::get_text_box(const char* name)
{
  auto it = TextBoxes_.find(name);
  return (it != TextBoxes_.end()) ? it->second.get() : 0;
}


void GLSLText::set_text_box(ITextBox* ptr)
{
  TextBoxes_[ptr->GetName()] = std::shared_ptr<ITextBox>(ptr);
}


void GLSLText::delete_text_box(const char* name)
{
  TextBoxes_.erase(name);
}


#undef MAX_FACES

