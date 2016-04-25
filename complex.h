#ifndef __COMPLEX_H__
#define __COMPLEX_H__

#include <math.h>



const long double PI=3.14159265358979323846;  /* pi */


struct complex{
	long double x, y;
	complex(){};
	complex(long double _x, long double _y):x(_x), y(_y) {};
	complex(long double t){ //e^it
		x = cos(t);
		y = sin(t);
	}
	complex operator+(const complex& a){
		return complex(x+a.x, y+a.y);
	}
	complex& operator+=(const complex& a){
		x+=a.x;
		y+=a.y;
		return *this;
	}
	complex operator-(const complex& a){
		return complex(x-a.x, y-a.y);
	}
	complex& operator-=(const complex& a){
		x-=a.x;
		y-=a.y;
		return *this;
	}
	complex operator*(const complex& a){
		return complex(x*a.x - y*a.y, x*a.y + y*a.x);
	}
	complex& operator*=(const complex& a){
		long double tempx = x;
		x = x*a.x-y*a.y;
		y = tempx*a.y + y*a.x;
		return *this;
	}
	void pup(PUP::er &p){
		p|x; p|y;
	}
};


complex ei2PI(long double k){
	return complex(2*PI*k);
}


#endif
