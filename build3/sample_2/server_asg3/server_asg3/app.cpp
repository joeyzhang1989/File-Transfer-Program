#include "UDP.h"

int main()
{
	UDP udp(5001,7001);
	udp.run();
	return 0;
}