#ifndef NDEBUG
#ifndef CMX_SINK
#define CMX_SINK

// lib
#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>

namespace cmx
{

template <typename mutex_t> class EditorSink : public spdlog::sinks::base_sink<mutex_t>
{
  public:
    const std::vector<std::pair<std::string, spdlog::level::level_enum>> &getLines() const
    {
        return _lines;
    }

  protected:
    std::vector<std::pair<std::string, spdlog::level::level_enum>> _lines;

    virtual void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);
        _lines.push_back({fmt::to_string(formatted), msg.level});
    }
    virtual void flush_() override
    {
    }
};

} // namespace cmx

#endif
#endif
