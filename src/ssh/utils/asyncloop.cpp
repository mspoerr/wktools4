// asyncloop.cpp
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

#include "asyncloop.h"

#include <boost/bind.hpp>
using boost::bind;

namespace pt { namespace pcaetano { namespace bluesy {
namespace utils
{

void AsyncLoopSocket(boost::function<TaskState()> doWork, boost::asio::ip::tcp::socket& sock, bool isRead)
{
    if (sock.get_io_service().stopped())
    {
        return;
    }

    TaskState st = doWork();

    if (st == TASK_WORKING)
    {
        if (isRead)
        {
            sock.async_read_some(boost::asio::null_buffers(), bind(AsyncLoopSocket, doWork, boost::ref(sock),
                isRead));
        }
        else
        {
            sock.async_write_some(boost::asio::null_buffers(), bind(AsyncLoopSocket, doWork, boost::ref(sock),
                isRead));
        }

        return;
    }

    // Work is over. stop any outstanding events.
    // NOTE: this isn't 100% reliable, and there may be events that linger in the queue.
    // The next time we reset() and run() the io_service, these events will be processed,
    // and io_service::stopped() will return false, because we've just done reset() and run().
    // The state management (SSHSession::State) is our first step in controlling this, and we
    // may need to implement our own cancel mechanism
    //
    // this io_service feature poses an additional problem - by spreading the implementation
    // responsibility across multiple classes, we create scenarios where io_service's queue
    // may contain outstanding events referring to objects that were, in the meantime, destroyed.
    // this is why we're favouring, for now, the use of AsyncLoopTimer, where we can use an
    // io_service for each timer, and destroy it right after being used. this makes sure we
    // process no "zombie" outstanding events.
    sock.get_io_service().stop();
}

void AsyncLoopTimer(boost::function<TaskState()> doWork, boost::asio::deadline_timer& dTimer, int millisecs)
{
    TaskState st = doWork();

    if (st == TASK_WORKING)
    {
        dTimer.expires_from_now(boost::posix_time::milliseconds(millisecs));
        dTimer.async_wait(boost::bind(AsyncLoopTimer, doWork, boost::ref(dTimer), millisecs));
    }

    // Work is over; nothing to do, as the timer is not rearmed
}


} // namespace utils
}}}

