#ifndef siftexe_VERSION_H
#define siftexe_VERSION_H

namespace siftexe_AutoVersion{
	
	//Date Version Types
	static const char siftexe_DATE[] = "24";
	static const char siftexe_MONTH[] = "04";
	static const char siftexe_YEAR[] = "2014";
	static const char siftexe_UBUNTU_VERSION_STYLE[] =  "14.04";
	
	//Software Status
	static const char siftexe_STATUS[] =  "Alpha";
	static const char siftexe_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long siftexe_MAJOR  = 1;
	static const long siftexe_MINOR  = 0;
	static const long siftexe_BUILD  = 40;
	static const long siftexe_REVISION  = 0;
	
	//Miscellaneous Version Types
	static const long siftexe_BUILDS_COUNT  = 4;
	#define siftexe_RC_FILEVERSION 1,0,40,0
	#define siftexe_RC_FILEVERSION_STRING "1, 0, 40, 0\0"
	static const char siftexe_FULLVERSION_STRING [] = "1.0.40.0";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long siftexe_BUILD_HISTORY  = 0;
	

}
#endif //siftexe_VERSION_H
