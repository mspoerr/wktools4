// sshcommand.h
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

#ifndef SSHCOMMAND_H
#define SSHCOMMAND_H

#include "misc.h"
#include "channelhandle.h"

#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif
#include <boost/function.hpp>

#include <string>

class ChannelHandle;


namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunction = boost::function<void(std::string)>;
using ReportFunctionRef = ReportFunction&;
using ReportFunctionPtr = ReportFunction*;

/*!
 * \brief The SSHCommand class Executes a command on a remote host on an SSH session.
 *
 * \todo Add an execute method that takes as parameter a vector of strings.
 */
class SSHCommand
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    SSHCommand(boost::function<void(std::string)> pClientCallback, ReportFunctionRef pReportStatus,
        SessionHandleRef pSessionHandle);
    ~SSHCommand();

    /*!
     * \brief Execute Executes a command on a remote host.
     * \param pCommand Command to execute. May be any command string valid in the remote host.
     * \param wantStatus If true,we will return the exit status/signal to the caller, via clientCallback.
     */
    void Execute(std::string const& pCommand, bool wantStatus = false);

#ifndef _MSC_VER
    SSHCommand(SSHCommand&& other) = delete;
    SSHCommand& operator=(SSHCommand&&) = delete;
    SSHCommand(SSHCommand const&) = delete;
    SSHCommand& operator=(SSHCommand const&) = delete;
#endif

private:
#ifdef _MSC_VER
    //SSHCommand(SSHCommand&& other) {}
    SSHCommand& operator=(SSHCommand&&) {}
#endif

    void Execute();
    void ReadResult();
    void CheckExitStatus(bool reportStatus);
    void CheckExitSignal(bool reportStatus);

    utils::TaskState DoExecute();
    utils::TaskState DoReadResult();

    /*!
     * \brief clientCallback We use this callback to send the command results to the caller.
     */
    boost::function<void(std::string)> clientCallback;

    /*!
     * \brief reportStatus Client callback to report to the caller. We do not send command results
     *  via this callback.
     */
    ReportFunctionPtr reportStatus;

    SessionHandlePtr sessionHandle;
    ChannelHandle channelHandle;
    std::string command;
};

} // namespace ssh
}}}

#endif // SSHCOMMAND_H
