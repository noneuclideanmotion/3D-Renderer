#include "Light.h"

namespace lc
{
	namespace geometry
	{

		lc::geometry::Sun::Sun(lc::geometry::vector3D position)
		{
			m_position = position.normalise();
			m_abs_position = position;
		}

		//Returns value between 0 and 1 of intensity of light
		double Sun::illumination_amount(lc::geometry::vector3D normal) const
		{
			return geometry::vector3D::dot(normal, m_position);
		}

		Sun::~Sun()
		{
		}

	}
}