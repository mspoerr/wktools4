// remotesessionhandle.cpp
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

#include "remotesessionhandle.h"
using pt::pcaetano::bluesy::utils::AsyncLoopTimer;
using pt::pcaetano::bluesy::utils::AsyncLoopSocket;
using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::socket_base;

#include "sshexception.h"
#include "sessionhandle.h"

#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::bind;
using boost::protect;
using boost::lexical_cast;
using boost::asio::deadline_timer;
using boost::posix_time::milliseconds;

#include <string>
using std::string;

#include <utility>
using std::move;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

RemoteSessionHandle::RemoteSessionHandle(ReportFunctionRef pReportStatus, std::string const& host,
    std::string const& port) :
    reportStatus{pReportStatus}, ios{new io_service()}, sock{ConnectSocket(host, port)},
    sessionHandle{pReportStatus}, cleanupState{utils::CleanupState::NoCleanup}
{
    if (!reportStatus.empty())
    {
        reportStatus("Performing SSH handshake");
    }

    io_service& ios = sock->get_io_service();
    ios.reset();

    sock->async_read_some
    (
        boost::asio::null_buffers{},
        bind(AsyncLoopSocket, protect(bind(&RemoteSessionHandle::DoHandshake, this)),
            boost::ref(*sock), true)
    );

    ios.run();
}

RemoteSessionHandle::RemoteSessionHandle(RemoteSessionHandle&& other) :
    reportStatus{std::move(other.reportStatus)}, ios{move(other.ios)}, sock{move(other.sock)},
    sessionHandle{move(other.sessionHandle)}, cleanupState{other.cleanupState}
{
    other.cleanupState = utils::CleanupState::CleanupDone;
}

/*!
 * \todo Use current_exception to log exceptions.
 */
RemoteSessionHandle::~RemoteSessionHandle()
{
    try
    {
        CloseSession();
    }
    catch (...)
    { }
}

void RemoteSessionHandle::CloseSession()
{
    if (cleanupState != utils::CleanupState::NoCleanup)
    {
        return;
    }

    if (!reportStatus.empty())
    {
        reportStatus("Disconnecting SSH remote session.");
    }

    io_service ios{};
    deadline_timer dt{ios};

    dt.expires_from_now(milliseconds{10});
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&RemoteSessionHandle::DoCloseSession, this)),
        boost::ref(dt), 10));

    cleanupState = utils::CleanupState::CleanupInProgress;
    ios.run();

    cleanupState = utils::CleanupState::CleanupDone;
}

utils::TaskState RemoteSessionHandle::DoHandshake()
{
    int rc = libssh2_session_handshake(sessionHandle.GetSession(), sock->native_handle());

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHHandshakeError{} <<
            ssh_error_string{"RemoteSessionHandle: Error " + lexical_cast<string>(rc) +
            " during SSH handshake"} << ssh_error_id{rc});
    }

    return utils::TASK_DONE;
}

utils::TaskState RemoteSessionHandle::DoCloseSession()
{
    int rc = libssh2_session_disconnect(sessionHandle.GetSession(), "Disconnect SSH session");

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHDisconnectSessionError{} <<
            ssh_error_string{"RemoteSessionHandle: Error " + lexical_cast<string>(rc) +
            " disconnecting SSH session."} << ssh_error_id{rc});
    }

    return utils::TASK_DONE;
}

Socket* RemoteSessionHandle::ConnectSocket(std::string const& host, std::string const& port)
{
    if (!reportStatus.empty())
    {
        reportStatus("Connecting to host " + host + " on port " + port);
    }

    try
    {
    tcp::resolver rsv(*ios);
    tcp::resolver::query query(host, port);

    tcp::resolver::iterator iter = rsv.resolve(query);

    tcp::socket* s = new tcp::socket(*ios);

    // TODO: Change this to connect async?
    boost::asio::connect(*s, iter);

    socket_base::keep_alive ka_option(true);
    s->set_option(ka_option);

    return s;
    }
    catch (boost::system::system_error& ex)
    {
        BOOST_THROW_EXCEPTION(SSHConnectionError() <<
            ssh_error_string("SessionConnection: Error connecting to host " + host + " on port " + port + ". " +
            ex.what()) << ssh_error_id(ex.code().value()));
    }
}

} // namespace ssh
}}}
