// sessionauthenticator.cpp
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

#include "sessionauthenticator.h"
using pt::pcaetano::bluesy::utils::AsyncLoopTimer;
using std::string;

#include "sshexception.h"
#include "remotesessionhandle.h"

#include <boost/asio.hpp>
using boost::asio::io_service;
using boost::asio::deadline_timer;
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::bind;
using boost::protect;
using boost::lexical_cast;
using boost::asio::deadline_timer;
using boost::posix_time::milliseconds;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

SessionAuthenticator::SessionAuthenticator(std::string const& user, std::string const& pwd,
    RemoteSessionHandleRef sessionHandle, ReportFunctionRef reportStatus)
{
    Authenticate(user, pwd, sessionHandle, reportStatus);
}

SessionAuthenticator::SessionAuthenticator(SessionAuthenticator&& /*other*/)
{
}

SessionAuthenticator::~SessionAuthenticator()
{
}

void SessionAuthenticator::Authenticate(std::string const& user, std::string const& pwd,
    RemoteSessionHandleRef sessionHandle, ReportFunctionRef reportStatus)
{
    if (!reportStatus.empty())
    {
        reportStatus("Performing SSH Authenticate for user " + user);
    }

    io_service ios;
    deadline_timer dt(ios);

    dt.expires_from_now(milliseconds(10));
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&SessionAuthenticator::DoAuthenticate, this,
        boost::ref(user), boost::ref(pwd), boost::ref(sessionHandle))), boost::ref(dt), 10));
    ios.run();
}

utils::TaskState SessionAuthenticator::DoAuthenticate(std::string const& user, std::string const& pwd,
    RemoteSessionHandleRef sessionHandle)
{
    int rc = libssh2_userauth_password(sessionHandle.GetSession(), user.c_str(), pwd.c_str());

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHAuthenticationError() <<
            ssh_error_string("SessionAuthenticator: Error " + lexical_cast<string>(rc) +
            " during authentication of user " + user) << ssh_error_id(rc));
    }

    return utils::TASK_DONE;
}

} // namespace ssh
}}}
