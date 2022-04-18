template<class T>
void subdivide_edge(T& v0, T& v1, int L, T* output)
{
	float fL = (float)(L);
	for (int i = 0; i <= L; i++) {
		output[i] = lerp(v0, v1, (float)i / fL);
	}
}


//OL: outer edge tess factor
//IL: inner edge tess factor
void tessellation_quad_edge_integer(int OL[4], int IL[2], bool CW, void(*emitTriangle)(CVector2& v1, CVector2& v2, CVector2& v3))
{
	//64 is maximum
	CVector2 output0[64];
	CVector2 output1[64];

	int IIL[4] = { IL[0], IL[1], IL[0], IL[1] };

	CVector2 lastVertices[4][2];
	int lastVerticesCnt[4];

	CVector2 corners[4] = {
		CVector2(0,0),
		CVector2(1,0),
		CVector2(1,1),
		CVector2(0,1),
	};

	for (int side = 0; side < 4; side++) {
		CVector2* band0 = output0;
		CVector2* band1 = output1;

		int side1 = side + 1;
		if (side1 == 4)side1 = 0;

		subdivide_edge(corners[side], corners[side1], OL[side], output0);

		// inner subdivision is on a regular grid
		int L = IIL[side];
		float fL = (float)(L);
		int oppL = IIL[side1];
		float foppL = (float)oppL;

//#define DUMP_GRID_NUMBER
#ifdef DUMP_GRID_NUMBER
		fL = 1;
		foppL = 1;
#endif
		int lastVertexCnt = OL[side] + 1;

		for (int loop = 0;; loop++) {
			//ODS("loop %d\n", loop);
			int vertexCnt = L - loop * 2 - 1;

			int idx = 0;
			

			switch (side) {
			case 0:
				for (int j = 1 + loop; j <= vertexCnt + loop; j++) {
					band1[idx++].set((float)j / fL, (float)(loop + 1) / foppL);
				}
				break;
			case 1:
				for (int j = 1 + loop; j <= vertexCnt + loop; j++) {
					band1[idx++].set((float)(oppL - loop - 1) / foppL, (float)j / fL);
				}
				break;
			case 2:
				for (int j = vertexCnt + loop; j > loop; j--) {
					band1[idx++].set((float)(j) / fL, (float)(oppL - loop - 1) / foppL);
				}
				break;
			case 3:
				for (int j = vertexCnt + loop; j > loop; j--) {
					band1[idx++].set((float)(loop + 1) / foppL, (float)(j) / fL);
				}
				break;
			}

			//
			// generate primitives: idea: avoid thin triangles.
			//
			// https://www.khronos.org/opengl/wiki_opengl/images/QuadFull.png  referencing this, it's pretty straightfoward.
			// align the vertical triangle edges along the band
			int cntLong, cntShort;
			CVector2* pLong, * pShort;

			// outside tess doesn't have to be longer than inner ones, so, can't assume inner is shorter.
			bool order = CW;
			if (vertexCnt > lastVertexCnt) {
				cntLong = vertexCnt;
				cntShort = lastVertexCnt;
				pLong = band1;
				pShort = band0;
			}
			else {
				cntLong = lastVertexCnt;
				cntShort = vertexCnt;
				pLong = band0;
				pShort = band1;
				order = !CW;
			}

			int nTris = cntLong - 1;
			int shortIdx = 0;

#if 0
			bool order = CW;
			if (loop == 0) {
				// shorter edge for inner tess are always more inner, but, uncertain between outer tess and inner tess.
				if (OL[side] < L) {
					order = !CW;
				}
			}
#endif

			// 1, usually, emit two vertices on long edge, one vertex on short edge
			// 2, insert triangle when (inserted triangle has two vertices on short edge, one on long edge)

			//
			// iterate over ring(edge)  --> note: if referencing https://www.khronos.org/opengl/wiki_opengl/images/QuadFull.png
			// the order there may not be the same as the one here. for example, verticle OL 4, algorithm here goes from top to down.
			//

			for (int i = 0; i < nTris; i++) {
				bool greater = false;
				switch (side) {
				case 0:greater = pLong[i].x >= pShort[shortIdx].x;
					break;
				case 1:greater = pLong[i].y >= pShort[shortIdx].y;
					break;
				case 2:greater = pLong[i].x <= pShort[shortIdx].x;
					break;
				case 3:greater = pLong[i].y <= pShort[shortIdx].y;
					break;
				}

				if (greater && shortIdx < cntShort - 1) {
					// insert triangle
					if (order) {
						//two vertices on short, short edge must be outer edge
						emitTriangle(pShort[shortIdx], pShort[shortIdx + 1], pLong[i]);

#if 0
						ODS("S(%d), ", shortIdx);
						ODS("S(%d), ", shortIdx + 1);
						ODS("L(%d), ", i);
						ODS("\n");
#endif
					}
					else {
						emitTriangle(pShort[shortIdx], pLong[i], pShort[shortIdx + 1]);
					}

					shortIdx++;
				}
				if (order) {
					emitTriangle(pLong[i], pShort[shortIdx], pLong[i + 1]);
#if 0
					ODS("L(%d), ", i);
					ODS("L(%d), ", i + 1);
					ODS("S(%d), ", shortIdx);
					ODS("\n");
#endif
				}
				else {
					emitTriangle(pLong[i], pLong[i + 1], pShort[shortIdx]);
				}
			}

			if (shortIdx != cntShort - 1) {
				if (order) {
					emitTriangle(pShort[shortIdx], pShort[shortIdx + 1], pLong[nTris]);
				}
				else {
					emitTriangle(pShort[shortIdx], pLong[nTris], pShort[shortIdx + 1]);
				}
#if 0
				ODS("S(%d), ", shortIdx);
				ODS("L(%d), ", nTris);
				ODS("S(%d), ", shortIdx + 1);
				ODS("\n");
#endif
			}


			if (vertexCnt <= 2) {
				lastVerticesCnt[0] = vertexCnt;
				for (int i = 0; i < vertexCnt; i++) {
					lastVertices[0][i] = band1[i];
				}
				break;
			}

			lastVertexCnt = vertexCnt;
			temp_swap(band0, band1);
		}
	}

	//
	// maybe a missing geometry here, if so, reference tessellation_tri_edge_integer()
	//
}


//OL: outer edge tess factor
//IL: inner edge tess factor
void tessellation_tri_edge_integer(int OL[3], int IL, bool CW, void(*emitTriangle)(CVector4D<>& v1, CVector4D<>& v2, CVector4D<>& v3))
{
	//64 is maximum
	CVector4D<> output0[64];
	CVector4D<> output1[64];

	CVector4D<> lastVertices[4][2];
	int lastVerticesCnt[4];

	CVector4D<> corners[3] = {
		CVector4D<>(1,0,0),
		CVector4D<>(0,0,1),
		CVector4D<>(0,1,0),
	};

	float oppX;

	//all IL(internal tess level) are the same
	int L = IL;
	float fL = (float)(L);

	int oppL = IL;
	float foppL = (float)oppL;

	int nInnerRings = L / 2;

	// NOTE: when testing/debugging, change of start side or the end side causes missing data in middle triangle.
	for (int side = 0; side < 3; side++) {
		CVector4D<>* band0 = output0;
		CVector4D<>* band1 = output1;

		int side1 = side + 1;
		if (side1 == 3)side1 = 0;

		subdivide_edge(corners[side], corners[side1], OL[side], output0);

		//#define DUMP_GRID_NUMBER
#ifdef DUMP_GRID_NUMBER
		fL = 1;
		foppL = 1;
#endif
		int lastVertexCnt = OL[side] + 1;

		for (int loop = 0;; loop++) {
			//ODS("loop %d\n", loop);
			int vertexCnt = L - loop * 2 - 1;

			int idx = 0;

			oppX = 1.f - (float)((IL - loop) - 1.f) / (float)IL;
			//think of triangles as the same thing as quad, except triangles have 3 vertices.
			float x;

			for (int j = 1 + loop; j <= vertexCnt + loop; j++) {
				x = (float)j / fL;
				switch (side) {
				case 0:
					band1[idx++].set(1.f - x, oppX, x);
					break;
				case 1:
					band1[idx++].set(oppX, x, 1.f - x);
					break;
				case 2:
					band1[idx++].set(x, 1.f - x, oppX);
					break;
				}
			}

			//
			// generate primitives: idea: avoid thin triangles.
			//
			// https://www.khronos.org/opengl/wiki_opengl/images/QuadFull.png  referencing this, it's pretty straightfoward.
			// align the vertical triangle edges along the band
			int cntLong, cntShort;
			CVector4D<>* pLong, * pShort;

			// outside tess doesn't have to be longer than inner ones, so, can't assume inner is shorter.
			bool order = CW;
			if (vertexCnt > lastVertexCnt) {
				cntLong = vertexCnt;
				cntShort = lastVertexCnt;
				pLong = band1;
				pShort = band0;
			}
			else {
				cntLong = lastVertexCnt;
				cntShort = vertexCnt;
				pLong = band0;
				pShort = band1;
				order = !CW;
			}

			int nTris = cntLong - 1;
			int shortIdx = 0;

#if 0
			bool order = CW;
			if (loop == 0) {
				// shorter edge for inner tess are always more inner, but, uncertain between outer tess and inner tess.
				if (OL[side] < L) {
					order = !CW;
				}
			}
#endif

			// 1, usually, emit two vertices on long edge, one vertex on short edge
			// 2, insert triangle when (inserted triangle has two vertices on short edge, one on long edge)

			//
			// iterate over ring(edge)  --> note: if referencing https://www.khronos.org/opengl/wiki_opengl/images/QuadFull.png
			// the order there may not be the same as the one here. for example, verticle OL 4, algorithm here goes from top to down.
			//

			for (int i = 0; i < nTris; i++) {
				bool greater = false;
				switch (side) {
				case 0:greater = pLong[i].z >= pShort[shortIdx].z;
					break;
				case 1:greater = pLong[i].y >= pShort[shortIdx].y;
					break;
				case 2:greater = pLong[i].x >= pShort[shortIdx].x;
					break;
				}

				if (greater && shortIdx < cntShort - 1) {
					// insert triangle
					if (order) {
						//two vertices on short, short edge must be outer edge
						emitTriangle(pShort[shortIdx], pLong[i], pShort[shortIdx + 1]);

#if 0
						ODS("S(%d), ", shortIdx);
						ODS("S(%d), ", shortIdx + 1);
						ODS("L(%d), ", i);
						ODS("\n");
#endif
					}
					else {
						emitTriangle(pShort[shortIdx], pShort[shortIdx + 1], pLong[i]);
					}

					shortIdx++;
				}
				if (order) {
					emitTriangle(pLong[i], pLong[i + 1], pShort[shortIdx]);
#if 0
					ODS("L(%d), ", i);
					ODS("L(%d), ", i + 1);
					ODS("S(%d), ", shortIdx);
					ODS("\n");
#endif
				}
				else {
					emitTriangle(pLong[i], pShort[shortIdx], pLong[i + 1]);
				}
			}

			if (shortIdx != cntShort - 1) {
				if (order) {
					emitTriangle(pShort[shortIdx], pLong[nTris], pShort[shortIdx + 1]);
				}
				else {
					emitTriangle(pShort[shortIdx], pShort[shortIdx + 1], pLong[nTris]);
				}
#if 0
				ODS("S(%d), ", shortIdx);
				ODS("L(%d), ", nTris);
				ODS("S(%d), ", shortIdx + 1);
				ODS("\n");
#endif
			}

			//break;		//break ring loop -> 1 ring only for debug

			if (vertexCnt <= 2) {
				lastVerticesCnt[side] = vertexCnt;
				for (int i = 0; i < vertexCnt; i++) {
					lastVertices[side][i] = band1[i];
				}
				break;
			}

			lastVertexCnt = vertexCnt;
			temp_swap(band0, band1);
		
		}	//for (int loop = 0;; loop++)
	}  //for (int side = 0; side < 3; side++)

	if (lastVerticesCnt[0] != 1) {
		if (CW) {
			emitTriangle(lastVertices[0][0], lastVertices[2][0], lastVertices[1][0]);
		}
		else {
			emitTriangle(lastVertices[0][0], lastVertices[1][0], lastVertices[2][0]);
		}
	}

}


#if 0
CGrowableArrayWithLast<FACE_TRIANGLE16>ib;
CGrowableArrayWithLast<CVector3>vb;

void emitTriangle(CVector2& v1, CVector2& v2, CVector2& v3)
{
	int cnt = vb.get_count();
	FACE_TRIANGLE16 face;
	face.a = cnt;
	face.b = cnt + 1;
	face.c = cnt + 2;
	ib.push(face);

	vb.push(CVector3(v1.x, 0, v1.y));
	vb.push(CVector3(v2.x, 0, v2.y));
	vb.push(CVector3(v3.x, 0, v3.y));
}


CGrowableArrayWithLast<CVector4D<>>vb4d;
void emitTriangle4D(CVector4D<>& v1, CVector4D<>& v2, CVector4D<>& v3)
{
	int cnt = vb4d.get_count();
	FACE_TRIANGLE16 face;
	face.a = cnt;
	face.b = cnt + 1;
	face.c = cnt + 2;
	ib.push(face);

#if 1
	CVector4D<>* tris[] = {
		&v1,
		&v2,
		&v3
	};
	for (int i = 0; i < 3; i++) {
		ODS("(%f,%f,%f),", tris[i]->x, tris[i]->y, tris[i]->z);
	}
	ODS("\n");
#endif

	CVector4D<> tri[3] = {
		CVector4D<>(-0.5f,0,0),
		CVector4D<>(0.5f, 0, 0),
		CVector4D<>(0, 0, 1),
	};

	CVector4D<>va = tri[0] * v1.x + tri[1] * v1.y + tri[2] * v1.z;
	CVector4D<>vb = tri[0] * v2.x + tri[1] * v2.y + tri[2] * v2.z;
	CVector4D<>vc = tri[0] * v3.x + tri[1] * v3.y + tri[2] * v3.z;

	vb4d.push(va);
	vb4d.push(vb);
	vb4d.push(vc);
}


void test_tess1(void)
{
#if 1
	int OL[4] = { 2,9,3,4 };
	int IL[2] = { 6,7 };
#endif
	tessellation_quad_edge_integer(OL, IL, false, emitTriangle);

	writeWaveFront16(L"d:\\engine\\media\\tmp\\test.obj", ib, vb, false, true, true);
}

void test_tess(void)
{
#if 1
	int OL[3] = { 11, 2, 23 };
	int IL = 5;
#endif
	tessellation_tri_edge_integer(OL, IL, false, emitTriangle4D);

	writeWaveFront16(L"d:\\engine\\media\\tmp\\test.obj", ib, vb4d, false, true, true);
}

#endif
