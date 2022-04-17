#ifndef _INCLUDE_TESSELLATION_H_
#define _INCLUDE_TESSELLATION_H_


#include <atomic>

#include "sp/sp_dynbvh.h"
#include "sp/sp_bvh.h"
#include "sp/sp_bvh_fv.h"

#include "ge_util/allocators.h"
#include "ge_util/array_utils.h"
#include "ray_tracing.h"

#include "math/vector2.h"
#include "math/vector4.h"

#include "helper/face_types.h"
#include "helper/libIdmap.h"

#include "helper/xml.h"

#include "ge_util/avl_key_string.h"

#include "math/sg.h"
#include "math/sh.h"

#include "ray_tracing_common.h"

void tessellation_quad_edge_integer(int OL[4], int IL[2], bool CW,
	void(*emitTriangle)(GE_MATH::CVector2& v1, GE_MATH::CVector2& v2, GE_MATH::CVector2& v3));
void tessellation_tri_edge_integer(int OL[3], int IL, bool CW, 
	void(*emitTriangle)(GE_MATH::CVector4D<>& v1, GE_MATH::CVector4D<>& v2, GE_MATH::CVector4D<>& v3));

#if 0
void test_tess(void);
#endif

#endif
