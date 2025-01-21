#include <optional>
#include "json.h"

namespace json {


    //----------------- Error ----------------

    class BuildError : public std::logic_error {
    public:
        using logic_error::logic_error;
    };

    //---------------- Builder ---------------

    class Builder {
    private:

        // Item contexts

        class ArrayItemContext;
        class DictItemContext;
        class DictKeyContext;

        class BaseContext {
        public:
            BaseContext(Builder& builder);
            ArrayItemContext StartArray();
            DictItemContext StartDict();
            DictKeyContext Key(std::string&& key);
            Builder& EndDict();
            Builder& EndArray();
        protected:
            Builder& builder_;
        };

        class DictItemContext : public BaseContext {
        public:
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
            Builder& EndArray() = delete;
        };

        class DictKeyContext : public BaseContext {
        public:
            DictKeyContext Key(std::string&& key) = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
        public:
            DictItemContext Value(Node&& value);
        };

        class ArrayItemContext : public BaseContext {
        public:
            DictKeyContext Key(std::string&& key) = delete;
            Builder& EndDict() = delete;
        public:
            ArrayItemContext Value(Node&& value);
        };

    public:

        Builder& Value(Node&& value);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

        DictKeyContext Key(std::string&& key);
        const Node& Build() const;

    private:

        bool RootAssigned_() const;
        bool LastIsArray_() const;
        bool LastIsDict_() const;
        const Node::Value& GetValue_() const;
        Node::Value& GetValue_();

    private:

        std::optional<std::string> key_;
        std::optional<Node> root_;
        std::vector<Node*> nodes_stack_;

    };


} // namespace json