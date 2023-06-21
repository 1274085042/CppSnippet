#pragma once

#include "c10/TensorImpl.h"
#include <memory>
#include <iostream>

using namespace std;

class Tensor
{
public:
  Tensor():impl_(new TensorImpl())
  {}
  void sizes() const 
  {
    cout<<"sizes: "<<impl_->sizes()<<endl;
  }

  int64_t strides() const 
  {
    return impl_->strides();
  }
  
protected:
    shared_ptr<TensorImpl> impl_;
};

