#ifndef MAPED_BUFFER_HPP_INCLUDED
#define MAPED_BUFFER_HPP_INCLUDED

#include <glsl/buffer.hpp>


class mapped_buffer
{
public:
  mapped_buffer(): pBuffer_(nullptr), size_(0), capacity_(0) {}

  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }
  void* get() const { return pBuffer_; }

  bool is_flushed() const { return (pBuffer_ == nullptr) && size_; }
  bool is_mapped() const { return (pBuffer_ != nullptr) && size_; }

  void bind(GLenum target) { buffer_.bind(target); }
  void unbind() { buffer_.unbind(); }
  
  void map(size_t size)
  {
    unmap();
    size_ = 0;
    if ((size > capacity_) || (size < (capacity_ >> 1)))
    {
      buffer_.data((GLsizeiptr)size, 0, GL_STREAM_DRAW);
      capacity_ = size;
    }
    if (size)
    {
      pBuffer_ = buffer_.map_range_write(0, (GLsizeiptr)size, GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    }
    if (pBuffer_)
    {
      size_ = size;
    }
  }

  void flush_range(size_t offset, size_t length)
  {
     if (is_mapped()) 
     {
       buffer_.flush_mapped_range((GLintptr)offset, (GLsizeiptr)length);
     }
     else
     {
       size_ = 0;
     }
  }

  void flush()
  {
    flush_range(0, size_);
  }

  void unmap()
  {
    if (is_mapped())
    {
      buffer_.unmap();
    }
    pBuffer_ = nullptr;
  }

private:
  glsl::buffer buffer_;
  void* pBuffer_;
  size_t size_;
  size_t capacity_;
};

#endif
