#ifndef siftexe_VERSION_H
#define siftexe_VERSION_H

namespace siftexe_AutoVersion{
	
	//Date Version Types
	static const char siftexe_DATE[] = "09";
	static const char siftexe_MONTH[] = "05";
	static const char siftexe_YEAR[] = "2014";
	static const char siftexe_UBUNTU_VERSION_STYLE[] =  "14.05";
	
	//Software Status
	static const char siftexe_STATUS[] =  "Alpha";
	static const char siftexe_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long siftexe_MAJOR  = 1;
	static const long siftexe_MINOR  = 0;
	static const long siftexe_BUILD  = 41;
	static const long siftexe_REVISION  = 1;
	
	//Miscellaneous Version Types
	static const long siftexe_BUILDS_COUNT  = 4;
	#define siftexe_RC_FILEVERSION 1,0,41,1
	#define siftexe_RC_FILEVERSION_STRING "1, 0, 41, 1\0"
	static const char siftexe_FULLVERSION_STRING [] = "1.0.41.1";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long siftexe_BUILD_HISTORY  = 1;
	

}
#endif //siftexe_VERSION_H
