#pragma once

#include <set>
#include <typeindex>

namespace lune
{
	class SystemBase
	{
	public:
		SystemBase() = default;
		SystemBase(const SystemBase&) = delete;
		SystemBase(SystemBase&&) = default;
		virtual ~SystemBase() = default;

		virtual void update(class Scene* scene, double deltaTime) {};

		const std::set<std::type_index>& getDependecies() const { return mDependecies; }

	protected:
		template <typename T>
		void addDependecy()
		{
			mDependecies.emplace(typeid(T));
		}

	private:
		std::set<std::type_index> mDependecies{};
	};

	class PrepareRenderSystemInterface
	{
	public:
		virtual void prepareRender(class Scene* scene) = 0;
	};

	class RenderSystemInterface
	{
	public:
		virtual void render(class Scene* scene) = 0;
	};

	class ImGuiRenderSystemInterface
	{
	public:
		virtual void imGuiRender(class Scene* scene) = 0;
	};

} // namespace lune