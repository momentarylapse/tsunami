/*
 * ray.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef RAY_H_
#define RAY_H_


class Ray
{
public:
	Ray();
	Ray(const vector &a, const vector &b);
	vector u, v;
	float _cdecl dot(const Ray &r) const;
	bool _cdecl intersect_plane(const plane &pl, vector &c) const;
};


#endif /* RAY_H_ */
