#include "json_builder.h"

using namespace std::literals;

namespace json {
	using namespace std;

	Builder::KeyItemContext Builder::Key(const std::string& str) {
		if (complite_container_) {
			throw std::logic_error("Container already build");
		}
		if (node_stack_.empty()) {
			throw std::logic_error("Key must use only in Dict container");
		}
		else if (!node_stack_.back()->IsDict()) {
			throw std::logic_error("Key must use only in Dict container");
		}

		keys_.emplace_back(str);
		KeyItemContext key_context{ *this };
		return key_context;
	}

	Builder& Builder::Value(Node::Value val){
		if (complite_container_) {
			throw std::logic_error("Container already build");
		}

		Node node;
		*node = val;

		if (!node_stack_.empty()) {
			if (node_stack_.back()->IsArray()) {
				Array& ar = const_cast<Array&>(node_stack_.back()->AsArray());
				ar.emplace_back(std::move(node));
			}
			else if (node_stack_.back()->IsDict()) {
				Dict& dict = const_cast<Dict&>(node_stack_.back()->AsDict());
				dict[keys_.back()] = std::move(node);
				keys_.pop_back();
			}
		} else {
			root_ = std::move(node);
			complite_container_ = true;
		}

		return *this;
	}


	Builder::ArrayItemContext Builder::StartArray() {
		return StartContainer<ArrayItemContext, Array>();
	}

	Builder::DictItemContext Builder::StartDict() {
		return StartContainer<DictItemContext, Dict>();
	}

	Builder& Builder::EndArray() {
		return EndContainer<Array>();
	}

	Builder& Builder::EndDict() {
		return EndContainer<Dict>();
	}

	Node Builder::Build() {
		if (!complite_container_) {
			throw std::logic_error("Container not ready");
		}
		return root_;
	}

	Builder::KeyItemContext Builder::BaseContext::Key(const std::string& str) {
		return bld_.Key(str);
	}

	Builder::ArrayItemContext Builder::BaseContext::StartArray() {
		return bld_.StartArray();
	}

	Builder::DictItemContext Builder::BaseContext::StartDict() {
		return bld_.StartDict();
	}

	Builder& Builder::BaseContext::EndArray() {
		bld_.EndArray();
		return bld_;
	}

	Builder& Builder::BaseContext::EndDict() {
		bld_.EndDict();
		return bld_;
	}

	Builder::KeyItemContext Builder::ValueKeyContext::Key(const std::string& str) {
		return bld_.Key(str);
	}

	Builder& Builder::ValueKeyContext::EndDict() {
		return bld_.EndDict();
	}

	Builder::ValueKeyContext Builder::KeyItemContext::Value(Node::Value val) {
		bld_.Value(val);
		ValueKeyContext value_context{ bld_ };
		return value_context;
	}

	Builder::ArrayItemContext& Builder::ArrayItemContext::Value(Node::Value val) {
		bld_.Value(val);
		return *this;
	}

}

