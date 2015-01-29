#include <boost/random/uniform_01.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <glmutils/random.hpp>
#include "initial.hpp"
#include "omp_random.hpp"


CInitializer::CInitializer(const libParam::Initial& param)
:  bucket_(0), param_(param)
{
  if (param_.Pdf == "const") 
  {
    pdf_ = PDF_CONST;
  }
  else if (param_.Pdf == "geometric") 
  {
    pdf_ = PDF_GEOMETRIC;
  }
  int flocks = (PDF_CONST == pdf_) ? static_cast<int>(param_.distParam) : 1000;
  omp_rnd_eng& eng(omp_eng());
  boost::random::uniform_01<float> rnd01;
  for (int i=0; i<=flocks; ++i)
  {
    float r = param_.minRadius + (param_.maxRadius - param_.minRadius) * rnd01(eng);
    vec2 dir = glmutils::unit_vec2(eng);
    centers_.push_back( vec3(r * dir.x, param.altitude, r * dir.y) );
    forwards_.push_back( glmutils::vec3_in_sphere(eng) );
  }
}


vec3 CInitializer::position()
{
  return (pdf_ == PDF_CONST) ? do_const_position() : do_geometric_position();
}


vec3 CInitializer::forward()
{
  return normalize( forwards_[bucket_] + 0.01f * glmutils::vec3_in_sphere(omp_eng()) );
}


vec3 CInitializer::do_const_position()
{
  float flocks = param_.distParam;
  float nf = float(param_.numPrey) / flocks;
  float Vf = nf / param_.density;
  float flockR = std::powf((3.0f * Vf) / (4.0f * Pif), 0.3333333f);
  omp_rnd_eng& eng(omp_eng());
  boost::random::uniform_01<float> rnd01;
  bucket_ = size_t((centers_.size()-1) * rnd01(eng));
  vec3 ret = centers_[bucket_] + flockR * glmutils::vec3_in_sphere(eng);
  ret.y = glm::abs(ret.y);
  return ret;
}


vec3 CInitializer::do_geometric_position()
{
  float p = param_.distParam;
  omp_rnd_eng& eng(omp_eng());
  boost::geometric_distribution<int, float> geo(p);
  do bucket_ = static_cast<size_t>(geo(eng))-1; while (bucket_ >= centers_.size());
  float nf = (1-p)*std::pow(p, static_cast<float>(bucket_)) * float(param_.numPrey);
  float Vf = nf / param_.density;
  float flockR = std::pow((3.0f * Vf) / (4.0f * Pif), 0.3333333f);
  vec3 ret = centers_[bucket_] + flockR * glmutils::vec3_in_sphere(eng);
  ret.y = glm::abs(ret.y);
  return ret;
}


