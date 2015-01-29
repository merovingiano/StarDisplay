#include "debug.hpp"
#include <set>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <algorithm>


#ifndef NDEBUG
  #include <DbgHelp.h>
  #pragma comment(lib,"Dbghelp")
#endif


namespace debug
{

#ifndef NDEBUG

  void StackDump(int level) 
  {
    unsigned int   i;
    void         * stack[ 100 ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;

    level = std::min(100, level);
    if (level <= 0) return;

    process = GetCurrentProcess();
    SymSetOptions(SYMOPT_LOAD_LINES);
    SymInitialize( process, NULL, TRUE );

    frames               = CaptureStackBackTrace( 0, level, stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    std::cerr << "C++ stack dump:\n";
    for( i = 0; i < frames; i++ )
    {
      SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );
      DWORD  dwDisplacement;
      IMAGEHLP_LINE64 line;

      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      if (!strstr(symbol->Name,"debug::") &&
        SymGetLineFromAddr64(process, ( DWORD64 )( stack[ i ] ), &dwDisplacement, &line)) {

          std::cerr << "function: " << symbol->Name << 
            " - line: " << line.LineNumber << "\n";

      }
      if (0 == strcmp(symbol->Name,"main"))
        break;
    }

    free( symbol );
  }

#else

  void StackDump(int) {}

#endif


  // aux function to translate source to string
  const char*  getStringForSource(GLenum source) 
  {
    switch(source) {
    case GL_DEBUG_SOURCE_API_ARB: 
      return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
      return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
      return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
      return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
      return "Application";
    case GL_DEBUG_SOURCE_OTHER_ARB:
      return "Other";
    default:
      return "";
    }
  }

  // aux function to translate severity to string
  const char*  getStringForSeverity(GLenum severity) 
  {
    switch(severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB: 
      return "High";
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
      return "Medium";
    case GL_DEBUG_SEVERITY_LOW_ARB:
      return "Low";
    default:
      return "";
    }
  }

  // aux function to translate type to string
  const char* getStringForType(GLenum type) 
  {
    switch(type) {
    case GL_DEBUG_TYPE_ERROR_ARB: 
      return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
      return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
      return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
      return "Portability Issue";
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
      return "Performance Issue";
    case GL_DEBUG_TYPE_OTHER_ARB:
      return "Other";
    default:
      return "";
    }
  }


  std::ostream& DebugLogStream(std::ostream& os, GLenum source, GLenum type, GLuint id, GLenum severity,
                               GLsizei length, const GLchar* message, GLvoid* userParam)
  {
    os << "Source: " << getStringForSource(source);
    os << "  Severity: " << getStringForSeverity(severity) << '\n';
    os << message << '\n';
    return os;
  }


  void __stdcall GLDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar* message, GLvoid* userParam)
  {
    std::cerr << "\nOpenGL log (" << id << "):  ";
    DebugLogStream(std::cerr, source, type, id, severity, length, message, userParam);
    StackDump((int)(userParam));
  }


  void __stdcall GLDebugLogOnce(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, GLvoid* userParam)
  {
    static std::set<std::string> set;
    std::ostringstream ss;
    DebugLogStream(ss, source, type, id, severity, length, message, userParam);
    std::pair<std::set<std::string>::iterator, bool> ip = set.insert(ss.str());
    if (ip.second)
    {
      std::cerr << "\nOpenGL log (" << id << "):  " << *ip.first;
      StackDump((int)(userParam));
    }
  }

}

