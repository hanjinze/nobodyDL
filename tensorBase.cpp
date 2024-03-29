#ifndef TENSOR_BASE_
#define TENSOR_BASE_

#include "../include/tensor.h"

using std::ios;

#ifndef __CUDACC__
Shape::Shape () :
  rows(0), cols(0), chls(0), nums(0)
{ set_dims ();
}
Shape::Shape (const int a, const int b, const int c, const int d) :
  rows(a), cols(b), chls(c), nums(d)
{ set_dims ();
}
Shape::Shape (const int a, const int b, const int c, const int d, const int e) :
  rows(a), cols(b), chls(c), nums(d)
{ set_dims ();
  size = e;
}
bool Shape::operator == (const Shape &s) const
{ return (rows == s.rows && cols == s.cols && chls == s.chls && nums == s.nums);
}
bool Shape::operator != (const Shape &s) const
{ return !(*this == s);
}
void Shape::set_dims ()
{ size = rows * cols * chls * nums;
  if      (nums > 1)  dims = 4;
  else if (chls > 1)  dims = 3;
  else if (cols > 1)  dims = 2;
  else                dims = 1;
}
void Shape::set_nums (const int n)
{ CHECK_EQ ((chls * nums) % n, 0);
  chls = chls * nums / n;
  nums = n;
  set_dims ();
}
void Shape::set_chls (const int c)
{ CHECK_EQ ((rows * chls) % c, 0);
  rows = rows * chls / c;
  chls = c;
  set_dims ();
}
void Shape::set_cols (const int c)
{ CHECK_EQ ((rows * cols) % c, 0);
  rows = rows * cols / c;
  cols = c;
  set_dims ();
}
int Shape::get_dimsX (const int d) const
{ switch (d) {
  case 2: return cols;  break;
  case 1: return rows;  break;
  case 3: return chls;  break;
  case 4: return nums;  break;
  default: return 1;
  }
}
int Shape::get_sizeX (const int d) const
{ switch (d) {
  case 2: return 1;  break;
  case 1: return cols;  break;
  case 3: return cols*rows;  break;
  case 4: return cols*rows*chls;  break;
  default: return size;
  }
}
int Shape::get_strdX (const int d) const
{ switch (d) {
  case 2: return cols;  break;
  case 1: return cols*rows;  break;
  case 3: return cols*rows*chls;  break;
  case 4: return size;  break;
  default: return size;
  }
}
void Shape::re_shape (const int a, const int b, const int c, const int d)
{ CHECK_EQ (a * b * c * d, size);
  rows = a;
  cols = b;
  chls = c;
  nums = d;
  set_dims ();
}
void Shape::print ()
{ char shapestr[64];  sprintf (shapestr, "\tshape\t%d\t%d\t%d\t%d\n", rows, cols, chls, nums);
  LOG (INFO) << shapestr;
}
#endif



#ifndef __CUDACC__
Shape Patch::get_pack_size (const Shape &in)
{ h_col  = (in.rows + 2 * pad - ksize) / stride + 1;
  w_col  = (in.cols + 2 * pad - ksize) / stride + 1;
  return Shape (ksize * ksize * in.chls, h_col * w_col, 1, in.nums);
}

Shape  Pool::get_pool_size (const Shape &in)
{ h_pool = (in.rows + 2 * pad - ksize) / stride + 1;  // (rows - ksize + stride - 1) / stride + 1
  w_pool = (in.cols + 2 * pad - ksize) / stride + 1;  // (cols - ksize + stride - 1) / stride + 1
  return Shape (h_pool, w_pool, in.chls, in.nums);
}
#endif



template <typename XPU, typename DT>
Tensor<XPU, DT>:: Tensor() : shape(), dptr(NULL), did_(0), cherry(false)
{ }
template <typename XPU, typename DT>
Tensor<XPU, DT>::~Tensor()
{ if (cherry)
    mem_free ();
}
#ifdef __CUDACC__
template TensorGPUf:: Tensor();
template TensorGPUd:: Tensor();
template TensorGPUf::~Tensor();
template TensorGPUd::~Tensor();
#else
template TensorCPUf:: Tensor();
template TensorCPUd:: Tensor();
template TensorCPUf::~Tensor();
template TensorCPUd::~Tensor();
#endif

template <typename XPU, typename DT>
void Tensor<XPU, DT>::create (const Shape &s, const int did)
{ shape = s;
  did_  = did;
  mem_free ();
  mem_alloc();
  cherry = true;
}
#ifdef __CUDACC__
template void TensorGPUf::create (const Shape &s, const int did);
template void TensorGPUd::create (const Shape &s, const int did);
#else
template void TensorCPUf::create (const Shape &s, const int did);
template void TensorCPUd::create (const Shape &s, const int did);
#endif



template <typename XPU, typename DT>
Tensor<XPU, DT> Tensor<XPU, DT>::section (const int begin, const int end) const
{ Tensor<XPU, DT> t;  // TODO
    int slices = end - begin;    CHECK (slices >= 1);
    if      (shape.dims == 4)  { CHECK (slices <= nums());  t.shape = Shape (rows(), cols(), chls(), slices);  }
    else if (shape.dims == 3)  { CHECK (slices <= chls());  t.shape = Shape (rows(), cols(), slices, nums());  }
    else                       { CHECK (slices <= rows());  t.shape = Shape (slices, cols(), chls(), nums());  }
//  else                       { CHECK (shape.dims == 2 && begin == 0 && end == 1);  t.shape = shape;  }  // TODO
  t.dptr = dptr + (t.size() / slices) * begin;
  return t;  // TODO
}
#ifndef __CUDACC__
template TensorGPUf TensorGPUf::section (const int begin, const int end) const;
template TensorGPUd TensorGPUd::section (const int begin, const int end) const;
template TensorCPUf TensorCPUf::section (const int begin, const int end) const;
template TensorCPUd TensorCPUd::section (const int begin, const int end) const;
#endif

template <typename XPU, typename DT>
Tensor<XPU, DT>& Tensor<XPU, DT>::operator= (const Tensor<XPU, DT> &t)
{ if (&t != this)
  { shape = t.shape;
    dptr  = t.dptr;
    did_  = t.did_;
    cherry = false;
  }
  return *this;
}
#ifndef __CUDACC__
template TensorGPUf& TensorGPUf::operator= (const TensorGPUf &t);
template TensorGPUd& TensorGPUd::operator= (const TensorGPUd &t);
template TensorCPUf& TensorCPUf::operator= (const TensorCPUf &t);
template TensorCPUd& TensorCPUd::operator= (const TensorCPUd &t);
#endif



template <typename XPU, typename DT>
void Tensor<XPU, DT>::mem_alloc ()
{ 
#ifdef __CUDACC__
  LOG_IF (INFO, size_d() > 1e6) << "\tGPU  " << did_ << "  memory required for Tensor\t" << size_d() / 1e6 << " MB";
  cuda_malloc ((void**)&dptr, size_d());
#else
  LOG_IF (INFO, size_d() > 1e9) << "\tCPU memory required for Tensor\t" << size_d() / 1e6 << " MB";
  dptr = (DT*) malloc (size_d());
#endif
}

template <typename XPU, typename DT>
void Tensor<XPU, DT>::mem_free ()
{ 
#ifdef __CUDACC__
  if (dptr)  cuda_check (cudaFree ((void*)dptr));
#else
  if (dptr)  free (dptr);
#endif
  dptr = NULL;
}



#ifdef __CUDACC__
template <typename XPU, typename DT>
void Tensor<XPU, DT>::mem_set (const unsigned char a)
{ cuda_check (cudaMemset ((void*)dptr, a, size_d()));
}
template void TensorGPUf::mem_set (const unsigned char a);
template void TensorGPUd::mem_set (const unsigned char a);
#else
template <typename XPU, typename DT>
void Tensor<XPU, DT>::mem_set (const unsigned char a)
{ memset ((void*)dptr, a, size_d());
}
template void TensorCPUf::mem_set (const unsigned char a);
template void TensorCPUd::mem_set (const unsigned char a);
#endif



#define CPU2GPU cudaMemcpyHostToDevice
#define GPU2CPU cudaMemcpyDeviceToHost
#define CPU2CPU cudaMemcpyHostToHost
#define GPU2GPU cudaMemcpyDeviceToDevice

#ifdef __CUDACC__
template <>
void TensorGPUf::memcpy_from_gpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), GPU2GPU);
}
template <>
void TensorGPUd::memcpy_from_gpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), GPU2GPU);
}
template <>
void TensorGPUf::memcpy_from_cpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), CPU2GPU);
}
template <>
void TensorGPUd::memcpy_from_cpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), CPU2GPU);
}
template <>
void TensorCPUf::memcpy_from_gpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), GPU2CPU);
}
template <>
void TensorCPUd::memcpy_from_gpu (void *ptr)
{ cuda_memcpy ((void*)dptr, ptr, size_d(), GPU2CPU);
}
#else
template <>
void TensorCPUf::memcpy_from_cpu (void *ptr)
{      memcpy ((void*)dptr, ptr, size_d());
}
template <>
void TensorCPUd::memcpy_from_cpu (void *ptr)
{      memcpy ((void*)dptr, ptr, size_d());
}
#endif

#ifdef __CUDACC__
template <>
void TensorGPUf::memcpy_to_gpu (void *ptr) const
{ cuda_memcpy (ptr, (void*)dptr, size_d(), GPU2GPU);
}
template <>
void TensorGPUf::memcpy_to_cpu (void *ptr) const
{ cuda_memcpy (ptr, (void*)dptr, size_d(), GPU2CPU);
}
template <>
void TensorCPUf::memcpy_to_gpu (void *ptr) const
{ cuda_memcpy (ptr, (void*)dptr, size_d(), CPU2GPU);
}
#else
template <>
void TensorCPUf::memcpy_to_cpu (void *ptr) const
{      memcpy (ptr, (void*)dptr, size_d());
}
#endif



template <typename XPU, typename DT>
void Tensor<XPU, DT>::copy (const Tensor<GPU, DT> &in)
{ CHECK_EQ (size(), in.size());
  memcpy_from_gpu (in.dptr);
}
template <typename XPU, typename DT>
void Tensor<XPU, DT>::copy (const Tensor<CPU, DT> &in)
{ CHECK_EQ (size(), in.size());
  memcpy_from_cpu (in.dptr);
}
#ifdef __CUDACC__
template void TensorGPUf::copy (const TensorGPUf &in);
template void TensorGPUd::copy (const TensorGPUd &in);
template void TensorCPUf::copy (const TensorGPUf &in);
template void TensorCPUd::copy (const TensorGPUd &in);
template void TensorGPUf::copy (const TensorCPUf &in);
template void TensorGPUd::copy (const TensorCPUd &in);
#else
template void TensorCPUf::copy (const TensorCPUf &in);
template void TensorCPUd::copy (const TensorCPUd &in);
#endif



#endif
