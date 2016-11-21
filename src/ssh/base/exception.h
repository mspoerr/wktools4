// Copyright (c) 2014-2016, Paulo Caetano
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of the copyright holder nor the names of any other
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H

#include <boost/exception/all.hpp>

namespace pt { namespace pcaetano { namespace bluesy {
namespace base
{
// An unique ID given to the error according to its location. Probably redundant,
// since we'll be using __FILE__ and __LINE__, but it's a low-maintenance scheme, so we'll keep it.
using error_key = boost::error_info<struct tag_error_id, unsigned long>;

// Similar to boost::errinfo_errno, but without the automatic error message mechanism.
// E.g., if we have an error code 3 and use boost::errinfo_errno, it'll include the message
// "No such process", which won't be suitable for our error, which has a different meaning.
using error_id = boost::error_info<struct tag_error_id, int>;
using error_message = boost::error_info<struct tag_error_message, std::string>;

// We'll be using nested exceptions to implement a very basic stack trace. It's not the real
// thing, but it's a start.
using nested_exception = boost::error_info<struct tag_nested_exception, boost::exception_ptr>;

// Base exception for all the components in the pt::pcaetano::bluesy namespace
struct PCBBaseException : virtual std::exception, virtual boost::exception { };

} // namespace base
}}}
#endif // BASE_EXCEPTION_H
