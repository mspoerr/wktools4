// channelhandle.h
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

#ifndef CHANNELHANDLE_H
#define CHANNELHANDLE_H

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

/*!
 * \brief RAII wrapper for a LIBSSH2_CHANNEL*.
 *
 *  It's not a perfect wrapper because the channel life cycle in libssh2 is assymetric -
 *  open vs. close and free. Plus, we must take into account that we can only get the
 *  channel's exit status and signal after we close it, but before we free it.
 *
 *  Currently, we do nothing on the ctor and free the libssh2 resources on the dtor.
 *  We need the LIBSSH2_CHANNEL* to perform this cleanup, and since it's created by
 *  ChannelHandleRemote, we have the SetChannel() method, so that ChannelHandleRemote
 *  can inject the LIBSSH2_CHANNEL* into ChannelHandle.
 *
 *  \todo Not happy with this design. Review.
 */
class ChannelHandle
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    // This ctor is nop. The channel life cycle in libssh2 is assymetric:
    // - Setup: Open channel
    // - Cleanup: Close channel, free channel
    //
    // After we close the channel, but before we free it, we can get its exit status
    // and exit signal, if any. We could open the channel here, and it would be a
    // cleaner design, since we'd then hand over the LIBSSH2_CHANNEL* to
    // RemoteChannelHandle, which is created later. However, we'd risk freeing an
    // open channel if an exception was fired before RemoteSessionHandle is
    // fully constructed (since we close the channel in its dtor). This may
    // not be a problem, assuming libssh2 handles it correctly
    // (i.e., closes the channel before freeing), but i prefer keeping
    // the flow inversion for now, i.e.:
    // - RemoteChannelhandle is created after ChannelHandle.
    // - RemoteChannelhandle supplies ChannelHandle with the LIBSSH2_CHANNEL*.
    /*!
     * \brief ChannelHandle
     * \param pReportStatus Callback to report to the caller.
     */
    explicit ChannelHandle(ReportFunctionRef pReportStatus);
    ~ChannelHandle();

#ifndef _MSC_VER
    ChannelHandle(ChannelHandle&&) = delete;
    ChannelHandle& operator=(ChannelHandle&&) = delete;
    ChannelHandle(ChannelHandle const&) = delete;
    ChannelHandle& operator=(ChannelHandle const&) = delete;
#endif

    LIBSSH2_CHANNEL* GetChannel() { return channel; }
    void SetChannel(LIBSSH2_CHANNEL* pChannel) { channel = pChannel; }

    /*!
     * \brief FreeChannel Releases the local channel resources. Does nothing if we have no
     *  LIBSSH2_CHANNEL* or if FreeChannel() has been previously called and didn't finish normally.
     *
     * \pre If ChannelHandle.channel is not nullptr, it must be a valid LIBSSH2_CHANNEL*.
     *  FreeChannel() does not check for this, and will call libssh2_channel_free() with any
     *  non-null pointer.
     *  \exception SSHFreeChannelError if libssh2_channel_free() returns any error other than LIBSSH2_ERROR_EAGAIN.
     */
    void FreeChannel();
private:
#ifdef _MSC_VER
    ChannelHandle(ChannelHandle&&) {}
    ChannelHandle& operator=(ChannelHandle&&) {}
#endif

    utils::TaskState DoFreeChannel();

    LIBSSH2_CHANNEL* channel;
    ReportFunctionPtr reportStatus;

    /*!
     * \brief cleanupState When the channel is explicitely freed, by calling FreeChannel(),
     *  the state is updated, to prevent the dtor from trying to free it again.
     */
    utils::CleanupState cleanupState;
};

} // namespace ssh
}}}
#endif // CHANNELHANDLE_H
