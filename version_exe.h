#ifndef siftexe_VERSION_H
#define siftexe_VERSION_H

namespace siftexe_AutoVersion{
	
	//Date Version Types
	static const char siftexe_DATE[] = "03";
	static const char siftexe_MONTH[] = "06";
	static const char siftexe_YEAR[] = "2014";
	static const char siftexe_UBUNTU_VERSION_STYLE[] =  "14.06";
	
	//Software Status
	static const char siftexe_STATUS[] =  "Alpha";
	static const char siftexe_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long siftexe_MAJOR  = 1;
	static const long siftexe_MINOR  = 0;
	static const long siftexe_BUILD  = 42;
	static const long siftexe_REVISION  = 3;
	
	//Miscellaneous Version Types
	static const long siftexe_BUILDS_COUNT  = 5;
	#define siftexe_RC_FILEVERSION 1,0,42,3
	#define siftexe_RC_FILEVERSION_STRING "1, 0, 42, 3\0"
	static const char siftexe_FULLVERSION_STRING [] = "1.0.42.3";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long siftexe_BUILD_HISTORY  = 2;
	

}
#endif //siftexe_VERSION_H
