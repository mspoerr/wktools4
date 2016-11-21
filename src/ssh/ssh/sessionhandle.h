// sessionhandle.h
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

#ifndef SESSIONHANDLE_H
#define SESSIONHANDLE_H

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
 * \brief The SessionHandle class Manages the SSH session's local resources.
 *
 * \todo Remove RemoteChannelHandle from the friend class list.
 */
class SessionHandle
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    SessionHandle(ReportFunctionRef pReportStatus);
    SessionHandle(SessionHandle&&);
    ~SessionHandle();

    void CloseSession();

#ifndef _MSC_VER
    SessionHandle& operator=(SessionHandle&&) = delete;
    SessionHandle(SessionHandle const&) = delete;
    SessionHandle& operator=(SessionHandle const&) = delete;
#endif
private:
#ifdef _MSC_VER
    SessionHandle& operator=(SessionHandle&&) {}
#endif

    friend class RemoteSessionHandle;
    friend class RemoteChannelHandle;

    LIBSSH2_SESSION* GetSession() { return session; }

    utils::TaskState DoCloseSession();

    LIBSSH2_SESSION *session;

    /*!
     * \brief reportStatus Client callback to report to the caller.
     */
    ReportFunction reportStatus;

    /*!
     * \brief cleanupState When the session is explicitely closed, by calling CloseSession(),
     *  the state is updated, to prevent the dtor from trying to close it again.
     */
    utils::CleanupState cleanupState;
};

} // namespace ssh
}}}

#endif // SESSIONHANDLE_H
