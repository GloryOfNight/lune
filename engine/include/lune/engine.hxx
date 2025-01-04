#pragma once

#include <string>
#include <vector>

namespace lune
{
	class engine final
	{
	public:
		engine() = default;
		engine(const engine&) = delete;
		engine(engine&&) = delete;
		~engine() = default;

		static engine* get();

		bool initialize(std::vector<std::string> args);

        void shutdown();

		void run();

	private:
		std::vector<std::string> mArgs{};
	};
} // namespace lune
