#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <strstream>

namespace lc
{

	namespace geometry
	{
		struct vector2D
		{
			vector2D() {}
			vector2D(double a, double b) : x(a), y(b) {}
			sf::Vector2f sfmlvec() const { return sf::Vector2f(x, y); }
			double x, y;
		};

	}

	namespace math
	{
		const lc::geometry::vector2D screen(1080, 720);
	}

	namespace geometry
	{

		struct vector3D
		{
			vector3D() {}
			vector3D(double a, double b, double c) : x(a), y(b), z(c) {}
			vector3D operator+(const vector3D& RHS) const { return vector3D(x + RHS.x, y + RHS.y, z + RHS.z); }
			vector3D operator-(const vector3D& RHS) const { return vector3D(x - RHS.x, y - RHS.y, z - RHS.z); }
			vector3D& operator*(const double& RHS) { return vector3D(x *RHS, y *RHS, z*RHS); }
			vector3D& operator/(const double& RHS) { return vector3D(x / RHS, y / RHS, z / RHS); }

			//Cross product
			static vector3D cross(const vector3D& left, const vector3D& right)
			{
				return{ left.y*right.z - left.z*right.y, left.z*right.x - left.x*right.z, left.x*right.y - left.y*right.x };
			}

			//Dot product
			static double dot(const vector3D& left, const vector3D& right)
			{
				return left.y*right.y + left.z*right.z + left.x*right.x;
			}

			//magnitude
			double magnitude()
			{
				return sqrt(x*x + y*y + z*z);
			}
			//normalise
			vector3D& normalise()
			{
				*this = *this / magnitude();
				return *this;
			}
			double x, y, z;
		};
	}

	namespace math
	{
		geometry::vector3D rotate(geometry::vector3D point, geometry::vector3D orientation, geometry::vector3D origin = { 0,0,0 });
	}


	namespace geometry
	{

		struct triangle
		{
			triangle() {}
			triangle(vector3D a, vector3D b, vector3D c) { points[0] = a; points[1] = b; points[2] = c; }
			vector3D normal() const
			{
				vector3D lineA = points[1] - points[0];
				vector3D lineB = points[2] - points[0];

				//Cross product of lineA and line B
				return vector3D::cross(lineA, lineB).normalise();
			}

			sf::Color color = sf::Color::White;

			vector3D points[3];
		};
	

		struct mesh
		{
			mesh() : transform_origin({ 0,0,0 }), m_orientation({ 0,0,0 }) {}
			mesh(std::vector<triangle> t) : transform_origin({ 0,0,0 }), m_orientation({ 0,0,0 }) { triangles = t; }
			mesh(std::vector<triangle> t, vector3D orientation) : transform_origin({ 0,0,0 }), m_orientation(orientation) { triangles = t; }
			mesh(std::vector<triangle> t, vector3D orientation, vector3D origin) : transform_origin(origin), m_orientation(orientation) { triangles = t; }


			//Load mesh from triangulated obj file
			bool loadFromFile(std::string filename)
			{
				//Clear mesh
				triangles.clear();

				//Attempt to open file
				std::ifstream file;
				file.open(filename);
				if (!file.is_open())
				{
					std::cout << "Failed to open obj file: " << filename << std::endl;
					return false;
				}
				//Success in opening file

				//Create vertex vector
				std::vector<lc::geometry::vector3D> verticies;
				while (!file.eof())
				{
					//assume line no longer than 256 characters
					char l[128];
					file.getline(l, 128);
					std::strstream line;
					line << l;

					//Get type of data on line
					char data_type;

					//Vertex data
					if (l[0] == 'v')
					{
						geometry::vector3D v;
						//Get vertex data
						line >> data_type >> v.x >> v.y >> v.z;
						verticies.push_back(v);
					}

					//Face data
					if (l[0] == 'f')
					{
						int faces[3];
						//Get face data
						line >> data_type >> faces[0] >> faces[1] >> faces[2];
						//Load triangle (index in file starts from one)
						triangles.push_back({ verticies[faces[0] - 1], verticies[faces[1] - 1], verticies[faces[2] - 1] });
					}
				}

				return true;
			}

			//Rotate mesh, to be simplified
			void rotate(vector3D orientation, vector3D origin = { 0,0,0 })
			{
				for (auto& triangle : triangles)
				{
					for (int i = 0; i < 3; ++i)
					{
						//Apply rotational matrix to each point
						double cos_x = cos(orientation.x);	double sin_x = sin(orientation.x);
						double cos_y = cos(orientation.y);	double sin_y = sin(orientation.y);
						double cos_z = cos(orientation.z);	double sin_z = sin(orientation.z);

						double t_x = triangle.points[i].x - origin.x;
						double t_y = triangle.points[i].y - origin.y;
						double t_z = triangle.points[i].z - origin.z;

						double d_x = cos_y*(sin_z*t_y + cos_z*t_x) - sin_y*t_z;
						double d_y = sin_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) + cos_x*(cos_z*t_y - sin_z*t_x);
						double d_z = cos_x*(cos_y*t_z + sin_y*(sin_z*t_y + cos_z*t_x)) - sin_x*(cos_z*t_y - sin_z*t_x);

						triangle.points[i].x = d_x + origin.x;
						triangle.points[i].y = d_y + origin.y;
						triangle.points[i].z = d_z + origin.z;
						m_orientation = m_orientation + orientation;
					}
				}
			}

			//set mesh rotation to specificed orientation
			void orientation(vector3D orientation, vector3D origin = { 0,0,0 })
			{
				if (!(orientation.x == m_orientation.x&&orientation.y == m_orientation.y&&orientation.z == m_orientation.z))
				{
					for (auto& triangle : triangles)
					{
						for (int i = 0; i < 3; ++i)
						{
							triangle.points[i] = lc::math::rotate(triangle.points[i], m_orientation - orientation, origin);
						}
					}
					m_orientation = orientation;
				}
			}

			//Translate position
			void translate(vector3D amount)
			{
				for (auto& triangle : triangles)
				{
					for (int i = 0; i < 3; ++i)
					{
						triangle.points[i] = triangle.points[i] + amount;
					}
				}
			}

			//Scale object
			void scale(vector3D amount, vector3D origin = { 0,0,0 })
			{
				translate(origin*-1);
				for (auto& triangle : triangles)
				{
					for (int i = 0; i < 3; ++i)
					{
						triangle.points[i].x *= amount.x;
						triangle.points[i].y *= amount.y;
						triangle.points[i].z *= amount.z;
					}
				}
				translate(origin);
			}

			//Get position (of the first point in the mesh,useful for relative transformations);
			vector3D position()
			{
				return triangles[0].points[0];
			}

			//Sets all triangles to specific colour - fairly slow
			void change_color(sf::Color c)
			{
				for (auto& triangle : triangles)
				{
					triangle.color = c;
				}
			}

			//Vector of triangles this mesh represents
			std::vector<triangle> triangles;

			//Default origin for transformations if none is specified, defaults to 0,0
			vector3D transform_origin;
		private:
			vector3D m_orientation;
		};

		class Camera
		{
		public:
			Camera(lc::geometry::vector3D position_, lc::geometry::vector3D orientation_, lc::geometry::vector2D fov_ = { 3.141 / 1.5,  3.141 / (1.5 * 1.14) }, lc::geometry::vector2D drawScreen_ = lc::math::screen)
				: position(position_), orientation(orientation_), drawScreen(drawScreen_) {
				fov(fov_); m_facing_orientation = orientation_;
			}

			lc::geometry::vector3D position;
			lc::geometry::vector3D orientation;

			double fov_x() const { return m_fov_x; }
			double fov_y() const { return m_fov_y; }

			//get direction vector of camera
			lc::geometry::vector3D facing() const
			{
				//0,0,1 is direction vector with zero rotation
				return lc::math::rotate(this->position, this->orientation, { 0,0,1 }).normalise();
			}

			void fov(lc::geometry::vector2D f)
			{
				m_fov_x = drawScreen.x / tan(f.x / 2);
				m_fov_y = drawScreen.y / tan(f.y / 2);
			}
			lc::geometry::vector2D drawScreen;
		protected:
			double m_fov_x;
			double m_fov_y;
			lc::geometry::vector3D m_facing_orientation;
			lc::geometry::vector3D m_facing_current;
		};
	}
}