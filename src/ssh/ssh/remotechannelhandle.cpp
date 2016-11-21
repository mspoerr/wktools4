// remotechannelhandle.cpp
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

#include "remotechannelhandle.h"
using pt::pcaetano::bluesy::utils::AsyncLoopTimer;

#include "sshexception.h"
#include "sessionhandle.h"
#include "channelhandle.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::asio::io_service;
using boost::bind;
using boost::protect;
using boost::lexical_cast;
using boost::asio::deadline_timer;
using boost::posix_time::milliseconds;

#include <string>
using std::string;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

RemoteChannelHandle::RemoteChannelHandle(ChannelHandleRef pChannelHandle, SessionHandleRef pSessionHandle,
    ReportFunctionRef pReportStatus)
    :
    channel{nullptr}, channelHandle{&pChannelHandle}, sessionHandle{&pSessionHandle},
    reportStatus{&pReportStatus}, cleanupState{utils::CleanupState::NoCleanup}
{
    if ((reportStatus != nullptr) && (!reportStatus->empty()))
    {
        (*reportStatus)("Opening SSH Channel");
    }

    io_service ios{};
    deadline_timer dt{ios};

    dt.expires_from_now(milliseconds{10});
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&RemoteChannelHandle::DoOpenChannel, this)),
        boost::ref(dt), 10));

    ios.run();

    // Now we have a LIBSSH2_CHANNEL*, we must supply it to our channel handle,
    // since it will handle the last step on cleanup.
    channelHandle->SetChannel(channel);
}

/*!
 * \todo Use current_exception to log exceptions.
 */
RemoteChannelHandle::~RemoteChannelHandle()
{
    try
    {
        CloseChannel();
    }
    catch (...)
    { }
}

void RemoteChannelHandle::CloseChannel()
{
    if ((channel == nullptr) || (cleanupState != utils::CleanupState::NoCleanup))
    {
        return;
    }

    if ((reportStatus != nullptr) && (!reportStatus->empty()))
    {
        (*reportStatus)("Closing SSH channel");
    }

    io_service ios{};
    deadline_timer dt{ios};

    dt.expires_from_now(milliseconds{10});
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&RemoteChannelHandle::DoCloseChannel, this)),
        boost::ref(dt), 10));

    cleanupState = utils::CleanupState::CleanupInProgress;
    ios.run();

    channel = nullptr;
    cleanupState = utils::CleanupState::CleanupDone;
}

utils::TaskState RemoteChannelHandle::DoOpenChannel()
{
    LIBSSH2_SESSION *session = sessionHandle->GetSession();
    channel = libssh2_channel_open_session(session);

    if (channel == nullptr)
    {
        int rc = libssh2_session_last_error(session, nullptr, nullptr, 0);

        if (rc == LIBSSH2_ERROR_EAGAIN)
        {
            return utils::TASK_WORKING;
        }

        if (rc)
        {
            BOOST_THROW_EXCEPTION(SSHOpenChannelError{} <<
                ssh_error_string{"RemoteChannelHandle: Error " + lexical_cast<string>(rc) +
                " opening SSH Channel"} << ssh_error_id{rc});
        }
    }

    return utils::TASK_DONE;
}

utils::TaskState RemoteChannelHandle::DoCloseChannel()
{
    int rc = libssh2_channel_close(channel);

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHCloseChannelError{} <<
            ssh_error_string{"RemoteChannelHandle: Error " + lexical_cast<string>(rc) +
            " closing SSH channel"} << ssh_error_id{rc});
    }

    return utils::TASK_DONE;
}

} // namespace ssh
}}}
