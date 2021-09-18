#include "transformation.h"

using namespace pbrt;

Matrix4x4::Matrix4x4(
	float t00, float t01, float t02, float t03,
	float t10, float t11, float t12, float t13,
	float t20, float t21, float t22, float t23,
	float t30, float t31, float t32, float t33) :
	m{
		{t00, t01, t02, t03},
		{t10, t11, t12, t13},
		{t20, t21, t22, t23},
		{t30, t31, t32, t33}}
{}

Matrix4x4::Matrix4x4(float mat[4][4]): m()
{
	memcpy(m, mat, 16 * sizeof(float));
}

Matrix4x4 pbrt::Inverse(const Matrix4x4& m)
{
	int indxc[4], indxr[4];
	int ipiv[4] = { 0, 0, 0, 0 };
	float minv[4][4];
	memcpy(minv, m.m, 4 * 4 * sizeof(float));
	for (int i = 0; i < 4; i++) {
		int irow = 0, icol = 0;
		float big = 0.f;
		// Choose pivot
		for (int j = 0; j < 4; j++) {
			if (ipiv[j] != 1) {
				for (int k = 0; k < 4; k++) {
					if (ipiv[k] == 0) {
						if (std::abs(minv[j][k]) >= big) {
							big = float(std::abs(minv[j][k]));
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1)
					{
						std::cerr << "Singular matrix in MatrixInvert" << std::endl;
					}
				}
			}
		}
		++ipiv[icol];
		// Swap rows _irow_ and _icol_ for pivot
		if (irow != icol) {
			for (int k = 0; k < 4; ++k) std::swap(minv[irow][k], minv[icol][k]);
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (minv[icol][icol] == 0.f) std::cerr << "Singular matrix in MatrixInvert" << std::endl;

		// Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
		float pivinv = 1. / minv[icol][icol];
		minv[icol][icol] = 1.;
		for (int j = 0; j < 4; j++) minv[icol][j] *= pivinv;

		// Subtract this row from others to zero out their columns
		for (int j = 0; j < 4; j++) {
			if (j != icol) {
				float save = minv[j][icol];
				minv[j][icol] = 0;
				for (int k = 0; k < 4; k++) minv[j][k] -= minv[icol][k] * save;
			}
		}
	}
	// Swap columns to reflect permutation
	for (int j = 3; j >= 0; j--) {
		if (indxr[j] != indxc[j]) {
			for (int k = 0; k < 4; k++)
				std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
		}
	}
	return Matrix4x4(minv);
}

Matrix4x4 pbrt::Transpose(const Matrix4x4& m)
{
	return Matrix4x4(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0], m.m[0][1],
		m.m[1][1], m.m[2][1], m.m[3][1], m.m[0][2], m.m[1][2],
		m.m[2][2], m.m[3][2], m.m[0][3], m.m[1][3], m.m[2][3],
		m.m[3][3]);
}


