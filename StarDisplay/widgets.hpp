#ifndef WIDGETS_HPP_INCLUDED
#define WIDGETS_HPP_INCLUDED

//#include <map>
//#include <string>
//#include <vector>
//#include <istream>
//#include <boost/shared_ptr.hpp>
//#include <glsl/texture.hpp>
//#include <glsl/vertexarray.hpp>
//#include <glsl/buffer.hpp>
//#include "glmfwd.hpp"
//
//
//class Widget
//{
//public:
//  Widget() : ext_(0) {};
//  virtual ~Widget() {};
//
//  virtual void read(std::istream& is);
//
//  const std::string& Name() { return name; }
//
//  bool IsHidden() const { return hidden; };
//  void Show() { hidden = false; }
//  void Hide() { hidden = true; }
//
//  ! Returns <hit, need tracking> pair
//  virtual std::pair<bool, bool> OnLButtonDown(const ivec2& mouse) { return std::make_pair(false, false); }
//  virtual bool OnLButtonUp(const ivec2& mouse) { return false; }
//  
//  virtual void EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va);
//  virtual void Render() {};
//
//protected:
//  bool HitTest(const ivec2& mouse) const;
//
//  std::string name;
//  bool        hidden; 
//  ivec2       position;     // relative position
//  ivec2       wpos_;    // position on window
//  ivec2       ext_;
//  unsigned    vaOfs_;   // Offset in vertex buffer
//};
//
//
//class Static : public Widget
//{
//public:
//  virtual void read(std::istream& is);
//  virtual bool OnLButtonUp(const ivec2& mouse);
//  virtual void EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va);
//  virtual void Render();
//
//private:
//  std::string onClick;
//  glsl::texture tex_;
//};
//
//
//enum ButtonState
//{
//  BUTTON_UP,
//  BUTTON_DOWN,
//  BUTTON_PRESSED,
//  BUTTON_MAX_STATE
//};
//
//
//class Button : public Widget
//{
//public:
//  virtual void read(std::istream& is);
//
//  ButtonState State() { return state; }
//  virtual std::pair<bool, bool> OnLButtonDown(const ivec2& mouse);
//  virtual bool OnLButtonUp(const ivec2& mouse);
//  virtual void EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va);
//
//private:
//  bool Switch;
//  ButtonState state;
//  std::string onDown;
//  std::string onUp;
//  glsl::texture tex_;
//};
//
//
//class Widgets
//{
//  typedef std::map<std::string, boost::shared_ptr<Widget> > map_type;
//
//private:
//  Widgets();
//  void read(std::istream& is);
//
//public:
//  ~Widgets();
//  void Init();
//  bool empty() const { return !BackgroundTileTex_.isValid() && map_.empty(); }
//  const ivec4& ViewportDecr() const  { return ViewportDecr_; }
//
//  std::pair<bool, bool> OnLButtonDown(const ivec2& mouse);
//  bool OnLButtonUp(const ivec2& mouse);
//  void OnSize(const ivec4& WindowsRect);
//  void Render();
//
//private:
//  vec4              ViewportBorder;              // Viewport border
//  ivec4             ViewportDecr_;
//  map_type          map_;
//
//  glsl::texture     BackgroundTileTex_;
//  ivec2             ext_;
//  glsl::buffer      buffer_;
//  glsl::vertexarray vao_;
//  std::string       tracked_;
//
//  friend boost::shared_ptr<Widgets> CreateWidgets();
//};
//
//
//boost::shared_ptr<Widgets> CreateWidgets();
//
//
#endif
