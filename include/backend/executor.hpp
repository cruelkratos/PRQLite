#include<include/frontend/parser/AST.hpp>
#include<memory>

namespace db::executor{
	class AbstractExecutor{
		private:
		std::unique_ptr<AbstractExecutor> _child;
		public:
		virtual void init() = 0;
		virtual void next() = 0; // won't be void just putting for now
		virtual ~AbstractExecutor();
	};
}