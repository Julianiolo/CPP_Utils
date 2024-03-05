#include <cstdio>
#include "../src/DataUtils.h"
#include "../src/comps/BlockingQueue.h"

#define DBG_PRINT 0

int cnt = 0;

void no_copy_test() {
	class NoCopyPls {
	public:
		int v = 0;
		NoCopyPls(int v) : v(v) {}
		NoCopyPls(const NoCopyPls& o) {
			printf("[%d] AAAAAAH COPY @%d\n", v, cnt);
			abort();
		}
		NoCopyPls(NoCopyPls&& o) {
			v = o.v;
#if DBG_PRINT
			printf("[%d] Move from %p to %p [@%d]\n", o.v, (void*)&o, (void*)this, cnt);
#endif
		}
	};
	BlockingQueue<NoCopyPls> q;
	{
		cnt++;
		NoCopyPls zero(0);
		q.push(std::move(zero));
		cnt++;
		NoCopyPls one(1);
		cnt++;
		q.push(std::move(one));
		cnt++;
	}
#if DBG_PRINT
	printf("Done pushing\n");
#endif
	{
		NoCopyPls zero = std::move(q.take().value());
		cnt++;
#if DBG_PRINT
		printf("Zero: %d\n", zero.v);
#endif
		NoCopyPls one = std::move(q.take().value());
		cnt++;
#if DBG_PRINT
		printf("One: %d\n", one.v);
#endif
	}
}

int main() {
	no_copy_test();
	printf("Done :)\n");
}
