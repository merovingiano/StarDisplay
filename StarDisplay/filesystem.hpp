#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED

#ifdef USE_BOOST_FILESYSTEM

#include <boost/filesystem.hpp>
namespace filesystem = boost::filesystem;

#else

#include <filesystem>
namespace filesystem = std::tr2::sys;

// Adding missing operator/ in tr2 proposal

inline filesystem::path operator/(const filesystem::path& lhs, const std::string& rhs)
{
  return filesystem::path(lhs) /= rhs;
}

inline filesystem::path operator/(const filesystem::path& lhs, const char* rhs)
{
  return filesystem::path(lhs) /= rhs;
}

inline filesystem::path operator/(filesystem::path&& lhs, const std::string& rhs)
{
  return lhs /= rhs;
}

inline filesystem::path operator/(filesystem::path&& lhs, const char* rhs)
{
  return lhs /= rhs;
}

#endif


#endif

