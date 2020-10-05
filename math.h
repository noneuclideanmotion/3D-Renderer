#pragma once
#include "geometry.h"
#include <iostream>
#include "Light.h"
#include <algorithm>
namespace lc
{
	namespace math
	{
		//Distance between point and plane
		double distance(geometry::vector3D v, geometry::vector3D plane_normal, geometry::vector3D plane_point)
		{
			return (v.x*plane_normal.x + v.y*plane_normal.y + v.z*plane_normal.z - geometry::vector3D::dot(plane_normal, plane_point));
		};

		//Point where line intersects plane
		geometry::vector3D intersection(geometry::vector3D& plane_point, geometry::vector3D& plane_normal, geometry::vector3D& line_start, geometry::vector3D& line_end)
		{
			plane_normal.normalise();
			float plane_d = -geometry::vector3D::dot(plane_normal, plane_point);
			float ad = geometry::vector3D::dot(line_start, plane_normal);
			float bd = geometry::vector3D::dot(line_end, plane_normal);
			double t = (-plane_d - ad) / (bd - ad);
			geometry::vector3D line_intersect = (line_end - line_start * t);
			return line_start + line_intersect;
		}

		//Clip triangle function, with point and normal to plane - 1 input triangle which can generate up to two output triangles
		int clip(geometry::vector3D plane_point, geometry::vector3D plane_normal, geometry::triangle& input_triangle, geometry::triangle& output_triangle1, geometry::triangle& output_triangle2)
		{
			plane_normal.normalise();

			geometry::vector3D* inside_points[3];
			geometry::vector3D* outside_points[3];
			int inside_point_count = 0;
			int outside_point_count = 0;

			double distance_p[3];
			distance_p[0] = distance(input_triangle.points[0], plane_normal, plane_point);
			distance_p[1] = distance(input_triangle.points[1], plane_normal, plane_point);
			distance_p[2] = distance(input_triangle.points[2], plane_normal, plane_point);


			for (int i = 0; i < 3; ++i)
			{
				//std::cout << distance_p[i] << std::endl;
				if (distance_p[i] >= 0)
				{
					inside_points[inside_point_count++] = &input_triangle.points[i];
				}
				else
				{
					outside_points[outside_point_count++] = &input_triangle.points[i];
				}
			}

			//the triangle is entirely outside plane
			if (inside_point_count == 0)
			{
				return 0;
			}

			//there is 2 triangles to be created
			if (inside_point_count == 2 && outside_point_count == 1)
			{
				//triangle 1
				output_triangle1.color = input_triangle.color;
				output_triangle1.points[0] = *inside_points[0];
				output_triangle1.points[1] = *inside_points[1];
				output_triangle1.points[2] = intersection(plane_point, plane_normal, *inside_points[0], *outside_points[0]);

				//triangle 2
				output_triangle2.color = input_triangle.color;
				output_triangle2.points[0] = *inside_points[1];
				output_triangle2.points[1] = output_triangle1.points[2];
				output_triangle2.points[2] = intersection(plane_point, plane_normal, *inside_points[1], *outside_points[0]);

				return 2;
			}
			//there is 1 triangle to be created
			if (inside_point_count == 1 && outside_point_count == 2)
			{
				output_triangle1.color = input_triangle.color;
				output_triangle1.points[0] = *inside_points[0];

				output_triangle1.points[1] = math::intersection(plane_point, plane_normal, *inside_points[0], *outside_points[0]);
				output_triangle1.points[2] = math::intersection(plane_point, plane_normal, *inside_points[0], *outside_points[1]);
				return 1;
			}

			//the triangle is entirely inside the plane
			if (inside_point_count == 3)
			{
				output_triangle1 = input_triangle;
				return 1;
			}
		}

		lc::geometry::vector2D project(const lc::geometry::vector3D& p, const lc::geometry::Camera& c)
		{

			double cos_x = cos(c.orientation.x);	double sin_x = sin(c.orientation.x);
			double cos_y = cos(c.orientation.y);	double sin_y = sin(c.orientation.y);
			double cos_z = cos(c.orientation.z);	double sin_z = sin(c.orientation.z);

			double t_x = p.x - c.position.x;
			double t_y = p.y - c.position.y;
			double t_z = p.z - c.position.z;

			double d_x = cos_y*(sin_z*t_y + cos_z*t_x) - sin_y*t_z;
			double d_y = sin_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) + cos_x*(cos_z*t_y - sin_z*t_x);
			double d_z = cos_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) - sin_x*(cos_z*t_y - sin_z*t_x);

			return lc::geometry::vector2D((d_x / d_z)*c.fov_x() + c.drawScreen.x / 2, (d_y / d_z)*c.fov_y() + c.drawScreen.y / 2);
		}

		void draw(lc::geometry::mesh& m, const lc::geometry::Camera& c, sf::RenderWindow &w, const lc::geometry::Sun s)
		{
			int size = 0;

			int i = 0;
			//Set points of vertext array to projected coordinates of each point of mesh

			//Create rasterised list of triangles
			std::vector<geometry::triangle> sortedTriangles;

			//assume camera doesnt move during rendering
			geometry::vector3D camera_facing = c.facing();
			std::cout << camera_facing.x << std::endl;
			for (geometry::triangle& t : m.triangles)
			{
				geometry::vector3D normal = t.normal();
				//If we can see the triangle
				if (geometry::vector3D::dot(normal, t.points[0] - c.position) < 0)
				{
					++size;
					//Increase triangle count by one, set positions
					i += 3;

					//additional triangles
					int additional_triangles = 0;
					geometry::triangle clipped_triangles[2];
					additional_triangles = clip(c.position+geometry::vector3D(0,0,10), camera_facing, t, clipped_triangles[0], clipped_triangles[1]);
					sortedTriangles.push_back(t);
					
					for (int j = 0; j < additional_triangles; ++j)
					{
						sortedTriangles.push_back(clipped_triangles[j]);
					}
				}
			}

			std::sort(sortedTriangles.begin(), sortedTriangles.end(), [](geometry::triangle &t1, geometry::triangle &t2)
			{
				double z1 = (t1.points[0].z + t1.points[1].z + t1.points[2].z) / 3;
				double z2 = (t2.points[0].z + t2.points[1].z + t2.points[2].z) / 3;
				return (z1 > z2);
			});

			i = 0;

			//Create vertex array with that many triangles
			sf::VertexArray meshtriangles(sf::Triangles, 3 * sortedTriangles.size());

			for (auto& t : sortedTriangles)
			{
				geometry::vector3D normal = t.normal();

				meshtriangles[i + 0].position = lc::math::project(t.points[0], c).sfmlvec();
				meshtriangles[i + 1].position = lc::math::project(t.points[1], c).sfmlvec();
				meshtriangles[i + 2].position = lc::math::project(t.points[2], c).sfmlvec();

				//Illuminate the triangles
				double illumination = abs(s.illumination_amount(normal));
				meshtriangles[i].color = sf::Color(t.color.r*illumination, t.color.g*illumination, t.color.b*illumination, t.color.a);
				meshtriangles[i + 1].color = sf::Color(t.color.r * illumination, t.color.g * illumination, t.color.b * illumination, t.color.a);
				meshtriangles[i + 2].color = sf::Color(t.color.r * illumination, t.color.g * illumination, t.color.b * illumination, t.color.a);
				i += 3;
			}

			//If we can see any triangles
			if (size != 0)
			{
				//ignore triangles we cant see
				meshtriangles.resize(3 * (size));

				//Draw
				w.draw(meshtriangles);
			}
		}

		geometry::vector3D rotate(geometry::vector3D point, geometry::vector3D orientation, geometry::vector3D origin)
		{
			//Apply rotational matrix to point
			double cos_x = cos(orientation.x);	double sin_x = sin(orientation.x);
			double cos_y = cos(orientation.y);	double sin_y = sin(orientation.y);
			double cos_z = cos(orientation.z);	double sin_z = sin(orientation.z);

			double t_x = point.x - origin.x;
			double t_y = point.y - origin.y;
			double t_z = point.z - origin.z;

			double d_x = cos_y*(sin_z*t_y + cos_z*t_x) - sin_y*t_z;
			double d_y = sin_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) + cos_x*(cos_z*t_y - sin_z*t_x);
			double d_z = cos_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) - sin_x*(cos_z*t_y - sin_z*t_x);

			point.x = d_x + origin.x;
			point.y = d_y + origin.y;
			point.z = d_z + origin.z;
			return{ point.x, point.y, point.z };
		}
	}
}