/*----------------------------------------------------------------------------*\
| Nix light                                                                    |
| -> handling light sources                                                    |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{


// light-sources
/*enum{
	LightTypeDirectional,
	LightTypeRadial
};
struct sLight{
	bool Used,Allowed,Enabled;
	int Type;
	int OGLLightNo;
	int Light;
	vector Pos,Dir;
	float Radius;
	color Ambient,Diffuse,Specular;
};

Array<sLight> NixLight;*/

Material material;
Light lights[8];

bool LightingEnabled;

void TestGLError(const char*);


// general ability of using lights
void EnableLighting(bool enabled)
{
	LightingEnabled = enabled;
}

// Punkt-Quelle
void SetLightRadial(int index, const vector &pos, float radius, const color &diffuse, float ambient, float specular)
{
	if ((index < 0) or (index >= 8))
		return;
	index = 0;

	lights[index].diffusive = diffuse;
	lights[index].ambient = ambient;
	lights[index].specular = specular;
	lights[index].pos = pos;
	lights[index].radius = radius;
}

// parallele Quelle
// dir =Richtung, in die das Licht scheinen soll
void SetLightDirectional(int index, const vector &dir, const color &diffuse, float ambient, float specular)
{
	if ((index < 0) or (index >= 8))
		return;
	index = 0;

	lights[index].diffusive = diffuse;
	lights[index].ambient = ambient;
	lights[index].specular = specular;
	lights[index].pos = dir;
	lights[index].radius = -1;
}

void EnableLight(int index,bool enabled)
{
	if ((index < 0) or (index >= 8))
		return;
	lights[index].enabled = true;
}

void SetAmbientLight(const color &c)
{
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT,(float*)&c);
	TestGLError("SetAmbient");
}

void SetMaterial(const color &ambient,const color &diffuse,const color &specular,float shininess,const color &emission)
{
	material.ambient = ambient;
	material.diffusive = diffuse;
	material.specular = specular;
	material.shininess = shininess;
	material.emission = emission;
}



void UpdateLights()
{
#if 0
	// OpenGL muss Lichter neu ausrichten, weil sie in Kamera-Koordinaten gespeichert werden!
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//glLoadIdentity();
	glLoadMatrixf((float*)&NixViewMatrix);

	foreachi(sLight &l, NixLight, i){
		if (!l.Used)
			continue;
		if (!l.Enabled)
			continue;
	//	if (OGLLightNo[i]<0)	continue;
		float f[4];
		/*f[0]=LightVector[i].x;	f[1]=LightVector[i].y;	f[2]=LightVector[i].z;
		if (LightDirectional[i])
			f[3]=0;
		else
			f[3]=1;
		glLightfv(OGLLightNo[i],GL_POSITION,f);*/
		if (l.Type == LightTypeDirectional){
			f[0] = l.Dir.x;
			f[1] = l.Dir.y;
			f[2] = l.Dir.z;
			f[3] = 0;
		}else if (l.Type == LightTypeRadial){
			f[0] = l.Pos.x;
			f[1] = l.Pos.y;
			f[2] = l.Pos.z;
			f[3] = 1;
		}
		glLightfv(GL_LIGHT0+i,GL_POSITION,f);
		//msg_write(i);
	}
	glPopMatrix();
	TestGLError("UpdateLights");
#endif
}

};

#endif
