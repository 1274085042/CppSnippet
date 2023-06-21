#pragma once

#include<cstdint>

struct TensorImpl
{
public:
  TensorImpl();
  virtual ~TensorImpl();
  virtual int64_t sizes() const;
  virtual int64_t sizes_help() const;
  virtual int64_t strides() const;
  virtual int64_t storage_offset() const;

protected:
    int64_t sizes_ = 5;
    int64_t sizes_help_ = 5;
    int64_t strides_ = 1;
    int64_t storage_offset_ = 0;
};
