#ifndef GLSLLOCALPROG_HPP_INCLUDED
#define GLSLLOCALPROG_HPP_INCLUDED

#include <memory>


class GLSLLocals
{
public:
  GLSLLocals(const char* Progname);
  void Emmit();
  void Flush();
  void Render();

private:
  std::unique_ptr<class GLSLImm> imm_;
};


#endif
