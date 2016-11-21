// sshcommand.cpp
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

#include "sshcommand.h"
using pt::pcaetano::bluesy::utils::AsyncLoopTimer;
using std::string;

#include "remotechannelhandle.h"
#include "sshexception.h"

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

#include <utility>
using std::move;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

SSHCommand::SSHCommand(boost::function<void(std::string)> pClientCallback, ReportFunctionRef pReportStatus,
    SessionHandleRef pSessionHandle) :
    clientCallback(pClientCallback), reportStatus(&pReportStatus), sessionHandle(&pSessionHandle),
    channelHandle(pReportStatus), command("NO_CMD")
{
}

SSHCommand::~SSHCommand()
{
}

void SSHCommand::Execute(const std::string &pCommand, bool wantStatus)
{
    command = pCommand;

    {
        // Opens the channel, and places it in channelHandle, which becomes responsible
        // for freeing it. rch's dtor is responsible for closing the channel. the reason
        // for this scope block is that we can't get exit status/signal without closing the
        // channel first.
        RemoteChannelHandle rch(channelHandle, *sessionHandle, *reportStatus);

        Execute();

        ReadResult();
    }

    // Now the channel is closed (rch went out of scope), we can take care of exit status/signal
    CheckExitStatus(wantStatus);
    CheckExitSignal(wantStatus);
}

void SSHCommand::Execute()
{
    if (!reportStatus->empty())
        (*reportStatus)("Executing SSH command");

    io_service ios;
    deadline_timer dt(ios);

    dt.expires_from_now(milliseconds(10));
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&SSHCommand::DoExecute, this)), boost::ref(dt), 10));

    ios.run();
}

void SSHCommand::ReadResult()
{
    if (!reportStatus->empty())
        (*reportStatus)("Reading SSH command result");

    io_service ios;
    deadline_timer dt(ios);

    dt.expires_from_now(milliseconds(10));
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&SSHCommand::DoReadResult, this)), boost::ref(dt), 10));

    ios.run();
}

/*!
 * \todo There must be some sort of protocol to report the exit status,
 *       so the client can easily tell apart this and the regular result data
 */

void SSHCommand::CheckExitStatus(bool reportStatus)
{
    LIBSSH2_CHANNEL* channel = channelHandle.GetChannel();
    int exitStatus = libssh2_channel_get_exit_status(channel);

    auto es = lexical_cast<string>(exitStatus);
    if (reportStatus)
    {
        clientCallback("Exit Status: " + es);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
/*!
 * \todo There must be some sort of protocol to report the exit signal,
 *       so the client can easily tell apart this and the regular result data
 * \todo Even though libssh2 examples don't free the char* used for exit signal,
 *       the doc says "Note that the string is stored in a newly allocated buffer".
 *       Must test this, and see if I need to free anything.
 */
void SSHCommand::CheckExitSignal(bool reportStatus)
{
    LIBSSH2_CHANNEL* channel = channelHandle.GetChannel();
    char *exitSignal = static_cast<char*>("A buffer for getting the exit signal, if any");

    libssh2_channel_get_exit_signal(channel, &exitSignal, nullptr, nullptr, nullptr, nullptr, nullptr);

    // We're assuming the default case is command execution OK, no exit signal
    auto exs = string{"No exit signal"};

    // If that's not the case, i.e., if we have an exit signal
    if (exitSignal != nullptr)
    {
        exs = string{"Exit signal: "} + exitSignal;

        // We've had at least one case where the server returned no exit signal description.
        // since we expect this to be the least common case, it's our last test.
        if (exs.length() == 13)
        {
            exs = "No exit signal description";
        }

        // Even though we're allowing the client to ignore an exit signal, we should log it
    }

    if (reportStatus)
    {
        clientCallback(exs);
    }
}
#pragma GCC diagnostic pop

utils::TaskState SSHCommand::DoExecute()
{
    LIBSSH2_CHANNEL* channel = channelHandle.GetChannel();

    int rc = libssh2_channel_exec(channel, command.c_str());

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHExecuteCommandError() <<
            ssh_error_string("SSHCommand: Error " + lexical_cast<string>(rc) +
            " executing SSH command: " + command) << ssh_error_id(rc));
    }

    return utils::TASK_DONE;
}

utils::TaskState SSHCommand::DoReadResult()
{
    char buffer[0x4001];
    LIBSSH2_CHANNEL* channel = channelHandle.GetChannel();

    int rc = libssh2_channel_read(channel, buffer, sizeof(buffer)-1);

    if (rc > 0)
    {
        // Buffer is not an sz-string, so we turn it into one
        buffer[rc] = '\0';
        string str = buffer;
        clientCallback(str);

        // We're not finished until we read 0
        return utils::TASK_WORKING;
    }

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHReadResultError() <<
            ssh_error_string("SSHCommand: Error " + lexical_cast<string>(rc) +
            " reading result of SSH command: " + command) << ssh_error_id(rc));
    }

    return utils::TASK_DONE;
}

} // namespace ssh
}}}
