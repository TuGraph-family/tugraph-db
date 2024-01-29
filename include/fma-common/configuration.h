//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <list>
#include <mutex>
#include <vector>
#include <utility>
#include "fma-common/option.h"
#include "tools/json.hpp"
#include "fma-common/fma_stream.h"

namespace fma_common {

class ConfigurationError : public std::exception {
    std::string str;

 public:
    explicit ConfigurationError(std::string str) : str(str) {}

    virtual const char *what() const noexcept { return str.c_str(); }
};

/*!
 * \class   Configuration
 *
 * \brief   A configuration which can be loaded from commandline arguments
 *          and configuration file. (Currently only supports reading from
 *          command line.)
 *
 * \todo    Enable reading configuration file.
 */
class Configuration {
 private:
    using OptionPtr = std::shared_ptr<OptionBase>;

    std::map<std::string, OptionPtr> named_options_;
    std::vector<OptionPtr> positional_options_;
    std::set<std::string> bool_options_;
    size_t positional_index_;
    std::list<OptionPtr> options_;

    std::map<std::string, Configuration *> children_;

    bool builtin_help_ = false;
    bool exit_after_help_ = true;

    void AddOption(OptionPtr option) {
        if (option->Keys().empty()) {
            positional_options_.push_back(option);
        } else {
            for (std::string key : option->Keys()) {
                auto it = named_options_.find(key);
                if (it == named_options_.end()) {
                    named_options_.emplace(key, option);
                } else {
                    throw ConfigurationError(std::string("Option name duplicated : ") + key);
                }
            }
        }
        options_.emplace_back(option);
    }

 public:
    Configuration() : positional_index_(0) {
        Add().SetDefault("Unknown").SetPreserve(true);  // parse argv[0]
    }

    void Finalize() {
        if (builtin_help_ && GetValue<bool>("help")) {
            LOG_INFO() << HelpString();
            if (exit_after_help_) exit(0);
        }
        for (auto &option : options_) {
            try {
                option->Check();
            } catch (std::exception &e) {
                throw std::runtime_error(std::string("Failed to parse option ") +
                                         option->StrKeys() + ": " + e.what());
            }
        }
        for (auto &child : children_) {
            try {
                child.second->Finalize();
            } catch (std::exception &e) {
                throw std::runtime_error(std::string("Failed to parse option ") + child.first +
                                         ": " + e.what());
            }
        }
    }

    /*!
     * \fn  Option<T>& Configuration::Add(T &val, std::string options)
     *
     * \brief   Adds an option.
     *
     * \param [in,out]  val         The option value. Original value will be
     *                              set as default if required=false.
     * \param           option_str     Option name, either long or short.
     *
     * \return  A reference to a fma_common::Option&lt;T&gt;
     */
    template <class T = std::string>
    Option<T> &Add(T &val, const char *option_str = "", bool has_default = false) {
        OptionRef<T> *option_ptr = new OptionRef<T>(val, fma_common::Split(option_str, ","));
        if (has_default) option_ptr->SetDefault(val);
        AddOption(OptionPtr(option_ptr));
        return *option_ptr;
    }

    Option<bool> &AddBinary(bool &val, const char *option_str = "", bool has_default = false) {
        OptionRef<bool> *option_ptr = new OptionRef<bool>(val, fma_common::Split(option_str, ","));
        if (has_default)
            option_ptr->SetDefault(val);
        else
            option_ptr->SetDefault(false);
        AddOption(OptionPtr(option_ptr));
        for (auto k : option_ptr->Keys()) bool_options_.insert(k);
        return *option_ptr;
    }

    /*!
     * \fn  Option<T>& ArgParser::Add(const std::string& long_option, const std::string&
     * short_option = "")
     *
     * \brief   Adds an option of type T.
     *
     * \param   option_str     Option string, Split by ",". keep empty if add a positional option
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    template <typename T = std::string>
    Option<T> &Add(const char *option_str = "") {
        Option<T> *option_ptr = new Option<T>(Split(option_str, ","));
        AddOption(OptionPtr(option_ptr));
        return *option_ptr;
    }

    Option<bool> &AddBinary(const char *option_str = "") {
        Option<bool> *option_ptr = new Option<bool>(Split(option_str, ","));
        option_ptr->SetDefault(false);
        AddOption(OptionPtr(option_ptr));
        for (auto k : option_ptr->Keys()) bool_options_.insert(k);
        return *option_ptr;
    }

    /*!
     * \fn  Option<T>& Configuration::Add(T &val, std::string options)
     *
     * \brief   Adds an option.
     *
     * \param [in,out]  val         The option value. Original value will be
     *                              set as default if required=false.
     * \param           option_str     Option name, either long or short.
     *
     * \return  A reference to a fma_common::Option&lt;T&gt;
     */
    void AddChild(Configuration *child, const char *prefix) {
        if (children_.find(prefix) != children_.end() ||
            named_options_.find(prefix) != named_options_.end()) {
            throw ConfigurationError(std::string("duplicated child prefix : ") + prefix);
        }
        children_.emplace(prefix, child);
    }

    /*!
     * \fn  void ArgParser::Parse(int argc, char** argv, bool permissive, std::vector<bool>& taken)
     *
     * \brief   Parses parameters.
     *
     * \param           argc        The arg count.
     * \param [in]      argv        The arg values.
     * \param           permissive  If set to true, enables permissive mode, in which excessive
     *                              parameters are ignored without raising an error. Otherwise,
     *                              an error is raised in case any unrecognized parameter is
     *                              given.
     * \param [in,out]  taken       taken[i]==true if the arg in the ith position is parsed as
     *                              an argument.
     */
    void Parse(int argc, char **argv, bool permissive, std::vector<bool> &taken,
               bool exit_after_help = true) {
        // add --help if not added
        if (named_options_.find("help") == named_options_.end()) {
            AddBinary("help,h")
                .Comment("Print this help message")
                .SetDefault(false)
                .SetPreserve(true);
            builtin_help_ = true;
        }

        if (taken.empty()) taken.resize(argc, false);

        for (int i = 0; i < argc; i++) {
            if (taken[i]) continue;
            OptionBase *opt = nullptr;
            std::string value_string;
            if (strstr(argv[i], "-") == argv[i]) {
                char *key_str = argv[i] + 1;
                if (strstr(argv[i], "--") == argv[i]) {
                    key_str = argv[i] + 2;
                }
                auto it = named_options_.find(key_str);
                if (it == named_options_.end()) {
                    if (permissive) {
                        if (i + 1 < argc && argv[i + 1][0] != '-') i++;
                        continue;
                    } else {
                        // trigger error
                        throw ConfigurationError(std::string("Option ") + argv[i] +
                                                 " cannot be recognized.");
                    }
                }
                opt = it->second.get();
                if (bool_options_.find(key_str) ==
                    bool_options_.end()) {  // normal option  --key val, -k val
                    if (i + 1 >= argc)
                        throw ConfigurationError(
                            std::string("Value was not specified for option ") + argv[i]);
                    if (!opt->Preserve()) {
                        taken[i] = true;
                        taken[i + 1] = true;
                    }
                    i++;
                    value_string = argv[i];
                } else {  // bool option --flag, -f
                    if (!opt->Preserve()) {
                        taken[i] = true;
                    }
                    value_string = "true";
                }
            } else {
                if (positional_index_ >= positional_options_.size()) continue;
                opt = positional_options_[positional_index_].get();
                positional_index_++;
                if (!opt->Preserve()) taken[i] = true;
                value_string = argv[i];
            }
            opt->SetValueString(value_string);
        }
    }

    /*!
     * \fn  void ArgParser::Parse(int argc, char** argv, bool permissive = false)
     *
     * \brief   Parses the arguments.
     *
     * \param           argc        The arg count.
     * \param [in]      argv        The arg values.
     * \param           permissive  If set to true, enables permissive mode, in which excessive
     *                              parameters are ignored without raising an error. Otherwise,
     *                              an error is raised in case any unrecognized parameter is
     *                              given.
     */
    void Parse(int argc, char **argv, bool permissive = false) {
        std::vector<bool> taken;
        Parse(argc, argv, permissive, taken);
    }

    void ParseAndFinalize(int argc, char **argv) {
        std::vector<bool> taken;
        Parse(argc, argv, false, taken);
        Finalize();
    }

    /*!
     * \fn  void ArgParser::ParseAndRemove(int* argc, char*** argv)
     *
     * \brief   Parse the arguments and remove parsed arguments from the list.
     *
     * \param [in,out]  argc    Pointer to argc, will store remaining arg count.
     * \param [in,out]  argv    Pointer to argv, will store the remaining arg values.
     */
    void ParseAndRemove(int *argc, char ***argv) {
        std::vector<bool> taken;
        Parse(*argc, *argv, true, taken, false);
        int args_left = 0;
        for (size_t i = 0; i < taken.size(); i++) {
            if (!taken[i]) {
                args_left++;
            }
        }
        // put new argv into a static list to avoid memory leak
        static std::list<std::vector<char *>> argvs;
        static std::mutex mutex;
        char **new_argv;
        {
            std::lock_guard<std::mutex> l(mutex);
            argvs.emplace_back(args_left, nullptr);
            new_argv = &argvs.back()[0];
        }
        *argc = args_left;
        int n = 0;
        for (size_t i = 0; i < taken.size(); i++) {
            if (!taken[i]) {
                new_argv[n++] = (*argv)[i];
            }
        }
        *argv = new_argv;
    }

    template <class T = std::string>
    T GetValue(std::string name) {
        auto it = named_options_.find(name);
        if (it == named_options_.end()) {
            throw ConfigurationError(std::string("GetValue not found. name = ") + name);
        }
        Option<T> *p = static_cast<Option<T> *>(it->second.get());
        return p->Value();
    }

    template <class T = std::string>
    T GetValue(size_t position) {
        if (position >= positional_options_.size()) {
            throw ConfigurationError(std::string("GetValue not found. position = ") +
                                     std::to_string(position));
        }
        Option<T> *p = static_cast<Option<T> *>(positional_options_[position].get());
        return p->Value();
    }

    /*!
     * \fn  std::string ArgParser::HelpString()
     *
     * \brief   Get help string of the parser.
     *
     * \return  The whole help string, usually containing multiple lines.
     */
    std::string HelpString() {
        std::ostringstream os;
        os << "Available command line options:" << std::endl;
        for (auto it : this->options_) {
            if (it->Keys().size() > 0) {
                os << it->HelpString(4, 20, 56) << std::endl;
            }
        }

        return os.str();
    }

    void ExitAfterHelp(bool exit = true) { exit_after_help_ = exit; }

    void ParseJson(const char *str) {
        nlohmann::json j = nlohmann::json::parse(str);
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::string key = it.key();
            std::string value;
            if (it.value().is_string()) {
                value = it.value().operator std::string();  // work around vs 2019 16.5.4 bug
            } else {
                value = it->dump();
            }
            auto iter = named_options_.find(key);
            if (iter != named_options_.end()) {
                iter->second->SetValueString(value);
                continue;
            }
            auto iter2 = children_.find(key);
            if (iter2 != children_.end()) {
                iter2->second->ParseJson(value.c_str());
                continue;
            }
            LOG_WARN() << "ParseJson skipping " << key << " : " << value;
        }
    }

    void ParseJsonFile(const std::string &path) {
        std::string json;
        {
            fma_common::InputFmaStream f(path);
            if (!f.Good()) {
                throw std::runtime_error("Failed to read json config file [" + path + "]");
            }
            json.resize(f.Size());
            f.Read(&json[0], json.size());
        }
        ParseJson(json.c_str());
    }
};

using ArgParser = Configuration;
}  // namespace fma_common
