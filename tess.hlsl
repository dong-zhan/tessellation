//
// the code below is used to remove cracks in tessellation, by using dominant edges and dominant vertex
// idea is: when sampling along edge, only sample dominant edge, when sampling vertex, only sample dominant vertex, then, inconsistency on seam is avoided -> cracks removed.
//
// it's for reading, maybe you can tell me how to remove those ugly branchings(ifs).
// well, I put this here because I googled a lot, and failed to find one solution. so, hopefully, it's useful for programmers who are struggling with tessellation cracks.
// and this version works!
    
    if(all(BarycentricCoordinates.xy == 0)){
			hmTex = TrianglePatch[2].texDVertex;
		}else if(all(BarycentricCoordinates.yz == 0)){
			hmTex = TrianglePatch[0].texDVertex;
		}else if(all(BarycentricCoordinates.xz == 0)){
			hmTex = TrianglePatch[1].texDVertex;
		}else if(any(BarycentricCoordinates.xyz == 0)){
			float2 texx, texy, texz;
			if(BarycentricCoordinates.x == 0){		
				texy = TrianglePatch[1].texDEdge.xy;		//xy is the coordinates of the first vertex (uv)
				texz = TrianglePatch[1].texDEdge.zw;		//zw is the coordinates of the second vertex (uv)
			}
			if(BarycentricCoordinates.y == 0){		
				texz = TrianglePatch[2].texDEdge.xy;
				texx = TrianglePatch[2].texDEdge.zw;
			}
			if(BarycentricCoordinates.z == 0){		
				texx = TrianglePatch[0].texDEdge.xy;
				texy = TrianglePatch[0].texDEdge.zw;
			}
		
			hmTex = BarycentricCoordinates.x * texx + 
						  BarycentricCoordinates.y * texy + 
						  BarycentricCoordinates.z * texz;
		}
