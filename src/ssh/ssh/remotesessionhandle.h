// remotesessionhandle.h
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

#ifndef REMOTESESSIONHANDLE_H
#define REMOTESESSIONHANDLE_H

#include "sessionhandle.h"
#include "misc.h"
#include "../utils/asyncloop.h"
#include "../utils/misc.h"

#include <boost/asio.hpp>
#include <boost/function.hpp>

#ifdef _MSC_VER
#include <boost/noncopyable.hpp>
#endif

#include <string>
#include <memory>

namespace pt { namespace pcaetano { namespace bluesy {
namespace ssh
{

using ReportFunction = boost::function<void(std::string)>;
using ReportFunctionRef = ReportFunction&;
using ReportFunctionPtr = ReportFunction*;

using Socket = boost::asio::ip::tcp::socket;
using SocketRef = Socket&;
using SocketPtr = Socket*;

/*!
 * \brief The RemoteSessionHandle class Manages the SSH session with the remote host.
 *
 *  It also handles the necessary local resources to create/maintain the SSH session.
 *
 * \todo Review the GetSession() and GetSessionHandle() members.
 */
class RemoteSessionHandle
#ifdef _MSC_VER
        : boost::noncopyable
#endif
{
public:
    /*!
     * \brief RemoteSessionHandle Performs the SSH handshake with the remote host.
     *
     *  Before the handshake, there are two previous steps performed here:
     *  - Connect the socket. Performed on the ctor init list, by calling ConnectSocket();
     *  - Create the session local resources, performed by sessionHandle.
     */
    RemoteSessionHandle(ReportFunctionRef pReportStatus, std::string const& host, std::string const& port);
    RemoteSessionHandle(RemoteSessionHandle&&);

    /*!
     * \brief ~RemoteSessionHandle Closes the SSH session on the remote host.
     *
     *  After closing the session, in the dtor's body, it releases the local SSH resources,
     *  when sessionHandle is destroyed; and closes the socket, when sock is destroyed.
     */
    ~RemoteSessionHandle();

    void CloseSession();

    SessionHandleRef GetSessionHandle() { return sessionHandle; }
    LIBSSH2_SESSION* GetSession() { return sessionHandle.GetSession(); }

#ifndef _MSC_VER
    RemoteSessionHandle& operator=(RemoteSessionHandle&&) = delete;
    RemoteSessionHandle(RemoteSessionHandle const&) = delete;
    RemoteSessionHandle& operator=(RemoteSessionHandle const&) = delete;
#endif
private:
#ifdef _MSC_VER
    RemoteSessionHandle& operator=(RemoteSessionHandle&&) {}
#endif

    utils::TaskState DoHandshake();
    utils::TaskState DoCloseSession();

    Socket* ConnectSocket(std::string const& host, std::string const& port);

    // !!! THE MEMBER ORDER IS ESSENTIAL !!!
    // - sock needs ios.
    // - sessionHandle needs sock.

    /*!
     * \brief reportStatus Client callback to report to the caller.
     */
    ReportFunction reportStatus;

    /*!
     * \brief ios Required by socket. io_service is not moveable,
     * so we use an unique_ptr.
     */
    std::unique_ptr<boost::asio::io_service> ios;

    /*!
     * \brief sock Socket to communicate with the remote host. We need the socket because
     *  libssh2_session_handshake() requires it.
     *
     * If socket is moveable, why are we using a pointer?
     * See http://cidebycide.blogspot.pt/2013/02/lets-keep-moving-will-puns-ever-end.html.
     * Adding to this, while it's true that socket is moveable, the doc states that the moved-from socket
     * "is in the same state as if constructed using the basic_stream_socket(io_service&)
     * constructor". This means that, when being destroyed, it'll operate upon its
     * io_service. However, io_service is not moveable, so we're using an unique_ptr;
     * when we move the unique_ptr, the moved-from unique_ptr is left with nullptr.
     * So, using socket would mean the moved-from socket would be left pointing at
     * a null unique_ptr<io_service>.
     * So, for now, we'll use a unique_ptr<socket>, to avoid complications in
     * move operations.
     * As a final note, looking at the C++11 examples in boost asio, all the examples where
     * a socket is moved avoid this by creating the socket's io_service in main().
     *
     * \todo There has to be a better way. I'm missing something here.
     */
    std::unique_ptr<Socket> sock;

    /*!
     * \brief sessionHandle Manages the SSH session local resources. Contains the
     *  libssh2 session we need for our operations.
     */
    // This used to be in SessionConnection, but I moved it in here. Why?
    // See here: http://cidebycide.blogspot.pt/2013/07/back-to-libssh2boost-asio.html
    SessionHandle sessionHandle;

    /*!
     * \brief cleanupState When the session is explicitely closed, by calling CloseSession(),
     *  the state is updated, to prevent the dtor from trying to close it again.
     */
    utils::CleanupState cleanupState;
};

} // namespace ssh
}}}

#endif // REMOTESESSIONHANDLE_H
