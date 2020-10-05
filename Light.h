#pragma once
#include "geometry.h"

namespace lc
{
	namespace geometry
	{
		class Sun
		{
		public:
			//Simplest of lights, infinitely big, infinitely far away- points towards centre
			Sun(lc::geometry::vector3D position);

			//move sun by amount
			void move(lc::geometry::vector3D amount)
			{
				m_abs_position = m_abs_position + amount;
				m_position = m_abs_position.normalise();
			}

			//set position absolutely
			void position(lc::geometry::vector3D place)
			{
				m_abs_position = place;
				m_position = m_abs_position.normalise();
			}

			//Calculate the illumination amount for a specified triangle t
			double illumination_amount(lc::geometry::vector3D normal) const;
			~Sun();
		protected:	
			lc::geometry::vector3D m_position;
			lc::geometry::vector3D m_abs_position;
		};
	}
}