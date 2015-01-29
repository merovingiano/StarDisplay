#ifndef IGUIBOX_HPP_INCLUDED
#define IGUIBOX_HPP_INCLUDED

#include <string>
#include <glm/glm.hpp>


class __declspec(novtable) IGuiBox
{
public:
  virtual ~IGuiBox() {}
  virtual glm::ivec4 GetBox() const = 0;
  virtual const std::string& GetName() const = 0;
  virtual void resize() = 0;
};


#endif


