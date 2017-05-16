#pragma once

#include "cocaine/framework/detail/forwards.hpp"
#include "cocaine/framework/forwards.hpp"
#include "cocaine/framework/service.hpp"

#include <string>
#include <cstdint>

namespace cocaine { namespace framework {

class service_manager_t;

struct token_manager_t {
    virtual ~token_manager_t() = default;

    virtual
    auto
    token() const -> token_t = 0;

    static
    auto
    make(detail::loop_t& io, service_manager_t& manager, const options_t& options)
        -> std::shared_ptr<token_manager_t>;

protected:
    token_manager_t() = default;
};

class null_token_manager_t final : public token_manager_t {
public:
    auto
    token() const -> token_t override;
};

class tvm_token_manager_t final : public token_manager_t {
    struct tvm_service_impl_t;
    std::shared_ptr<tvm_service_impl_t> d;
public:
    tvm_token_manager_t(detail::loop_t& io, service_manager_t& manager, const options_t& options);

    auto
    token() const -> token_t override;
};

}
}
