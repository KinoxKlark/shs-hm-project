#include <cmath>
#include <initializer_list>

#define EPSILON 0.0001
#define INF std::numeric_limits<float>::infinity()
#define PI 3.14159265358979323846264338327950288419716939937510
#define PI_2 .5*PI

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

inline r32 clamp(r32 nb, r32 min, r32 max)
{
	if(nb < min)
		return min;
	if(nb > max)
		return max;
	return nb;
}

template <typename T>
inline T modulo(T val)
{
	T integer_part = std::floor(val);
	T result = val-integer_part;
	return result;
}

template <typename T>
inline T sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
inline T safe_sgn(T val) {
	return (T)(+1 | (-(int)signbit(val))); 
}

inline // Thank's Nic Taylor
r32 fast_atan(r32 z)
{
    const r32 n1 = 0.97239411f;
    const r32 n2 = -0.19194795f;
    return (n1 + n2 * z * z) * z;
}

inline // Thank's Nic Taylor
r32 fast_atan2(r32 y, r32 x)
{
    const r32 n1 = 0.97239411f;
    const r32 n2 = -0.19194795f;    
    r32 result = 0.0f;
    if (x != 0.0f)
    {
        const union { r32 flVal; u32 nVal; } tYSign = { y };
        const union { r32 flVal; u32 nVal; } tXSign = { x };
        if (fabsf(x) >= fabsf(y))
        {
            union { r32 flVal; u32 nVal; } tOffset = { PI };
            // Add or subtract PI based on y's sign.
            tOffset.nVal |= tYSign.nVal & 0x80000000u;
            // No offset if x is positive, so multiply by 0 or based on x's sign.
            tOffset.nVal *= tXSign.nVal >> 31;
            result = tOffset.flVal;
            const r32 z = y / x;
            result += (n1 + n2 * z * z) * z;
        }
        else // Use atan(y/x) = pi/2 - atan(x/y) if |y/x| > 1.
        {
            union { r32 flVal; u32 nVal; } tOffset = { PI_2 };
            // Add or subtract PI/2 based on y's sign.
            tOffset.nVal |= tYSign.nVal & 0x80000000u;            
            result = tOffset.flVal;
            const r32 z = x / y;
            result -= (n1 + n2 * z * z) * z;            
        }
    }
    else if (y > 0.0f)
    {
        result = PI_2;
    }
    else if (y < 0.0f)
    {
        result = -PI_2;
    }
    return result;
}

typedef sf::Rect<r32> rect;
typedef sf::Rect<r64> rect64;

typedef sf::Vector3<r32> v3;
typedef sf::Vector3<i32> v3i;
typedef sf::Vector3<u32> v3u;
typedef sf::Vector3<r64> v3_64;

typedef sf::Vector2<r32> v2;
typedef sf::Vector2<i32> v2i;
typedef sf::Vector2<u32> v2u;
typedef sf::Vector2<r64> v2_64;

template<typename T>
struct Vector4 {
	union {
		struct {
			T left, right, top, bottom;
		};
		struct {
			T x, y, z, w;
		};
	};
};

typedef Vector4<r32> v4;
typedef Vector4<i32> v4i;
typedef Vector4<u32> v4u;
typedef Vector4<r64> v4_64;

struct m2x2 {
	union{
	r32 data[2][2];
	struct{
		r32 d11, d12;
		r32 d21, d22;
	};
	};

	m2x2() {};
	m2x2(r32 (&&data)[2][2]);
	void operator=(r32 (&&data)[2][2]);
};

struct m3x3 {
	union{
	r32 data[3][3];
	struct{
		r32 d11, d12, d13;
		r32 d21, d22, d23;
		r32 d31, d32, d33;
	};
	};

	m3x3() {};
	m3x3(r32 (&&data)[3][3]);
	void operator=(r32 (&&data)[3][3]);
};

m2x2::m2x2(r32 (&&data)[2][2])
{
	memcpy(&(this->data), &data, 2*2*sizeof(r32));
}

m3x3::m3x3(r32 (&&data)[3][3])
{
	memcpy(&(this->data), &data, 3*3*sizeof(r32));
}

void m2x2::operator=(r32 (&&data)[2][2])
{
	memcpy(&(this->data), &data, 2*2*sizeof(r32));
}

void m3x3::operator=(r32 (&&data)[3][3])
{
	memcpy(&(this->data), &data, 3*3*sizeof(r32));
}

v2 operator*(m2x2& mat, v2& vec)
{
	v2 out;
	out.x = mat.d11*vec.x+mat.d12*vec.y;
	out.y = mat.d21*vec.x+mat.d22*vec.y;
	return out;
}


v3 operator*(m3x3& mat, v3& vec)
{
	v3 out;
	out.x = mat.d11*vec.x+mat.d12*vec.y+mat.d13*vec.z;
	out.y = mat.d21*vec.x+mat.d22*vec.y+mat.d23*vec.z;
	out.z = mat.d31*vec.x+mat.d32*vec.y+mat.d33*vec.z;
	return out;
}

m2x2 operator*(m2x2& L, m2x2& R)
{
	m2x2 mat = m2x2({
			{
				L.d11*R.d11+L.d12*R.d21,
				L.d11*R.d12+L.d12*R.d22,
			},
			{
				L.d21*R.d11+L.d22*R.d21,
				L.d21*R.d12+L.d22*R.d22,
			},
		});

	return mat;
}

m3x3 operator*(m3x3& L, m3x3& R)
{
	m3x3 mat = m3x3({
			{
				L.d11*R.d11+L.d12*R.d21+L.d13*R.d31,
				L.d11*R.d12+L.d12*R.d22+L.d13*R.d32,
				L.d11*R.d13+L.d12*R.d23+L.d13*R.d33,
			},
			{
				L.d21*R.d11+L.d22*R.d21+L.d23*R.d31,
				L.d21*R.d12+L.d22*R.d22+L.d23*R.d32,
				L.d21*R.d13+L.d22*R.d23+L.d23*R.d33,
			},
			{
				L.d31*R.d11+L.d32*R.d21+L.d33*R.d31,
				L.d31*R.d12+L.d32*R.d22+L.d33*R.d32,
				L.d31*R.d13+L.d32*R.d23+L.d33*R.d33,
			},
		});

	return mat;
}

inline
r32 determinant(m2x2& mat)
{
	r32 det = mat.d11*mat.d22-mat.d21*mat.d12;

	return det;
}

inline
r32 determinant(m3x3& mat)
{
	r32 det =
		 mat.d11*(mat.d22*mat.d33-mat.d32*mat.d23)
		-mat.d21*(mat.d12*mat.d33-mat.d32*mat.d13)
		+mat.d31*(mat.d12*mat.d23-mat.d22*mat.d13);

	return det;
}

inline
m2x2 inverse(m2x2& mat)
{
	r32 det = determinant(mat);
	Assert(abs(det) > 1e-6);

	r32 invDet = 1.f/det;

	m2x2 invMat = m2x2({
			{ invDet*mat.d22, -invDet*mat.d12 },
			{ -invDet*mat.d21, invDet*mat.d11 },
		});

	return invMat;
}

inline
m3x3 inverse(m3x3& mat)
{
	r32 det = determinant(mat);
	Assert(abs(det) > 1e-6);

	r32 invDet = 1.f/det;

	m3x3 invMat = m3x3({
			{
				+invDet*(mat.d22*mat.d33-mat.d23*mat.d32),
				-invDet*(mat.d12*mat.d33-mat.d13*mat.d32),
				+invDet*(mat.d12*mat.d23-mat.d13*mat.d22),
			},
			{
				-invDet*(mat.d21*mat.d33-mat.d23*mat.d31),
				+invDet*(mat.d11*mat.d33-mat.d13*mat.d31),
				-invDet*(mat.d11*mat.d23-mat.d13*mat.d21),
			},
			{
				+invDet*(mat.d21*mat.d32-mat.d22*mat.d31),
				-invDet*(mat.d11*mat.d32-mat.d12*mat.d31),
				+invDet*(mat.d11*mat.d22-mat.d12*mat.d21),
			},
		});

	return invMat;
}

template<typename T>
inline sf::Vector3<T> from_euler_to_homogenous(sf::Vector2<T> vec)
{
	sf::Vector3<T> homo_vec = {vec.x, vec.y, 1};
	return homo_vec;
}

template<typename T>
inline sf::Vector2<T> from_homogenous_to_euler(sf::Vector3<T> vec)
{
	sf::Vector2<T> euler_vec = {vec.x/vec.z, vec.y/vec.z};
	return euler_vec;
}

template<typename T>
inline T length2(sf::Vector3<T> vec)
{
	T length = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
	return length;
}

template<typename T>
inline T length2(sf::Vector2<T> vec)
{
	T length = vec.x*vec.x + vec.y*vec.y;
	return length;
}

template<typename T>
inline T length(sf::Vector3<T> vec)
{
	T length = std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
	return length;
}

template<typename T>
inline T length(sf::Vector2<T> vec)
{
	T length = std::sqrt(vec.x*vec.x + vec.y*vec.y);
	return length;
}

template<typename T>
inline sf::Vector3<T> normalize(sf::Vector3<T> vec)
{
	return vec/length(vec);
}

template<typename T>
inline sf::Vector2<T> normalize(sf::Vector2<T> vec)
{
	return vec/length(vec);
}

template<typename T>
inline sf::Vector2<T> safe_normalize(sf::Vector2<T> vec)
{
	T norm = length(vec);
	if(abs(norm) < 0.001) return sf::Vector2<T>();
	return vec/norm;
}

template<typename T>
inline sf::Vector3<T> safe_normalize(sf::Vector3<T> vec)
{
	T norm = length(vec);
	if(abs(norm) < 0.001) return sf::Vector3<T>();
	return vec/norm;
}


template<typename T>
inline T dot(sf::Vector3<T> v1, sf::Vector3<T> v2)
{
	T result = v1.x*v2.x;
	result += v1.y*v2.y;
	result += v1.z*v2.z;
	return result;
}

template<typename T>
inline T dot(sf::Vector2<T> v1, sf::Vector2<T> v2)
{
	T result = v1.x*v2.x + v1.y*v2.y;
	return result;
}

template<typename T>
inline sf::Vector3<T> cross(sf::Vector3<T> v1, sf::Vector3<T> v2)
{
	sf::Vector3<T> result;

	result.x = v1.y*v2.z-v1.z*v2.y;
	result.y = v1.z*v2.x-v1.x*v2.z;
	result.z = v1.x*v2.y-v1.y*v2.x;
	
	return result;
}

template<typename T>
inline T cross_signed_norm(sf::Vector2<T> v1, sf::Vector2<T> v2)
{
	T result;
	result = v1.x*v2.y - v1.y*v2.x;
	return result;
}

template<typename T>
inline sf::Vector2<T> orth(sf::Vector2<T> vec)
{
	T tmp = vec.x;
	vec.x = -vec.y;
	vec.y = tmp;

	return vec;
}

template<typename T>
inline sf::Vector3<T> hadamar(sf::Vector3<T> v1, sf::Vector3<T> v2)
{
	sf::Vector3<T> result;

	result.x = v1.x*v2.x;
	result.y = v1.y*v2.y;
	result.z = v1.z*v2.z;

	return result;
}

template<typename T>
inline sf::Vector3<T> hadamar_inv(sf::Vector3<T> v1, sf::Vector3<T> v2)
{
	sf::Vector3<T> result;

	result.x = v1.x/v2.x;
	result.y = v1.y/v2.y;
	result.z = v1.z/v2.z;

	return result;
}

template<typename T>
inline sf::Vector2<T> hadamar(sf::Vector2<T> v1, sf::Vector2<T> v2)
{
	sf::Vector2<T> result;

	result.x = v1.x*v2.x;
	result.y = v1.y*v2.y;

	return result;
}

template<typename T>
inline sf::Vector2<T> hadamar_inv(sf::Vector2<T> v1, sf::Vector2<T> v2)
{
	sf::Vector2<T> result;

	result.x = v1.x/v2.x;
	result.y = v1.y/v2.y;

	return result;
}

template<typename T>
inline sf::Vector2<T> rect_pos(sf::Rect<T> rect)
{
	sf::Vector2<T> result = {rect.left, rect.top};
	return result;
}

template<typename T>
inline sf::Vector2<T> rect_size(sf::Rect<T> rect)
{
	sf::Vector2<T> result = {rect.width, rect.height};
	return result;
}
