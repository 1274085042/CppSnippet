#pragma once
#include <memory>
#include <iostream>

using namespace std;

struct Type:enable_shared_from_this<Type>
{
    template<typename T>
    shared_ptr<T> withContained()
    {
        return static_pointer_cast<T>(shared_from_this());
    }

};

struct TensorType : public Type 
{
  static shared_ptr<TensorType> create() 
  {
    return shared_ptr<TensorType> (new TensorType);
  }
  void show()
  {
    cout<<"TensorType::show"<<endl;
  }
private:
    TensorType() 
    {
        cout<<"!!!!!!!!!!!!TensorType!!!!!!!!!!!!!"<<endl;
    };
};