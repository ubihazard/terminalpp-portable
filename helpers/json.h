#pragma once

#include <string>
#include <iomanip>
#include <vector>
#include <unordered_map>

#include "helpers.h"
#include "string.h"

namespace helpers {

    class JSONError : public Exception {
    public:
        JSONError() = default;

        JSONError(unsigned line, unsigned col) {
            what_ = STR("Parser error at [" << line << "," << col << "]:");
        }
    }; // helpers::JSONError

    /* A simple class for storing, serializing and deserializing JSON values. 
     */
    class JSON {
    public:

        /** Parses the given input stream into a JSON object and returns it. 

            The following grammar is supported:

            JSON := [ COMMENT ] ELEMENT
            ELEMENT := null | true | false | int | double | STR | ARRAY | OBJECT
            STR := double quoted string
            COMMENT := C/Javascript single or multi-line style comment
            ARRAY := '[' [ JSON { ',' JSON } ] ']'
            OBJECT := '{' [ [ COMMENT ] STR ':' ELEMENT { ',' [COMMENT] STR ':' ELEMENT } ] }
         */
        static JSON Parse(std::string const & from) {
            std::stringstream ss(from);
            return Parse(ss);
        }

        static JSON Parse(std::istream & s);

        enum class Kind {
            Null,
            Boolean,
            Integer,
            Double,
            String,
            Array,
            Object,
        };

        explicit JSON(Kind kind = Kind::Null):
            kind_(kind),
            valueInt_(0) {
            switch (kind) {
                case Kind::Null:
                case Kind::Integer:
                    break; // already dealt with
                case Kind::Boolean:
                    valueBool_ = false;
                    break;
                case Kind::Double:
                    valueDouble_ = 0;
                    break;
                case Kind::String:
                    new (&valueStr_) std::string();
                    break;
                case Kind::Array:
                    new (&valueArray_) std::vector<JSON *>();
                    break;
                case Kind::Object:
                    new (&valueObject_) std::unordered_map<std::string, JSON *>();
                    break;
            }
        }

        explicit JSON(bool value):
            kind_(Kind::Boolean),
            valueBool_(value ? 1 : 0) {
        }

        explicit JSON(int value):
            kind_(Kind::Integer),
            valueInt_(value) {
        }

        explicit JSON(double value):
            kind_(Kind::Double),
            valueDouble_(value) {
        }

        explicit JSON(char const * value):
            kind_(Kind::String),
            valueStr_(value) {
        }

        explicit JSON(std::string const & value):
            kind_(Kind::String),
            valueStr_(value) {
        }

        JSON(JSON const & other):
            kind_(other.kind_),
            comment_(other.comment_),
            valueInt_(other.valueInt_) {
            switch (kind_) {
                case Kind::Null:
                case Kind::Integer:
                    break; // already dealt with
                case Kind::Boolean:
                    valueBool_ = other.valueBool_;
                    break;
                case Kind::Double:
                    valueDouble_ = other.valueDouble_;
                    break;
                case Kind::String:
                    new (&valueStr_) std::string(other.valueStr_);
                    break;
                case Kind::Array:
                    new (&valueArray_) std::vector<JSON *>(other.valueArray_);
                    break;
                case Kind::Object:
                    new (&valueObject_) std::unordered_map<std::string, JSON *>(other.valueObject_);
                    break;
            }
        }

        JSON(JSON && other):
            kind_(other.kind_),
            comment_(std::move(other.comment_)),
            valueInt_(other.valueInt_) {
            switch (kind_) {
                case Kind::Null:
                case Kind::Integer:
                    break; // already dealt with
                case Kind::Boolean:
                    valueBool_ = other.valueBool_;
                    break;
                case Kind::Double:
                    valueDouble_ = other.valueDouble_;
                    break;
                case Kind::String:
                    new (&valueStr_) std::string(std::move(other.valueStr_));
                    break;
                case Kind::Array:
                    new (&valueArray_) std::vector<JSON *>(std::move(other.valueArray_));
                    break;
                case Kind::Object:
                    new (&valueObject_) std::unordered_map<std::string, JSON *>(std::move(other.valueObject_));
                    break;
            }
        }

        ~JSON() {
            destroy();
        }

        JSON & operator = (JSON const & other) {
            if (this == &other)
                return *this;
            if (kind_ != other.kind_) {
                destroy();
                kind_ = other.kind_;
            }
            comment_ = other.comment_;
            switch (kind_) {
                case Kind::Null:
                case Kind::Integer:
                    valueInt_ = other.valueInt_;
                    break; 
                case Kind::Boolean:
                    valueBool_ = other.valueBool_;
                    break;
                case Kind::Double:
                    valueDouble_ = other.valueDouble_;
                    break;
                case Kind::String:
                    new (&valueStr_) std::string(other.valueStr_);
                    break;
                case Kind::Array:
                    new (&valueArray_) std::vector<JSON *>(other.valueArray_);
                    break;
                case Kind::Object:
                    new (&valueObject_) std::unordered_map<std::string, JSON *>(other.valueObject_);
                    break;
            }
            return *this;
        }

        JSON & operator = (JSON && other) {
            if (this == &other)
                return *this;
            if (kind_ != other.kind_) {
                destroy();
                kind_ = other.kind_;
            }
            comment_ = std::move(other.comment_);
            switch (kind_) {
                case Kind::Null:
                case Kind::Integer:
                    valueInt_ = other.valueInt_;
                    break; 
                case Kind::Boolean:
                    valueBool_ = other.valueBool_;
                    break;
                case Kind::Double:
                    valueDouble_ = other.valueDouble_;
                    break;
                case Kind::String:
                    new (&valueStr_) std::string(std::move(other.valueStr_));
                    break;
                case Kind::Array:
                    new (&valueArray_) std::vector<JSON *>(std::move(other.valueArray_));
                    break;
                case Kind::Object:
                    new (&valueObject_) std::unordered_map<std::string, JSON *>(std::move(other.valueObject_));
                    break;
            }
            return *this;
        }

        JSON & operator = (nullptr_t) {
            if (kind_ != Kind::Null)
                destroy();
            kind_ = Kind::Null;
            valueInt_ = 0;;
            return *this;
        }


        JSON & operator = (bool value) {
            if (kind_ != Kind::Boolean)
                destroy();
            kind_ = Kind::Boolean;
            valueBool_ = value;
            return *this;
        }

        JSON & operator = (int value) {
            if (kind_ != Kind::Integer)
                destroy();
            kind_ = Kind::Integer;
            valueInt_ = value;
            return *this;
        }

        JSON & operator = (double value) {
            if (kind_ != Kind::Double)
                destroy();
            kind_ = Kind::Double;
            valueDouble_ = value;
            return *this;
        }

        JSON & operator = (std::string const & value) {
            if (kind_ != Kind::String)
                destroy();
            kind_ = Kind::String;
            new (&valueStr_) std::string(value);
            return *this;
        }

        JSON & operator = (char const * value) {
            if (kind_ != Kind::String)
                destroy();
            kind_ = Kind::String;
            new (&valueStr_) std::string(value);
            return *this;
        }

        bool isNull() const {
            return kind_ == Kind::Null;
        }

        std::string const & comment() const {
            return comment_;
        }

        void setComment(std::string const & value) {
            comment_ = value;
        }

        template<typename T> 
        T & value();

        JSON & operator [] (size_t index) {
            if (kind_ != Kind::Array)
                THROW(JSONError()) << "Cannot index JSON element holding " << kind_;
            while (index >= valueArray_.size())
                valueArray_.push_back(new JSON());
            return *valueArray_[index];
        }

        JSON & operator [] (std::string const & index) {
            if (kind_ != Kind::Object)
                THROW(JSONError()) << "Cannot obtain property of JSON element holding " << kind_;
            auto i = valueObject_.find(index);
            if (i == valueObject_.end())
                i = valueObject_.insert(std::make_pair(index, new JSON())).first;
            return *(i->second);
        }

        void add(JSON const & what) {
            if (kind_ != Kind::Array)
                THROW(JSONError()) << "Cannot add array elemnt to element holding " << kind_;
            valueArray_.push_back(new JSON(what));
        }

        void add(std::string const & key, JSON const & value) {
            if (kind_ != Kind::Object)
                THROW(JSONError()) << "Cannot add member elemnt to element holding " << kind_;
            valueObject_.insert(std::make_pair(key, new JSON(value)));
        }

        void emit(std::ostream & s, unsigned tabWidth = 4) const {
            emitComment(s, tabWidth, 0);
            emit(s, tabWidth, 0);
        }

        friend std::ostream & operator << (std::ostream & s, Kind kind) {
            switch (kind) {
                case Kind::Null:
                    s << "null";
                    break;
                case Kind::Boolean:
                    s << "boolean";
                    break;
                case Kind::Integer:
                    s << "integer";
                    break;
                case Kind::Double:
                    s << "double";
                    break;
                case Kind::String:
                    s << "string";
                    break;
                case Kind::Array:
                    s << "array";
                    break;
                case Kind::Object:
                    s << "object";
                    break;
            }
            return s;
        }

        friend std::ostream & operator << (std::ostream & s, JSON const & json) {
            json.emit(s);
            return s;
        }


    private:

        class Parser;


        void destroy() {
            switch (kind_) {
                case Kind::Null:
                case Kind::Boolean:
                case Kind::Integer:
                case Kind::Double:
                    break; // no need to destroy pods
                case Kind::String:
                    valueStr_.~basic_string();
                    break;
                case Kind::Array:
                    for (auto i : valueArray_) 
                        delete i;
                    valueArray_.~vector();
                    break;
                case Kind::Object:
                    for (auto i : valueObject_) 
                        delete i.second;
                    valueObject_.~unordered_map();
                    break;
            }
        }

        void emitComment(std::ostream & s, unsigned tabWidth, unsigned offset) const {
            if (comment_.empty())
                return;
            std::vector<std::string> lines(Split(comment_, "\n"));
            auto i = lines.begin();
            s << std::setw(offset) << "" << "/* " << *i << std::endl;
            ++i;
            offset += tabWidth;
            while (i != lines.end()) {
                s << std::setw(offset) << "" << (*i) << std::endl;
                ++i;
            }
            offset -= tabWidth;
            s << std::setw(offset) << "" << " */" << std::endl;
        }

        void emit(std::ostream & s, unsigned tabWidth, unsigned offset) const {
            switch (kind_) {
                case Kind::Null:
                    s << "null";
                    break;
                case Kind::Boolean:
                    s << (valueBool_ ? "true" : "false");
                    break;
                case Kind::Integer:
                    s << valueInt_;
                    break;
                case Kind::Double:
                    s << valueDouble_;
                    break;
                case Kind::String:
                    s << Quote(valueStr_);
                    break;
                case Kind::Array:
                    s << "[" << std::endl;
                    offset += tabWidth;
                    for (auto i = valueArray_.begin(), e = valueArray_.end(); i != e;) {
                        (*i)->emitComment(s,tabWidth, offset);
                        s << std::setw(offset) << "";
                        (*i)->emit(s, tabWidth, offset);
                        ++i;
                        if (i != e)
                            s << ",";
                        s << std::endl;
                    }
                    offset -= tabWidth;
                    s << std::setw(offset) << "" << "]";
                    break;
                case Kind::Object:
                    s << "{" << std::endl;
                    offset += tabWidth;
                    for (auto i = valueObject_.begin(), e = valueObject_.end(); i != e;) {
                        i->second->emitComment(s,tabWidth, offset);
                        s << std::setw(offset) << "" << Quote(i->first) << " : ";
                        i->second->emit(s, tabWidth, offset);
                        ++i;
                        if (i != e)
                            s << ",";
                        s << std::endl;
                    }
                    offset -= tabWidth;
                    s << std::setw(offset) << "" << "}";
                    break;
            }
        }

        Kind kind_;

        std::string comment_;

        union {
            int valueInt_;
            bool valueBool_;
            double valueDouble_;
            std::string valueStr_;
            std::vector<JSON *> valueArray_;
            std::unordered_map<std::string, JSON *> valueObject_;
        };


    }; // helpers::JSON

    template<> 
    inline bool & JSON::value() {
        if (kind_ != Kind::Boolean)
            THROW(JSONError()) << "Cannot obtain boolean value from element holding " << kind_;
        return valueBool_;
    }

    template<> 
    inline int & JSON::value() {
        if (kind_ != Kind::Integer)
            THROW(JSONError()) << "Cannot obtain integer value from element holding " << kind_;
        return valueInt_;
    }

    template<> 
    inline double & JSON::value() {
        if (kind_ != Kind::Double)
            THROW(JSONError()) << "Cannot obtain double value from element holding " << kind_;
        return valueDouble_;
    }

    template<> 
    inline std::string & JSON::value() {
        if (kind_ != Kind::String)
            THROW(JSONError()) << "Cannot obtain string value from element holding " << kind_;
        return valueStr_;
    }


    class JSON::Parser {
    public:

        Parser(std::istream & s):
            line_(1),
            col_(1),
            input_(s) {
        }

        /** JSON := [ COMMENT ] ELEMENT
         */
        JSON parseJSON() {
            skipWhitespace();
            if (top() == '/') {
                std::string comment = parseComment();
                JSON result = parseElement();
                result.setComment(comment);
                return result;
            } else {
                return parseElement();
            }
        }

        /** ELEMENT := null | true | false | int | double | STR | ARRAY | OBJECT
         */
        JSON parseElement() {
            int sign = 1;
            switch (top()) {
                case 'n':
                    pop("null");
                    return JSON(Kind::Null);
                    break;
                case 't':
                    pop("true");
                    return JSON(true);
                case 'f':
                    pop("false");
                    return JSON(false);
                case '\"':
                    return JSON(parseStr());
                case '[':
                    return parseArray();
                case '{':
                    return parseObject();
                case '-':
                    sign = -1;
                    pop();
                    // fallthrough
                default:
                    // it's number
                    if (! IsDecimalDigit(top()))
                        THROW(JSONError(line_, col_)) << "Expected number, bool, null, string, array, or object but " << top() << " found";
                    int value = 0;
                    do {
                        value = value * 10 + DecCharToNumber(pop());
                    } while (IsDecimalDigit(top()));
                    if (condPop('.')) {
                        double result = value;
                        double n = 10.0;
                        while (IsDecimalDigit(top())) {
                            result = result + DecCharToNumber(pop()) / n;
                            n *= 10;
                        }
                        return JSON(result * sign);
                    } else {
                        return JSON(value * sign);
                    }
                    break;
            }
        }

        /** STR := double quoted string
         */
        std::string parseStr() {
            unsigned l = line_;
            unsigned c = col_;
            std::stringstream result;
            pop('\"');
            while (true) {
                if (input_.eof())
                    THROW(JSONError(l, c)) << "Unterminated string";
                char t = top();
                if ( t == '"') {
                    pop();
                    break;
                }
                if (t == '\\') {
                    pop();
                    switch (top()) {
                        case '\\':
                        case '\'':
                        case '"':
                            result << pop();
                            continue;
                        case '\n':
                            pop();
                            continue;
                        case 'n':
                            result << '\n';
                            pop();
                            continue;
                        case '\t':
                            result << '\t';
                            pop();
                            continue;
                        default:
                            THROW(JSONError(line_, col_)) << "Invalid escape sequence " << top();
                    }
                } else {
                    result << pop();
                }
            }
            return result.str();
        }

        /** COMMENT := C/Javascript single or multi-line style comment
         */
        std::string parseComment() {
            pop('/');
            std::stringstream result;
            switch (top()) {
                case '/': {
                    pop();
                    while (! input_.eof() && ! condPop('\n'))
                        result << pop();
                    break;
                }
                case '*': {
                    pop();
                    while (! input_.eof()) {
                        if (top() == '*') {
                            pop();
                            if (condPop('/'))
                                break;
                            result << '*';
                        } else {
                            result << pop();
                        }
                    }
                    break;
                }
                default:
                    THROW(JSONError(line_, col_)) << "Invalid comment detected";
            }
            return Trim(result.str());
        }

        /** ARRAY := '[' [ JSON { ',' JSON } ] ']'
         */
        JSON parseArray() {
            pop('[');
            JSON result(Kind::Array);
            skipWhitespace();
            if (top() != ']') {
                result.add(JSON());
                skipWhitespace();
                while (condPop(',')) {
                    skipWhitespace();
                    result.add(JSON());
                    skipWhitespace();
                }
            }
            pop(']');
            return result;
        }

        /** OBJECT := '{' [ [ COMMENT ] STR ':' ELEMENT { ',' [COMMENT] STR ':' ELEMENT } ] }
         */
        JSON parseObject() {
            pop('{');
            JSON result(Kind::Object);
            skipWhitespace();
            if (top() != '}') {
                std::string comment;
                if (top() == '/') {
                    comment = parseComment();
                    skipWhitespace();
                }
                std::string key = parseStr();
                skipWhitespace();
                pop(':');
                skipWhitespace();
                JSON value = parseElement();
                value.setComment(comment);
                result.add(key, value);
                while (condPop(',')) {
                    skipWhitespace();
                    if (top() == '/') {
                        comment = parseComment();
                        skipWhitespace();
                    }
                    skipWhitespace();
                    key = parseStr();
                    skipWhitespace();
                    pop(':');
                    skipWhitespace();
                    value = parseElement();
                    value.setComment(comment);
                    result.add(key, value);
                    skipWhitespace();
                }
            }
            pop('}');
            return result;
        }

    private:

        friend class JSON;

        char top() const {
            if (input_.eof())
                return 0;
            return static_cast<char>(input_.peek());
        }

        char pop() {
            char x = top();
            if (input_.get() == '\n') {
                ++line_;
                col_ = 1;
            } else if (x >= 0) { // beginning of a character in UTF8
                ++col_;
            } 
            return x;
        }

        char pop(char what) {
            char x = top();
            if (x != what)
                THROW(JSONError(line_, col_)) << "Expected " << what << ", but " << x << " found";
            return pop();
        }

        void pop(char const * what) {
            char const * w = what;
            unsigned l = line_;
            unsigned c = col_;
            while (*w != 0) {
                if (input_.eof())
                    THROW(JSONError(l, c)) << "Expected " << what << ", but EOF found";
                if (input_.get() != *w)
                    THROW(JSONError(l, c)) << "Expected " << what;
                ++w;
            }
        }

        bool condPop(char what) {
            if (top() == what) {
                pop();
                return true;
            } else {
                return false;
            }
        }

        void skipWhitespace() {
            while(IsWhitespace(top()))
                pop();
        }


        unsigned line_;
        unsigned col_;
        std::istream & input_;



    }; // JSON::Parser

    inline JSON JSON::Parse(std::istream & s) {
        Parser p(s);
        JSON result = p.parseJSON();
        p.skipWhitespace();
        if (! s.eof())
            THROW(JSONError(p.line_, p.col_)) << "Unparsed contents";
        return result;
    }


} // namespace helpers
