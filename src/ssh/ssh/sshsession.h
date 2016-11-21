// sshsession.h
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

#ifndef SSHSESSION_H
#define SSHSESSION_H

#include "sessionconnection.h"
#include "misc.h"

#include <boost/function.hpp>
#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif

#include <string>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunction = boost::function<void(std::string)>;
using ResultFunction = boost::function<void(std::string)>;

/*!
 * \brief The SSHSession class is the client interface for an SSH session.
 *
 * An instance of this class represents an established SSH session to a remote host,
 * through which the client can execute commands.
 *
 * \todo Turn the above function objects into template parameters. This will allow us
 * to make this interface more flexible - i.e., it can be any callable type T that
 * accepts an std::string and has a return type void.
 */
class SSHSession
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    SSHSession(std::string const& pHost, std::string const& pPort, std::string const& pUser,
        std::string const& pPwd, ReportFunction pReportStatus = 0);
    explicit SSHSession(ConnectionInfo ci, ReportFunction pReportStatus = 0);
    SSHSession(SSHSession&& other);
    ~SSHSession();

#ifndef _MSC_VER
    SSHSession& operator=(SSHSession&&) = delete;
    SSHSession(SSHSession const&) = delete;
    SSHSession& operator=(SSHSession const&) = delete;
#endif

    /*!
     * \brief ExecuteCommand Executes an SSH command on a remote host.
     *
     * The client callback will be invoked when the read buffer from the socket is full. This callback
     * must be prepared to handle partial content, i.e., content usually considered atomic, such as
     * a line on an "ls -l" or a "cat file", will probably arrive at this callback broken. E.g.,
     * assuming an "ls -l" returns 10 lines - the first callback invocation may contain 3 full lines and
     * part of the 4th; the following invocation may contain the rest of the 4th line, 2 full lines and
     * part of the 7th; the following invocation may contain the rest of the 7th line and the remaining
     * 3 lines.
     *
     * \param command The command to execute.
     * \param pClientCallback The callback called when the read buffer from the socket is full.
     * \param wantExitStatus If true, it means the caller wants to receive the exit status/signal.
     *
     * \pre The SSH session with the remote host is established.
     * The callback is valid
     *
     * \post The command was executed on the remote host.
     * The callback received the output of the command execution.
     *
     *
     * \todo Redesign the exit status communication.
     * \todo Determine when will we create the logger.
     * \todo Implement move assignment(??) Must pay attention on releasing the resources of the
     * moved-to object.
     */
    void ExecuteCommand(std::string const& command, ResultFunction pClientCallback, bool wantExitStatus = false);

private:
#ifdef _MSC_VER
    ChannelHandle& operator=(ChannelHandle&&) {}
#endif

    /*!
     * \brief reportStatus Callback to notify the client of the operations being performed.
     */
    ReportFunction reportStatus;

    /*!
     * \brief sessionConnection Implementation of an SSH session on a remote host.
     */
    SessionConnection sessionConnection;
};

} // namespace ssh
}}}


#endif // SSHSESSION_H
