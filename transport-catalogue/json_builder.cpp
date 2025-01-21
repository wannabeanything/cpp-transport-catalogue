#include "json_builder.h"

namespace json {

    Builder& Builder::Value(Node&& value) {
        Node* last_ptr;
        if (!RootAssigned_()) {
            root_ = std::move(value);
            last_ptr = &(*root_);
        }
        else {
            if (LastIsArray_()) {
                last_ptr = &(std::get<Array>(GetValue_()).emplace_back(std::move(value)));
            }
            else if (LastIsDict_()) {
                /*
                if (!key_.has_value()) {
                    using namespace std::string_literals;
                    throw BuildError("Can't add Value. Key is not defined"s);
                }
                */
                last_ptr = &((*(std::get<Dict>(GetValue_()).emplace(
                    *key_,
                    std::move(value)
                ).first)).second);
                key_ = std::nullopt;
            }
            else {
                using namespace std::string_literals;
                throw BuildError("First node is not an Array or Dict. Can't add more nodes"s);
            }
        }

        if (last_ptr->IsArray() || last_ptr->IsMap()) {
            nodes_stack_.push_back(last_ptr);
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        Value(Dict{});
        return { *this };
    }

    Builder& Builder::EndDict() {
        if (!LastIsDict_()) {
            using namespace std::string_literals;
            throw BuildError("Can't close Dict. Last node is not Dict"s);
        }
        key_ = std::nullopt;
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        Value(Array{});
        return { *this };
    }

    Builder& Builder::EndArray() {
        if (!LastIsArray_()) {
            using namespace std::string_literals;
            throw BuildError("Can't close Array. Last node is not Array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::DictKeyContext Builder::Key(std::string&& key) {
        key_ = std::move(key);
        return { *this };
    }

    const Node& Builder::Build() const {
        if (!RootAssigned_()) {
            using namespace std::string_literals;
            throw BuildError("root_ is not defined"s);
        }
        return *(root_);
    }

    bool Builder::RootAssigned_() const {
        return root_.has_value();
    }

    bool Builder::LastIsArray_() const {
        return !nodes_stack_.empty() && nodes_stack_.back()->IsArray();
    }

    bool Builder::LastIsDict_() const {
        return !nodes_stack_.empty() && nodes_stack_.back()->IsMap();
    }

    const Node::Value& Builder::GetValue_() const {
        return const_cast<Builder*>(this)->GetValue_();
    }

    Node::Value& Builder::GetValue_() {
        return nodes_stack_.back()->GetValue();
    }

    //---------------- Item contexts ---------------    


    Builder::BaseContext::BaseContext(Builder& builder) :
        builder_{ builder }
    {

    }

    Builder::ArrayItemContext Builder::BaseContext::StartArray(){
        return builder_.StartArray();
    }

    Builder::DictItemContext Builder::BaseContext::StartDict(){
        return builder_.StartDict();
    }

    Builder::DictKeyContext Builder::BaseContext::Key(std::string&& key){
        return builder_.Key(std::move(key));
    }

    Builder& Builder::BaseContext::EndDict(){
        return builder_.EndDict();
    }

    Builder::DictItemContext Builder::DictKeyContext::Value(Node&& value){
        return { builder_.Value(std::move(value)) };
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node&& value){
        return { builder_.Value(std::move(value)) };
    }

    Builder& Builder::BaseContext::EndArray(){
        return builder_.EndArray();
    }

} // namespace json