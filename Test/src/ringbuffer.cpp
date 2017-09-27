#include "../../src/Tsunami.h"
#include "../../src/Data/RingBuffer.h"
#include <assert.h>

const string AppName = "";

bool Tsunami::allowTermination(){ return true; }

Tsunami *tsunami;

void title(const string &name)
{
	printf("%s", name.c_str());
	for (int i=name.num; i<32; i++)
		printf(" ");
}

void ok()
{
	printf("ok \\(^_^)/\n");
}

int hui_main(Array<string> const &arg)
{
	const int N = 16;
	RingBuffer rb(N + N/2);

	title("write linear");
	AudioBuffer wbuf;
	wbuf.resize(N);
	for (int i=0; i<N; i++)
		wbuf.c[0][i] = i;
	rb.write(wbuf);
	assert(rb.available() == N);
	ok();

	title("read linear");
	AudioBuffer rbuf;
	rbuf.resize(N);
	rb.read(rbuf);
	assert(rb.available() == 0);

	for (int i=0; i<N; i++)
		assert(wbuf.c[0][i] == rbuf.c[0][i]);
	ok();


	title("write wrapped");
	rb.write(wbuf);
	assert(rb.available() == N);
	ok();
	
	title("read wrapped");
	AudioBuffer rbuf2;
	rbuf2.resize(N);
	int n = rb.read(rbuf2);
	//printf("%d  %d\n", n, rb.available());
	assert(rb.available() == 0);

	for (int i=0; i<N; i++)
		assert(wbuf.c[0][i] == rbuf2.c[0][i]);
	ok();
	return 0;
}

