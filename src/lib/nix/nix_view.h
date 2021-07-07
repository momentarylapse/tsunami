/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once

class Image;

namespace nix {

class Texture;
class DepthBuffer;


void _cdecl set_projection_perspective();
void _cdecl set_projection_perspective_ext(float center_x, float center_y, float width_1, float height_1, float z_min, float z_max);
void _cdecl set_projection_ortho_relative();
void _cdecl set_projection_ortho_pixel();
void _cdecl set_projection_ortho_ext(float center_x, float center_y, float map_width, float map_height, float z_min, float z_max);
void _cdecl set_projection_matrix(const matrix &mat);

void _cdecl set_model_matrix(const matrix &mat);
void _cdecl set_view_matrix(const matrix &view_mat);

void _cdecl set_viewport(const rect &area);

void _cdecl set_scissor(const rect &r);

void _cdecl screen_shot_to_image(Image &image);


void start_frame_hui();
void end_frame_hui();
#if HAS_LIB_GLFW
void start_frame_glfw(void *win);
void end_frame_glfw(void *win);
#endif

};

#endif

