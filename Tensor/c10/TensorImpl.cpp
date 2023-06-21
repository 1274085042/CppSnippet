#include "TensorImpl.h"

TensorImpl::TensorImpl() = default;
TensorImpl::~TensorImpl()
  { }
int64_t TensorImpl::sizes() const
{
    return sizes_ + sizes_help();
}

int64_t TensorImpl::strides() const
{
    return strides_;
}

int64_t  TensorImpl::storage_offset() const
{
    return storage_offset_;
}

int64_t TensorImpl::sizes_help() const
{
    return sizes_help_;
}