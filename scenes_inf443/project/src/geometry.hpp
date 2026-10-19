#pragma once

#include "cgp/cgp.hpp"

// Intersection des segments [p1,p2] et [q1,q2] du plan.
// S'ils se croisent, t et u donnent la position du croisement sur chacun des deux segments (entre 0 et 1).
inline bool segments_intersect(cgp::vec2 const& p1, cgp::vec2 const& p2, cgp::vec2 const& q1, cgp::vec2 const& q2, float& t, float& u)
{
	auto cross = [](cgp::vec2 const& a, cgp::vec2 const& b) { return a.x * b.y - a.y * b.x; };

	cgp::vec2 const r = p2 - p1;
	cgp::vec2 const s = q2 - q1;
	float const denom = cross(r, s);
	if (std::abs(denom) < 1e-8f)
		return false; // segments parallèles

	t = cross(q1 - p1, s) / denom;
	u = cross(q1 - p1, r) / denom;
	return t >= 0 && t <= 1 && u >= 0 && u <= 1;
}

inline cgp::vec2 xy(cgp::vec3 const& p) { return { p.x, p.y }; }
