// sessionconnection.h
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

#ifndef SESSIONCONNECTION_H
#define SESSIONCONNECTION_H

#include "remotesessionhandle.h"
#include "sessionauthenticator.h"
#include "misc.h"

#include <boost/function.hpp>
#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif

#include <string>
#include <memory>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunction = boost::function<void(std::string)>;

/*!
 * \brief The SessionConnection class Implements an SSH session to a remote host.
 *
 * \todo Turn the ReportFunction object into a template parameter. This will allow us
 * to make this interface more flexible - i.e., it can be any callable type T that
 * accepts an std::string and has a return type void.
 */
class SessionConnection
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    /*!
     * \brief SessionConnection The constructor(s) open an SSH session in the remote host.
     *
     * This session remains open until the SessionConnection instance is destroyed; or if
     * the system (local or remote) closes it.
     */
    SessionConnection(std::string const& pHost, std::string const& pPort, std::string const& pUser,
        std::string const& pPwd, ReportFunction pReportStatus = 0);
    explicit SessionConnection(ConnectionInfo ci, ReportFunction pReportStatus = 0);
    SessionConnection(SessionConnection&& other);
    ~SessionConnection();

#ifndef _MSC_VER
    SessionConnection& operator=(SessionConnection&&) = delete;
    SessionConnection(SessionConnection const&) = delete;
    SessionConnection& operator=(SessionConnection const&) = delete;
#endif

    SessionHandleRef GetSessionHandle() { return remoteSessionHandle.GetSessionHandle(); }
private:
#ifdef _MSC_VER
    ChannelHandle& operator=(ChannelHandle&&) {}
#endif

    // !!! THE MEMBER ORDER IS ESSENTIAL !!!
    // - remoteSessionHandle REQUIRES reportStatus
    // - sessionAuthenticator REQUIRES user, pwd, remoteSessionHandle, reportStatus

    /*!
     * \brief reportStatus Client callback to report to the caller.
     */
    ReportFunction reportStatus;

    /*!
     * \todo These members will probably go to SSHSession. If the connection is dropped,
     *  SSHSession will need to destruct and rebuild SSHConnection.
     * \todo pwd must be encrypted.
     */
    std::string host;
    std::string port;
    std::string user;
    std::string pwd;

    /*!
     * \brief remoteSessionHandle Handles the creation of the SSH session, both on the remote
     *  host and on the local host.
     *
     *  Despite the name, it also handles the management of the local resources required
     *  to create/maintain the SSH session.
     */
    RemoteSessionHandle remoteSessionHandle;

    /*!
     * \brief sessionAuthenticator Performs the authentication on the remote host.
     */
    SessionAuthenticator sessionAuthenticator;
};

} // namespace ssh
}}}

#endif // SESSIONCONNECTION_H
