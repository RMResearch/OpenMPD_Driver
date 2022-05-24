#define _TIME_PROFILING
// Define ASIERINHO version
#define ASIERINHO_V3_VERSION_MAJOR 1
#define ASIERINHO_V3_VERSION_MINOR 0
#define ASIERINHO_V3_VERSION_PATCH 0
#define ASIERINHO_V3_VERSION_SUFFIX ""
#define ASIERINHO_V3_VERSION_NAME "Zoe"

#define ASIERINHO_V3_VERSION    ((ASIERINHO_VERSION_MAJOR << 16) | (ASIERINHO_VERSION_MINOR << 8) | ASIERINHO_VERSION_PATCH)

#if defined( ASIERINHO_V3_NONCLIENT_BUILD )
#    define _AsierInho_Export_V3 __declspec( dllexport )
#else
#    if defined( __MINGW32__ )
#        define _AsierInho_Export_V3 
#    else
#        define _AsierInho_Export_V3  __declspec( dllimport )
#    endif
#endif

#ifndef ASIERINHO_V3_VEC3
#define ASIERINHO_V3_VEC3
namespace AsierInho_V3 {
	typedef struct _vec3 {
		union {
			struct { float x, y, z; };
			struct { float r, g, b; };
		};
		_vec3() : x(0), y(0), z(0) { ; }
		_vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { ; }
		_vec3(const _vec3& pt) : x(pt.x), y(pt.y), z(pt.z) {}
		_vec3& operator=(const _vec3& v) {
			if (this != &v) {
				x = v.x; y = v.y; z = v.z;
			}
			return *this;
		}
	} vec3;
};
#define AsierInho_V3_Handler long long
#endif