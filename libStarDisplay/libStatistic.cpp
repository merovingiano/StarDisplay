#include <ostream>
#include <string>
#include <luabind/luabind.hpp>
#include "IStatistic.hpp"


using namespace luabind;


namespace libStatistic
{

  class Statistic : public IStatistic
  {
  public:
    Statistic(object& self);
    virtual ~Statistic();
    virtual void apply(double stat_dt);
    virtual void finalize();
    virtual void Reset();
    virtual std::string labelText() const;
    virtual void Display() const;
    virtual void save(const char* fname, bool append) const;

  private:
    object self_;
  };


  Statistic::Statistic(object& self) : IStatistic(), self_(self) 
  {
  }

  Statistic::~Statistic() 
  {
  }

  void Statistic::finalize()
  {
    self_["finalize"](self_);
  }

  void Statistic::Reset()
  {
    self_["reset"](self_);
  }

  void Statistic::apply(double stat_dt)
  {
    self_["apply"](self_, stat_dt);
  }

  std::string Statistic::labelText() const
  {
    return "Custom statistic";
  }

  void Statistic::Display() const
  {
    self_["display"](self_);
  }

  void Statistic::save(const char* fname, bool append) const
  {
    self_["save"](self_, fname, append);
  }


  IStatistic* newStatistic(object self)
  {
    return new Statistic(self);
  }
}


void luaopen_libStatistic(lua_State* L)
{
  module(L, "Statistic")
  [
    def("new", &libStatistic::newStatistic),

    class_<IStatistic>("__statistic_base"),
    class_<libStatistic::Statistic, IStatistic>("__statistic")
  ];
}

