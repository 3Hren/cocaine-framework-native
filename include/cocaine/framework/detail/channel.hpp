/*
    Copyright (c) 2015 Evgeny Safronov <division494@gmail.com>
    Copyright (c) 2011-2015 Other contributors as noted in the AUTHORS file.
    This file is part of Cocaine.
    Cocaine is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.
    Cocaine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>

#include <cocaine/rpc/asio/encoder.hpp>
#include <cocaine/rpc/asio/decoder.hpp>
#include <cocaine/rpc/asio/readable_stream.hpp>
#include <cocaine/rpc/asio/writable_stream.hpp>

namespace cocaine {

namespace framework {

namespace detail {

template<class Protocol, class Encoder = io::encoder_t, class Decoder = io::decoder_t>
struct channel {
    typedef Protocol protocol_type;
    typedef Encoder encoder_type;
    typedef Decoder decoder_type;
    typedef typename protocol_type::socket socket_type;

    explicit
    channel(std::unique_ptr<socket_type> socket):
        socket(std::move(socket)),
        reader(new io::readable_stream<protocol_type, decoder_type>(this->socket)),
        writer(new io::writable_stream<protocol_type, encoder_type>(this->socket))
    {
        this->socket->non_blocking(true);
    }

   ~channel() {
        try {
            socket->shutdown(socket_type::shutdown_both);
            socket->close();
        } catch(const asio::system_error&) {
            // Might be already disconnected by the remote peer, so ignore all errors.
        }
    }

    // The underlying shared socket object.
    const std::shared_ptr<socket_type> socket;

    // Unidirectional channel streams.
    const std::shared_ptr<io::readable_stream<protocol_type, decoder_type>> reader;
    const std::shared_ptr<io::writable_stream<protocol_type, encoder_type>> writer;
};

}

}

}
