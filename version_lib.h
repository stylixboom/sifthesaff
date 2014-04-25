#ifndef siftlib_VERSION_H
#define siftlib_VERSION_H

namespace siftlib_AutoVersion{
	
	//Date Version Types
	static const char siftlib_DATE[] = "24";
	static const char siftlib_MONTH[] = "04";
	static const char siftlib_YEAR[] = "2014";
	static const char siftlib_UBUNTU_VERSION_STYLE[] =  "14.04";
	
	//Software Status
	static const char siftlib_STATUS[] =  "Alpha";
	static const char siftlib_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long siftlib_MAJOR  = 1;
	static const long siftlib_MINOR  = 1;
	static const long siftlib_BUILD  = 8;
	static const long siftlib_REVISION  = 52;
	
	//Miscellaneous Version Types
	static const long siftlib_BUILDS_COUNT  = 12;
	#define siftlib_RC_FILEVERSION 1,1,8,52
	#define siftlib_RC_FILEVERSION_STRING "1, 1, 8, 52\0"
	static const char siftlib_FULLVERSION_STRING [] = "1.1.8.52";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long siftlib_BUILD_HISTORY  = 8;
	

}
#endif //siftlib_VERSION_H
