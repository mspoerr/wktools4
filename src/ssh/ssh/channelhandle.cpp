// channelhandle.cpp
//
// Copyright (c) 2012-2016, Paulo Caetano
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of the copyright holder nor the names of any other
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "sshexception.h"
#include "channelhandle.h"
using pt::pcaetano::bluesy::utils::AsyncLoopTimer;

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
using boost::asio::io_service;
using boost::asio::deadline_timer;
using boost::bind;
using boost::posix_time::milliseconds;
using boost::protect;
using boost::lexical_cast;

#include <string>
using std::string;

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

ChannelHandle::ChannelHandle(ReportFunctionRef pReportStatus) :
    channel{nullptr}, reportStatus{&pReportStatus}, cleanupState{utils::CleanupState::NoCleanup}
{
}

/*!
 * \todo Use current_exception to log exceptions.
 */
ChannelHandle::~ChannelHandle()
{
    try
    {
        FreeChannel();
    }
    catch (...)
    { }
}

/*!
 * \brief ChannelHandle::FreeChannel This method sets up the boost asio async calls to DoFreeChannel(),
 *  where the actual work happens.
 */
void ChannelHandle::FreeChannel()
{
    if ((channel == nullptr) || (cleanupState != utils::CleanupState::NoCleanup))
    {
        return;
    }

    if ((reportStatus != nullptr) && (!reportStatus->empty()))
    {
        (*reportStatus)("Cleaning up SSH channel local resources");
    }

    io_service ios{};
    deadline_timer dt{ios};

    dt.expires_from_now(milliseconds{10});
    dt.async_wait(bind(AsyncLoopTimer, protect(bind(&ChannelHandle::DoFreeChannel, this)), boost::ref(dt), 10));

    cleanupState = utils::CleanupState::CleanupInProgress;
    ios.run();

    channel = nullptr;
    cleanupState = utils::CleanupState::CleanupDone;
}

/*!
 * \brief ChannelHandle::DoFreeChannel Gets called every 10 millisecs; calls libssh2_channel_free()
 *  repeteadly, until the channel is freed.
 * \return Returns the state of the task it's performing, i.e., whether the channel is freed.
 */
utils::TaskState ChannelHandle::DoFreeChannel()
{
    int rc = libssh2_channel_free(channel);

    if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        return utils::TASK_WORKING;
    }

    if (rc)
    {
        BOOST_THROW_EXCEPTION(SSHFreeChannelError{} <<
            ssh_error_string{"ChannelHandle: Error " + lexical_cast<string>(rc) +
            " during cleanup SSH channel local resources"} <<
            ssh_error_id{rc});
    }

    return utils::TASK_DONE;
}

} // namespace ssh
}}}
