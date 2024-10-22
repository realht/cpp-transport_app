#include "json.h"

namespace json {

	class Builder {
	private:
		class KeyItemContext;
		class ArrayItemContext;
		class DictItemContext;
	public:
	
		KeyItemContext Key(const std::string& str);
		Builder& Value(Node::Value val);

		ArrayItemContext StartArray();
		DictItemContext StartDict();

		Builder& EndArray();
		Builder& EndDict();

		Node Build();

	private:
		Node root_;
		std::vector<std::string> keys_;
		std::vector<Node*> node_stack_;
		bool complite_container_ = false;

		template <typename ItemContext, typename Container>
		ItemContext StartContainer();

		template <typename Container>
		Builder& EndContainer();

		class BaseContext {
		public:
			BaseContext(Builder& bld) : bld_(bld) {}

			KeyItemContext Key(const std::string& str);
			DictItemContext StartDict();
			ArrayItemContext StartArray();
			Builder& EndDict();
			Builder& EndArray();

		private:
			Builder& bld_;
		};

		class ValueKeyContext {
		public:
			ValueKeyContext(Builder& bld) : bld_(bld) {}

			KeyItemContext Key(const std::string& str);
			Builder& EndDict();

		private:
			Builder& bld_;
		};

		class KeyItemContext : public BaseContext {
		public:
			KeyItemContext(Builder& bld)
				: BaseContext(bld), bld_(bld) {}

			ValueKeyContext Value(Node::Value val);

			KeyItemContext& Key(const std::string& str) = delete;
			Builder& EndDict() = delete;
			Builder& EndArray() = delete;

		private:
			Builder& bld_;
		};

		class ArrayItemContext : public BaseContext {
		public:
			ArrayItemContext(Builder& bld)
				: BaseContext(bld), bld_(bld) {}

			ArrayItemContext& Value(Node::Value val);
			KeyItemContext Key(const std::string& str) = delete;
			Builder& EndDict() = delete;

		private:
			Builder& bld_;
		};

		class DictItemContext : public BaseContext {
		public:
			ArrayItemContext& StartArray() = delete;
			DictItemContext StartDict() = delete;
			Builder& EndArray() = delete;
		};

	};


	template <typename ItemContext, typename Container>
	ItemContext Builder::StartContainer() {
		if (complite_container_) {
			throw std::logic_error("Container already build");
		}
		Container cont;
		node_stack_.emplace_back(new Node{ cont });
		ItemContext item_context{ *this };
		return item_context;
	}

	template <typename Container>
	Builder& Builder::EndContainer() {
		if (complite_container_) {
			throw std::logic_error("Container already build");
		}
		if (node_stack_.empty()) {
			throw std::logic_error("Container wrong ending");
		}

		Node* node = node_stack_.back();
		node_stack_.pop_back();

		if (!node_stack_.empty()) {
			if (node_stack_.back()->IsArray()) {
				Array& ar = const_cast<Array&>(node_stack_.back()->AsArray());
				ar.emplace_back(*node);
			}
			else if (node_stack_.back()->IsDict()) {
				Dict& dict = const_cast<Dict&>(node_stack_.back()->AsDict());
				dict[keys_.back()] = *node;
				keys_.pop_back();
			}
		}
		else {
			complite_container_ = true;
		}

		root_ = *node;
		delete node;
		return *this;
	}

}