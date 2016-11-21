// sessionconnection.cpp
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

#include "sessionconnection.h"
using std::string;

#include "sshsession.h"
#include "sshexception.h"

#include <utility>
using std::move;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

SessionConnection::SessionConnection(const std::string &pHost, const std::string &pPort,
    const std::string &pUser, const std::string &pPwd, ReportFunction pReportStatus) :
    reportStatus(pReportStatus), host(pHost), port (pPort), user(pUser), pwd(pPwd),
    remoteSessionHandle(reportStatus, host, port),
    sessionAuthenticator(user, pwd, remoteSessionHandle, reportStatus)
{
}

SessionConnection::SessionConnection(ConnectionInfo ci, ReportFunction pReportStatus) :
    reportStatus(pReportStatus), host(ci.host), port (ci.port), user(ci.user), pwd(ci.pwd),
    remoteSessionHandle(reportStatus, host, port),
    sessionAuthenticator(user, pwd, remoteSessionHandle, reportStatus)
{
}

SessionConnection::SessionConnection(SessionConnection&& other) :
    reportStatus(std::move(other.reportStatus)), host(move(other.host)), port(move(other.port)),
    user(move(other.user)), pwd(move(other.pwd)),
    remoteSessionHandle(move(other.remoteSessionHandle)), sessionAuthenticator(move(other.sessionAuthenticator))
{
    // This isn't strictly necessary, as the instance is destructible without it.
    // All other members are properly "nullified" upon move - string
    // nullifies as necessary, and so do our classes
    other.reportStatus = 0;
}

SessionConnection::~SessionConnection()
{
}

} // namespace ssh
}}}
