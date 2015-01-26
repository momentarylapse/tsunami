#include "base.h"


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>

#ifdef OS_WINDOWS
	#include <stdio.h>
	#include <io.h>
	#include <direct.h>
	#include <stdarg.h>
	#include <windows.h>
	#include <winbase.h>
	#include <winnt.h>
#endif
#ifdef OS_LINUX
	#include <unistd.h>
	#include <dirent.h>
	#include <stdarg.h>
	#include <sys/timeb.h>
	#include <sys/stat.h>
#endif



string::string()
{
	init(sizeof(unsigned char));
}

string::string(const char *str)
{
	init(sizeof(unsigned char));
	int l = strlen(str);
	resize(l);
	if (l > 0)
		memcpy(data, str, l);
}

string::string(const void *str, int l)
{
	init(sizeof(unsigned char));
	resize(l);
	if (l > 0)
		memcpy(data, str, l);
}

string::string(const string &s)
{
	init(sizeof(unsigned char));
	assign(&s);
}

void string::__init__()
{
	new(this) string;
}

string::~string()
{
	clear();
}

string string::substr(int start, int length) const
{
	string r;
	if (start >= num)
		return r;
	if (start < 0){
		// start from the end
		start = num + start;
		if (start < 0)
			return r;
	}
	if (length < 0){
		length = num - start + length + 1;
	}
	if (start + length > num)
		length = num - start;
	if (length > 0){
		r.resize(length);
		memcpy(r.data, &((unsigned char*)data)[start], length);
	}
	return r;
}

string string::head(int size) const
{	return substr(0, size);	}

string string::tail(int size) const
{	return substr(num - size, size);	}

int string::find(const string &s, int start) const
{
	unsigned char *b = (unsigned char*)data;
	unsigned char *aa = (unsigned char*)s.data;
	for (int i=start;i<num - s.num + 1;i++){
		bool ok = true;
		for (int j=0;j<s.num;j++)
			if (b[i + j] != aa[j]){
				ok = false;
				break;
			}
		if (ok)
			return i;
	}
	return -1;
}

int string::rfind(const string &s, int start) const
{
	unsigned char *b = (unsigned char*)data;
	unsigned char *aa = (unsigned char*)s.data;
	if (start < 0)
		start = num - 1;
	for (int i=start;i>=0;i--){
		bool ok = true;
		for (int j=0;j<s.num;j++)
			if (b[i + j] != aa[j]){
				ok = false;
				break;
			}
		if (ok)
			return i;
	}
	return -1;
}

int string::compare(const string &s) const
{
	unsigned char *a = (unsigned char*)data;
	int n = num;
	if (num > s.num)
		n = s.num;
	for (int i=0;i<n;i++){
		if (s[i] != a[i])
			return (int)a[i] - (int)s[i];
	}
	return num - s.num;
}

inline int ichar(unsigned char a)
{
	if ((a >= 'A') && (a <= 'Z'))
		return (int)a - (int)'A' + (int)'a';
	return (int)a;
}

int string::icompare(const string &s) const
{
	unsigned char *a = (unsigned char*)data;
	int n = num;
	if (num > s.num)
		n = s.num;
	for (int i=0;i<n;i++){
		if (ichar(s[i]) != ichar(a[i]))
			return ichar(a[i]) - ichar(s[i]);
	}
	return num - s.num;
}

string string::reverse() const
{
	string r;
	r.resize(num);
	unsigned char *a = (unsigned char*)data;
	unsigned char *b = (unsigned char*)r.data;
	for (int i=0;i<num;i++)
		b[num - i - 1] = a[i];
	return r;
}

Array<string> string::explode(const string &s) const
{
	Array<string> r;

	int pos = 0;
	while(true){
		int pos2 = find(s, pos);
		if (pos2 < 0)
			break;

		r.add(substr(pos, pos2 - pos));

		pos = s.num + pos2;
	}
	if ((r.num > 0) || (pos < num))
		r.add(substr(pos, num - pos));
	return r;
}


void string::replace0(int start, int length, const string &str)
{
	if (start + length > num)
		return;
	unsigned char *s = (unsigned char*)data;
	int d = str.num - length;
	if (d > 0){
		resize(num + d);
		s = (unsigned char*)data;
		for (int i=num-1;i>=start+length;i--)
			s[i] = s[i - d];
	}
	memcpy(&s[start], str.data, str.num);
	if (d < 0){
		for (int i=start+str.num;i<num+d;i++)
			s[i] = s[i - d];
		resize(num + d);
	}
}

string string::replace(const string &sub, const string &by) const
{
	string r = *this;
	int i = r.find(sub, 0);
	while (i >= 0){
		r.replace0(i, sub.num, by);
		i = r.find(sub, i + by.num);
	}
	return r;
}

string string::lower() const
{
	string r = *this;
	for (int i=0;i<r.num;i++)
		if ((r[i] >= 'A') && (r[i] <= 'Z'))
			r[i] += 'a' - 'A';
	return r;
}

string string::upper() const
{
	string r = *this;
	for (int i=0;i<r.num;i++)
		if ((r[i] >= 'a') && (r[i] <= 'z'))
			r[i] += 'A' - 'a';
	return r;
}

#define STR_SMALL_STACK_DEPTH			32
#define STR_LARGE_STACK_DEPTH			8
#define STR_SMALL_SIZE					256

static int _current_stack_small_pos_ = 0;
static char _stack_small_str_[STR_SMALL_STACK_DEPTH][STR_SMALL_SIZE];
static int _current_stack_large_pos_ = 0;
static char *_stack_large_str_[STR_LARGE_STACK_DEPTH];
inline char *get_str(int size)
{
	if (size < STR_SMALL_SIZE){
		_current_stack_small_pos_ = (_current_stack_small_pos_ + 1) % STR_SMALL_STACK_DEPTH;
		return _stack_small_str_[_current_stack_small_pos_];
	}else{
		_current_stack_large_pos_ = (_current_stack_large_pos_ + 1) % STR_LARGE_STACK_DEPTH;
		if (_stack_large_str_[_current_stack_large_pos_])
			delete[]_stack_large_str_[_current_stack_large_pos_];
		_stack_large_str_[_current_stack_large_pos_] = new char[size + 1];
		return _stack_large_str_[_current_stack_large_pos_];
	}
}


const char *string::c_str() const
{
	if (allocated > num){
		((char*)data)[num] = 0;
		return (const char*)data;
	}
	char *s = get_str(num);
	if (num > 0)
		memcpy(s, data, num);
	s[num] = 0;
	return s;
}


// transposes path-strings to the current operating system
// accepts windows and linux paths ("/" and "\\")
string string::sys_filename() const
{
#ifdef OS_WINDOWS
	return replace("/", "\\");
#endif
	return replace("\\", "/");
}

// ends with '/' or '\'
string string::dirname() const
{
	int i = max(rfind("/"), rfind("\\"));
	if (i >= 0)
		return head(i + 1);
	return "";
}

string string::basename() const
{
	int i = max(rfind("/"), rfind("\\"));
	if (i >= 0)
		return tail(num - i - 1);
	return *this;
}

// make sure the name ends with a shlash
void string::dir_ensure_ending()
{
	if (num > 0){
		char lc = (*this)[num - 1];
		if ((lc != '/') && (lc != '\\'))
			add('/');
	}
}

// remove "/../"
string string::no_recursion() const
{
	string str = replace("\\", "/");
	Array<string> p = str.explode("/");

	for (int i=1;i<p.num;i++)
		if (p[i] == ".."){
			p.erase(i);
			p.erase(i - 1);
			i -= 2;
		}

	return implode(p, "/");
}

string string::extension() const
{
	int pos = rfind(".");
	if (pos >= 0)
		return tail(num - pos - 1).lower();
	return "";
}

static bool format_locale_set = false;

// connecting strings
string format(const string str,...)
{
	string tmp;
	va_list args;

    // retrieve the variable arguments
    va_start(args, str);

	//if (!format_locale_set){
		setlocale(LC_NUMERIC, "C");
		//format_locale_set = true;
	//}

#ifdef OS_WINDOWS
	int len = _vscprintf(str.c_str(), args);
#else
	int len = vsnprintf(NULL, 0, str.c_str(), args);
#endif
	tmp.resize(len + 1);
    va_start(args, str);
    vsprintf((char*)tmp.data, str.c_str(), args); // C4996
    // Note: vsprintf is deprecated; consider using vsprintf_s instead
	tmp.resize(len);
	va_end(args);
	
	return tmp;
#if 0
	char *tmp=_file_get_str_();
	tmp[0]=0;

	va_list marker;
	va_start(marker,str);

	int l=0,s=strlen(str);
	for (int i=0;i<s;i++){
		if ((str[i]=='%')&&(str[i+1]=='s')){
			strcat(tmp,va_arg(marker,char*));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='d')){
			strcat(tmp,i2s(va_arg(marker,int)));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='f')){
			int fl=3;
			if (str[i+2]==':'){
				fl=str[i+3]-'0';
				i+=3;
			}else
				i++;
			strcat(tmp,f2s((float)va_arg(marker,double),fl));
			l=strlen(tmp);
		}else if ((str[i]=='%')&&(str[i+1]=='v')){
			int fl=3;
			if (str[i+2]==':'){
				fl=str[i+3]-'0';
				i+=3;
			}else
				i++;
			/*float *v=(float*)&va_arg(marker,double);
			va_arg(marker,float);
			va_arg(marker,float);
			strcat(tmp,"( ");
			strcat(tmp,f2s(v[0],fl));
			strcat(tmp," , ");
			strcat(tmp,f2s(v[1],fl));
			strcat(tmp," , ");
			strcat(tmp,f2s(v[2],fl));
			strcat(tmp," )");
			l=strlen(tmp);*/
msg_write>Error("Todo:  %v");
		}else{
			tmp[l]=str[i];
			tmp[l+1]=0;
			l++;
		}
	}
	va_end(marker);

	return tmp;
#endif
}

// cut the string at the position of a substring
/*void strcut(char *str,const char *dstr)
{
	if (strstr(str,dstr))
		strstr(str,dstr)[0]=0;
}*/

// convert an integer to a string (with a given number of decimals)
string i2s2(int i,int l)
{
	string r;
	for (int n=l-1;n>=0;n--){
		r.add(i%10+48);
		i /= 10;
	}
	return r;
}

// convert an integer to a string
string i2s(int i)
{
	string r;	
	int l=0;
	bool m=false;
	if (i<0){
		i=-i;
		m=true;
	}
	char a[128];
	while (1){
		a[l]=(i%10)+48;
		l++;
		i=(int)(i/10);
		if (i==0)
			break;
	}
	if (m){
		a[l]='-';
		l++;
	}
	r.resize(l);
	for (int j=0;j<l;j++)
		r[l-j-1]=a[j];
	return r;
}

string i642s(long long i)
{
	string r;
	int l=0;
	bool m=false;
	if (i<0){
		i=-i;
		m=true;
	}
	char a[128];
	while (1){
		a[l]=(i%10)+48;
		l++;
		i=(long long)(i/10);
		if (i==0)
			break;
	}
	if (m){
		a[l]='-';
		l++;
	}
	r.resize(l);
	for (int j=0;j<l;j++)
		r[l-j-1]=a[j];
	return r;
}

// convert a float to a string
string f2s(float f,int dez)
{
	/*strcpy(str,"");
	if (f<0){
		strcat(str,"-");
		f=-f;
	}
	strcat(str,i2s(int(f)));
	if (dez>0){
		strcat(str,",");
		int e=1;
		for (int i=0;i<dez;i++)
			e*=10;
		strcat(str,i2sl(int(f*(float)e)%e,dez));
	}*/
	if (dez > 9)
		dez = 9;
	char tmp[128], fmt[8];
	strcpy(fmt, "%.0f");
	fmt[2] += dez;
	sprintf(tmp, fmt, f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a float to a string
string f642s(double f,int dez)
{
	/*strcpy(str,"");
	if (f<0){
		strcat(str,"-");
		f=-f;
	}
	strcat(str,i2s(int(f)));
	if (dez>0){
		strcat(str,",");
		int e=1;
		for (int i=0;i<dez;i++)
			e*=10;
		strcat(str,i2sl(int(f*(float)e)%e,dez));
	}*/
	if (dez > 9)
		dez = 9;
	char tmp[128], fmt[8];
	strcpy(fmt, "%.0f");
	fmt[2] += dez;
	sprintf(tmp, fmt, f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a float to a string
string f2sf(float f)
{
	char tmp[128];
	sprintf(tmp, "%f", f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a float to a string
string f642sf(double f)
{
	char tmp[128];
	sprintf(tmp, "%f", f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a bool to a string
string b2s(bool b)
{
	if (b)
		return string("true");
	return string("false");
}

// convert a pointer to a string
string p2s(void *p)
{
	char tmp[64];
	sprintf(tmp, "%p", p);
	return string(tmp);
}

// convert binary data to a hex-code-string
// inverted:
//    false:   12.34.56.78
//    true:    0x78.56.34.12
string string::hex(bool inverted) const
{
	string str;
	if (inverted)
		str = "0x";
	unsigned char *c_data = (unsigned char *)data;
	for (int i=0;i<num;i++){
		int dd;
		if (inverted)
			dd = c_data[num - i - 1];
		else
			dd = c_data[i];
		int c1 = (dd & 15);
		int c2 = (dd >> 4);
		if (c2 < 10)
			str.add('0' + c2);
		else
			str.add('a' + c2 - 10);
		if (c1 < 10)
			str.add('0' + c1);
		else
			str.add('a' + c1 - 10);
		if ((!inverted)&&(i < num - 1))
			str.add('.');
	}
	return str;
}

inline int hex_nibble_to_value(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	return 0;
}

string string::unhex() const
{
	string r;
	bool rev = ((*this)[1] == 'x');
	int i0 = rev ? 2 : 0;
	for (int i=i0; i<num;i++){
		if ((*this)[i] == '.')
			continue;
		int v1 = hex_nibble_to_value((*this)[i]);
		i ++;
		int v2 = hex_nibble_to_value((*this)[i]);
		r.add(v1 * 16 + v2);
	}
	if (rev)
		return r.reverse();
	return r;
}

string d2h(const void *data, int bytes, bool inverted)
{	return string((const char*)data, bytes).hex(inverted);	}

string ia2s(const Array<int> &a)
{
	string s = "[";
	for (int i=0;i<a.num;i++){
		if (i > 0)
			s += ", ";
		s += i2s(a[i]);
	}
	s += "]";
	return s;
}

string fa2s(const Array<float> &a)
{
	string s = "[";
	for (int i=0;i<a.num;i++){
		if (i > 0)
			s += ", ";
		s += f2s(a[i], 6);
	}
	s += "]";
	return s;
}
string ba2s(const Array<bool> &a)
{
	string s = "[";
	for (int i=0;i<a.num;i++){
		if (i > 0)
			s += ", ";
		s += b2s(a[i]);
	}
	s += "]";
	return s;
}

string sa2s(const Array<string> &a)
{
	string s = "[";
	for (int i=0;i<a.num;i++){
		if (i > 0)
			s += ", ";
		s += "\"" + a[i] + "\"";
	}
	s += "]";
	return s;
}

// convert a string to an integer
int string::_int() const
{
	bool minus=false;
	int res=0;
	for (int i=0;i<num;i++){
		if ((*this)[i]=='-')
			minus=true;
		else res=res*10+((*this)[i]-48);
	}
	if (minus)
		res=-res;
	return res;
}

long long string::i64() const
{
	bool minus=false;
	long long res=0;
	for (int i=0;i<num;i++){
		if ((*this)[i]=='-')
			minus=true;
		else res=res*10+((*this)[i]-48);
	}
	if (minus)
		res=-res;
	return res;
}

int s2i(const string &s)
{	return s._int();	}

// convert a string to a float
float string::_float() const
{
	bool minus=false;
	int e=-1;
	float res=0;
	for (int i=0;i<num;i++){
		if (e>0)
			e*=10;
		if ((*this)[i]=='-'){
			minus=true;
		}else{
			if (((*this)[i]==',')||((*this)[i]=='.')){
				e=1;
			}else{
				if((*this)[i]!='\n'){
					if (e<0)
						res=res*10+((*this)[i]-48);
					else
						res+=float((*this)[i]-48)/(float)e;
				}
			}
		}
	}
	if (minus)
		res=-res;
	return res;
}

// convert a string to a float
double string::f64() const
{
	bool minus=false;
	int e=-1;
	double res=0;
	for (int i=0;i<num;i++){
		if (e>0)
			e*=10;
		if ((*this)[i]=='-'){
			minus=true;
		}else{
			if (((*this)[i]==',')||((*this)[i]=='.')){
				e=1;
			}else{
				if((*this)[i]!='\n'){
					if (e<0)
						res=res*10+((*this)[i]-48);
					else
						res+=double((*this)[i]-48)/(double)e;
				}
			}
		}
	}
	if (minus)
		res=-res;
	return res;
}


float s2f(const string &s)
{	return s._float();	}

double s2f64(const string &s)
{	return s.f64();	}

bool string::_bool() const
{
	return (*this == "true") || (*this == "yes");
}

bool s2b(const string &s)
{	return s._bool();	}

int string::hash() const
{
	int id = 0;
	int n = num / 4;
	int *str_i = (int*)data;
	for (int i=0;i<n;i++){
		int t = str_i[i];
		id = id ^ t;
	}
	int r = num - (n * 4);
	int t = 0;
	if (r == 1)
		t = (str_i[n] & 0x000000ff);
	else if (r == 2)
		t = (str_i[n] & 0x0000ffff);
	else if (r == 3)
		t = (str_i[n] & 0x00ffffff);
	id = id ^ t;
	return id;
}

string string::trim() const
{
	int i0 = 0, i1 = num-1;
	for (i0=0;i0<num;i0++)
		if (((*this)[i0] != ' ') && ((*this)[i0] != '\t') && ((*this)[i0] != '\n') && ((*this)[i0] != '\r'))
			break;
	for (i1=num-1;i1>=0;i1--)
		if (((*this)[i1] != ' ') && ((*this)[i1] != '\t') && ((*this)[i1] != '\n') && ((*this)[i1] != '\r'))
			break;
	return substr(i0, i1 - i0 + 1);
}

string implode(const Array<string> &a, const string &glue)
{
	string r;
	for (int i=0;i<a.num;i++){
		if (i > 0)
			r += glue;
		r += a[i];
	}
	return r;
}

bool string::match(const string &glob) const
{
	Array<string> g = glob.explode("*");

	// no *'s -> direct match
	if (g.num < 2)
		return (*this == glob);

	if (head(g[0].num) != g[0])
		return false;
	int pos = g[0].num;
	for (int i=1; i<g.num-1; i++){
		pos = find(g[i], pos);
		if (pos < 0)
			return false;
		pos += g[i].num;
	}
	if (pos > num - g.back().num)
		return false;
	if (tail(g.back().num) != g.back())
		return false;
	return true;
}

int string::utf8len() const
{
	int l = 0;
	for (int i=0; i<num; i++)
		if (((*this)[i] & 0x80) == 0)
			l ++;
	return l;
}


string utf8_char(unsigned int code)
{
	char r[6] = "";
	if ((code & 0xffffff80) == 0){ // 7bit
		return string((char*)&code, 1);
	}else if ((code & 0xfffff800) == 0){ // 11bit
		r[1] = (code & 0x003f) | 0x80;        // 00-05
		r[0] = ((code & 0x07c0) >> 6) | 0xc0; // 06-10
		return string(r, 2);
	}else if ((code & 0xffff0000) == 0){ // 16bit
		r[2] = (code & 0x003f) | 0x80;         // 00-05
		r[1] = ((code & 0x0fc0) >> 6) | 0x80;  // 06-11
		r[0] = ((code & 0xf000) >> 12) | 0xe0; // 12-15
		return string(r, 3);
	}else if ((code & 0xffe00000) == 0){ // 21bit
		r[3] = (code & 0x0000003f) | 0x80;         // 00-05
		r[2] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[1] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[0] = ((code & 0x001c0000) >> 18) | 0xf0; // 18-20
		return string(r, 4);
	}else if ((code & 0xffe00000) == 0){ // 26bit
		r[4] = (code & 0x0000003f) | 0x80;         // 00-05
		r[3] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[2] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[1] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x03000000) >> 24) | 0xf4; // 24-25
		return string(r, 5);
	}else{ // 31bit
		r[5] = (code & 0x0000003f) | 0x80;         // 00-05
		r[4] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[3] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[2] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x3f000000) >> 24) | 0x80; // 24-29
		r[0] = ((code & 0x40000000) >> 30) | 0xfc; // 30
		return string(r, 6);
	}
}

Array<int> string::utf16_to_utf32() const
{
	Array<int> r;
	bool big_endian = false;
	unsigned int last = 0;
	for (int i=0; i<num-1; i+=2){
		if (((*this)[i] == 0xff) && ((*this)[i+1] == 0xfe)){
			big_endian = false;
			continue;
		}else if (((*this)[i] == 0xfe) && ((*this)[i+1] == 0xff)){
			big_endian = true;
			continue;
		}
		unsigned int code = (*this)[i] | ((*this)[i+1] << 8);
		if (big_endian)
			code = (*this)[i+1] | ((*this)[i] << 8);
		//msg_write(string((char*)&code, 2).hex());

		if ((code < 0xd800) || (code > 0xdbff))
			r.add(code);
		else if ((last >= 0xdc00) && (last <= 0xdfff))
			r.add(0x010000 | ((code - 0xd800) << 12) | (last - 0xdc00));
		last = code;
	}
	return r;
}

string string::utf16_to_utf8() const
{
	return utf32_to_utf8(utf16_to_utf32());
}

string string::latin_to_utf8() const
{
	string r;
	for (int i=0; i<num; i++)
		r += utf8_char((*this)[i]);
	return r;
}

string utf32_to_utf8(const Array<int> &s)
{
	string r;
	for (int i=0; i<s.num; i++)
		r += utf8_char(s[i]);
	return r;
}

/*
char *regex_out_match[REGEX_MAX_MATCHES];
int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];

int regex_match(char *rex,char *str,int max_matches)
{
	int ss=strlen(str);
	int rs=strlen(rex);

	if ((max_matches<=0)||(max_matches>REGEX_MAX_MATCHES))
		max_matches=REGEX_MAX_MATCHES;

	int n_matches=0;

	for (int i=0;i<ss;i++){
		bool match=true;
		for (int j=0;j<rs;j++){
			if (i+j>=ss)
				match=false;
			else if (str[i+j]!=rex[j])
				match=false;
		}
		if (match){
			regex_out_pos[n_matches]=i;
			regex_out_length[n_matches]=rs;
			regex_out_match[n_matches]=&str[i];
			n_matches++;
			if (n_matches>=max_matches)
				return n_matches;
		}
	}
	return n_matches;
}*/

/*char *regex_replace(char *rex,char *str,int max_matches)
{
}*/

