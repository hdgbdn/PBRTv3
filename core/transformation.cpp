#include "transformation.h"
#include "interaction.h"

namespace pbrt
{
	Matrix4x4::Matrix4x4(float mat[4][4]) : m()
	{
		memcpy(m, mat, 16 * sizeof(float));
	}

	Matrix4x4 Inverse(const Matrix4x4& m)
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

	Matrix4x4 Transpose(const Matrix4x4& m)
	{
		return Matrix4x4(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0], m.m[0][1],
			m.m[1][1], m.m[2][1], m.m[3][1], m.m[0][2], m.m[1][2],
			m.m[2][2], m.m[3][2], m.m[0][3], m.m[1][3], m.m[2][3],
			m.m[3][3]);
	}

	bool SolveLinearSystem2x2(const float A[2][2], const float B[2], float* x0, float* x1)
	{
		float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
		if (std::abs(det) < 1e-10f)
			return false;
		*x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
		*x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
		if (std::isnan(*x0) || std::isnan(*x1))
			return false;
		return true;
	}

	Transform::Transform(const float mat[4][4])
		: m(
			mat[0][0], mat[0][1], mat[0][2], mat[0][3],
			mat[1][0], mat[1][1], mat[1][2], mat[1][3],
			mat[2][0], mat[2][1], mat[2][2], mat[2][3],
			mat[3][0], mat[3][1], mat[3][2], mat[3][3]),
		mInv(Inverse(m)) {}

	Transform::Transform(const Matrix4x4& m, const Matrix4x4& mInv)
		:m(m), mInv(mInv) {}

	bool Transform::HasScale() const
	{
		float la2 = (*this)(Vector3f(1, 0, 0)).LengthSquared();
		float lb2 = (*this)(Vector3f(0, 1, 0)).LengthSquared();
		float lc2 = (*this)(Vector3f(0, 0, 1)).LengthSquared();
		return (la2 < .999f || la2 > 1.001f) ||
			(lb2 < .999f || lb2 > 1.001f) ||
			(lc2 < .999f || lc2 > 1.001f);
	}

	bool Transform::SwapsHandedness() const
	{
		float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
			m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
			m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
		return det < 0;
	}


	Transform Transpose(const Transform& t)
	{
		return Transform(Transpose(t.m), Transpose(t.mInv));
	}

	Transform Inverse(const Transform& t)
	{
		return Transform(t.mInv, t.m);
	}

	Transform Translate(float x, float y, float z)
	{
		return Transform(
			Matrix4x4(
				1, 0, 0, x,
				0, 1, 0, y,
				0, 0, 1, z,
				0, 0, 0, 1),
			Matrix4x4(
				1, 0, 0, -x,
				0, 1, 0, -y,
				0, 0, 1, -z,
				0, 0, 0, 1)
		);
	}

	Transform Translate(const Vector3f& delta)
	{
		return Translate(delta.x, delta.y, delta.z);
	}

	Transform Scale(float x, float y, float z)
	{
		return Transform(
			Matrix4x4(
				x, 0, 0, 0,
				0, y, 0, 0,
				0, 0, z, 0,
				0, 0, 0, 1),
			Matrix4x4(
				1 / x, 0, 0, 0,
				0, 1 / y, 0, 0,
				0, 0, 1 / z, 0,
				0, 0, 0, 1)
		);
	}

	Transform Scale(const Vector3f& delta)
	{
		return Scale(delta.x, delta.y, delta.z);
	}

	Transform RotateX(float theta)
	{
		float sinTheta = std::sin(Radians(theta));
		float cosTheta = std::cos(Radians(theta));
		return Transform(
			Matrix4x4(
				1, 0, 0, 0,
				0, cosTheta, -sinTheta, 0,
				0, sinTheta, cosTheta, 0,
				0, 0, 0, 1),
			Matrix4x4(
				1, 0, 0, 0,
				0, cosTheta, sinTheta, 0,
				0, -sinTheta, cosTheta, 0,
				0, 0, 0, 1));
	}

	Transform RotateY(float theta)
	{
		float sinTheta = std::sin(Radians(theta));
		float cosTheta = std::cos(Radians(theta));
		return Transform(
			Matrix4x4(
				cosTheta, 0, sinTheta, 0,
				0, 1, 0, 0,
				-sinTheta, 0, cosTheta, 0,
				0, 0, 0, 1),
			Matrix4x4(
				cosTheta, 0, -sinTheta, 0,
				0, 1, 0, 0,
				sinTheta, 0, cosTheta, 0,
				0, 0, 0, 1));
	}

	Transform RotateZ(float theta)
	{
		float sinTheta = std::sin(Radians(theta));
		float cosTheta = std::cos(Radians(theta));
		return Transform(
			Matrix4x4(
				cosTheta, -sinTheta, 0, 0,
				sinTheta, cosTheta, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1),
			Matrix4x4(
				cosTheta, sinTheta, 0, 0,
				-sinTheta, cosTheta, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1));
	}

	Transform Rotate(float theta, const Vector3f& axis)
	{
		Vector3f a = Normalize(axis);
		float sinTheta = std::sin(Radians(theta));
		float cosTheta = std::cos(Radians(theta));
		Matrix4x4 m;
		m.m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
		m.m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
		m.m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
		m.m[0][3] = 0;
		
		m.m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
		m.m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
		m.m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
		m.m[1][3] = 0;

		m.m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
		m.m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
		m.m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
		m.m[2][3] = 0;
		return Transform(m, Transpose(m));
	}

	Transform LookAt(const Point3f& pos, const Point3f& look, const Vector3f& up)
	{
		Matrix4x4 cameraToWorld;
		cameraToWorld.m[0][3] = pos.x;
		cameraToWorld.m[1][3] = pos.y;
		cameraToWorld.m[2][3] = pos.z;
		cameraToWorld.m[3][3] = 1;

		Vector3f dir = Normalize(look - pos);
		Vector3f left = Normalize(Cross(Normalize(up), dir));
		Vector3f newUp = Cross(dir, left);
		cameraToWorld.m[0][0] = left.x;
		cameraToWorld.m[1][0] = left.y;
		cameraToWorld.m[2][0] = left.z;
		cameraToWorld.m[3][0] = 0.f;
		cameraToWorld.m[0][1] = newUp.x;
		cameraToWorld.m[1][1] = newUp.y;
		cameraToWorld.m[2][1] = newUp.z;
		cameraToWorld.m[3][1] = 0.f;
		cameraToWorld.m[0][2] = dir.x;
		cameraToWorld.m[1][2] = dir.y;
		cameraToWorld.m[2][2] = dir.z;
		cameraToWorld.m[3][2] = 0.f;
		return Transform(Inverse(cameraToWorld), cameraToWorld);
	}

	Transform Orthographic(float zNear, float zFar)
	{
		return Scale(1, 1, 1 / (zFar - zNear)) *
			Translate(Vector3f(0, 0, -zNear));
	}

	Transform Perspective(float fov, float n, float f)
	{
		Matrix4x4 persp(1, 0, 0, 0,
		                0, 1, 0, 0,
		                0, 0, f / (f - n), -f * n / (f - n),
		                0, 0, 1, 0);
		float invTanAng = 1 / std::tan(Radians(fov) / 2);
		return Scale(invTanAng, invTanAng, 1) * Transform(persp);
	}

    SurfaceInteraction Transform::operator()(const SurfaceInteraction& si) const
	{
		SurfaceInteraction ret;
		//TODO Transform p and pError in SurfaceInteraction
		//TODO Transform remaining members of SurfaceInteraction
		return ret;
	}

    void AnimatedTransform::Interpolate(float time, Transform *t) const
    {
        // TODO need implement
    }

    Bounds3f AnimatedTransform::MotionBounds(const Bounds3f &b) const
    {
        // TODO need implement
        return pbrt::Bounds3f();
    }
}