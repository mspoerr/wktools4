// misc.h
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

#ifndef SSH_MISC_H
#define SSH_MISC_H

#include <string>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

/*!
 * \todo Divide this into different headers?
 */

class ChannelHandle;
class SessionHandle;
class RemoteSessionHandle;
class SSHSession;

using ChannelHandleRef = ChannelHandle&;
using ChannelHandlePtr = ChannelHandle*;

using SessionHandleRef = SessionHandle&;
using SessionHandlePtr = SessionHandle*;

using RemoteSessionHandleRef = RemoteSessionHandle&;
using RemoteSessionHandlePtr = RemoteSessionHandle*;


/*!
 * \brief ConnectionInfo struct To cut on the number of parameters on SSH calls.
 */
struct ConnectionInfo
{
    ConnectionInfo() : host{"NO_HOST"}, port{"NO_PORT"}, user{"NO_USER"}, pwd{"NO_PWD"} {}
    ConnectionInfo(std::string pHost, std::string pPort, std::string pUser, std::string pPwd) :
        host{pHost}, port{pPort}, user{pUser}, pwd{pPwd} {}

    std::string host;
    std::string port;
    std::string user;
    std::string pwd;
};

} // namespace ssh
}}}


#endif // SSH_MISC_H
