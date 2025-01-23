#pragma once

#include <functional>
#include <map>
#include <set>

namespace lune
{
	using DelegateHandle = uint32_t;

	template <typename... Args>
	struct DelegateBase
	{
	public:
		using Func = std::function<void(Args...)>;

		struct FuncWrapper
		{
			void* object{};
			Func func{};

			void operator()(Args... args) const
			{
				func(std::forward<Args>(args)...);
			}
		};

		// Bind a non-member function
		DelegateHandle bindRaw(void (*funcPtr)(Args...))
		{
			mBindings.emplace(++mBindCounter, FuncWrapper{nullptr, funcPtr});
			return mBindCounter;
		}

		// Bind a member function of an object
		template <typename T, typename... Placeholders>
		DelegateHandle bindObject(T* object, void (T::*funcPtr)(Args...), Placeholders... placeholders)
		{
			mBindings.emplace(++mBindCounter, FuncWrapper{object, std::bind(funcPtr, object, std::forward<Placeholders>(placeholders)...)});
			return mBindCounter;
		}

		// Unbind a specific delegate handle
		void unbind(DelegateHandle handle)
		{
			if (auto it = mBindings.find(handle); it != mBindings.end())
			{
				mBindings.erase(it);
			}
		}

		// Unbind all from specific object
		void unbind(void* object)
		{
			if (object == nullptr)
				return;

			std::set<DelegateHandle> handles{};
			for (const auto& [handle, funcWrap] : mBindings)
			{
				if (funcWrap.object == object)
					handles.emplace(handle);
			}

			for (auto handle : handles)
				unbind(handle);
		}

		// Unbind all bindings. Avoid using if not owner of delegate!
		void unbindAll()
		{
			mBindings.clear();
		}

	protected:
		std::map<DelegateHandle, FuncWrapper> mBindings{};

	private:
		DelegateHandle mBindCounter{};
	};

	// Delegate bound functions of which could be executed by anyone
	template <typename... Args>
	struct Delegate final : public DelegateBase<Args...>
	{
		// Execute all bound functions
		void execute(Args&&... args) const
		{
			for (const auto& [binding, funcWrap] : this->mBindings)
			{
				funcWrap(std::forward<Args>(args)...);
			}
		}
	};

	// Delegate bound functions of which could only be executed in Owner
	template <typename Owner, typename... Args>
	struct DelegateOwned final : public DelegateBase<Args...>
	{
	protected:
		// Execute all bound functions
		void execute(Args&&... args) const
		{
			for (const auto& [binding, funcWrap] : this->mBindings)
			{
				funcWrap(std::forward<Args>(args)...);
			}
		}
		friend Owner;
	};
} // namespace lune