/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#ifndef _NIX_VIEW_EXISTS_
#define _NIX_VIEW_EXISTS_

namespace nix{

class Texture;

void _cdecl SetProjectionPerspective();
void _cdecl SetProjectionPerspectiveExt(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max);
void _cdecl SetProjectionOrtho(bool relative);
void _cdecl SetProjectionOrthoExt(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max);
void _cdecl SetProjectionMatrix(const matrix &mat);

void _cdecl SetWorldMatrix(const matrix &mat);

void _cdecl SetViewPosAng(const vector &view_pos, const quaternion &view_ang);
void _cdecl SetViewPosAngV(const vector &view_pos, const vector &view_ang);
void _cdecl SetViewMatrix(const matrix &view_mat);

void _cdecl GetVecProject(vector &vout, const vector &vin);
void _cdecl GetVecUnproject(vector &vout, const vector &vin);
void _cdecl GetVecProjectRel(vector &vout, const vector &vin);
void _cdecl GetVecUnprojectRel(vector &vout, const vector &vin);
bool _cdecl IsInFrustrum(const vector &pos, float radius);
void _cdecl SetClipPlane(int index, const plane &pl);
void _cdecl EnableClipPlane(int index, bool enabled);
void _cdecl Resize(int width, int height);

bool _cdecl Start();
bool _cdecl StartIntoTexture(Texture *t);
void _cdecl Scissor(const rect &r);
void _cdecl End();

void _cdecl ScreenShot(const string &filename, int width = -1, int height = -1);
void _cdecl ScreenShotToImage(Image &image);

extern float view_jitter_x, view_jitter_y;

};

#endif
