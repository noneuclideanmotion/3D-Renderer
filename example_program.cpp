#include <SFML/Graphics.hpp>
#include "math.h"
#include <iostream>

int main()
{
	lc::geometry::Camera c({ 0,0,-17 }, { 0,0,0 });
	sf::RenderWindow w(sf::VideoMode(lc::math::screen.sfmlvec().x, lc::math::screen.sfmlvec().y), "test");
	lc::geometry::mesh m;
	m.loadFromFile(".\\\\objects\\tea.obj");
	lc::geometry::Sun s = lc::geometry::Sun(c.position + lc::geometry::vector3D(5,5,0));
	w.setFramerateLimit(60);
	lc::geometry::vector3D f;
	m.translate({ 0,0,-3});
	double count = 0.001;
	double c2 = 0.001;
	while (true)
	{
		m.rotate({ -count, count * 3, -count * 2 }, m.position());
		m.change_color(sf::Color(127*(sin(c2)+1), 127 * (cos(c2) + 1), -127 * (cos(c2) + 1)));
		lc::math::draw(m, c, w, s);
		f = c.facing();
		w.display();
		w.clear();
		c2+=0.001;
	}
	return 0;
}