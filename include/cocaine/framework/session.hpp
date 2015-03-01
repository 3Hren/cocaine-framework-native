#pragma once

#include <cstdint>
#include <queue>
#include <unordered_map>

#include <boost/asio/ip/tcp.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>

#include <cocaine/common.hpp>
#include <cocaine/locked_ptr.hpp>
#include <cocaine/rpc/asio/channel.hpp>

#include "cocaine/framework/config.hpp"
#include "cocaine/framework/forwards.hpp"
#include "cocaine/framework/receiver.hpp"
#include "cocaine/framework/sender.hpp"

#include "cocaine/framework/detail/channel.hpp"

namespace cocaine {

namespace framework {

// TODO: Deprecated docs.
/*!
 * \note I can't guarantee lifetime safety in other way than by making this class living as shared
 * pointer. The reason is: in particular case the connection's event loop runs in a separate
 * thread, other in that the connection itself lives.
 * Thus no one can guarantee that all asynchronous operations are completed before the connection
 * instance be destroyed.
 *
 * \threadsafe
 */
// TODO: Thing, that sends/receives messages & thing, that manages with channel map.
class basic_session_t : public std::enable_shared_from_this<basic_session_t> {
    typedef asio::ip::tcp protocol_type;
    typedef protocol_type::socket socket_type;
public:
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

private:
    typedef detail::channel<protocol_type, io::encoder_t, detail::decoder_t> channel_type;

    enum class state_t : std::uint8_t {
        disconnected = 0,
        connecting,
        connected
    };

    scheduler_t& scheduler;

    std::unique_ptr<channel_type> channel;

    mutable std::mutex mutex;
    std::atomic<std::uint8_t> state;

    std::atomic<std::uint64_t> counter;

    detail::decoder_t::message_type message;
    synchronized<std::unordered_map<std::uint64_t, std::shared_ptr<detail::shared_state_t>>> channels;

    class push_t;
public:
    typedef std::tuple<
        std::shared_ptr<basic_sender_t<basic_session_t>>,
        std::shared_ptr<basic_receiver_t<basic_session_t>>
    > invoke_result;

    /*!
     * \warning the scheduler reference should be valid until all asynchronous operations complete
     * otherwise the behavior is undefined.
     */
    explicit basic_session_t(scheduler_t& scheduler) noexcept;

    ~basic_session_t();

    /*!
     * Checks whether the session is in connected state.
     *
     * \note the session does passive connection monitoring, e.g. it won't be immediately notified
     * if the real connection has been lost, but after the next send/recv attempt.
     */
    bool connected() const noexcept;

    /// \threadsafe
    auto connect(const endpoint_type& endpoint) -> typename task<std::error_code>::future_type;

    /// \threadsafe
    auto connect(const std::vector<endpoint_type>& endpoints) -> typename task<std::error_code>::future_type;

    // TODO: Obtain an endpoint if is in connected state.
    // auto endpoint() const -> boost::optional<endpoint_type>;

    /*!
     * Emits a disconnection request to the current session.
     *
     * All pending requests should result in operation aborted error.
     */
    void disconnect();

    /*!
     * Obtain the next channel id number, that is guaranteed to be unique for further invocation.
     */
    auto next() -> std::uint64_t;

    /*!
     * Sends an invocation event and creates a new channel accociated with it.
     *
     * \note if the future returned throws an exception that means that the data will never be
     * received, but if it doesn't - the data is not guaranteed to be received. It is possible for
     * the other end of connection to hang up immediately after the future returns ok.
     *
     * If you send a **mute** event, there is no way to obtain guarantees of successful message
     * transporting.
     */
    auto invoke(std::uint64_t span, io::encoder_t::message_type&& message) -> typename task<invoke_result>::future_type;

    /*!
     * Sends an event without creating a new channel.
     */
    auto push(io::encoder_t::message_type&& message) -> typename task<void>::future_type;

    /*!
     * Unsubscribes a channel with the given span.
     *
     * \todo return operation result (revoked/notfound).
     */
    void revoke(std::uint64_t span);

private:
    void on_disconnect();
    void on_revoke(std::uint64_t span);
    void on_connect(const std::error_code& ec, typename task<std::error_code>::promise_type& promise, std::unique_ptr<socket_type>& s);
    void on_read(const std::error_code& ec);
    void on_error(const std::error_code& ec);
};

/*!
 * RAII class that manages with connection queue and returns a typed sender/receiver.
 */
template<class BasicSession = basic_session_t>
class session {
public:
    typedef BasicSession basic_session_type;
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    template<class Event>
    struct invoke_result {
        typedef sender  <typename io::event_traits<Event>::dispatch_type, basic_session_t> sender_type;
        typedef receiver<typename io::event_traits<Event>::upstream_type, basic_session_t> receiver_type;
        typedef std::tuple<sender_type, receiver_type> type;
    };

private:
    class impl;
    std::shared_ptr<impl> d;
    scheduler_t& scheduler;
    std::shared_ptr<basic_session_type> sess;

public:
    explicit session(scheduler_t& scheduler);
    ~session();

    bool connected() const;

    auto connect(const endpoint_type& endpoint) -> typename task<void>::future_type;
    auto connect(const std::vector<endpoint_type>& endpoints) -> typename task<void>::future_type;

    void disconnect();

    template<class Event, class... Args>
    typename task<typename invoke_result<Event>::type>::future_type
    invoke(Args&&... args) {
        const std::uint64_t span(sess->next());
        return sess->invoke(span, io::encoded<Event>(span, std::forward<Args>(args)...))
            .then(scheduler, std::bind(&session::on_invoke<Event>, std::placeholders::_1));
    }

private:
    template<class Event>
    static
    typename invoke_result<Event>::type
    on_invoke(typename task<typename basic_session_type::invoke_result>::future_move_type f) {
        typedef typename invoke_result<Event>::sender_type sender_type;
        typedef typename invoke_result<Event>::receiver_type receiver_type;

        auto channel = f.get();
        sender_type tx(std::get<0>(channel));
        receiver_type rx(std::get<1>(channel));
        return std::make_tuple(std::move(tx), std::move(rx));
    }
};

} // namespace framework

} // namespace cocaine
