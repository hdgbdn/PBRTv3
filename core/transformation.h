#ifndef PBRT_CORE_TRANSFORMATION_H
#define PBRT_CORE_TRANSFORMATION_H
#include "geometry.h"
#include "pbrt.h"

namespace pbrt
{
    struct Matrix4x4 {
        Matrix4x4() :
            m{
                {1.f, 0.f, 0.f, 0.f},
                {0.f, 1.f, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {0.f, 0.f, 0.f, 1.f} }
        {}
        Matrix4x4(float mat[4][4]);
        Matrix4x4(
            float t00, float t01, float t02, float t03,
            float t10, float t11, float t12, float t13,
            float t20, float t21, float t22, float t23,
            float t30, float t31, float t32, float t33);
        bool operator==(const Matrix4x4& m2) const {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    if (m[i][j] != m2.m[i][j]) return false;
            return true;
        }
        bool operator!=(const Matrix4x4& m2) const {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    if (m[i][j] != m2.m[i][j]) return true;
            return false;
        }
        friend Matrix4x4 Transpose(const Matrix4x4&);
        void Print(FILE* f) const {
            fprintf(f, "[ ");
            for (int i = 0; i < 4; ++i) {
                fprintf(f, "  [ ");
                for (int j = 0; j < 4; ++j) {
                    fprintf(f, "%f", m[i][j]);
                    if (j != 3) fprintf(f, ", ");
                }
                fprintf(f, " ]\n");
            }
            fprintf(f, " ] ");
        }
        static Matrix4x4 Mul(const Matrix4x4& m1, const Matrix4x4& m2) {
            Matrix4x4 r;
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    r.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] +
                    m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
            return r;
        }
        friend Matrix4x4 Inverse(const Matrix4x4&);

        friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) {
            os << fmt::format("[ [ %f, %f, %f, %f ] "
                "[ %f, %f, %f, %f ] "
                "[ %f, %f, %f, %f ] "
                "[ %f, %f, %f, %f ] ]",
                m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
                m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
                m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
                m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
            return os;
        }

        float m[4][4];
    };

    Matrix4x4 Inverse(const Matrix4x4& m);
    Matrix4x4 Transpose(const Matrix4x4& m);

	class Transform
	{
        friend Transform Inverse(const Transform& t);
        friend Transform Transpose(const Transform& t);
	public:
        Transform() = default;
        Transform(const float mat[4][4]);
        Transform(const Matrix4x4& m, const Matrix4x4& mInv);
        bool HasScale()const;
        template<typename T>
        Vector3<T> operator()(const Vector3<T>& v) const;
        bool operator==(const Transform& t) const { return m == t.m && mInv == t.mInv; }
        bool operator!=(const Transform& t) const { return m != t.m || mInv != t.mInv; }
	private:
        Matrix4x4 m, mInv;
	};

    template <typename T>
    Vector3<T> Transform::operator()(const Vector3<T>& v) const
    {
        T x = v.x, y = v.y, z = v.z;
        return Vector3<T>(
            m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
            m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
            m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
    }


    Transform Inverse(const Transform& t);
    Transform Transpose(const Transform& t);
    Transform Translate(const Vector3f& delta);
    Transform Translate(float x, float y, float z);
    Transform Scale(float x, float y, float z);
    Transform Scale(const Vector3f& delta);
    Transform RotateX(float theta);
    Transform RotateY(float theta);
    Transform RotateZ(float theta);
    Transform Rotate(float theta, const Vector3f& axis);
    Transform LookAt(const Point3f& pos, const Point3f& look,
        const Vector3f& up);
}
#endif