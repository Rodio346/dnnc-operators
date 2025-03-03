// Copyright 2018 The AITS DNNC Authors.All Rights Reserved.
//
// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.See the License for the
// specific language governing permissionsand limitations
// under the License.
//
// This file is part of AITS DNN compiler maintained at
// https://github.com/ai-techsystems/dnnCompiler
//

#pragma once

#include "core/datatypes.h"
#include "core/macros.h"

#ifndef SWIGPYTHON
#include <iostream>
#include <stdexcept>
#include <stdlib.h> // malloc, free
#include <string>
#include <vector>
#endif
#include <random>

namespace dnnc {
typedef size_t INDEX;
typedef size_t DIMENSION;
enum INIT_TYPE { INIT_NONE = 0, INIT_RANDOM, INIT_ZERO, INIT_ONE };

template <typename T> class baseOperator;

// Tensor with arbitrary rank.
template <typename T> class tensor {
  friend class baseOperator<T>;

protected:
  //////////// protected members /////////////////
  size_t *_ref;
  T *_mem_layout; // TODO: add tiling.
  std::string _name;
  std::vector<DIMENSION> _shape;

  //////////// protected methods /////////////////
  T *getMemory(size_t sz) {
    _mem_layout = sz ? static_cast<T *>(malloc(sizeof(T) * sz)) : 0x0;
    _ref = static_cast<size_t *>(malloc(sizeof(size_t)));
    if ((sz && !_mem_layout) || !_ref)
      throw std::bad_alloc();
    return _mem_layout;
  }
  // only constructors  call init method
  void init(INIT_TYPE fill = INIT_NONE) {
    size_t msize = length(); // flat array length
#ifndef SWIGPYTHON
    if (rank() == 0)
      throw std::invalid_argument("tensor with no shape.");
#endif
    _mem_layout = getMemory(msize);

    *_ref = 1; // init reference count.
    // initilize with normal distribution.
    if (fill == INIT_NONE) {
      ; // no fill
    } else if (fill == INIT_RANDOM) {
      std::default_random_engine generator;
      std::normal_distribution<double> distribution(127.5, 20.0);
      for (size_t i = 0; i < msize; i++)
        _mem_layout[i] = static_cast<T>(distribution(generator));
    } else if (fill == INIT_ZERO) {
      for (size_t i = 0; i < msize; i++)
        _mem_layout[i] = static_cast<T>(0);
    } else if (fill == INIT_ONE) {
      for (size_t i = 0; i < msize; i++)
        _mem_layout[i] = static_cast<T>(1);
    }
  }

public:
  // tensor constructor with arbitrary dimension
  tensor(std::vector<DIMENSION> dimn, std::string n = "",
         INIT_TYPE fill = INIT_NONE)
      : _ref(0x0), _mem_layout(0x0), _name(n) {
    _shape = dimn;
    init(fill);
  }
  tensor(DIMENSION x = 0, DIMENSION y = 0, DIMENSION z = 0, DIMENSION w = 0,
         std::string n = "", INIT_TYPE fill = INIT_NONE)
      : _ref(0x0), _mem_layout(0x0), _name(n) {
    if (x) {
      _shape.push_back(x);
      if (y)
        _shape.push_back(y);
      if (z)
        _shape.push_back(z);
      if (w)
        _shape.push_back(w);
    }
    init(fill);
  }
  tensor(const tensor &other) {
    _ref = other._ref;
    _name = other._name;
    _shape = other._shape;
    _mem_layout = other._mem_layout;
    (*_ref)++;
  }
  tensor &operator=(const tensor &other) {
    // Gracefully handle self assignment
    if (this == &other)
      return *this;

    _ref = other._ref;
    _name = other._name;
    _shape = other._shape;
    _mem_layout = other._mem_layout;
    (*_ref)++;

    return *this;
  }
  ~tensor() {
    --(*_ref);
    if (*_ref == 0 && _mem_layout) {
      free(_ref);
      free(_mem_layout);
    }
  }

  void load(std::vector<T> data) {
    size_t sz = length();
    for (size_t i = 0; i < data.size() && i < sz; i++)
      _mem_layout[i] = data[i];
  }
  void load(T *data) {
    if (!data)
      return;
    for (size_t i = 0; i < length(); i++)
      _mem_layout[i] = data[i];
  }
  friend std::ostream &operator<<(std::ostream &os, const tensor<T> &t) {
    if (t._name.size())
      os << t._name << "=";
    os << t.to_string();
    return os;
  }

  std::string to_string() const {
    std::string str;
#define DNNC_MAX_SHOW 30
    if ((rank() == 1) || ((rank() == 2) && (_shape[0] == 1))) {
      if (_name.size())
        str = _name + "=\n";
      str += "[";
      for (size_t i = 0; i < length(); i++) {
        if (i != 0)
          str += " ";
        str += std::to_string(_mem_layout[i]);
        if (i > DNNC_MAX_SHOW) {
          str += "...\n";
          break;
        }
        if (i < (length() - 1))
          str += "\n";
      }
      str += "]\n";
    } else if (rank() == 2) {
      if (_name.size())
        str = _name + "=\n";
      for (size_t i = 0; i < _shape[0]; i++) {
        str += "  [";
        for (size_t j = 0; j < _shape[1]; j++) {
          size_t index = i + _shape[0] * j;
          str += std::to_string(_mem_layout[index]) + " ";
          if (j > DNNC_MAX_SHOW) {
            str += "...";
            break;
          }
        }
        if (i > DNNC_MAX_SHOW) {
          str += "...";
          break;
        }
        str += "]\n";
      }
    } else if (rank() == 3) {
      for (size_t k = 0; k < _shape[2]; k++) {
        if (_name.size())
          str += _name + "[" + std::to_string(k) + "]=\n";
        str += "[\n";
        for (size_t i = 0; i < _shape[0]; i++) {
          str += "  [";
          for (size_t j = 0; j < _shape[1]; j++) {
            size_t index = i + _shape[0] * j + _shape[0] * _shape[1] * k;
            str += std::to_string(_mem_layout[index]) + " ";
            if (j > DNNC_MAX_SHOW) {
              str += "...";
              break;
            }
          }
          if (i > DNNC_MAX_SHOW) {
            str += "...";
            break;
          }
          str += "]\n";
        }
        str += "]\n";
        if (k > DNNC_MAX_SHOW) {
          str += "...\n";
          break;
        }
      }
    } else {
      // For now, print it like a vector for tensor with rank higher than 3
      // TODO: support rank 4 tensors as well.
      if (_name.size())
        str = _name + "=\n";
      str += "[";
      for (size_t i = 0; i < length(); i++) {
        if (i != 0)
          str += " ";
        str += std::to_string(_mem_layout[i]);
        if (i > DNNC_MAX_SHOW) {
          str += "...\n";
          break;
        }
        if (i < (length() - 1))
          str += "\n";
      }
      str += "]\n";
    }

    return str;
  }

  char *__str__() {
    std::string str = to_string();
    size_t sz = str.size();
    char *result = (char *)malloc(sz + 1);
    for (size_t i = 0; i <= str.size(); i++)
      result[i] = str.at(i);
    result[sz] = '\0';
    return result;
  }

  // public methods

  const DIMENSION length() const {
    DIMENSION sz = rank() ? 1 : 0;
    for (size_t i = 0; i < rank(); i++)
      sz = sz * _shape[i];
    return sz;
  }
  std::string name() const { return _name; }
  const DIMENSION rank() const { return _shape.size(); }
  const std::vector<DIMENSION> shape() const { return _shape; }
  void reshape(std::vector<size_t> &new_shape) {
    DIMENSION newLength = new_shape.size() ? 1 : 0;
    for (size_t i = 0; i < new_shape.size(); i++)
      newLength = newLength * new_shape[i];

    // ensure new_shape is same length as original length
    if (newLength == 0)
      throw std::invalid_argument("new reshape length can't be zero.");
    if (newLength != length()) {
      std::string msg = "new reshape length " + std::to_string(newLength) +
                        " does not match tensor\'s length" +
                        std::to_string(length()) + ".\n";
      throw std::invalid_argument(msg);
    }

    _shape = new_shape;
  }
  // reference: https://github.com/onnx/onnx/blob/master/docs/Broadcasting.md
  bool broadcast(const tensor<T> &other) {
    // TODO:
    // 1. uni-directional broadcasting
    // 2. multi-directional broadcasting
    return true;
  }
  // flat index, unsafe method
  T &operator[](const INDEX &index) const {
    if (index >= length())
      throw std::out_of_range("illegal tensor index.");

    return _mem_layout[index];
  }

  T &operator()(std::vector<INDEX> &indices) const {
    INDEX index = 0;
    // column-major:Loc(A[i][j][k])= base+w((i-x)+d0*d1(k-z)+d0*(j-y))
    if (rank() == 3) {
      index = indices[0] + _shape[0] * indices[1] +
              _shape[0] * _shape[1] * indices[2];
    } else if (rank() == 2) {
      index = indices[0] + _shape[0] * indices[1];
    } else {
      for (size_t i = 0; i < indices.size(); i++) {
        DIMENSION dsz = 1;
        for (size_t j = i + 1; j < rank(); j++)
          dsz *= _shape[j];
        index += indices[i] * dsz;
      }
    }
    return this->operator[](index);
  }

  T &operator()(const INDEX x = 0, const INDEX y = 0, const INDEX z = 0,
                const INDEX w = 0) const {
    std::vector<INDEX> indices;
    indices.push_back(x);
    if (rank() > 1)
      indices.push_back(y);
    if (rank() > 2)
      indices.push_back(z);
    if (rank() > 3)
      indices.push_back(w);

    return this->operator()(indices);
  }
  std::string dtype() { return dtype_str[typeid(T).name()[0] - 'a']; }
  bool empty() { return length() == 0; }
  std::string to_proto() // return proto string
  {
    std::string tensor_proto = "";
    return tensor_proto;
  }
};
} // namespace dnnc
