//! \file accumulators.hpp Generic data logger
//! \ingroup Analysis

#ifndef ACCUMULATORS_HPP_INCLUDED
#define ACCUMULATORS_HPP_INCLUDED


#include <limits>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>
#include "glmfwd.hpp"


template<typename T> class kahan;
template<typename T> class average;
template<typename T> class min_max_mean;
template<typename T> class accumulator;


namespace std 
{
  // count mean
  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const ::average<T>& x);

  // count min max mean
  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const ::min_max_mean<T>& x);

  // count min max mean variance
  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const ::accumulator<T>& x);
}


template<typename T>
class kahan
{
public:
  kahan(): c_(0), sum_(0) {}
  explicit kahan(T initial): c_(0), sum_(initial) {}

  kahan<T>& operator = (T x)
  {
    c_ = T(0);
    sum_ = x;
    return *this;
  }

  kahan<T>& operator()(T x) 
  {
    if (! glmutils::any_nan(x))
    {
      T y = x - c_;
      T t = sum_ + y;
      c_ = (t - sum_) - y;
      sum_ = t;
    }
    return *this;
  }

  T value() const { return sum_; }

private:
  T c_, sum_;
};



template<typename T>
class average
{
public:
  typedef T value_type;

  average();
  void reset();
  unsigned count() const { return count_; }
  value_type sum() const { return sum_; }
  value_type mean() const;
  void append(const average<T>& av) { count_ += av.count_; sum_ += av.sum_; }
  template<typename U>
  void operator()(U value);

private:
  unsigned    count_;
  value_type  sum_;
};



template<typename T>
class min_max_mean
{
public:
  typedef T value_type;

  min_max_mean();
  void reset();
  unsigned count() const { return mean_.count(); }
  value_type sum() const { return mean_.sum(); }
  value_type min() const { return min_; }
  value_type max() const { return max_; }
  value_type mean() const { return mean_.mean(); }

  template<typename U>
  void operator()(U value);

private:
  average<T> mean_;
  value_type  min_; 
  value_type  max_;
};


template<typename T>
class accumulator
{
public:
  typedef T value_type;
  typedef std::vector<value_type> data_vector;

  accumulator();
  void reset() const;
  
  template<typename U>
  void operator()(U val) const;
  
  template<typename U>
  void operator()(U val, size_t times) const;

  unsigned count() const { return mmm_.count(); }
  value_type sum() const { return mmm_.sum(); }
  value_type min() const { return mmm_.min(); }
  value_type max() const { return mmm_.max(); }
  value_type mean() const { return mmm_.mean(); }
  value_type variance() const;

  const data_vector& data() const { return data_; }

private:
  mutable min_max_mean<value_type> mmm_;
  mutable data_vector data_;
};

////////////////////////////////////////////////////////////////////////////


template<bool>
struct acc_min_max
{
  template<typename T>
  static T min(const T& a, const T& b) { return std::min(a, b); }
  template<typename T>
  static T max(const T& a, const T& b) { return std::max(a, b); }
};


template<>
struct acc_min_max<false>
{
  template<typename T>
  static T min(const T& a, const T& b) { return glm::min(a, b); }
  template<typename T>
  static T max(const T& a, const T& b) { return glm::max(a, b); }
};


template<typename T>
inline T acc_min(const T& a, const T& b) { return acc_min_max< boost::is_arithmetic<T>::value >::min(a, b); }

template<typename T>
inline T acc_max(const T& a, const T& b) { return acc_min_max< boost::is_arithmetic<T>::value >::max(a, b); }



template<typename T>
inline average<T>::average() 
{ 
  reset(); 
}


template<typename T>
inline void average<T>::reset()
{
  count_ = 0;
  sum_ = value_type(0);
}


template<typename T>
inline T average<T>::mean() const 
{ 
  return (count_) ? sum_ / value_type(count_) : value_type(std::numeric_limits<double>::quiet_NaN());
}


template<typename T>
template<typename U>
inline void average<T>::operator()(U value)
{
  if (! glmutils::any_nan(value))
  {
    value_type tval(value);
    ++count_;
    sum_ += tval;
  }
}


template<typename T>
inline min_max_mean<T>::min_max_mean() 
{ 
  reset(); 
}

template<typename T>
inline void min_max_mean<T>::reset()
{
  mean_.reset();
  min_ = value_type(std::numeric_limits<float>::max());
  max_ = value_type(-std::numeric_limits<float>::max());
}


template<typename T>
template<typename U>
inline void min_max_mean<T>::operator()(U value)
{
  value_type tval(value);
  if (! glmutils::any_nan(tval))
  {
    mean_(tval);
    min_ = acc_min(tval, min_);
    max_ = acc_max(tval, max_);
  }
}


template<typename T>
inline accumulator<T>::accumulator() 
{
  reset();
} 

template<typename T>
inline void accumulator<T>::reset() const 
{ 
  mmm_.reset(); data_.clear(); 
}


template<typename T>
inline T accumulator<T>::variance() const
{
  value_type res(0);
  if (count() > 2)
  {
    value_type xq = mean();
    std::for_each(data_.begin(), data_.end(), [xq, &res] (const data_vector::value_type & x) { 
      value_type dist(x - xq);
      res += (dist*dist);
    });
    res /= (double(count()) - 1.0);
  }
  return res;
}



template<typename T>
template<typename U>
inline void accumulator<T>::operator()(U val) const 
{ 
  value_type tval(val);
  if (! glmutils::any_nan(tval))
  {
    mmm_(tval); data_.push_back(tval); 
  }
}

template<typename T>
template<typename U>
inline void accumulator<T>::operator()(U val, size_t times) const 
{
  value_type tval(val);
  if (! glmutils::any_nan(tval))
  {
    for (size_t i=0; i<times; ++i) { mmm_(tval); data_.push_back(tval); } 
  }
}



namespace std 
{
  template<typename ACC>
  inline std::ostream& print_accumulator(std::ostream& os, const ACC& acc)
  {
    os << acc.result(); 
    return os;
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const average<T>& x)
  {
    return print_accumulator(os, x);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const min_max_mean<T>& x)
  {
    return print_accumulator(os, x);
  }

  template<typename T>
  inline std::ostream& operator << (std::ostream& os, const accumulator<T>& x)
  {
    return print_accumulator(os, x);
  }
}



#endif  //ACCUMULATORS_HPP_INCLUDED
