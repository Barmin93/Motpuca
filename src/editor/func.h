#ifndef FUNC_H
#define FUNC_H

#define SWAP(type, x, y) { type vvv = (x); (x) = (y); (y) = vvv; }
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SQR(x) ((x)*(x))

#endif // FUNC_H
