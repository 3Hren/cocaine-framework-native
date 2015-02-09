#include <asio/read.hpp>
#include <asio/write.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cocaine/framework/connection.hpp>
#include <cocaine/framework/session.hpp>

#include "mock/event.hpp"
#include "util/net.hpp"

using namespace cocaine::framework;

using namespace testing;
using namespace testing::util;

namespace testing {

namespace mock {

class basic_session_t {
public:
    MOCK_CONST_METHOD0(connected, bool());
    MOCK_METHOD1(connect, future_t<std::error_code>&(const endpoint_t&));
};

} // namespace mock

} // namespace testing

TEST(session, Constructor) {
    auto d = std::make_shared<mock::basic_session_t>();
    auto s = std::make_shared<session<mock::event_tag, mock::basic_session_t>>(d);

    EXPECT_CALL(*d, connected())
        .WillOnce(Return(false));

    EXPECT_FALSE(s->connected());
}

TEST(session, Connect) {
    const io::ip::tcp::endpoint endpoint(io::ip::tcp::v4(), 42);

    auto d = std::make_shared<mock::basic_session_t>();
    auto s = std::make_shared<session<mock::event_tag, mock::basic_session_t>>(d);

    auto f = make_ready_future<std::error_code>::value(std::error_code());

    EXPECT_CALL(*d, connect(endpoint))
        .WillOnce(ReturnRef(f));

    EXPECT_NO_THROW(s->connect(endpoint).get());
}

TEST(session, ConnectionRefused) {
    const io::ip::tcp::endpoint endpoint(io::ip::tcp::v4(), 42);

    auto d = std::make_shared<mock::basic_session_t>();
    auto s = std::make_shared<session<mock::event_tag, mock::basic_session_t>>(d);

    auto f = make_ready_future<std::error_code>::value(io::error::connection_refused);

    EXPECT_CALL(*d, connect(endpoint))
        .WillOnce(ReturnRef(f));

    EXPECT_THROW(s->connect(endpoint).get(), std::system_error);
}

TEST(session, ConnectWhileConnecting) {
    const io::ip::tcp::endpoint endpoint(io::ip::tcp::v4(), 42);

    auto d = std::make_shared<mock::basic_session_t>();
    auto s = std::make_shared<session<mock::event_tag, mock::basic_session_t>>(d);

    promise_t<std::error_code> p1;
    auto f1 = p1.get_future();
    auto f2 = make_ready_future<std::error_code>::value(io::error::already_started);

    EXPECT_CALL(*d, connect(endpoint))
        .WillOnce(ReturnRef(f1))
        .WillOnce(ReturnRef(f2));

    auto f11 = s->connect(endpoint);
    auto f12 = s->connect(endpoint);

    p1.set_value(std::error_code());

    EXPECT_NO_THROW(f11.get());
    EXPECT_NO_THROW(f12.get());
}

TEST(session, ConnectWhileConnectingError) {
    const io::ip::tcp::endpoint endpoint(io::ip::tcp::v4(), 42);

    auto d = std::make_shared<mock::basic_session_t>();
    auto s = std::make_shared<session<mock::event_tag, mock::basic_session_t>>(d);

    promise_t<std::error_code> p1;
    auto f1 = p1.get_future();
    auto f2 = make_ready_future<std::error_code>::value(io::error::already_started);

    EXPECT_CALL(*d, connect(endpoint))
        .WillOnce(ReturnRef(f1))
        .WillOnce(ReturnRef(f2));

    auto f11 = s->connect(endpoint);
    auto f12 = s->connect(endpoint);

    p1.set_value(io::error::connection_refused);

    EXPECT_THROW(f11.get(), std::system_error);
    EXPECT_THROW(f12.get(), std::system_error);
}

/// functional
// - connect 1 ok.
// - connect 1 err - throw.
// - connect 2 seq ok.
// - connect 2 1by1 ok.
// - connect 2 seq with error
// - connect 2 different endpoint seq - throw
// - connect different endpoint when connected
// = 70m + refactoring 20m => 1.5h