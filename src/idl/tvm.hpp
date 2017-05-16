//
// Note: protocol definition should corresponds with source tvm IDL, mostly
// copy-pasted
//
#pragma once

#include <string>

#include <boost/mpl/list.hpp>

#include <cocaine/rpc/protocol.hpp>
#include <cocaine/rpc/tags.hpp>

namespace cocaine { namespace io {

struct tvm_tag;

struct tvm {

/// Refreshes an application ticket.
///
/// This is meant to be called from applications that wishes to refresh its ticket(s) due to soon
/// expiration.
///
struct refresh_ticket {
    typedef tvm_tag tag;

    static auto alias() noexcept -> const char* {
        return "refresh_ticket";
    }

    typedef boost::mpl::list<
     /* Source, application name */
        std::string,
     /* Ticket. */
        std::string
    >::type argument_type;

    typedef option_of<
        std::string
    >::tag upstream_type;
};

};

template<>
struct protocol<tvm_tag> {
    typedef boost::mpl::int_<
        1
    >::type version;

    typedef boost::mpl::list<
        void,
        void,
        void,
        void,
        tvm::refresh_ticket,
        void
    >::type messages;

    typedef tvm scope;
};

}
}
