//! \file histogram.hpp Generic histogram logger.
//! \ingroup Analysis

#ifndef HISTOGRAM_HPP_INCLUDED
#define HISTOGRAM_HPP_INCLUDED

#include <vector>
#include "glmfwd.hpp"
#include "Params.hpp"

class histogram;
class histogram2D;


class histogram
{
  typedef std::vector<double>         counts_vect;
  typedef counts_vect::const_iterator const_iterator;
  typedef counts_vect::iterator       iterator;

public:
  typedef std::vector<glm::dvec2> cdf_vect;
  typedef glm::dvec3              quartiles_t;

  histogram();
  explicit histogram(const Param::FeatureMap::hist& h);
  histogram(double x_min, double x_max, int bins);

  void reset();
  void reset(const Param::FeatureMap::hist& h);
  void reset(double x_min, double x_max, int bins);

  void reduce(double factor);
  void append(const histogram& hist);

  template<typename U>
  void operator()(U value);

  template<typename U>
  void operator()(U value, int times);

  double count() const { return samples_; }
  double max_count() const;
  unsigned num_bins() const { return static_cast<unsigned>(counts_.size()); }
  glm::dvec2 operator[](size_t i) const;          //!< \return dvec2(bin_center, count)
  double quantile(double Q) const;
  double quantile(double Q, const cdf_vect& cdf) const;
  quartiles_t quartiles() const;
  quartiles_t quartiles(const cdf_vect& cdf) const;
  void CDF(cdf_vect& cdf) const;
  const counts_vect bins() const { return counts_; }
  double mean() const 
  {

  }

private:
  double bin_lo(int i) const { return i / binsScale_ + x_min(); }
  double x_min() const { return -binsOffs_/binsScale_; }
  double x_max() const { return x_min() + binsScale_*(counts_.size()-1); }

  double binsScale_;
  double binsOffs_;
  double samples_;
  counts_vect counts_;
};


class histogram2D
{
  typedef std::vector<histogram>      hists_vect;
  typedef hists_vect::const_iterator  const_iterator;
  typedef hists_vect::iterator        iterator;

public:
  histogram2D();
  histogram2D(double x_min, double x_max, int x_bins,
              double y_min, double y_max, int y_bins);
  void reset();
  void reset(double x_min, double x_max, int x_bins,
             double y_min, double y_max, int y_bins);

  template<typename U>
  void operator()(U x_value, U y_value);

  template<typename U>
  void operator()(U x_value, U y_value, int times);

  size_t size() const { return hists_.size(); }
  size_t count() const { return count_; }
  const histogram& operator[](size_t i) const { assert(i < hists_.size()); return hists_[i]; }
  histogram& operator[](size_t i) { assert(i < hists_.size()); return hists_[i]; }

private:
  double binsScale_;
  double binsOffs_;
  size_t count_;
  hists_vect hists_;
};


////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////

namespace detail {

  inline double linp(double b0, double b1, double ql, double qh, double q)
  {
    double dq = qh - ql;
    double x = (std::abs(dq) > 10e-20) ? (q - ql) / dq : 0.5;
    double y = glm::mix(b0, b1, x);
    return y;
  }
}


inline histogram::histogram()
{
  reset(0, 1, 0);
}


inline histogram::histogram(const Param::FeatureMap::hist& h)
{
  reset(h);
}


inline histogram::histogram(double x_min, double x_max, int bins)
{
  reset(x_min, x_max, bins);
}


inline void histogram::reset()
{
  samples_ = 0;
  counts_.assign(num_bins(), 0);
}


inline void histogram::reset(const Param::FeatureMap::hist& h)
{
  reset(h.min, h.max, h.bins);
}


inline void histogram::reset(double x_min, double x_max, int bins)
{
  binsScale_ = static_cast<double>(bins) / double(x_max - x_min);
  binsOffs_ = - binsScale_ * double(x_min);
  counts_.resize(bins);
  reset();
}


inline void histogram::reduce(double factor)
{
  std::for_each(counts_.begin(), counts_.end(), [factor] (double& val)
  {
    val *= factor;
  });
  samples_ *= factor;
}


inline void histogram::append(const histogram& h)
{
  bool compatible = (num_bins() == h.num_bins()) && (binsScale_ == h.binsScale_) && (binsOffs_ == h.binsOffs_);
  if (!compatible)
  {
    throw std::exception("histogram::append called with incompatible argument.");
  }
  counts_vect::const_iterator arg = h.counts_.begin();
  std::for_each(counts_.begin(), counts_.end(), [arg] (double& val) { val += *arg; });
  samples_ += h.samples_;
}


inline double histogram::max_count() const 
{ 
  const_iterator it = std::max_element(counts_.begin(), counts_.end()); 
  return (it != counts_.end()) ? *it : 0;
}


inline double histogram::quantile(double q) const
{
  cdf_vect cdf;
  CDF(cdf);
  return quantile(q, cdf);
}


inline double histogram::quantile(double Q, const cdf_vect& cdf) const
{
  const size_t bins = cdf.size()-1; 
  if (0 == bins) return std::numeric_limits<double>::quiet_NaN();
  size_t i = 0;
  while ((i<bins) && (cdf[i+1].y < Q)) { ++i; }
  return detail::linp(cdf[i].x, cdf[i+1].x, cdf[i].y, cdf[i+1].y, Q);
}


inline histogram::quartiles_t histogram::quartiles() const
{
  cdf_vect cdf;
  CDF(cdf);
  return quartiles(cdf);
}


inline histogram::quartiles_t histogram::quartiles(const cdf_vect& cdf) const
{
  return quartiles_t(quantile(0.25, cdf), quantile(0.50, cdf), quantile(0.75, cdf));
}


inline void histogram::CDF(cdf_vect& cdf) const
{
  cdf.clear();
  double scale = 1.0 / samples_;
  const unsigned n = num_bins();
  double cs = 0;
  unsigned i = 0;
  for (; (i<n) && (counts_[i] < 10e-10); ++i);
  cdf.emplace_back( bin_lo(i-1), 0.0 );
  for (; i<n; ++i)
  {
    if (counts_[i] > 10e-100)
    {
      cdf.emplace_back( bin_lo(i+1), scale * (cs += counts_[i]) );
    }
  }
}


template<typename U>
inline void histogram::operator()(U value)
{
  double dval(value);
  if (! glmutils::any_nan(dval))
  {
    ++samples_;
    int bin = static_cast<int>( binsScale_ * dval + binsOffs_ );
    // clamp bin to [0,maxBin]
    ++counts_[std::max(std::min(bin, static_cast<int>(num_bins())-1), 0)];
  }
}


template<typename U>
inline void histogram::operator()(U value, int times)
{
  double dval(value);
  if (! glmutils::any_nan(dval))
  {
    samples_ += times;
    int bin = static_cast<int>( binsScale_ * dval + binsOffs_ );
    // clamp bin to [0,maxBin]
    counts_[std::max(std::min(bin, static_cast<int>(num_bins())-1), 0)] += times;
  }
}


inline glm::dvec2 histogram::operator[](size_t i) const 
{ 
  assert(i < counts_.size()); 
  return glm::dvec2(bin_lo(static_cast<int>(i)), counts_[i]); 
}


template<typename U>
inline void histogram2D::operator()(U x_value, U y_value)
{
  double dxval(value);
  if (! glmutils::any_nan(dxval))
  {
    ++count_;
    int bin = static_cast<int>( binsScale_ * double(dxval) + binsOffs_ );
    // clamp bin to [0,maxBin]
    hists_[std::max(std::min(bin, static_cast<int>(size())-1), 0)](y_value);
  }
}

template<typename U>
inline void histogram2D::operator()(U x_value, U y_value, int times)
{
  double dxval(value);
  if (! glmutils::any_nan(dxval))
  {
    count_ += times;
    int bin = static_cast<int>( binsScale_ * double(dxval) + binsOffs_ );
    // clamp bin to [0,maxBin]
    hists_[std::max(std::min(bin, static_cast<int>(size())-1), 0)](y_value, times);
  }
}


inline histogram2D::histogram2D() 
{
}


inline histogram2D::histogram2D(double x_min, double x_max, int x_bins,
                                double y_min, double y_max, int y_bins)
{
  reset(x_min, x_max, x_bins, y_min, y_max, y_bins);
}


inline void histogram2D::reset()
{
  count_ = 0;
  for (size_t i=0; i<hists_.size(); ++i)
  {
    hists_[i].reset();
  }
}


inline void histogram2D::reset(double x_min, double x_max, int x_bins,
                               double y_min, double y_max, int y_bins)
{
  binsScale_ = static_cast<double>(x_bins) / double(x_max - x_min);
  binsOffs_ = - binsScale_ * double(x_min);
  count_ = 0;
  hists_.clear();
  for (int i=0; i<x_bins; ++i)
  {
    hists_.emplace_back( y_min, y_max, y_bins );
  }
}


#endif
