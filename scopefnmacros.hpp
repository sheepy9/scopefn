#ifndef SCOPEFN_MACROS_H
#define SCOPEFN_MACROS_H

#define RUN_(x) [self = &x]()
#define RUN(x,y) run(RUN_(x) y)

#define LET_(x) [](decltype(x)& it)
#define LET(x,y) let(LET_(x) y)

#define ALSO_(x) [](decltype(x)& it)
#define ALSO(x,y) also(ALSO_(x) y) 

#define APPLY_(x) [self = &x]()
#define APPLY(x,y) apply(APPLY_(x) y)

#endif