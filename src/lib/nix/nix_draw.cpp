/*----------------------------------------------------------------------------*\
| Nix draw                                                                     |
| -> drawing functions                                                         |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#include "nix.h"
#include "nix_common.h"

namespace nix{

float line_width = 1;
bool smooth_lines = false;
//static color current_color = White;

unsigned int line_buffer = 0;
unsigned int color_buffer = 0;


render_str_function *render_str = NULL;
extern Texture *tex_text;
extern Shader *current_shader;
extern VertexBuffer *vb_2d;



void SetColor(const color &c)
{
	material.emission = c;
}

color GetColor()
{
	return material.emission;
}

void DrawChar(float x, float y, char c)
{
	char str[2];
	str[0]=c;
	str[1]=0;
	DrawStr(x,y,str);
}

string str_utf8_to_ubyte(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++)
		if (((unsigned int)str[i] & 0x80) > 0){
			r.add(((str[i] & 0x1f) << 6) + (str[i + 1] & 0x3f));
			i ++;
		}else
			r.add(str[i]);
	return r;
}

void DrawStr(float x, float y, const string &str)
{
	if (render_str){
		Image im;
		(*render_str)(str, im);
		if (im.width > 0){
			tex_text->overwrite(im);
			SetTexture(tex_text);
			Draw2D(rect::ID, rect(x, x + im.width, y, y + im.height), 0);
		}
	}else{
		/*string str2 = str_utf8_to_ubyte(str);

		glRasterPos3f(x, (y+2+int(float(FontHeight)*0.75f)),-1.0f);
		glListBase(OGLFontDPList);
		glCallLists(str2.num,GL_UNSIGNED_BYTE,(char*)str2.data);
		glRasterPos3f(0,0,0);
		TestGLError("DrawStr");*/
	}
}

int GetStrWidth(const string &str)
{

	if (render_str){
		Image im;
		(*render_str)(str, im);
		return im.width;
	}else{
#if 0
		string str2 = str_utf8_to_ubyte(str);
		int w = 0;
		for (int i=0;i<str2.num;i++)
			w += FontGlyphWidth[(unsigned char)str2[i]];
		return w;
#endif
	}
	return 0;
}

void DrawLine(float x1, float y1, float x2, float y2, float depth)
{
	DrawLine3D(vector(x1, y1, depth), vector(x2, y2, depth));
	return;

	float dx=x2-x1;
	if (dx<0)	dx=-dx;
	float dy=y2-y1;
	if (dy<0)	dy=-dy;
	//_SetMode2d();

#ifdef OS_LINUX
	// internal line drawing function \(^_^)/
	if (smooth_lines){
		// antialiasing!
		glLineWidth(line_width + 0.5);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}else
		glLineWidth(line_width);
	glBegin(GL_LINES);
		glVertex3f(x1, y1, depth);
		glVertex3f(x2, y2, depth);
	glEnd();
	if (smooth_lines){
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}
#else

	// own line drawing function (T_T)
	if (dx>dy){
		if (x1>x2){
			float x=x2;	x2=x1;	x1=x;
			float y=y2;	y2=y1;	y1=y;
		}
		glBegin(GL_TRIANGLES);
			glVertex3f(x1,y1+1,depth);
			glVertex3f(x1,y1  ,depth);
			glVertex3f(x2,y2+1,depth);
			glVertex3f(x2,y2  ,depth);
			glVertex3f(x2,y2+1,depth);
			glVertex3f(x1,y1  ,depth);
		glEnd();
	}else{
		if (y1<y2){
			float x=x2;	x2=x1;	x1=x;
			float y=y2;	y2=y1;	y1=y;
		}
		glBegin(GL_TRIANGLES);
			glVertex3f(x1+1,y1,depth);
			glVertex3f(x1  ,y1,depth);
			glVertex3f(x2+1,y2,depth);
			glVertex3f(x2  ,y2,depth);
			glVertex3f(x2+1,y2,depth);
			glVertex3f(x1  ,y1,depth);
		glEnd();
	}
#endif
	TestGLError("DrawLine");
}

void DrawLines(const Array<vector> &p, bool contiguous)
{
	current_shader->set_default_data();
	Array<color> c;
	c.resize(p.num);
	for (int i=0; i<c.num; i++)
		c[i] = material.emission;
	DrawLinesColored(p, c, contiguous);
	return;

	if (line_buffer == 0)
		glGenBuffers(1, &line_buffer);

	TestGLError("dls-opt0");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, p.num * sizeof(p[0]), &p[0], GL_STATIC_DRAW);
	TestGLError("dls-opt1");

	TestGLError("dls-a");
	glEnableVertexAttribArray(0);
	TestGLError("dls-b1");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	TestGLError("dls-c1");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("dls-d1");

	if (contiguous)
		glDrawArrays(GL_LINE_STRIP, 0, p.num);
	else
		glDrawArrays(GL_LINES, 0, p.num);
	TestGLError("dls-e");

	glDisableVertexAttribArray(0);
	TestGLError("dls-f");
}

void DrawLinesColored(const Array<vector> &p, const Array<color> &c, bool contiguous)
{
	current_shader->set_default_data();

	if (line_buffer == 0)
		glGenBuffers(1, &line_buffer);
	if (color_buffer == 0)
		glGenBuffers(1, &color_buffer);

	TestGLError("dlc-opt0");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, p.num * sizeof(p[0]), &p[0], GL_STATIC_DRAW);
	TestGLError("dlc-opt1");
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, c.num * sizeof(c[0]), &c[0], GL_STATIC_DRAW);

	TestGLError("dlc-a1");
	glEnableVertexAttribArray(0);
	TestGLError("dlc-b1");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	TestGLError("dlc-c1");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("dlc-d1");

	TestGLError("dlc-2a");
	glEnableVertexAttribArray(1);
	TestGLError("dlc-b2");
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	TestGLError("dlc-c2");
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("dlc-d2");

	if (contiguous)
		glDrawArrays(GL_LINE_STRIP, 0, p.num);
	else
		glDrawArrays(GL_LINES, 0, p.num);
	TestGLError("dlc-e");

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	TestGLError("dlc-f");
}

void DrawLineV(float x, float y1, float y2, float depth)
{
	DrawLine(x, y1, x, y2, depth);
	/*if (y1>y2){
		float y=y2;	y2=y1;	y1=y;
	}
	NixDraw2D(rect::ID, rect(x, x + 1, y1, y2), depth);*/
}

void DrawLineH(float x1, float x2, float y, float depth)
{
	DrawLine(x1, y, x2, y, depth);
	/*if (x1>x2){
		float x=x2;
		x2=x1;
		x1=x;
	}
	NixDraw2D(rect::ID, rect(x1, x2, y, y + 1), depth);*/
}

void DrawLine3D(const vector &l1, const vector &l2)
{
	vector v[2] = {l1, l2};
	color c[2] = {material.emission, material.emission};

	current_shader->set_default_data();

	if (line_buffer == 0)
		glGenBuffers(1, &line_buffer);
	if (color_buffer == 0)
		glGenBuffers(1, &color_buffer);

	TestGLError("dl-opt0");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(v[0]), &v[0], GL_STATIC_DRAW);
	TestGLError("dl-opt1");
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(c[0]), &c[0], GL_STATIC_DRAW);

	TestGLError("dl-a");
	glEnableVertexAttribArray(0);
	TestGLError("dl-b1");
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	TestGLError("dl-c1");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("dl-d1");

	TestGLError("dlc-2a");
	glEnableVertexAttribArray(1);
	TestGLError("dlc-b2");
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	TestGLError("dlc-c2");
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("dlc-d2");

	glDrawArrays(GL_LINES, 0, 2);
	TestGLError("dl-e");

	glDisableVertexAttribArray(0);
	TestGLError("dl-f");
}

void DrawRect(float x1, float x2, float y1, float y2, float depth)
{
	/*float t;
	if (x1>x2){
		t=x1;	x1=x2;	x2=t;
	}
	if (y1>y2){
		t=y1;	y1=y2;	y2=t;
	}
	if (!Fullscreen){
		int pa=40;
		for (int i=0;i<int(x2-x1-1)/pa+1;i++){
			for (int j=0;j<int(y2-y1-1)/pa+1;j++){
				float _x1=x1+i*pa;
				float _y1=y1+j*pa;

				float _x2=x2;
				if (x2-x1-i*pa>pa)	_x2=x1+i*pa+pa;
				float _y2=y2;
				if (y2-y1-j*pa>pa)	_y2=y1+j*pa+pa;

				Draw2D(r_id, rect(_x1, _x2, _y1, _y2), depth);
			}
		}
		return;
	}*/
	Draw2D(rect::ID, rect(x1, x2, y1, y2), depth);
}

void Draw2D(const rect &src, const rect &dest, float depth)
{
	vb_2d->clear();
	vector a = vector(dest.x1, dest.y1, depth);
	vector b = vector(dest.x2, dest.y1, depth);
	vector c = vector(dest.x1, dest.y2, depth);
	vector d = vector(dest.x2, dest.y2, depth);
	vb_2d->addTria(a, v_0, src.x1, src.y1, b, v_0, src.x2, src.y1, c, v_0, src.x1, src.y2);
	vb_2d->addTria(c, v_0, src.x1, src.y2, b, v_0, src.x2, src.y1, d, v_0, src.x2, src.y2);
	Draw3D(vb_2d);
}



void Draw3D(VertexBuffer *vb)
{
	if (vb->dirty)
		vb->update();

	current_shader->set_default_data();

	TestGLError("a");
	glEnableVertexAttribArray(0);
	TestGLError("b1");
	glBindBuffer(GL_ARRAY_BUFFER, vb->buf_v);
	TestGLError("c1");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	TestGLError("d1");

	glEnableVertexAttribArray(1);
	TestGLError("b2");
	glBindBuffer(GL_ARRAY_BUFFER, vb->buf_n);
	TestGLError("c2");
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)0);
	TestGLError("d2");

	for (int i=0; i<vb->num_textures; i++){
		glEnableVertexAttribArray(2 + i);
		TestGLError("b3");
		glBindBuffer(GL_ARRAY_BUFFER, vb->buf_t[i]);
		TestGLError("c3");
		glVertexAttribPointer(2+i, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		TestGLError("d3");
	}

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3*vb->num_triangles); // Starting from vertex 0; 3 vertices total -> 1 triangle
	TestGLError("e");

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	for (int i=0; i<vb->num_textures; i++)
		glDisableVertexAttribArray(2 + i);
	TestGLError("f");


	NumTrias += vb->num_triangles;
	TestGLError("Draw3D");
}

void DrawSpriteR(const rect &src, const vector &pos, const rect &dest)
{
#if 0
	rect d;
	float depth;
	vector p;
	NixGetVecProject(p,pos);
	if ((p.z<=0.0f)||(p.z>=1.0))
		return;
	depth=p.z;
	vector u = NixViewMatrix * pos;
	float q=NixMaxDepth/(NixMaxDepth-NixMinDepth);
	float f=1.0f/(u.z*q*NixMinDepth*NixView3DRatio);
#ifdef NIX_API_OPENGL
	if (NixApi==NIX_API_OPENGL){
		//depth=depth*2.0f-1.0f;
		//f*=2;
	}
#endif
	//if (f>20)	f=20;
	d.x1=p.x+f*(dest.x1)*NixViewScale.x*NixTargetWidth;
	d.x2=p.x+f*(dest.x2)*NixViewScale.x*NixTargetWidth;
	d.y1=p.y+f*(dest.y1)*NixViewScale.y*NixTargetHeight*NixView3DRatio;
	d.y2=p.y+f*(dest.y2)*NixViewScale.y*NixTargetHeight*NixView3DRatio;
	NixDraw2D(src, d, depth);
#endif
}

void DrawSprite(const rect &src,const vector &pos,float radius)
{
	rect d;
	d.x1=-radius;
	d.x2=radius;
	d.y1=-radius;
	d.y2=radius;
	DrawSpriteR(src, pos, d);
}

void ResetToColor(const color &c)
{
	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetToColor");
}

void ResetZ()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	TestGLError("ResetZ");
}

};
#endif
