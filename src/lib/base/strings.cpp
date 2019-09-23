#include "base.h"


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <math.h>

#ifdef OS_WINDOWS
	#include <stdio.h>
	#include <io.h>
	#include <direct.h>
	#include <stdarg.h>
	#include <windows.h>
	#include <winbase.h>
	#include <winnt.h>
#endif
#if defined(OS_LINUX) || defined(OS_MINGW)
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
	simple_assign(&s);
}

string::string(string &&s)
{
	init(sizeof(unsigned char));
	exchange(s);
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
{ size = min(size, num); return substr(0, size); }

string string::tail(int size) const
{ size = min(size, num); return substr(num - size, size); }

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
	if ((a >= 'A') and (a <= 'Z'))
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
	if ((r.num > 0) or (pos < num))
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
		if ((r[i] >= 'A') and (r[i] <= 'Z'))
			r[i] += 'a' - 'A';
	return r;
}

string string::upper() const
{
	string r = *this;
	for (int i=0;i<r.num;i++)
		if ((r[i] >= 'a') and (r[i] <= 'z'))
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
#if defined(OS_WINDOWS) || defined(OS_MINGW)
	return replace("/", "\\");
#else
	return replace("\\", "/");
#endif
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
		if ((lc != '/') and (lc != '\\'))
			add('/');
	}
}

// remove "/../"
string string::no_recursion() const
{
	string str = replace("\\", "/");
	Array<string> p = str.explode("/");

	for (int i=1;i<p.num;i++)
		if ((p[i] == "..") and (p[i-1] != "..")){
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
string format(const string &str,...)
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
	int len = vsnprintf(nullptr, 0, str.c_str(), args);
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
		if ((str[i]=='%')and(str[i+1]=='s')){
			strcat(tmp,va_arg(marker,char*));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')and(str[i+1]=='d')){
			strcat(tmp,i2s(va_arg(marker,int)));
			i++;
			l=strlen(tmp);
		}else if ((str[i]=='%')and(str[i+1]=='f')){
			int fl=3;
			if (str[i+2]==':'){
				fl=str[i+3]-'0';
				i+=3;
			}else
				i++;
			strcat(tmp,f2s((float)va_arg(marker,double),fl));
			l=strlen(tmp);
		}else if ((str[i]=='%')and(str[i+1]=='v')){
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
string p2s(const void *p)
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
		if ((!inverted)and(i < num - 1))
			str.add('.');
	}
	return str;
}

inline int hex_nibble_to_value(char c)
{
	if ((c >= '0') and (c <= '9'))
		return c - '0';
	if ((c >= 'a') and (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') and (c <= 'F'))
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
	bool minus = false;
	int res = 0;
	for (int i=0; i<num; i++){
		char c = (*this)[i];
		if ((c == '-') and (i == 0))
			minus = true;
		else if ((c >= '0') and (c <= '9'))
			res = res * 10 + (c - 48);
	}
	if (minus)
		res = -res;
	return res;
}

long long string::i64() const
{
	bool minus = false;
	long long res = 0;
	for (int i=0; i<num; i++){
		char c = (*this)[i];
		if ((c == '-') and (i == 0))
			minus = true;
		else if ((c >= '0') and (c <= '9'))
			res = res * 10 + (c - 48);
	}
	if (minus)
		res = -res;
	return res;
}

int s2i(const string &s)
{	return s._int();	}

// convert a string to a float
float string::_float() const
{
	return (float)f64();
}

// convert a string to a float
double string::f64() const
{
	bool minus = false;
	int e = -1;
	double res = 0;
	for (int i=0; i<num; i++){
		if (e > 0)
			e *= 10;
		char c = (*this)[i];
		if ((c =='-') and (i == 0)){
			minus = true;
		}else if ((c == ',') or (c == '.')){
			e = 1;
		}else if ((c >= '0') and (c <= '9')){
			if (e < 0)
				res = res * 10 + (c-48);
			else
				res += float(c-48) / (float)e;
		}else if ((c == 'e') or (c == 'E')){
			int ex = substr(i+1, -1)._int();
			res *= pow(10.0, ex);
			break;
		}
	}
	if (minus)
		res = -res;
	return res;
}


float s2f(const string &s)
{	return s._float();	}

double s2f64(const string &s)
{	return s.f64();	}

bool string::_bool() const
{
	return (*this == "true") or (*this == "yes");
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

static const unsigned int md5_s[64] = {
	7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
	5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
	4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
	6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};
static const unsigned int md5_K[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

string string::md5() const
{
	unsigned int a0 = 0x67452301;
	unsigned int b0 = 0xEFCDAB89;
	unsigned int c0 = 0x98BADCFE;
	unsigned int d0 = 0x10325476;
	unsigned long long message_length = num * 8;
	string message = *this;
	message.add(0x80);
	while((message.num % 64) != 56)
		message.add(0x00);
	message += string(&message_length, 8);

	// blocks of 512 bit = 64 byte = 16 uints
	int n = message.num / 64;
	unsigned int *M = (unsigned int*)&message[0];

	for (int j=0; j<n; j++){

		unsigned int A = a0;
		unsigned int B = b0;
		unsigned int C = c0;
		unsigned int D = d0;

		unsigned int F, g;
		for (int i=0; i<64; i++){
			if (i < 16){
	            F = (B & C) | ((~B) & D);
	            g = i;
			}else if (i < 32){
	            F = (B & D) | (C & (~ D));
	            g = (5*i + 1) % 16;
			}else if (i < 48){
	            F = B ^ C ^ D;
	            g = (3*i + 5) % 16;
			}else{
	            F = C ^ (B | (~ D));
	            g = (7*i) % 16;
			}
	        unsigned int temp = D;
	        D = C;
	        C = B;
	        unsigned int t = A + F + md5_K[i] + M[g];
	        unsigned int shift = md5_s[i];
	        B = B + ((t << shift) | (t >> (32 - shift)));
	        A = temp;
		}
	    a0 = a0 + A;
	    b0 = b0 + B;
	    c0 = c0 + C;
	    d0 = d0 + D;
		M += 16;
	}
	string out;
	out += string(&a0, 4) + string(&b0, 4) + string(&c0, 4) + string(&d0, 4);

	return out.hex().replace(".", "");
}

string string::trim() const
{
	int i0 = 0, i1 = num-1;
	for (i0=0;i0<num;i0++)
		if (((*this)[i0] != ' ') and ((*this)[i0] != '\t') and ((*this)[i0] != '\n') and ((*this)[i0] != '\r'))
			break;
	for (i1=num-1;i1>=0;i1--)
		if (((*this)[i1] != ' ') and ((*this)[i1] != '\t') and ((*this)[i1] != '\n') and ((*this)[i1] != '\r'))
			break;
	return substr(i0, i1 - i0 + 1);
}

string string::escape() const {
	return str_escape(*this);
}

string string::unescape() const {
	return str_unescape(*this);
}

string string::repr() const {
	return "\"" + escape() + "\"";
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
		if (((*this)[i] == 0xff) and ((*this)[i+1] == 0xfe)){
			big_endian = false;
			continue;
		}else if (((*this)[i] == 0xfe) and ((*this)[i+1] == 0xff)){
			big_endian = true;
			continue;
		}
		unsigned int code = (*this)[i] | ((*this)[i+1] << 8);
		if (big_endian)
			code = (*this)[i+1] | ((*this)[i] << 8);
		//msg_write(string((char*)&code, 2).hex());

		if ((code < 0xd800) or (code > 0xdbff))
			r.add(code);
		else if ((last >= 0xdc00) and (last <= 0xdfff))
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

// LAZY... can only convert small code points (13bit)!!!!!
Array<int> string::utf8_to_utf32() const
{
	Array<int> r;
	for (int i=0; i<num; i++)
		if ((((unsigned int)(*this)[i] & 0x80) > 0) and (i < (*this).num - 1)){
			r.add((((*this)[i] & 0x1f) << 6) + ((*this)[i + 1] & 0x3f));
			i ++;
		}else
			r.add((*this)[i]);
	return r;
}



string str_unescape(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if ((str[i]=='\\')and(str[i+1]=='n')){
			r += "\n";
			i ++;
		}else if ((str[i]=='\\')and(str[i+1]=='\\')){
			r += "\\";
			i++;
		}else if ((str[i]=='\\')and(str[i+1]=='?')){
			r += "?";
			i++;
		}else if ((str[i]=='\\')and(str[i+1]=='t')){
			r += "\t";
			i++;
		}else if ((str[i]=='\\')and(str[i+1]=='"')){
			r += "\"";
			i++;
		}else
			r.add(str[i]);
	}
	return r;
}


string str_escape(const string &str)
{
	//return str.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\"", "\\\"");
	string r;
	for (int i=0;i<str.num;i++){
		if (str[i] == '\t')
			r += "\\t";
		else if (str[i] == '\n')
			r += "\\n";
		else if (str[i] == '\\')
			r += "\\\\";
		else if (str[i] == '\"')
			r += "\\\"";
		else
			r.add(str[i]);
	}
	return r;
}

string str_m_to_utf8(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if ((str[i] == '&') and (i < str.num - 1)){
			if (str[i+1]=='a'){
				r.add(0xc3);
				r.add(0xa4);
			}else if (str[i+1]=='o'){
				r.add(0xc3);
				r.add(0xb6);
			}else if (str[i+1]=='u'){
				r.add(0xc3);
				r.add(0xbc);
			}else if (str[i+1]=='s'){
				r.add(0xc3);
				r.add(0x9f);
			}else if (str[i+1]=='A'){
				r.add(0xc3);
				r.add(0x84);
			}else if (str[i+1]=='O'){
				r.add(0xc3);
				r.add(0x96);
			}else if (str[i+1]=='U'){
				r.add(0xc3);
				r.add(0x9c);
			}else if (str[i+1]=='&'){
				r.add('&');
			}else{
				r.add(str[i]);
				i --;
			}
			i ++;
		}else
			r.add(str[i]);
	}
	return r;
}

// Umlaute zu Vokalen mit & davor zerlegen
string str_utf8_to_m(const string &str)
{
	string r;
	const unsigned char *us = (const unsigned char*)str.c_str();

	for (int i=0;i<str.num;i++){
		if ((us[i]==0xc3) and (us[i+1]==0xa4)){
			r += "&a";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0xb6)){
			r += "&o";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0xbc)){
			r += "&u";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0x9f)){
			r += "&s";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0x84)){
			r += "&A";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0x96)){
			r += "&O";
			i ++;
		}else if ((us[i]==0xc3) and (us[i+1]==0x9c)){
			r += "&U";
			i ++;
		}else if (us[i]=='&'){
			r += "&&";
		}else
			r.add(str[i]);
	}
	return r;
}

bool sa_contains(const Array<string> &a, const string &s)
{
	for (string &aa: a)
		if (aa == s)
			return true;
	return false;
}

/*
char *regex_out_match[REGEX_MAX_MATCHES];
int regex_out_pos[REGEX_MAX_MATCHES],regex_out_length[REGEX_MAX_MATCHES];

int regex_match(char *rex,char *str,int max_matches)
{
	int ss=strlen(str);
	int rs=strlen(rex);

	if ((max_matches<=0)or(max_matches>REGEX_MAX_MATCHES))
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


