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

bytes bytes_repeat(const bytes &s, int n);
string str_unescape(const string &str);
string str_escape(const string &str);






bytes::bytes() {
	init(sizeof(unsigned char));
}

bytes::bytes(const char *str) {
	init(sizeof(unsigned char));
	if (str) {
		int l = strlen(str);
		resize(l);
		if (l > 0)
			memcpy(data, str, l);
	}
}

bytes::bytes(const void *str, int l) {
	init(sizeof(unsigned char));
	resize(l);
	if (l > 0)
		memcpy(data, str, l);
}

bytes::bytes(const bytes &s) {
	init(sizeof(unsigned char));
	simple_assign(&s);
}

bytes::bytes(bytes &&s) {
	init(sizeof(unsigned char));
	exchange(s);
	s.clear();
}

void bytes::__init__() {
	new(this) bytes;
}

bytes::~bytes() {
	clear();
}

void bytes::operator = (bytes &&s) {
	exchange(s);
	s.clear();
}

bool bytes::operator == (const bytes &s) const {
	if (num != s.num)
		return false;
	return compare(s) == 0;
}

int bytes::compare(const bytes &s) const {
	auto a = (unsigned char*)data;
	auto b = (unsigned char*)s.data;
	int n = min(num, s.num);
	for (int i=0; i<n; i++) {
		if (*a != *b)
			return (int)*a - (int)*b;
		a ++;
		b ++;
	}
	return num - s.num;
}

char hex_nibble(int v) {
	if (v < 10)
		return '0' + v;
	return 'a' + v - 10;
}

// convert binary data to a hex-code-string
// inverted:
//    false:   12.34.56.78
//    true:    78563412
string _bytes_hex_(const bytes &s, bool inverted) {
	string str;
	//if (inverted)
	//	str = "0x";
	unsigned char *c_data = (unsigned char *)s.data;
	for (int i=0;i<s.num;i++) {
		int dd;
		if (inverted)
			dd = c_data[s.num - i - 1];
		else
			dd = c_data[i];
		int c1 = (dd & 15);
		int c2 = (dd >> 4);
		str.add(hex_nibble(c2));
		str.add(hex_nibble(c1));
		if ((!inverted) and (i < s.num - 1))
			str.add('.');
	}
	return str;
}

string bytes::hex() const {
	return _bytes_hex_(*this, false);
}

int bytes::hash() const {
	int id = 0;
	int n = num / 4;
	int *str_i = (int*)data;
	for (int i=0;i<n;i++) {
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

string bytes::md5() const {
	unsigned int a0 = 0x67452301;
	unsigned int b0 = 0xEFCDAB89;
	unsigned int c0 = 0x98BADCFE;
	unsigned int d0 = 0x10325476;
	unsigned long long message_length = num * 8;
	bytes message = *this;
	message.add(0x80);
	while ((message.num % 64) != 56)
		message.add(0x00);
	message += bytes(&message_length, 8);

	// blocks of 512 bit = 64 byte = 16 uints
	int n = message.num / 64;
	unsigned int *M = (unsigned int*)&message[0];

	for (int j=0; j<n; j++) {

		unsigned int A = a0;
		unsigned int B = b0;
		unsigned int C = c0;
		unsigned int D = d0;

		unsigned int F, g;
		for (int i=0; i<64; i++) {
			if (i < 16) {
	            F = (B & C) | ((~B) & D);
	            g = i;
			} else if (i < 32) {
	            F = (B & D) | (C & (~ D));
	            g = (5*i + 1) % 16;
			} else if (i < 48) {
	            F = B ^ C ^ D;
	            g = (3*i + 5) % 16;
			} else {
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

bytes bytes_repeat(const bytes &s, int n) {
	bytes r;
	for (int i=0; i<n; i++)
		r += s;
	return r;
}

bytes bytes::repeat(int n) const {
	return bytes_repeat(*this, n);
}

bytes _cdecl bytes::sub_ref(int start, int end) const {
	return sub_ref_as<bytes>(start, end);
}

bytes _cdecl bytes::sub(int start, int end) const {
	auto r = sub_ref(start, end);
	r.make_own();
	return r;
}










string::string() : bytes() {}

string::string(const char *str) : bytes(str) {}

string::string(const void *str, int l) : bytes(str, l) {}

#if __cpp_char8_t
string::string(const char8_t *str) : bytes((const char*)str) {}
#endif

string::string(const string &s) {
	init(sizeof(unsigned char));
	simple_assign(&s);
}

string::string(const bytes &s) {
	init(sizeof(unsigned char));
	simple_assign(&s);
}

string::string(string &&s) {
	init(sizeof(unsigned char));
	exchange(s);
}

string::string(bytes &&s) {
	init(sizeof(unsigned char));
	exchange(s);
}

bool string::operator == (const string &s) const {
	if (num != s.num)
		return false;
	return compare(s) == 0;
}

string string::sub_ref(int start, int end) const {
	/*if (start < 0)
		start += num;
	if (end == MAGIC_END_INDEX)
		end = num;
	else if (end < 0)
		end += num;
	int new_num = 0;
	if (end >= start)
		new_num = end - start;*/

	return sub_ref_as<string>(start, end);
}

string string::sub(int start, int end) const {
	string r = sub_ref(start, end);
	r.make_own();
	return r;

}

string string::head(int size) const {
	//printf("HEAD\n");
	size = min(size, num);
	return sub(0, size);
}

string string::tail(int size) const {
	if (size == 0)
		return "";
	size = min(size, num);
	return sub(- size);
}

int string::find(const string &s, int start) const {
	unsigned char *b = (unsigned char*)data;
	unsigned char *aa = (unsigned char*)s.data;
	for (int i=start;i<num - s.num + 1;i++) {
		bool ok = true;
		for (int j=0;j<s.num;j++)
			if (b[i + j] != aa[j]) {
				ok = false;
				break;
			}
		if (ok)
			return i;
	}
	return -1;
}

int string::rfind(const string &s, int start) const {
	unsigned char *b = (unsigned char*)data;
	unsigned char *aa = (unsigned char*)s.data;
	if (start < 0)
		start = num - 1;
	for (int i=start;i>=0;i--) {
		bool ok = true;
		for (int j=0;j<s.num;j++)
			if (b[i + j] != aa[j]) {
				ok = false;
				break;
			}
		if (ok)
			return i;
	}
	return -1;
}

int string::icompare(const string &s) const {
	return lower().compare(s.lower());
}

string string::reverse() const {
	string r;
	r.resize(num);
	auto a = (unsigned char*)data;
	auto b = (unsigned char*)r.data;
	for (int i=0; i<num; i++)
		b[num - i - 1] = a[i];
	return r;
}

Array<string> string::explode(const string &s) const {
	Array<string> r;

	int pos = 0;
	while (true) {
		int pos2 = find(s, pos);
		if (pos2 < 0)
			break;

		r.add(sub(pos, pos2));

		pos = s.num + pos2;
	}
	if ((r.num > 0) or (pos < num))
		r.add(sub(pos));
	return r;
}


void string::_replace0(int start, int length, const string &str) {
	if (start + length > num)
		return;
	unsigned char *s = (unsigned char*)data;
	int d = str.num - length;
	if (d > 0) {
		resize(num + d);
		s = (unsigned char*)data;
		for (int i=num-1; i>=start+str.num; i--)
			s[i] = s[i - d];
	}
	memcpy(&s[start], str.data, str.num);
	if (d < 0) {
		for (int i=start+str.num; i<num+d; i++)
			s[i] = s[i - d];
		resize(num + d);
	}
}

string string::replace(const string &sub, const string &by) const {
	string r = *this;
	int i = r.find(sub, 0);
	while (i >= 0) {
		r._replace0(i, sub.num, by);
		i = r.find(sub, i + by.num);
	}
	return r;
}

string string::lower() const {
	string r = *this;
	for (int i=0;i<r.num;i++)
		if ((r[i] >= 'A') and (r[i] <= 'Z'))
			r[i] += 'a' - 'A';
	return r;
}

string string::upper() const {
	string r = *this;
	for (int i=0; i<r.num; i++)
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
inline char *get_str(int size) {
	if (size < STR_SMALL_SIZE) {
		_current_stack_small_pos_ = (_current_stack_small_pos_ + 1) % STR_SMALL_STACK_DEPTH;
		return _stack_small_str_[_current_stack_small_pos_];
	} else {
		_current_stack_large_pos_ = (_current_stack_large_pos_ + 1) % STR_LARGE_STACK_DEPTH;
		if (_stack_large_str_[_current_stack_large_pos_])
			delete[]_stack_large_str_[_current_stack_large_pos_];
		_stack_large_str_[_current_stack_large_pos_] = new char[size + 1];
		return _stack_large_str_[_current_stack_large_pos_];
	}
}


const char *string::c_str() const {
	if (allocated > num) {
		((char*)data)[num] = 0;
		return (const char*)data;
	}
	char *s = get_str(num);
	if (num > 0)
		memcpy(s, data, num);
	s[num] = 0;
	return s;
}




// convert an integer to a string (with a given number of decimals)
string i2s2(int i, int l) {
	string r;
	for (int n=l-1; n>=0; n--) {
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
	if (i<0) {
		i=-i;
		m=true;
	}
	char a[128];
	while (1) {
		a[l]=(i%10)+48;
		l++;
		i=(int)(i/10);
		if (i==0)
			break;
	}
	if (m) {
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
	if (i<0) {
		i=-i;
		m=true;
	}
	char a[128];
	while (1) {
		a[l]=(i%10)+48;
		l++;
		i=(long long)(i/10);
		if (i==0)
			break;
	}
	if (m) {
		a[l]='-';
		l++;
	}
	r.resize(l);
	for (int j=0;j<l;j++)
		r[l-j-1]=a[j];
	return r;
}

// convert a float to a string
string f2s(float f, int dez) {
	/*strcpy(str,"");
	if (f<0) {
		strcat(str,"-");
		f=-f;
	}
	strcat(str,i2s(int(f)));
	if (dez>0) {
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

string f2s_clean(float f, int dez) {
	auto s = f2s(f, dez);
	for (int i=s.num-1; i>=0; i--)
		if (s.back() == '0' and s[s.num-2] != '.')
			s.pop();
	return s;
}

// convert a float to a string
string f642s(double f, int dez) {
	/*strcpy(str,"");
	if (f<0) {
		strcat(str,"-");
		f=-f;
	}
	strcat(str,i2s(int(f)));
	if (dez>0) {
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
string f2sf(float f) {
	char tmp[128];
	sprintf(tmp, "%f", f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a float to a string
string f642sf(double f) {
	char tmp[128];
	sprintf(tmp, "%f", f);
	string t = string(tmp);
	for (int i=0;i<t.num;i++)
		if (t[i] == ',')
			t[i] = '.';
	return t;
}

// convert a bool to a string
string b2s(bool b) {
	if (b)
		return "true";
	return "false";
}

// convert a pointer to a string
string p2s(const void *p) {
	if (!p)
		return "(nil)";
	char tmp[64];
#ifdef OS_WINDOWS
	sprintf(tmp, "0x%p", p);
#else
	sprintf(tmp, "%p", p);
#endif
	return string(tmp);
}

inline int hex_nibble_to_value(char c) {
	if ((c >= '0') and (c <= '9'))
		return c - '0';
	if ((c >= 'a') and (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') and (c <= 'F'))
		return c - 'A' + 10;
	return 0;
}

string string::unhex() const {
	string r;
	for (int i=0; i<num;i++) {
		if ((*this)[i] == '.')
			continue;
		int v1 = hex_nibble_to_value((*this)[i]);
		i ++;
		int v2 = hex_nibble_to_value((*this)[i]);
		r.add(v1 * 16 + v2);
	}
	return r;
}

string d2h(const void *data, int num_bytes)
{	return bytes((const char*)data, num_bytes).hex();	}

string i2h(int64 data, int num_bytes)
{	return _bytes_hex_(bytes((const char*)&data, num_bytes), true);	}

string i2h_min(int64 data) {
	string r;
	do {
		r.add(hex_nibble(data & 0xf));
		data >>= 4;
	} while (data != 0);
	return r.reverse();
}

#define MAKE_ARRAY_STR(NAME, T, F) \
string NAME(const Array<T> &a) { \
	string s = "["; \
	for (int i=0; i<a.num; i++) { \
		if (i > 0) \
			s += ", "; \
		s += F(a[i]); \
	} \
	s += "]"; \
	return s; \
}

string str_quote(const string &s) { return s.repr(); }

MAKE_ARRAY_STR(ia2s, int, i2s);
MAKE_ARRAY_STR(fa2s, float, f2sf);
MAKE_ARRAY_STR(f64a2s, double, f642sf);
MAKE_ARRAY_STR(ba2s, bool, b2s);
MAKE_ARRAY_STR(sa2s, string, str_quote);


template<> string str(const int& i) {
	return i2s(i);
}
template<> string str(const unsigned int& i) {
	return i2s(i);
}
template<> string str(const int64& i) {
	return i642s(i);
}
template<> string str(const float& f) {
	return f2sf(f);
}
template<> string str(const double& d) {
	return f642sf(d);
}
template<> string str(const bool& b) {
	return b2s(b);
}
template<> string str(const Array<string> &a) {
	return sa2s(a);
}


struct xf_format_data {
	bool sign, left_justify, fill_zeros, sharp;
	int width;
	int decimals;
	char type;
	xf_format_data() {
		sign = false;
		left_justify = false;
		fill_zeros = false;
		sharp = false;
		type = 0;
		width = 0;
		decimals = -1;
	}
	string apply_justify(const string &s) {
		if (width <= 0 or fill_zeros)
			return s;
		int len = s.utf8len();
		if (left_justify)
			return s + bytes_repeat(" ", width - len);
		else
			return bytes_repeat(" ", width - len) + s;
	}
};

xf_format_data xf_parse(const string &f) {
	xf_format_data r;
	r.type = f.back();
	int i0 = 0;
	// flags
	for (int i=0; i<f.num; i++) {
		i0 = i;
		if (f[i] == '+')
			r.sign = true;
		else if (f[i] == '-')
			r.left_justify = true;
		else if (f[i] == '0')
			r.fill_zeros = true;
		else if (f[i] == '#')
			r.sharp = true;
		else
			break;
	}
	if (f[i0] >= '0' and f[i0] <= '9') {
		r.width = f[i0] - '0';
		i0 ++;
		if (f[i0] >= '0' and f[i0] <= '9') {
			r.width = r.width * 10 + (f[i0] - '0');
			i0 ++;
		}
	}
	if (f[i0] == '.') {
		i0 ++;
		if (f[i0] >= '0' and f[i0] <= '9') {
			r.decimals = f[i0] - '0';
			i0 ++;
		}
	}
	if (i0 < f.num-1)
		throw Exception("format evil: " + f);
	return r;
}

template<> string _xf_str_(const string &f, int64 value) {
	auto ff = xf_parse(f);
	bool negative = false;
	if (value < 0) {
		negative = true;
		value = - value;
	}
	string s;
	if (ff.type == 'd') {
		s = i642s(value);
	} else if (ff.type == 'x') {
		s = i2h_min(value);
	} else {
		throw Exception("format evil (int): " + f);
	}
		
	int size = s.num;
	if (negative or ff.sign)
		size ++;
	if (ff.type == 'x' and ff.sharp)
		size += 2;
	
	int n_zeros = 0;
	if (ff.fill_zeros)
		n_zeros = ff.width;
	if (ff.decimals >= 0)
		n_zeros = ff.decimals;
	s = bytes_repeat("0", n_zeros - size) + s;
	if (ff.type == 'x' and ff.sharp)
		s = "0x" + s;

	// sign
	if (negative)
		s = "-" + s;
	else if (ff.sign)
		s = "+" + s;

	return ff.apply_justify(s);
}

template<> string _xf_str_<double>(const string &f, double value) {
	auto ff = xf_parse(f);
	string s;
	int decimals = 6;
	if (ff.decimals >= 0)
		decimals = ff.decimals;
		
	if (ff.type == 'f') {
		s = f642s(value, decimals);
	} else {
		throw Exception("format evil (float): " + f);
	}
	return ff.apply_justify(s);
}

template<> string _xf_str_(const string &f, const string &value) {
	auto ff = xf_parse(f);
	if (ff.type == 's') {
	} else {
		throw Exception("format evil (string): " + f);
	}
	return ff.apply_justify(value);
}

template<> string _xf_str_(const string &f, const char *const value) {
	if (f != "s")
		throw Exception("format evil (string): " + f);
	return value;
}

template<> string _xf_str_(const string &f, bool value) { return _xf_str_(f, (int64)value); }
template<> string _xf_str_(const string &f, int value) { return _xf_str_(f, (int64)value); }
//template<> string _xf_str_(const string &f, long value) { return _xf_str_(f, (int64)value); }
template<> string _xf_str_(const string &f, unsigned int value) { return _xf_str_(f, (int64)value); }
template<> string _xf_str_(const string &f, unsigned long long value) { return _xf_str_(f, (int64)value); }
//template<> string _xf_str_(const string &f, unsigned long value) { return _xf_str_(f, (int64)value); }
template<> string _xf_str_(const string &f, float value) { return _xf_str_(f, (double)value); }
template<> string _xf_str_(const string &f, string value) { return _xf_str_<const string&>(f, value); }
template<> string _xf_str_(const string& f, char* const value) { return _xf_str_<const char*>(f, value); }

// %lx

string format(const string &s) {
	return s.replace("%%", "%");
}


bool _xf_split_first_(const string &s, string &pre, string &f, string &post) {
	for (int i=0; i<s.num-1; i++) {
		if (s[i] == '%') {
			if (s[i+1] == '%') {
				i ++;
				continue;
			}
			for (int j=i+1; j<s.num; j++) {
				if ((s[j] == '.') or (s[j] == '+') or (s[j] == '-') or (s[j] >= '0' and s[j] <= '9')) {
					f.add(s[j]);
				} else if ((s[j] == 's') or (s[j] == 'f') or (s[j] == 'd') or (s[j] == 'x') or (s[j] == 'p')) {
					f.add(s[j]);
					break;
				} else if (s[j] == 'l') {
				} else {
					throw Exception("xformat: evil format");
				}
			}
			pre = s.head(i).replace("%%", "%");
			post = s.sub(i + f.num + 1);
			return true;
		}
	}
	pre = s.replace("%%", "%");
	return false;
}




// convert a string to an integer
int string::_int() const {
	bool minus = false;
	int res = 0;
	for (int i=0; i<num; i++) {
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

int64 string::i64() const {
	bool minus = false;
	int64 res = 0;
	for (int i=0; i<num; i++) {
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

int s2i(const string &s) {
	return s._int();
}

// convert a string to a float
float string::_float() const {
	return (float)f64();
}

// convert a string to a float
double string::f64() const {
	bool minus = false;
	int e = -1;
	double res = 0;
	for (int i=0; i<num; i++) {
		if (e > 0)
			e *= 10;
		char c = (*this)[i];
		if ((c =='-') and (i == 0)) {
			minus = true;
		} else if ((c == ',') or (c == '.')) {
			e = 1;
		} else if ((c >= '0') and (c <= '9')) {
			if (e < 0)
				res = res * 10 + (c-48);
			else
				res += float(c-48) / (float)e;
		} else if ((c == 'e') or (c == 'E')) {
			int ex = sub(i+1)._int();
			res *= pow(10.0, ex);
			break;
		}
	}
	if (minus)
		res = -res;
	return res;
}


float s2f(const string &s) {
	return s._float();
}

double s2f64(const string &s) {
	return s.f64();
}

bool string::_bool() const {
	return (*this == "true") or (*this == "yes");
}

bool s2b(const string &s) {
	return s._bool();
}



bool str_is_integer(const string &s) {
	for (char c: s)
		if ((c < '0' or c > '9') and (c != '-'))
			return false;
	return true;
}

bool str_is_float(const string &s) {
	for (char c: s)
		if ((c < '0' or c > '9') and (c != '-') and (c != '.'))
			return false;
	return true;
}

bool is_whitespace_x(char c) {
	return ((c == ' ') or (c == '\t') or (c == '\n') or (c == '\r') or (c == '\0'));
}

string string::trim() const {
	int i0 = 0, i1 = num-1;
	for (i0=0; i0<num; i0++)
		if (!is_whitespace_x((*this)[i0]))
			break;
	for (i1=num-1; i1>=0; i1--)
		if (!is_whitespace_x((*this)[i1]))
			break;
	return sub(i0, i1 + 1);
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

string string::repeat(int n) const {
	return bytes::repeat(n);
}

string implode(const Array<string> &a, const string &glue) {
	string r;
	for (int i=0;i<a.num;i++) {
		if (i > 0)
			r += glue;
		r += a[i];
	}
	return r;
}

bool string::match(const string &glob) const {
	auto g = glob.explode("*");

	// no *'s -> direct match
	if (g.num < 2)
		return (*this == glob);

	if (head(g[0].num) != g[0])
		return false;
	int pos = g[0].num;
	for (int i=1; i<g.num-1; i++) {
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

int string::utf8len() const {
	int l = 0;
	for (int i=0; i<num; i++) {
		if (((*this)[i] & 0x80) == 0) // ASCII
			l ++;
		else if (((*this)[i] & 0xc0) == 0xc0) // begin multi-byte
			l ++;
	}
	return l;
}


string utf8_char(unsigned int code) {
	char r[6] = "";
	if ((code & 0xffffff80) == 0) { // 7bit
		return bytes((char*)&code, 1);
	} else if ((code & 0xfffff800) == 0) { // 11bit
		r[1] = (code & 0x003f) | 0x80;        // 00-05
		r[0] = ((code & 0x07c0) >> 6) | 0xc0; // 06-10
		return bytes(r, 2);
	} else if ((code & 0xffff0000) == 0) { // 16bit
		r[2] = (code & 0x003f) | 0x80;         // 00-05
		r[1] = ((code & 0x0fc0) >> 6) | 0x80;  // 06-11
		r[0] = ((code & 0xf000) >> 12) | 0xe0; // 12-15
		return bytes(r, 3);
	} else if ((code & 0xffe00000) == 0) { // 21bit
		r[3] = (code & 0x0000003f) | 0x80;         // 00-05
		r[2] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[1] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[0] = ((code & 0x001c0000) >> 18) | 0xf0; // 18-20
		return bytes(r, 4);
	} else if ((code & 0xffe00000) == 0) { // 26bit
		r[4] = (code & 0x0000003f) | 0x80;         // 00-05
		r[3] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[2] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[1] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x03000000) >> 24) | 0xf4; // 24-25
		return bytes(r, 5);
	} else { // 31bit
		r[5] = (code & 0x0000003f) | 0x80;         // 00-05
		r[4] = ((code & 0x00000fc0) >> 6) | 0x80;  // 06-11
		r[3] = ((code & 0x0003f000) >> 12) | 0x80; // 12-17
		r[2] = ((code & 0x00fc0000) >> 18) | 0x80; // 18-23
		r[1] = ((code & 0x3f000000) >> 24) | 0x80; // 24-29
		r[0] = ((code & 0x40000000) >> 30) | 0xfc; // 30
		return bytes(r, 6);
	}
}

Array<int> string::utf16_to_utf32() const {
	Array<int> r;
	bool big_endian = false;
	unsigned int last = 0;
	for (int i=0; i<num-1; i+=2) {
		if (((*this)[i] == 0xff) and ((*this)[i+1] == 0xfe)) {
			big_endian = false;
			continue;
		} else if (((*this)[i] == 0xfe) and ((*this)[i+1] == 0xff)) {
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

string string::utf16_to_utf8() const {
	return utf32_to_utf8(utf16_to_utf32());
}

string string::latin_to_utf8() const {
	string r;
	for (int i=0; i<num; i++)
		r += utf8_char((*this)[i]);
	return r;
}

string utf32_to_utf8(const Array<int> &s) {
	string r;
	for (int i=0; i<s.num; i++)
		r += utf8_char(s[i]);
	return r;
}

// LAZY... can only convert small code points (13bit)!!!!!
Array<int> string::utf8_to_utf32() const {
	Array<int> r;
	for (int i=0; i<num; i++)
		if ((((unsigned int)(*this)[i] & 0xf0) == 0xe0) and (i < (*this).num - 2)) {
			r.add((((*this)[i] & 0x0f) << 12) + (((*this)[i + 1] & 0x3f) << 6) + ((*this)[i + 2] & 0x3f));
			i += 2;
		} else if ((((unsigned int)(*this)[i] & 0xe0) == 0xc0) and (i < (*this).num - 1)) {
			r.add((((*this)[i] & 0x1f) << 6) + ((*this)[i + 1] & 0x3f));
			i ++;
		} else {
			r.add((*this)[i]);
		}
	return r;
}



string str_unescape(const string &str) {
	string r;
	for (int i=0;i<str.num;i++) {
		if ((str[i] == '\\') and (i < str.num-1)) {
			if (str[i+1] == 'n') {
				r.add('\n');
				i ++;
			} else if (str[i+1] == 'r') {
				r.add('\r');
				i ++;
			} else if (str[i+1] == '0') {
				r.add(0);
				i ++;
			} else if (str[i+1] == '\\') {
				r.add('\\');
				i ++;
			} else if (str[i+1] == '?') {
				r.add('?');
				i ++;
			} else if (str[i+1] == 't') {
				r.add('\t');
				i ++;
			} else if (str[i+1] == '"') {
				r.add('"');
				i ++;
			} else if (str[i+1] == '\'') {
				r.add('\'');
				i ++;
			} else if ((str[i+1] == 'x') and (i < str.num-3)) {
				r.add(hex_nibble_to_value(str[i+2]) * 16 + hex_nibble_to_value(str[i+3]));
				i += 3;
			} else {
				// error
				r.add(str[i]);
			}
		} else {
			r.add(str[i]);
		}
	}
	return r;
}


string str_escape(const string &str) {
	//return str.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\"", "\\\"");
	string r;
	for (int i=0; i<str.num; i++) {
		if (str[i] == '\t')
			r += "\\t";
		else if (str[i] == '\n')
			r += "\\n";
		else if (str[i] == '\r')
			r += "\\r";
		else if (str[i] == '\\')
			r += "\\\\";
		else if (str[i] == '\"')
			r += "\\\"";
		else if (str[i] == '\0')
			r += "\\0";
		else
			r.add(str[i]);
	}
	return r;
}

bool string::has_char(char c) const {
	for (int j=0; j<num; j++)
		if (c == (*this)[j])
			return true;
	return false;
}

Array<string> str_split_any(const string &s, const string &splitters) {
	Array<string> r;
	int prev = 0;
	for (int i=0; i<s.num; i++) {
		if (splitters.has_char(s[i])) {
			r.add(s.sub(prev, i));
			prev = i + 1;
			break;
		}
	}
	r.add(s.sub(prev));
	return r;
}

Array<string> string::split_any(const string &splitters) const {
	return str_split_any(*this, splitters);
}

Array<string> str_parse_tokens(const string &line, const string &splitters) {
	Array<string> tokens;
	string temp;
	bool keep_quotes = splitters.has_char('\"');

	auto end_token = [&tokens, &temp] {
		if (temp.num > 0)
			tokens.add(temp);
		temp = "";
	};

	for (int i=0; i<line.num; i++) {
		if (line[i] == ' ' or line[i] == '\t' or line[i] == '\n') {
			// whitespace
			end_token();
		} else if ((line[i] == '\"') or (line[i] == '\'')) {
			// string
			end_token();

			int start = i;
			for (int j=i+1; j<line.num; j++) {
				if (line[j] == '\\') {
					j ++;
				} else if (line[j] == line[start]) {
					i = j;
					if (keep_quotes)
						tokens.add(line.sub(start, i+1));
					else
						tokens.add(line.sub(start+1, i).unescape());
					break;
				}
			}
		} else 	if (splitters.has_char(line[i])) {
			// splitter character
			end_token();
			tokens.add(line.sub(i, i+1));
		} else {
			// regular token
			temp.add(line[i]);
		}
	}
	end_token();
	return tokens;
}

Array<string> string::parse_tokens(const string &splitters) const {
	return str_parse_tokens(*this, splitters);
}

bool sa_contains(const Array<string> &a, const string &s) {
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

	for (int i=0;i<ss;i++) {
		bool match=true;
		for (int j=0;j<rs;j++) {
			if (i+j>=ss)
				match=false;
			else if (str[i+j]!=rex[j])
				match=false;
		}
		if (match) {
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


