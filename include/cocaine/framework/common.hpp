/*
Copyright (c) 2013 Andrey Goryachev <andrey.goryachev@gmail.com>
Copyright (c) 2015+ Evgeny Safronov <division494@gmail.com>
Copyright (c) 2011-2015 Other contributors as noted in the AUTHORS file.

This file is part of Cocaine.

Cocaine is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Cocaine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COCAINE_FRAMEWORK_COMMON_HPP
#define COCAINE_FRAMEWORK_COMMON_HPP

#include <cassert>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <cocaine/common.hpp>
#include <cocaine/traits.hpp>

#if defined(__clang__) || defined(HAVE_GCC46) || __GNUC__ >= 5
#include <atomic>
#else
#include <cstdatomic>
#endif

namespace cocaine { namespace framework {

template<class Exception>
std::exception_ptr
make_exception_ptr(Exception&& e) {
    try {
        throw std::forward<Exception>(e);
    } catch (...) {
        return std::current_exception();
    }
}

template<class T>
typename std::add_rvalue_reference<T>::type
declval();

}} // namespace cocaine::framework

#endif // COCAINE_FRAMEWORK_COMMON_HPP
