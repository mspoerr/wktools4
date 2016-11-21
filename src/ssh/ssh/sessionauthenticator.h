// sessionauthenticator.h
//
// Copyright (c) 2012-2016, Paulo Caetano
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the
//      names of any other contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SESSIONAUTHENTICATOR_H
#define SESSIONAUTHENTICATOR_H

#include "misc.h"
#include "../utils/asyncloop.h"

#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif
#include <boost/function.hpp>

#include <string>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunctionRef = boost::function<void(std::string)>&;

/*!
 * \brief The SessionAuthenticator class Performs authentication on the remote host.
 * \todo Support the different authentication mechanisms implemented in libssh2.
 * \internal Should we authenticate in the ctor, creating a faux-RAII (there's no "release" to authenticate)?
 *      For now, that's what we'll do.
 *  \endinternal
 */
class SessionAuthenticator
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    SessionAuthenticator(std::string const& user, std::string const& pwd, RemoteSessionHandleRef sessionHandle,
        ReportFunctionRef ReportStatus);
    SessionAuthenticator(SessionAuthenticator&&);
    ~SessionAuthenticator();

#ifndef _MSC_VER
    SessionAuthenticator& operator=(SessionAuthenticator&&) = delete;
    SessionAuthenticator(SessionAuthenticator const&) = delete;
    SessionAuthenticator& operator=(SessionAuthenticator const&) = delete;
#endif
private:
#ifdef _MSC_VER
    SessionAuthenticator& operator=(SessionAuthenticator&&) {}
#endif

    void Authenticate(std::string const& user, std::string const& pwd, RemoteSessionHandleRef sessionHandle,
        ReportFunctionRef reportStatus);
    utils::TaskState DoAuthenticate(std::string const& user, std::string const& pwd,
        RemoteSessionHandleRef sessionHandle);
};

} // namespace ssh
}}}

#endif // SESSIONAUTHENTICATOR_H
