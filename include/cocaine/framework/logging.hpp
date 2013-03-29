/*
    Copyright (c) 2011-2012 Andrey Sibiryov <me@kobology.ru>
    Copyright (c) 2011-2012 Other contributors as noted in the AUTHORS file.

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

#ifndef COCAINE_FRAMEWORK_LOGGING_HPP
#define COCAINE_FRAMEWORK_LOGGING_HPP

#include <cocaine/forwards.hpp>
#include <cocaine/json.hpp>
#include <cocaine/format.hpp>

#include <string>


#define COCAINE_LOG(log, level, ...) \
    if(log->verbosity() >= level) log->emit(level, __VA_ARGS__);

#define COCAINE_LOG_DEBUG(log, ...) \
    COCAINE_LOG(log, logging::debug, __VA_ARGS__)

#define COCAINE_LOG_INFO(log, ...) \
    COCAINE_LOG(log, logging::info, __VA_ARGS__)

#define COCAINE_LOG_WARNING(log, ...) \
    COCAINE_LOG(log, logging::warning, __VA_ARGS__)

#define COCAINE_LOG_ERROR(log, ...) \
    COCAINE_LOG(log, logging::error, __VA_ARGS__)

namespace cocaine { namespace framework {

class logger_t {
public:
    virtual
    ~logger_t() {
        // Empty.
    }

    virtual
    cocaine::logging::priorities
    verbosity() const = 0;

    virtual
    void
    emit(cocaine::logging::priorities priority,
         const std::string& source,
         const std::string& message) = 0;
};

class log_t {
public:
    log_t(std::shared_ptr<logger_t> logger,
          const std::string& source) :
        m_logger(logger),
        m_source(source)
    {
        // pass
    }

    cocaine::logging::priorities
    verbosity() const {
        return m_logger->verbosity();
    }

    template<typename... Args>
    void
    emit(cocaine::logging::priorities level,
         const std::string& format,
         const Args&... args)
    {
        m_logger->emit(level, m_source, cocaine::format(format, args...));
    }

    void
    emit(cocaine::logging::priorities level,
         const std::string& message)
    {
        m_logger->emit(level, m_source, message);
    }

private:
    std::shared_ptr<logger_t> m_logger;
    const std::string m_source;
};

}} // namespace cocaine::framework

#endif // COCAINE_FRAMEWORK_LOGGING_HPP
