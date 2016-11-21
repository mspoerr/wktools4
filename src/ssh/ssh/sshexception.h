// sshexception.h
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

#ifndef SSHEXCEPTION_H
#define SSHEXCEPTION_H

#include "../utils/exception.h"

#include <boost/exception/all.hpp>
#include <string>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ssh_error_id = boost::error_info<struct tag_ssh_error_id, int>;
using ssh_error_string = boost::error_info<struct tag_ssh_error_string, std::string>;

struct SSHBaseException : virtual base::PCBBaseException { };
struct SSHConnectionError : virtual SSHBaseException { }; // ERROR ON SOCKET CONNECTION
struct SSHCreateSessionError : virtual SSHBaseException { };
struct SSHHandshakeError : virtual SSHBaseException { };
struct SSHAuthenticationError : virtual SSHBaseException { };
struct SSHDisconnectSessionError : virtual SSHBaseException { }; // ERROR ON SESSION TERMINATION
struct SSHFreeSessionError : virtual SSHBaseException { };
struct SSHDisconnectionError : virtual SSHBaseException { }; // ERROR ON SOCKET DISCONNECTION

struct SSHFreeChannelError : virtual SSHBaseException { };
struct SSHOpenChannelError : virtual SSHBaseException { };
struct SSHCloseChannelError : virtual SSHBaseException { };
struct SSHExecuteCommandError : virtual SSHBaseException { };
struct SSHReadResultError : virtual SSHBaseException { };

} // namespace ssh
}}}
#endif // SSHEXCEPTION_H
