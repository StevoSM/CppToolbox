//----------------------------------------------------------------------------------------------------------------------
//	C2DGeometry.h			©2012 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: T2DUtilities

template <typename T> struct T2DUtilities {
	// Methods
	inline	static	T	toDegrees(T angleInRadians) { return angleInRadians * 180.0 / M_PI; }
	inline	static	T	toRadians(T angleInDegrees) { return angleInDegrees * M_PI / 180.0; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: T2DPoint

template <typename T> struct T2DAffineTransform;
template <typename T> struct T2DPoint {
						// Lifecycle methods
						T2DPoint() : mX(0.0), mY(0.0) {}
						T2DPoint(T x, T y) : mX(x), mY(y) {}
						T2DPoint(const CString& string)
							{
								// {0.0,0.0} => ,0.0,0.0}
								CString			stringUse(string);
								TArray<CString>	array =
														stringUse.replaceSubStrings(CString("{"), CString(","))
																.breakUp(CString(","));
								if (array.getCount() >= 3) {
									// Extract values
									mX = array[1].getFloat32();
									mY = array[2].getFloat32();
								} else {
									// Use default values
									mX = 0.0;
									mY = 0.0;
								}
							}

						// Instance methods
	inline	T2DPoint	offset(T offsetX, T offsetY) const
							{ return T2DPoint(mX + offsetX, mY + offsetY); }
	inline	T2DPoint	midpoint(const T2DPoint& point, T ratio) const
							{ return T2DPoint(mX + (point.mX - mX) * ratio, mY + (point.mY - mY) * ratio); }
	inline	T2DPoint	reflectedBy(const T2DPoint& anchorPoint) const
							{
								return T2DPoint(anchorPoint.mX + (anchorPoint.mX - mX),
										anchorPoint.mY + (anchorPoint.mY - mY));
							}
	inline	T			distanceTo(const T2DPoint& point) const
							{ return sqrt(distanceSquaredTo(point)); }
	inline	T			distanceSquaredTo(const T2DPoint& point) const
							{
								T	dx = point.mX - mX;
								T	dy = point.mY - mY;

								return dx * dx + dy * dy;
							}

	inline	bool		operator==(const T2DPoint& other) const
							{ return (mX == other.mX) && (mY == other.mY); }
	inline	bool		operator!=(const T2DPoint& other) const
							{ return (mX != other.mX) || (mY != other.mY); }

	inline	T2DPoint&	applyTransform(const T2DAffineTransform<T>& affineTransform)
							{
								mX = mX * affineTransform.mA + mY * affineTransform.mC + affineTransform.mTX;
								mY = mX * affineTransform.mB + mY * affineTransform.mD + affineTransform.mTY;
								
								return *this;
							}

	inline	CString		asString() const
							{
								return CString("{") + CString(mX, 5, 3) + CString(",") + CString(mY, 5, 3) +
										CString("}");
							}

	// Properties
			T			mX;
			T			mY;

	static	T2DPoint<T>	mZero;
};

typedef	T2DPoint<Float32>	S2DPoint32;
typedef	T2DPoint<Float64>	S2DPoint64;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DSize

template <typename T> struct T2DSize {
					// Lifecycle methods
					T2DSize() : mWidth(0.0), mHeight(0.0) {}
					T2DSize(T width, T height) : mWidth(width), mHeight(height) {}
					T2DSize(const CString& string)
						{
							// {0.0,0.0} => ,0.0,0.0}
							CString			stringUse(string);
							TArray<CString>	array =
													stringUse.replaceSubStrings(CString("{"), CString(","))
															.breakUp(CString(","));
							if (array.getCount() >= 3) {
								// Extract values
								mWidth = array[1].getFloat32();
								mHeight = array[2].getFloat32();
							} else {
								// Use default values
								mWidth = 0.0;
								mHeight = 0.0;
							}
						}

					// Instance methods
	inline	bool	operator==(const T2DSize& other) const
						{ return (mWidth == other.mWidth) && (mHeight == other.mHeight); }
	inline	bool	operator!=(const T2DSize& other) const
						{ return (mWidth != other.mWidth) || (mHeight != other.mHeight); }

	inline	CString	asString() const
						{
							return CString("{") + CString(mWidth, 5, 3) + CString(",") + CString(mHeight, 5, 3) +
									CString("}");
						}
	// Properties
			T			mWidth;
			T			mHeight;

	static	T2DSize<T>	mZero;
};

typedef	T2DSize<Float32>	S2DSize32;
typedef	T2DSize<Float64>	S2DSize64;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DVector

template <typename T> struct T2DVector {
							// Lifecycle methods
							T2DVector() : mDX(0.0), mDY(0.0) {}
							T2DVector(T dx, T dy) : mDX(dx), mDY(dy) {}
							T2DVector(const T2DPoint<T>& startPoint, const T2DPoint<T>& endPoint) :
								mDX(endPoint.mX - startPoint.mX), mDY(endPoint.mY - startPoint.mY) {}

							// Instance methods
	inline	T				magnitude() const
								{ return sqrt(magnitudeSquared()); }
	inline	T				magnitudeSquared() const
								{ return mDX * mDX + mDY * mDY; }
	inline	T				dot(const T2DVector& vector) const
								{ return mDX * vector.mDX + mDY * vector.mDY; }
	inline	T				angle(const T2DVector& v) const
								{
									// Calculate value
									T	value = dot(v) / (magnitude() * v.magnitude());
									if (value > 1.0)
										value = 1.0;
									else if (value < -1.0)
										value = -1.0;
									
									// Return angle
									return ((mDX * v.mDY - mDY * v.mDX) >= 0.0) ? acosf(value) : -acosf(value);
								}
	inline	T2DVector<T>	normalized() const { return *this / magnitude(); }

	inline	T2DPoint<T>		operator+(const T2DPoint<T>& other) { return T2DPoint<T>(other.mX + mDX, other.mY + mDY); }
	inline	T2DVector<T>	operator+(const T2DVector<T>& other)
								{ return T2DVector<T>(mDX + other.mDX, mDY + other.mDY); }
	inline	T2DVector<T>	operator-(const T2DVector<T>& other)
								{ return T2DVector<T>(mDX - other.mDX, mDY - other.mDY); }

	inline	T2DVector<T>	operator*(T factor) const { return T2DVector<T>(mDX * factor, mDY * factor); }
	inline	T2DVector<T>&	operator*=(T factor) { return mDX *= factor, mDY *= factor; return *this; }
	inline	T2DVector<T>	operator/(T factor) const { return T2DVector<T>(mDX / factor, mDY / factor); }
	inline	T2DVector<T>&	operator/=(T factor) { return mDX /= factor, mDY /= factor; return *this; }

	// Properties
	T	mDX;
	T	mDY;
};

typedef	T2DVector<Float32>	S2DVector32;
typedef	T2DVector<Float64>	S2DVector64;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DRect

template <typename T> struct T2DRect {
					// Lifecycle methods
					T2DRect() : mOrigin(0.0, 0.0), mSize(0.0, 0.0) {}
					T2DRect(T x, T y, T width, T height) : mOrigin(x, y), mSize(width, height) {}
					T2DRect(const T2DPoint<T>& origin, const T2DSize<T>& size) : mOrigin(origin), mSize(size) {}
					T2DRect(const CString& string)
						{
							// {{0.0,0.0},{0.0,0.0}} => ,,0.0,0.0},,0.0,0.0}}
							CString			stringUse(string);
							TArray<CString>	array =
													stringUse.replaceSubStrings(CString("{"), CString(","))
															.breakUp(CString(","));
							if (array.getCount() >= 7) {
								// Extract values
								mOrigin.mX = array[2].getFloat32();
								mOrigin.mY = array[3].getFloat32();
								mSize.mWidth = array[5].getFloat32();
								mSize.mHeight = array[6].getFloat32();
							} else {
								// Use default values
								mOrigin.mX = 0.0;
								mOrigin.mY = 0.0;
								mSize.mWidth = 0.0;
								mSize.mHeight = 0.0;
							}
						}

					// Instance methods
	inline	T		getMinX() const
						{ return mOrigin.mX; }
	inline	T		getMidX() const
						{ return mOrigin.mX + 0.5 * mSize.mWidth; }
	inline	T		getMaxX() const
						{ return mOrigin.mX + mSize.mWidth; }
	inline	T		getMinY() const
						{ return mOrigin.mY; }
	inline	T		getMidY() const
						{ return mOrigin.mY + 0.5 * mSize.mHeight; }
	inline	T		getMaxY() const
						{ return mOrigin.mY + mSize.mHeight; }
	inline	bool	isEmpty() const
						{ return (mSize.mWidth == 0.0) && (mSize.mHeight == 0.0); }
	inline	bool	contains(const T2DPoint<T>& point) const
						{
							return (point.mX >= getMinX()) && (point.mX < getMaxX()) &&
									(point.mY >= getMinY()) && (point.mY < getMaxY());
						}
	inline	bool	intersects(const T2DRect& rect) const
						{
							return !((getMaxX() < rect.getMinX()) ||
									 (getMinX() > rect.getMaxX()) ||
									 (getMaxY() < rect.getMinY()) ||
									 (getMinY() > rect.getMaxY()));
						}

	inline	bool	operator==(const T2DRect& other) const
						{ return (mOrigin == other.mOrigin) && (mSize == other.mSize); }
	inline	bool	operator!=(const T2DRect& other) const
						{ return (mOrigin != other.mOrigin) || (mSize != other.mSize); }

	inline	CString	asString() const
						{
							return CString("{{") + CString(mOrigin.mX, 5, 3) + CString(",") +
									CString(mOrigin.mY, 5, 3) + CString("},{") + CString(mSize.mWidth, 5, 3) +
									CString(",") + CString(mSize.mHeight, 5, 3) + CString("}}");
						}

	// Properties
			T2DPoint<T>	mOrigin;
			T2DSize<T>	mSize;

	static	T2DRect<T>	mZero;
};

typedef	T2DRect<Float32>	S2DRect32;
typedef	T2DRect<Float64>	S2DRect64;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DAffineTransform

// See CGAffineTransform
/*
	--		   --
	|	mA	mB	|
	|	mC	mD	|
	|	mTX	mTY	|
	--		   --
		
	x' = mA * x + mC * y + mTX
	y' = mB * x + mD * y + mTY
	
	rotation by a
		mA = cos a
		mB = sin a
		mC = -sin a
		mD = cos a
	
	scale by sx, sy
		mA = sx
		mD = sy
	
	translate by tx, ty
		mTX = tx
		mTY = ty
*/

template <typename T> struct T2DAffineTransform {
								// Lifecycle methods
								T2DAffineTransform() : mA(1.0), mB(0.0), mC(0.0), mD(1.0), mTX(0.0), mTY(0.0) {}
								T2DAffineTransform(T a, T b, T c, T d, T tx, T ty) :
									mA(a), mB(b), mC(c), mD(d), mTX(tx), mTY(ty) {}

								// Instance methods
			void				invert()
									{ *this = inverted(); }
			T2DAffineTransform	inverted() const
									{
										T	det = 1.0 / (mA * mD - mB * mC);

										T	a = mD * det;
										T	b = -mB * det;
										T	c = -mC * det;
										T	d = mA * det;

										return T2DAffineTransform(a, b, c, d, -mTX * a - mTY * c, -mTX * b - mTY * d);
									}
			void				concat(const T2DAffineTransform& affineTransform)
									{ *this = concated(affineTransform); }
	inline	T2DAffineTransform	concated(const T2DAffineTransform& affineTransform) const
									{
										return T2DAffineTransform(
												mA * affineTransform.mA + mB * affineTransform.mC,
												mA * affineTransform.mB + mB * affineTransform.mD,
												mC * affineTransform.mA + mD * affineTransform.mC,
												mC * affineTransform.mB + mD * affineTransform.mD,
												mTX * affineTransform.mA + mTY * affineTransform.mC +
														affineTransform.mTX,
												mTX * affineTransform.mB + mTY * affineTransform.mD +
														affineTransform.mTY);
									}

	// Properties
			T						mA;
			T						mB;
			T						mC;
			T						mD;
			T						mTX;
			T						mTY;

	static	T2DAffineTransform<T>	mIdentity;
};

typedef	T2DAffineTransform<Float32>	S2DAffineTransform32;
typedef	T2DAffineTransform<Float64>	S2DAffineTransform64;
