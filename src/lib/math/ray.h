/*
 * ray.h
 *
 *  Created on: 25.05.2013
 *      Author: michi
 */

#ifndef RAY_H_
#define RAY_H_


struct Ray
{
	Ray();
	Ray(const vector &a, const vector &b);
	vector u, v;
	float dot(const Ray &r) const;
	bool intersect_plane(const plane &pl, vector &c) const;
};


#endif /* RAY_H_ */
