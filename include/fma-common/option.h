//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\option.h.
 *
 * \brief   Declares the option class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <atomic>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "fma-common/type_traits.h"
#include "fma-common/utils.h"

namespace fma_common {
class OptionCheckError : public std::exception {
    std::string str;

 public:
    explicit OptionCheckError(std::string str) : str(str) {}

    virtual const char *what() const throw() { return str.c_str(); }
};

/*!
 * \class   OptionBase
 *
 * \brief   Base class for different option types
 */
class OptionBase {
 protected:
    /*! \brief   options keys, such as {"A", "Ada"} */
    std::vector<std::string> keys_;
    /*! \brief   The value string */
    std::string value_string_;
    /*! \brief   True if value_string_ has been set */
    bool has_value_string_ = false;
    /*! \brief   Whether to preserve this option  */
    bool preserve_ = false;
    /*! \brief   True if this option has default value */
    bool has_default_ = false;

 public:
    /*!
     * \brief   Constructor.
     *
     * \param   short_opt   The short option.
     */
    explicit OptionBase(std::vector<std::string> keys) : keys_(keys) {}

    /*!
     * \fn  virtual OptionBase::~OptionBase()
     *
     * \brief   Destructor.
     */
    virtual ~OptionBase() {}

    std::vector<std::string> &Keys() { return keys_; }
    /*!
     * \fn  virtual void OptionBase::SetPreserve(bool preserve)
     *
     * \brief   Sets whether this options is preserve after parsing.
     *
     * \param   preserve    True to preserve.
     */

    virtual void SetPreserve(bool preserve) { preserve_ = preserve; }

    bool Preserve() const { return preserve_; }

    std::string StrKeys() const {
        std::string str = "(";
        bool first = true;
        for (auto key : keys_) {
            if (first)
                first = false;
            else
                str += " ,";
            str += key;
        }
        str += ")";
        return str;
    }

 public:
    /*!
     * \fn  virtual std::string& OptionBase::Value()
     *
     * \brief   Gets the string value of this option.
     *
     * \return  A reference to the value_ string.
     */
    virtual std::string ValueString() const {
        if (!has_value_string_)
            throw OptionCheckError(
                std::string("ValueString() error : value string not set, keys = ") + StrKeys());
        return value_string_;
    }

    virtual void SetValueString(std::string value) {
        value_string_ = value;
        has_value_string_ = true;
    }

    /*!
     * \fn  virtual std::string OptionBase::HelpString(size_t space_before, size_t options_width,
     * size_t comment_width)
     *
     * \brief   Generate help string for this option
     *
     * \param   space_before    Number of whitespace to leave for each line.
     * \param   options_width   Width of the options.
     * \param   comment_width   Width of the comments.
     *
     * \return  The generated help string
     */
    virtual std::string HelpString(size_t space_before, size_t options_width,
                                   size_t comment_width) {
        std::string short_option;
        std::string long_option;
        for (std::string key : Keys()) {
            if (key.size() == 1) {
                short_option = std::string("-") + key;
            } else {
                long_option = std::string("--") + key;
            }
        }
        std::string ret;
        ret.reserve(80);
        ret.append(space_before, ' ');
        if (short_option.size()) {
            ret += short_option;
        }
        if (long_option.size()) {
            if (short_option.size()) ret += ", ";
            ret += long_option;
        }
        if (ret.size() <= space_before + options_width) {
            ret.append(space_before + options_width - ret.size(), ' ');
        } else {
            ret.push_back('\n');
            ret.append(space_before + options_width, ' ');
        }

        std::string comment = GetComment();
        size_t comment_start = 0;
        const char *ptr = comment.data();
        while (comment_start < comment.size()) {
            // find a break point before line end
            size_t bp;
            if (comment_start + comment_width < comment.size()) {
                bp = comment.find_last_of(' ', comment_start + comment_width) + 1;
                if (bp == comment.npos || bp <= comment_start) {
                    bp = std::min<size_t>(comment_start + comment_width, comment.size());
                }
            } else {
                bp = comment.size();
            }
            ret.append(ptr + comment_start, ptr + bp);
            comment_start = bp;
            if (comment_start < comment.size()) {
                ret += "\n";
                ret.append(space_before + options_width, ' ');
            }
        }
        return ret;
    }

    /*!
     * \fn  virtual bool OptionBase::HasDefault() const
     *
     * \brief   Query if this option has default value.
     *
     * \return  True if has default, false if not.
     */
    virtual bool HasDefault() const { return has_default_; }

    /*!
     * \fn  virtual std::string OptionBase::Check() = 0;
     *
     * \brief   Check restrictions, such as defaults, min, max...
     *
     * \throw  Throw OptionCheckError when error occurs
     */
    virtual void Check() = 0;

    /*!
     * \fn  virtual std::string OptionBase::GetComment() = 0;
     *
     * \brief   Gets the option comment.
     *
     * \return  The comment string.
     */
    virtual std::string GetComment() = 0;
};

template <typename T>
struct GetBasicType {
    typedef T type;
};

template <typename T>
struct GetBasicType<std::atomic<T>> {
    typedef T type;
};

/*!
 * \class   Option template
 *
 * \brief   Template for all kinds of options
 *
 * \param T type of the option value
 */
template <typename T>
class Option : public OptionBase {
 protected:
    typedef typename GetBasicType<T>::type BT;

    /*! \brief   True if this object has minimum */
    bool has_min_ = false;
    /*! \brief   The minimum value */
    BT min_;
    /*! \brief   True if this object has maximum */
    bool has_max_ = false;
    /*! \brief   The maximum value */
    BT max_;
    /*! \brief   The default value */
    BT default_;
    /*! \brief   The comment string */
    std::string comment_;
    /*! \brief   The possible values */
    std::set<BT> possible_values_;

    bool checked_ = false;

    BT value_;

 public:
    /*!
     * \fn  virtual std::string OptionBase::Check() = 0;
     *
     * \brief   Check restrictions, such as defaults, min, max...
     *
     * \return  Error string, empty if success.
     */
    void Check() override {
        if (checked_) return;
        checked_ = true;  // checked_ is set here because Value() need checked
        if (has_default_) value_ = default_;
        if (has_value_string_) {
            if (!ParseString<BT>(value_string_, value_)) {
                throw OptionCheckError(std::string("Failed to parse value into ") +
                                       typeid(BT).name() + ", value is " + value_string_);
            }
        }
        if (!has_default_ && !has_value_string_) {
            throw OptionCheckError("Option required, but not passed");
        }
        if (has_min_ && Value() < min_) {
            throw OptionCheckError(std::string("Value less than min: min=") + ToString(min_) +
                                   ", value=" + ToString(Value()));
        }
        if (has_max_ && Value() > max_) {
            throw OptionCheckError(std::string("Value larger than max: max=") + ToString(max_) +
                                   ", max=" + ToString(Value()));
        }
        // now check for possible values
        if (!possible_values_.empty()) {
            if (possible_values_.find(Value()) == possible_values_.end()) {
                throw OptionCheckError(std::string("Value not in possbile values: value=") +
                                       ValueString() + ", possible values are " +
                                       ToString(possible_values_));
            }
        }
        SetValueRef(value_);
    }

    /*!
     * \fn  virtual std::string OptionBase::GetComment() = 0;
     *
     * \brief   Gets the option comment.
     *
     * \return  The comment string.
     */
    std::string GetComment() override {
        std::string ret = comment_;
        if (!ret.empty() && ret.back() != '.') ret.push_back('.');
        if (has_default_) {
            if (!ret.empty()) ret.push_back(' ');
            std::string default_str = ToString(default_);
            if (default_str.empty()) default_str = "\"\"";
            ret += std::string("Default=") + default_str + ". ";
        }
        if (!possible_values_.empty()) {
            if (!ret.empty()) ret.push_back(' ');
            ret += "Possible values: " + ToString(possible_values_);
        }
        return ret;
    }

 public:
    using OptionBase::OptionBase;

    /*!
     * \fn  Option<T>& Option::SetDefault(const T& v)
     *
     * \brief   Sets the default value.
     *
     * \param   v   default value
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    Option<T> &SetDefault(const BT &v) {
        has_default_ = true;
        default_ = v;
        return *this;
    }

    /*!
     * \fn  Option<T>& Option::SetMin(const T& v)
     *
     * \brief   Sets the minimum value.
     *
     * \param   v   minimum value.
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    Option<T> &SetMin(const BT &v) {
        has_min_ = true;
        min_ = v;
        return *this;
    }

    /*!
     * \fn  Option<T>& Option::SetMax(const T& v)
     *
     * \brief   Sets the maximum value.
     *
     * \param   v   maximum value to set
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    Option<T> &SetMax(const BT &v) {
        has_max_ = true;
        max_ = v;
        return *this;
    }

    /*!
     * \fn  Option<T>& Option::SetPossibleValues(const T(&arr)[N])
     *
     * \brief   Sets possible values for this option, for example, {client, server}
     *
     * \param   arr Array of possible values.
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    template <int N>
    Option<T> &SetPossibleValues(const BT (&arr)[N]) {
        FMA_CHECK_NEQ(N, 0) << "Possible values of option cannot be empty.";
        possible_values_.insert(arr, arr + N);
        return *this;
    }

    /*!
     * \fn  Option<T>& Option::SetPossibleValues(const std::vector<T>& arr)
     *
     * \brief   Sets possible values.
     *
     * \param   arr Vector of possible values
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    Option<T> &SetPossibleValues(const std::vector<BT> &arr) {
        FMA_CHECK_NEQ(arr.size(), (size_t)0) << "Possible values of option cannot be empty.";
        possible_values_.insert(arr.begin(), arr.end());
        return *this;
    }

    /*!
     * \fn  virtual Option<T>& Option::Comment(const std::string& msg)
     *
     * \brief   Set the comment string for this option.
     *
     * \param   msg The comment string.
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    virtual Option<T> &Comment(const std::string &msg) {
        comment_ = msg;
        return *this;
    }

    virtual void SetValueRef(const BT &v) {}

    virtual const BT &Value() {
        Check();
        return value_;
    }
};

template <class T>
class OptionRef : public Option<T> {
    T &value_ref_;

 public:
    OptionRef(T &ref, std::vector<std::string> keys)
        : Option<T>(std::move(keys)), value_ref_(ref) {}

    void SetValueRef(const typename Option<T>::BT &v) override { value_ref_ = v; }
};

/*!
 * \class   BinaryOption
 *
 * \brief   A binary option, which is false by default, and true if option is specified.
 */
class BinaryOption : public OptionBase {
    /*! \brief   The comment string */
    std::string comment_;

 public:
    using OptionBase::OptionBase;

 protected:
    /*!
     * \fn  virtual std::string BinaryOption::Check() = 0;
     *
     * \brief   Check restrictions, such as defaults, min, max...
     *
     * \return  Error string, empty if success.
     */
    void Check() override {}

    /*!
     * \fn  virtual std::string BinaryOption::GetComment() = 0;
     *
     * \brief   Gets the option comment.
     *
     * \return  The comment string.
     */
    std::string GetComment() override { return comment_; }

 public:
    /*!
     * \fn  virtual BinaryOption& BinaryOption::Comment(const std::string& msg)
     *
     * \brief   Set the comment string for this option.
     *
     * \param   msg The comment string.
     *
     * \return  A reference to an BinaryOption&lt;T&gt;
     */
    virtual BinaryOption &Comment(const std::string &msg) {
        comment_ = msg;
        return *this;
    }
};

/*!
 * \class   PositionalOption
 *
 * \brief   A positional option is an option that does not start with - or --.
 *          It starts from position 0.
 */
template <typename T>
class PositionalOption : public OptionBase {
    /*! \brief   True if this object has minimum value */
    bool has_min_ = false;
    /*! \brief   The minimum value */
    T min_;
    /*! \brief   True if this object has maximum value */
    bool has_max_ = false;
    /*! \brief   The maximum value */
    T max_;
    /*! \brief   The default value */
    T default_;
    /*! \brief   The comment string */
    std::string comment_;
    /*! \brief   The possible values */
    std::set<T> possible_values_;
    /*! \brief   The position, starting from 0 */
    int pos_;

 protected:
    /*!
     * \fn  virtual std::string PositionalOption::Check() = 0;
     *
     * \brief   Check restrictions, such as defaults, min, max...
     *
     * \return  Error string, empty if success.
     */
    void Check() override {
        std::string &value_string = ValueString();
        if (value_string.empty()) {
            if (!has_default_) {
                throw OptionCheckError("option required, but not passed");
            } else {
                value_string = ToString(default_);
            }
        }
        T value;
        if (!ParseString<T>(value_string, value)) {
            throw OptionCheckError(std::string("error parsing value into ") + typeid(value).name() +
                                   ", value is " + value_string);
        }
        if (has_min_ && value < min_) {
            throw OptionCheckError(std::string("value less than min: min=") + ToString(min_) +
                                   ", value=" + ToString(value));
        }
        if (has_max_ && value > max_) {
            throw OptionCheckError(std::string("value larger than max: max=") + ToString(max_) +
                                   ", max=" + ToString(value));
        }
        // now check for possible values
        if (!possible_values_.empty()) {
            if (possible_values_.find(value) == possible_values_.end()) {
                throw OptionCheckError(std::string("value not in possbile values: value=") +
                                       ValueString() + ", possible values are " +
                                       ToString(possible_values_));
            }
        }
    }

    /*!
     * \fn  virtual std::string PositionalOption::Parse(int argc, char** argv, std::vector<bool>&
     * taken)
     *
     * \brief   Parses the command line options.
     *
     * \param           argc    The arg count.
     * \param [in]      argv    If non-null, the arg values.
     * \param [in,out]  taken   taken[i]==true if ith argument is parsed successfully.
     *
     * \return  Error string, empty if success.
     */
    virtual std::string Parse(int argc, char **argv, std::vector<bool> &taken) {
        if (pos_ >= argc) {
            return "";
        } else if (!StartsWith(argv[pos_], "-")) {
            ValueString() = argv[pos_];
            taken[pos_] = true;
        }
        return Check();
    }

    /*!
     * \fn  virtual std::string PositionalOption::GetComment() = 0;
     *
     * \brief   Gets the option comment.
     *
     * \return  The comment string.
     */
    std::string GetComment() override {
        std::string ret = comment_ + ". ";
        ret += "Position=" + ToString(pos_) + ". ";
        if (has_default_) {
            ret += std::string("Default=") + ToString(default_) + ". ";
        }
        if (!possible_values_.empty()) {
            ret = ret + ". Possible values: " + ToString(possible_values_);
        }
        return ret;
    }

 public:
    /*!
     * \fn  PositionalOption::PositionalOption(int pos)
     *
     * \brief   Constructor.
     *
     * \param   pos The position, starting from 0.
     */
    explicit PositionalOption(int pos) : OptionBase({"", ""}), pos_(pos) {}

    /*!
     * \fn  PositionalOption<T>& PositionalOption::SetDefault(const T& v)
     *
     * \brief   Sets the default value.
     *
     * \param   v   default value
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    PositionalOption<T> &SetDefault(const T &v) {
        has_default_ = true;
        default_ = v;
        return *this;
    }

    /*!
     * \fn  PositionalOption<T>& PositionalOption::SetMin(const T& v)
     *
     * \brief   Sets the minimum value.
     *
     * \param   v   minimum value.
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    PositionalOption<T> &SetMin(const T &v) {
        has_min_ = true;
        min_ = v;
        return *this;
    }

    /*!
     * \fn  PositionalOption<T>& PositionalOption::SetMax(const T& v)
     *
     * \brief   Sets the maximum value.
     *
     * \param   v   maximum value to set
     *
     * \return  A reference to an Option&lt;T&gt;
     */
    PositionalOption<T> &SetMax(const T &v) {
        has_max_ = true;
        max_ = v;
        return *this;
    }

    /*!
     * \fn  PositionalOption<T>& PositionalOption::SetPossibleValues(const T(&arr)[N])
     *
     * \brief   Sets possible values for this option, for example, {client, server}
     *
     * \param   arr Array of possible values.
     *
     * \return  A reference to an PositionalOption&lt;T&gt;
     */
    template <int N>
    PositionalOption<T> &SetPossibleValues(const T (&arr)[N]) {
        FMA_CHECK_NEQ(N, (size_t)0) << "Possible values of option cannot be empty.";
        possible_values_.insert(arr, arr + N);
        return *this;
    }

    /*!
     * \fn  PositionalOption<T>& PositionalOption::SetPossibleValues(const std::vector<T>& arr)
     *
     * \brief   Sets possible values.
     *
     * \param   arr Vector of possible values
     *
     * \return  A reference to an PositionalOption&lt;T&gt;
     */
    PositionalOption<T> &SetPossibleValues(const std::vector<T> &arr) {
        FMA_CHECK_NEQ(arr.size(), (size_t)0) << "Possible values of option cannot be empty.";
        possible_values_.insert(arr.begin(), arr.end());
        return *this;
    }

    /*!
     * \fn  virtual PositionalOption<T>& PositionalOption::Comment(const std::string& msg)
     *
     * \brief   Comments the given message.
     *
     * \author  Hct
     * \date    2017/5/12
     *
     * \param   msg The message.
     *
     * \return  A reference to a PositionalOption&lt;T&gt;
     */

    virtual PositionalOption<T> &Comment(const std::string &msg) {
        comment_ = msg;
        return *this;
    }
};

}  // namespace fma_common
