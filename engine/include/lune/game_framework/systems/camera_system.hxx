#pragma once

#include "lune/math.hxx"

#include "system.hxx"

namespace lune
{
	class camera_system : public system
	{
	public:
		virtual void update(const std::vector<std::shared_ptr<entity>>& entities, double deltaTime) override;

		const lnm::mat4& getView() const { return mView; }
		const lnm::mat4& getProjection() const { return mProjection; }
		const lnm::mat4& getViewProjection() const { return mViewProjection; }

	private:
		lnm::mat4 mView{};
		lnm::mat4 mProjection{};
		lnm::mat4 mViewProjection{};
	};
} // namespace lune