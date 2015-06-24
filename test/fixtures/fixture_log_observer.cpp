#include "fixture_log_observer.hpp"
#include "../fixtures/util.hpp"

namespace mbgl {

FixtureLog::Message::Message(EventSeverity severity_,
                             Event event_,
                             int64_t code_,
                             const std::string& msg_)
    : severity(severity_), event(event_), code(code_), msg(msg_) {
}

FixtureLog::Message::Message() : severity(), event(), code(), msg() {
}

bool FixtureLog::Message::operator==(const Message& rhs) const {
    return (!severity || !rhs.severity || severity.get() == rhs.severity.get()) &&
           (!event || !rhs.event || event.get() == rhs.event.get()) &&
           (!code || !rhs.code || code.get() == rhs.code.get()) &&
           (!msg || !rhs.msg || msg.get() == rhs.msg.get());
}

FixtureLog::Observer::Observer(FixtureLog* log_) : log(log_) {
}

FixtureLog::Observer::~Observer() {
    if (log) {
        log->observer = nullptr;
    }
    std::cerr << unchecked();
}

bool FixtureLog::Observer::onRecord(EventSeverity severity,
                                    Event event,
                                    int64_t code,
                                    const std::string& msg) {
    messages.emplace_back(severity, event, code, msg);

    return true;
}

size_t FixtureLog::Observer::count(const LogMessage& message) const {
    size_t message_count = 0;
    for (const auto& msg : messages) {
        if (msg == message) {
            message_count++;
            msg.checked = true;
        }
    }
    return message_count;
}

FixtureLog::FixtureLog() : observer(new FixtureLogObserver(this)) {
    Log::setObserver(std::unique_ptr<Log::Observer>(observer));
}

size_t FixtureLog::count(const FixtureLog::Message& message) const {
    return observer ? observer->count(message) : 0;
}

FixtureLog::~FixtureLog() {
    if (observer) {
        Log::removeObserver();
    }
}

std::vector<FixtureLog::Message> FixtureLogObserver::unchecked() const {
    std::vector<LogMessage> unchecked_messages;
    for (const auto& msg : messages) {
        if (!msg.checked) {
            unchecked_messages.push_back(msg);
            msg.checked = true;
        }
    }
    return unchecked_messages;
}

::std::ostream& operator<<(::std::ostream& os, const std::vector<FixtureLog::Message>& messages) {
    for (const auto& message : messages) {
        os << "- " << message;
    }
    return os;
}

::std::ostream& operator<<(::std::ostream& os, const FixtureLog::Message& message) {
    os << "[\"" << message.severity.get() << "\", \"" << message.event.get() << "\"";
    if (message.code)
        os << ", " << message.code.get();
    if (message.msg)
        os << ", \"" << message.msg.get() << "\"";
    return os << "]" << std::endl;
}
}
