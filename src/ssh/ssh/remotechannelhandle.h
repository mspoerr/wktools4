// remotechannelhandle.h
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

#ifndef REMOTECHANNELHANDLE_H
#define REMOTECHANNELHANDLE_H

#include "misc.h"
#include "../utils/asyncloop.h"
#include "../utils/misc.h"

#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif
#include <boost/function.hpp>

#include <libssh2.h>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunction = boost::function<void(std::string)>;
using ReportFunctionRef = ReportFunction&;
using ReportFunctionPtr = ReportFunction*;

class RemoteChannelHandle
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    RemoteChannelHandle(ChannelHandleRef pChannelHandle, SessionHandleRef pSessionHandle,
        ReportFunctionRef pReportStatus);
    ~RemoteChannelHandle();

#ifndef _MSC_VER
    RemoteChannelHandle(RemoteChannelHandle&&) = delete;
    RemoteChannelHandle& operator=(RemoteChannelHandle&&) = delete;
    RemoteChannelHandle(RemoteChannelHandle const&) = delete;
    RemoteChannelHandle& operator=(RemoteChannelHandle const&) = delete;
#endif

    /*!
     * \brief CloseChannel Closes the channel to the remote host.
     */
    void CloseChannel();
private:
#ifdef _MSC_VER
    RemoteChannelHandle& operator=(RemoteChannelHandle&&) {}
#endif

    utils::TaskState DoOpenChannel();
    utils::TaskState DoCloseChannel();

    LIBSSH2_CHANNEL* channel;

    /*!
     * \brief channelHandle Manages the local channel resources. The channel is opened in this class
     *  (on the ctor), and is also closed on this class (on the dtor), but will be freed on
     *  ChannelHandle's dtor, so we need to "share" ownership.
     */
    ChannelHandlePtr channelHandle;

    /*!
     * \brief sessionHandle We need the session to create channel.
     */
    SessionHandlePtr sessionHandle;

    /*!
     * \brief reportStatus Client callback to report to the caller.
     */
    ReportFunctionPtr reportStatus;

    /*!
     * \brief cleanupState When the channel is explicitely closed, by calling CloseChannel(),
     *  the state is updated, to prevent the dtor from trying to close it again.
     */
    utils::CleanupState cleanupState;
};

} // namespace ssh
}}}
#endif // REMOTECHANNELHANDLE_H
