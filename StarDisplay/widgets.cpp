/********
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/istream.hpp>
#include <glsl/shader_pool.hpp>
#include "iotexture.hpp"
#include "widgets.hpp"
#include "Parameter.hpp"
#include "log.hpp"
#include "Camera.hpp"


using namespace param_utils;

extern glsl::shader_pool::pool gShaderPool;


void Widget::read(std::istream& is)
{
  READ_PARAMETER(is, name);
  READ_PARAMETER(is, hidden);
  READ_PARAMETER(is, position);
}


void Widget::EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va)
{
  wpos_.x = (position.x >= 0) ? position.x : WindowsRect.z + position.x;
  wpos_.y = (position.y >= 0) ? position.y : WindowsRect.w + position.y;
  vaOfs_ = (unsigned)va.size();
}


bool Widget::HitTest(const ivec2& mouse) const
{
  return !hidden && mouse.x > wpos_.x && mouse.x < (wpos_.x + ext_.x) && mouse.y > wpos_.y && mouse.y < (wpos_.y + ext_.y);
}


void Static::read(std::istream& is)
{
  Widget::read(is);
  tex_ = LoadTexture((CParam::MediaPath / "widgets" / (name + ".png")).string(), false, &ext_);
  tex_.set_wrap_filter(GL_REPEAT, GL_LINEAR);
  READ_PARAMETER(is, onClick);
}


bool Static::OnLButtonUp(const ivec2& mouse)
{
  if (Widget::HitTest(mouse))
  {
    nv::EvalChunk(onClick);
    return true;
  }
  return false;
}


void Static::EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va)
{
  Widget::EnqueVertexAttribs(WindowsRect, va);
  va.push_back( vec4(0, 0, wpos_.x, wpos_.y) );
  va.push_back( vec4(1, 1, wpos_.x + ext_.x, wpos_.y + ext_.y) );
}


void Static::Render()
{
  tex_.bind();
  glDrawArrays(GL_LINES, vaOfs_, 2);
}


std::pair<bool, bool> Button::OnLButtonDown(const ivec2& mouse)
{
  return Widget::HitTest(mouse) ? std::make_pair(true, false) : std::make_pair(false, false);
}


bool Button::OnLButtonUp(const ivec2& mouse)
{
  return Widget::HitTest(mouse);
}


void Button::EnqueVertexAttribs(const ivec4& WindowsRect, std::vector<vec4>& va)
{
  Widget::EnqueVertexAttribs(WindowsRect, va);
}


void Button::read(std::istream& is)
{
  Widget::read(is);
  READ_PARAMETER(is, Switch);
  READ_PARAMETER_ENUM(is, state, 0, BUTTON_MAX_STATE-1);
  tex_ = LoadTexture((CParam::MediaPath / "widgets" / (name + ".png")).string(), false, &ext_);
  tex_.set_wrap_filter(GL_REPEAT, GL_LINEAR);
  READ_PARAMETER(is, onDown); 
  READ_PARAMETER(is, onUp);
}


namespace {

  template <typename T>
  boost::shared_ptr<T> read_widget_as_array(std::istream& is)
  {
    char c;
    cpp_skip(is);
    is >> c; if (c != '{') throw parse_error("Widgets::read: missing \'{\'");
    boost::shared_ptr<T> P( new T() );
    P->read(is);
    cpp_skip(is);
    is >> c; if (c != '}') throw parse_error("Widgets::read: missing \'}\'");
    return P;
  }

}


Widgets::Widgets()
  : ViewportBorder(0), ViewportDecr_(0)
{
}


void Widgets::Init()
{
  if (empty()) return;
  gShaderPool.use("Widget");
  gShaderPool.attribute("vAttribs", 0);
  gShaderPool.uniform("WidgetTexture")->set1i(4);

  vao_.bind();
  buffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)(char*)(0));
}


Widgets::~Widgets()
{

}


void Widgets::read(std::istream& is)
{
  std::boolalpha(is);
  READ_PARAMETER(is, ViewportBorder);
  ViewportDecr_ = ivec4(ViewportBorder);
  ViewportDecr_.z = -(ViewportDecr_.z + ViewportDecr_.x);    // decrement in width
  ViewportDecr_.w = -(ViewportDecr_.w + ViewportDecr_.y);    // decrement in height
  std::string BackgroundTile;
  READ_PARAMETER(is, BackgroundTile);
  BackgroundTileTex_ = LoadTexture((CParam::MediaPath / "widgets" / (BackgroundTile + ".png")).string(), false, &ext_);
  BackgroundTileTex_.set_wrap_filter(GL_REPEAT, GL_LINEAR);
  if (!BackgroundTileTex_.isValid())
  {
    gLog << "Widgets::read: Invalid BackgroundTile texture";
  }

  while (is)
  {
    boost::shared_ptr<Widget> P;
    std::string what = match_unknown_assign(is);
    if (what == "Static") P = read_widget_as_array<Static>(is);
    else if (what == "Button") P = read_widget_as_array<Button>(is);
    else throw parse_error("Widgets::read: Unknown Widget");
    map_[P->Name()] = P;
    cpp_skip(is);
  }
}


std::pair<bool, bool> Widgets::OnLButtonDown(const ivec2& mouse)
{
  map_type::iterator first(map_.begin());
  map_type::iterator last(map_.end());
  for (; first != last; ++first)
  {
    std::pair<bool, bool> ret = first->second->OnLButtonDown(mouse);
    if (ret.second) tracked_ = first->second->Name();
    if (ret.first) return ret;
  }
  return std::make_pair(false, false);
}


bool Widgets::OnLButtonUp(const ivec2& mouse)
{
  map_type::iterator first(map_.begin());
  map_type::iterator last(map_.end());
  for (; first != last; ++first)
  {
    if (first->second->OnLButtonUp(mouse)) return true;
  }
  tracked_.clear();
  return false;
}


void Widgets::OnSize(const ivec4& WindowsRect)
{
  if (empty()) return;

  // Compute tile vertex attribs
  std::vector<vec4> va;

  if (BackgroundTileTex_.isValid())
  {
    vec4 wr(WindowsRect);
    float texX = 1.0f / ext_.x;
    float texY = 1.0f / ext_.y;

    // top stripe
    va.push_back( vec4(0, 0, 0, 0) );
    va.push_back( vec4(wr.z * texX, ViewportBorder.w * texY, wr.z, ViewportBorder.w) );
    // bottom stripe
    va.push_back( vec4(0, (wr.w - ViewportBorder.y) * texY, 0, wr.w - ViewportBorder.y) );
    va.push_back( vec4(wr.z * texX, wr.w * texY, wr.z, wr.w) );       
    // left stripe
    va.push_back( vec4(0, ViewportBorder.w * texY, 0, ViewportBorder.w) );
    va.push_back( vec4(ViewportBorder.x * texX, (wr.w - ViewportBorder.y) * texY, ViewportBorder.x, wr.w - ViewportBorder.y) );
    // right stripe
    va.push_back( vec4((wr.z - ViewportBorder.z) * texX, ViewportBorder.w * texY, wr.z - ViewportBorder.z, ViewportBorder.w) );
    va.push_back( vec4(wr.z * texX, (wr.w - ViewportBorder.y) * texY, wr.z, wr.w - ViewportBorder.y) );

  }
  map_type::iterator first(map_.begin());
  map_type::iterator last(map_.end());
  for (; first != last; ++first)
  {
    first->second->EnqueVertexAttribs(WindowsRect, va);
  }
  buffer_.bind(GL_ARRAY_BUFFER);
  buffer_.data(sizeof(vec4) * va.size(), (void*)&va[0], GL_STREAM_DRAW);
}


void Widgets::Render()
{
  if (empty()) return;

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  gShaderPool.use("Widget");
  vao_.bind();
  if (BackgroundTileTex_.isValid())
  {
    BackgroundTileTex_.bind(4);
    glDrawArrays(GL_LINES, 0, 8);
  }

  map_type::iterator first(map_.begin());
  map_type::iterator last(map_.end());
  for (; first != last; ++first)
  {
    if (!first->second->IsHidden()) first->second->Render();
  }
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}


boost::shared_ptr<Widgets> CreateWidgets()
{
  boost::shared_ptr<Widgets> P( new Widgets() );
  if (!CParam::widgets.empty()) 
  {
    try
    {
      std::ifstream ifs((CParam::ExePath / CParam::widgets).string());
      P->read(ifs);
    }
    catch (std::exception& e)
    {
      LOGEXCEPTION("CreateWidgets failed", e.what());
      P.reset( new Widgets() );
    }
    catch (...)
    {
      LOGEXCEPTION("CreateWidgets failed", "");
      P.reset( new Widgets() );
    }
  }
  return P;
}
**********/
