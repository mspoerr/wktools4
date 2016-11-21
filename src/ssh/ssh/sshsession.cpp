// sshsession.cpp
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

#include "sshsession.h"

#include "sshexception.h"
#include "sessionconnection.h"
#include "sshcommand.h"
#include "../utils/asyncloop.h"

#include <utility>
using std::move;
#include <string>
using std::string;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

SSHSession::SSHSession(std::string const& pHost, std::string const& pPort, std::string const& pUser,
    std::string const& pPwd, ReportFunction pReportStatus)
    :
    reportStatus(pReportStatus), sessionConnection(ConnectionInfo(pHost, pPort, pUser, pPwd), reportStatus)
{
}

SSHSession::SSHSession(ConnectionInfo ci, ReportFunction pReportStatus) :
    reportStatus(pReportStatus), sessionConnection(ci, reportStatus)
{
}

SSHSession::SSHSession(SSHSession&& other) :
    reportStatus(std::move(other.reportStatus)), sessionConnection(move(other.sessionConnection))
{
    reportStatus = 0;
}

SSHSession::~SSHSession()
{
}

void SSHSession::ExecuteCommand(std::string const& command, ResultFunction pClientCallback, bool wantExitStatus)
{
    /*!
     * \todo We must pass the session handle to SSHCommand, so it can access LIBSSH2_SESSION*
     *  to create the SSH channel. I'm considering whether I'll leave it like this (which means
     *  anyone can access the handle), or if I'll create a friend relation between these classes.
     */
    SSHCommand exec(pClientCallback, reportStatus, sessionConnection.GetSessionHandle());
    exec.Execute(command, wantExitStatus);
}

} // namespace ssh
}}}
