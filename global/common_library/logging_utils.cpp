// global
#include "global/common_library/logging_utils.hpp"

// Logme
#include <Logme/Backend/Backend.h>
#include <Logme/Backend/ConsoleBackend.h>
#include <Logme/Backend/FileBackend.h>
#include <Logme/Context.h>
#include <Logme/Logme.h>

// nlohmann_json
#include <nlohmann/json.hpp>


namespace global::logging_utils
{

namespace
{

Logme::OutputFlags text_flags(const bool signature)
{
    Logme::OutputFlags flags;
    flags.Value = 0;
    flags.Eol = 1;
    flags.Signature = signature ? 1 : 0;
    flags.Subsystem = 1;
    return flags;
}

void prefix_indent(Logme::Context& context)
{
    const auto indent = log_tabulator_t::current_indent();
    if (indent == 0 || context.TempBuffer == nullptr)
        return;

    std::string text(indent, ' ');
    text.append(context.TempBuffer, context.TempBufferSize);
    text.reserve(text.size() + 8);

    context.Storage = std::move(text);
    context.TempBuffer = context.Storage.data();
    context.TempBufferSize = context.Storage.size();
    context.TempBufferCapacity = context.Storage.capacity();
}

// First backend of the channel: appends the indent to the text before Context::Apply.
struct indent_backend_t : Logme::Backend
{
    static constexpr const char* TYPE_ID = "IndentBackend";

    explicit indent_backend_t(Logme::ChannelPtr owner)
        : Logme::Backend(owner, TYPE_ID)
    {}

    void Display(Logme::Context& context) override
    {
        prefix_indent(context);
    }
};

void add_indent_backend(const Logme::ChannelPtr& channel)
{
    channel->AddBackend(std::make_shared<indent_backend_t>(channel));
}

// Snapshot of channels except Default: IterateChannels holds Logger::DataLock,
// and Channel::Display may take the same mutex again.
//
std::vector<Logme::ChannelPtr> channels_except(const Logme::ChannelPtr& skip)
{
    std::vector<Logme::ChannelPtr> channels;
    Logme::Instance->IterateChannels([&](const std::string&, const Logme::ChannelPtr& channel)
    {
        if (channel && channel != skip)
            channels.push_back(channel);
    });
    return channels;
}

}// namespace

void use_console()
{
    auto channel = Logme::Instance->GetDefaultChannelPtr();
    const auto flags = text_flags(true);
    channel->RemoveBackends();
    channel->SetFlags(flags);
    channel->SetFilterLevel(Logme::Level::LEVEL_DEBUG);
    channel->SetEnabled(true);
    add_indent_backend(channel);
    channel->AddBackend(std::make_shared<Logme::ConsoleBackend>(channel));
}

void use_named_channel(const name_t& name, const name_t& log_file)
{
    // fLogme* writes to default. The named channel stays the receiver:
    // the current thread switches to it, other threads reach it through the default filter.
    Logme::Instance->CreateDefaultChannelLayout(true);

    const auto path = path_t(log_file);
    if (path.has_parent_path())
        fs::create_directories(path.parent_path());

    Logme::OutputFlags flags;
    flags.Value = 0;
    flags.Eol = 1;
    flags.Channel = 1;
    flags.Subsystem = 1;

    const Logme::ID id{name.c_str()};
    auto channel = Logme::Instance->CreateChannel(
        id,
        flags,
        Logme::Level::LEVEL_DEBUG
    );
    channel->RemoveBackends();
    channel->SetFlags(flags);
    channel->SetFilterLevel(Logme::Level::LEVEL_DEBUG);
    channel->SetEnabled(true);
    add_indent_backend(channel);
    channel->AddBackend(std::make_shared<Logme::ConsoleBackend>(channel));

    auto backend = std::make_shared<Logme::FileBackend>(channel);
    backend->SetAppend(false);
    if (!backend->CreateLog(log_file.c_str()))
        throw std::runtime_error("cannot open log file: " + log_file);
    channel->AddBackend(backend);

    Logme::Instance->SetThreadChannel(&id);

    if (const auto def = Logme::Instance->GetDefaultChannelPtr())
    {
        def->SetDisplayFilter([channel](Logme::Context& context, const char*)
        {
            channel->Display(context);
            return false;
        });
    }
}

void flush_log()
{
    Logme::Instance->FlushAll();
}

void apply_log_config(const file_content_with_path_t& cwp)
{
    // here we manually fix the working-directory path, making it relative to the config path, not to the .exe
    const path_ot home_path = [&] -> path_ot
    {
        try
        {
            // args for nlohmann::json::parse: (*cwp.content) is the string to parse,
            // nullptr disables a custom parser callback, "true" enables exceptions,
            // and the last "true" enables allow_exceptions (ignore comments in JSON)
            auto json = nlohmann::json::parse(*cwp.content, nullptr, true, true);
            auto& home = json.at("home-directory").at("path");
            if (home.is_string())
            {
                path_t resolved{home.get<name_t>()};
                if (!resolved.is_absolute())
                {
                    resolved = (path_t(cwp.path).parent_path() / resolved).lexically_normal();
                }
                return resolved;
            }
        }
        catch (const nlohmann::json::exception&)
        {
        }
        return {};
    }();

    // a relative "file" is opened under the home directory.
    // LoadConfiguration then stores the raw home-directory.path, so "." would mean the process directory.
    // the resolved path is applied before the open, and again after the load
    if (home_path)
    {
        Logme::Instance->SetHomeDirectory(home_path->string());
    }

    Logme::Instance->DeleteAllChannels();

    content_t error;
    if (!Logme::Instance->LoadConfiguration(*cwp.content, content_t(), &error))
        throw std::runtime_error(error.empty() ? "cannot load log configuration" : error);

    if (home_path)
    {
        Logme::Instance->SetHomeDirectory(home_path->string());
    }

    // fLogme* first asks the default channel's level and, if refused, does not write the record at all.
    // So the default threshold is lowered to the most verbose level among the channels,
    // and its own level from the config is applied later in the filter, after forwarding.
    // RemoveBackends() freezes FileBackend, so the indent backend is not inserted here.
    const auto def = Logme::Instance->GetDefaultChannelPtr();
    const auto others = channels_except(def);
    if (def)
    {
        const auto configured = def->GetFilterLevel();
        auto gate = configured;
        for (const auto& channel : others)
            gate = std::min(gate, channel->GetFilterLevel());
        def->SetFilterLevel(gate);

        def->SetDisplayFilter([others, configured](Logme::Context& context, const char*)
        {
            prefix_indent(context);
            for (const auto& channel : others)
                channel->Display(context);
            return context.ErrorLevel >= configured;
        });
    }
}

}// namespace global::logging_utils
