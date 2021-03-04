
#include "common.hpp"

void exitf(const char* format, ... ) {
    
    va_list arglist;
    va_start( arglist, format );
    
	char str[MAX_STRING];
	vsnprintf(str, MAX_STRING, format, arglist);
    va_end( arglist );
    
    printf("%s",str);
	exit(1);
}
