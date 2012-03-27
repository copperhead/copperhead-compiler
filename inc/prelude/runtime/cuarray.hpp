/*
 *   Copyright 2012      NVIDIA Corporation
 * 
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 * 
 *       http://www.apache.org/licenses/LICENSE-2.0
 * 
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 * 
 */

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <prelude/runtime/chunk.hpp>
#include <boost/scoped_ptr.hpp>

namespace copperhead {

typedef std::map<system_variant,
                 std::pair<std::vector<boost::shared_ptr<chunk> >,
                           bool> ,
                 system_variant_comparator> data_map;

//Forward declaration of PIMPL for hiding std::shared_ptr from NVCC
class cu_and_c_types;

struct cuarray {
    data_map m_d;
    std::vector<size_t> m_l;
    boost::scoped_ptr<cu_and_c_types> m_t;
    size_t m_o;
};

}