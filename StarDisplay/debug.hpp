#ifndef STARDISPLAY_DEBUG_HPP_INCLUDED
#define STARDISPLAY_DEBUG_HPP_INCLUDED

#include <exception>
#include <glsl/glsl.hpp>


namespace debug
{
  void StackDump(int level);


  void __stdcall GLDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar* message, GLvoid* userParam);


  void __stdcall GLDebugLogOnce(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, GLvoid* userParam);

}

#endif
