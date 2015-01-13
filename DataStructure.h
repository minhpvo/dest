#include <cstdlib>
#include <opencv\cv.hpp>
#include "GL\glut.h"


using namespace cv;
using namespace std;

#if !defined( DATASTRUCTURE_H )
#define DATASTRUCTURE_H

#define MAXSIFTPTS 15000
#define SIFTBINS 128
#define FISHEYE  -1
#define RADIAL_TANGENTIAL_PRISM  0
#define VisSFMLens 1
#define LUT  2
#define LIMIT3D 1e-6
#define Pi 3.1415926535897932


struct CameraData
{
	double K[9], distortion[7], R[9], T[3], rt[6], P[12], intrinsic[5], invK[9], invR[9];
	double Rgl[16], camCenter[3];
	int LensModel;
	double threshold, ninlierThresh;
	std::string filename;
	int nviews, width, height;
	bool notCalibrated;	
};

struct SurfDesc
{
	float desc[64];
};
struct SiftDesc
{
	float desc[128];
};

struct Corpus
{
	int nCamera, n3dPoints;
	vector<int> IDCumView;
	vector<string> filenames;
	CameraData *camera;

	vector<Point3d>  xyz;
	vector<Point3i >  rgb;	
	vector < vector<int>> viewIdAll3D; //3D -> visiable views index
	vector < vector<int>> pointIdAll3D; //3D -> 2D index in those visible views
	vector<vector<Point2d>> uvAll3D; //3D -> uv of that point in those visible views
	vector < vector<int>>threeDIdAllViews; //2D point in visible view -> 3D index
	vector < vector<int>> orgtwoDIdAll3D; //visualsfm output order
	vector<vector<Point2d>> uvAllViews;
	Mat SiftDesc, SurfDesc;
};

struct CorpusandVideo
{
	int nViewsCorpus, nVideos, startTime, stopTime, CorpusSharedIntrinsics;
	CameraData *CorpusInfo;
	CameraData *VideoInfo;
};
struct VideoData
{
	int nVideos, startTime, stopTime;
	CameraData *VideoInfo;
};

struct CamInfo
{
	float camCenter[3];
	GLfloat Rgl[16];
};
struct Trajectory2D
{
	int timeID, nViews;
	vector<int>viewIDs;
	vector<Point2d> uv;
	vector<float>angle;
};
struct TrajectoryData
{
	vector<Point3d> *cpThreeD;
	vector<Point3d> *fThreeD;
	vector<Point3d> *cpNormal;
	vector<Point3d> *fNormal;
	vector<Point2d> *cpTwoD;
	vector<Point2d> *fTwoD;
	vector<vector<int>> *cpVis;
	vector<vector<int>> *fVis;
	int nTrajectories, nViews;
	vector<Trajectory2D> *trajectoryUnit;
};

struct VisualizationManager
{
	Point3d g_trajactoryCenter;
	vector<Point3d> PointPosition;
	vector<Point3f> PointColor;
	vector<Point3d>PointNormal;
	vector<CamInfo> glCameraInfo;
	vector<CamInfo> *glCameraPoseInfo;
};


template <class T> class Points;
//#define USE_MEM_COLLECT


template <class T>
class Points
{
	typedef T(*TV2)[2];
	typedef T(*TV3)[3];
	typedef T(*TV5)[5];
	typedef T(*TV128)[128];
protected:
	T *		_data;
	T **	_matrix;
	int		_width;
	int		_height;
	bool	_subset;
private:
	void init(int width, int height)
	{
		_subset = 0;
		if (width >0 && height >0)
		{
			_matrix = (T**)malloc(sizeof(T)*width*height + sizeof(T*)*height);
			if (_matrix)
			{
				_data = (T*)(_matrix + height);
				T* temp = _data;
				for (int i = 0; i < height; _matrix[i++] = temp, temp += width);
				_width = width;
				_height = height;
			}
			else
			{
				_width = _height = 0;
				_data = NULL;
			}
		}
		else
		{
			_width = _height = 0;
			_data = NULL;
			_matrix = NULL;
		}
	}

public:
	Points()
	{
		_data = NULL;
		_matrix = NULL;
		_height = 0;
		_width = 0;
		_subset = 0;
	}
	Points(const Points<T> &ref)
	{
		init(ref.width(), ref.height());
		copy_from(ref, 0, 0);
	}
	Points(int width, int height)
	{
		init(width, height);
	}
	Points(int width, int height, T v)
	{
		init(width, height);
		fill(v);
	}
	Points(T** sup, int ndim, int num)
	{
		_subset = true;
		_width = ndim;
		_height = num;
		_data = NULL;
		if (num>0 && ndim>0)
		{
			_matrix = (T**)malloc(sizeof(T*)*num);
			for (int i = 0; i < num; i++) _matrix[i] = sup[i];
		}
		else
		{
			_matrix = NULL;
		}
	}
	void setx(int width, int height, T* data)
	{
		release();
		_subset = 1;
		_width = width;
		_height = height;
		_data = data;
		_matrix = (T**)malloc(sizeof(T*) * height);
		for (int i = 0; i < height; i++, data += width) _matrix[i] = data;
	}


	template <class POINTS>
	Points(POINTS&   sup, int index[], int num)
	{
		_subset = true;
		_width = sup.width();
		_height = num;
		_data = sup.data();
		if (num>0 && _width>0)
		{
			_matrix = (T**)malloc(sizeof(T*)*num);
			for (int i = 0; i < num; i++) _matrix[i] = sup[index[i]];
		}
		else
		{
			_matrix = NULL;
		}
	}


private:
	void release()
	{
		if (_matrix)	free(_matrix);
	}
public:
	void resize(int width, int height)
	{
		if (width == _width && height == _height) return;
		release();
		init(width, height);
	}
	void resize(int width, int height, T v)
	{
		resize(width, height);
		fill(v);
	}
	void expand(int width, int height)
	{
		//make sure size are 
		if (width < _width || height < _height) return;
		if (width == _width && height == _height && _subset == 0) return;
		T* newdata, **newmatrix, *temp;
		int i;
		newmatrix = (T**)malloc(sizeof(T)*width*height + sizeof(T*)*height);
		temp = newdata = (T*)(newmatrix + height);

		for (i = 0; i < height; i++, temp += width)
		{
			newmatrix[i] = temp;
		}
		for (i = 0; i< _height; i++)
		{
			memcpy(newmatrix[i], _matrix[i], _width*sizeof(T));
		}
		release();
		_subset = 0;
		_width = width;
		_height = height;
		_matrix = newmatrix;
		_data = newdata;
	}


	void expand(int width, int height, T v)
	{
		//make sure size are 
		if (width < _width || height < _height) return;
		if (width == _width && height == _height && _subset == 0) return;
		T* newdata, **newmatrix, *temp;
		int i, j;
		newmatrix = (T**)malloc(sizeof(T)*width*height + sizeof(T*)*height);
		temp = newdata = (T*)(newmatrix + height);

		for (i = 0; i < height; i++, temp += width)
		{
			newmatrix[i] = temp;
		}
		for (i = 0; i< _height; i++)
		{
			memcpy(newmatrix[i], _matrix[i], _width*sizeof(T));
			for (j = _width; j < width; ++j) newmatrix[i][j] = v;
		}
		for (; i < height; i++)
		{
			for (j = 0; j < width; ++j) newmatrix[i][j] = v;
		}
		release();
		_subset = 0;
		_width = width;
		_height = height;
		_matrix = newmatrix;
		_data = newdata;
	}

	void copy_from(const Points<T>& src, int dx, int dy)
	{
		int nw = min(src.width(), width() - dx);
		int nh = min(src.height(), height() - dy);
		/////////////////////////////////////////////
		for (int i = 0; i < nh; ++i)
		{
			for (int j = 0; j < nw; ++j)
			{
				_matrix[i + dy][j + dx] = src._matrix[i][j];
			}
		}
	}

	void swap(Points<T>& temp)
	{
		T *		data = _data;
		T **	matrix = _matrix;
		int		width = _width;
		int		height = _height;
		bool	subset = _subset;
		_data = temp._data;
		_matrix = temp._matrix;
		_width = temp._width;
		_height = temp._height;
		_subset = temp._subset;
		temp._data = data;
		temp._matrix = matrix;
		temp._width = width;
		temp._height = height;
		temp._subset = subset;
	}

	void make_sub_from(Points<T>& src, int dx, int dy)
	{
		int nw = min(src.width() - dx, width());
		int nh = min(src.height() - dy, height());
		/////////////////////////////////////////////
		for (int i = 0; i < nh; ++i)
		{
			for (int j = 0; j < nw; ++j)
			{
				_matrix[i][j] = src._matrix[i + dy][j + dx];
			}
		}
	}
	void shrink(int width, int height)
	{
		//make sure size are 
		if (width > _width || height > _height) return;
		if (width == _width && height == _height) return;
		T* newdata, **newmatrix, *temp;
		int i;
		newmatrix = (T**)malloc(sizeof(T)*width*height + sizeof(T*)*height);
		temp = newdata = (T*)(newmatrix + height);

		for (i = 0; i < height; i++, temp += width)
		{
			newmatrix[i] = temp;
		}
		for (i = 0; i< height; i++)
		{
			memcpy(newmatrix[i], _matrix[i], width*sizeof(T));
		}
		release();
		_subset = 0;
		_width = width;
		_height = height;
		_matrix = newmatrix;
		_data = newdata;
	}
	~Points()
	{
		release();
	}
	T* data()
	{
		return _data;
	}
	T* end()
	{
		return _data + _width * _height;
	}
	int invalid()
	{
		return _width == 0 || _height == 0;
	}
	int bsize()
	{
		return _width*_height*sizeof(T);
	}
	int width() const
	{
		return _width;
	}
	int height() const
	{
		return _height;
	}
	int ndim() const
	{
		return _width;
	}
	int npoint() const
	{
		return _height;
	}
	T* operator[](int i)
	{
		return _matrix[i];
	}
	T* operator[](int i) const
	{
		return _matrix[i];
	}
	Points<T>& operator = (const Points<T>& ref)
	{
		resize(ref.width(), ref.height());
		if (_width > 0 && _height > 0)
		{
			if (ref._subset)
			{
				for (int i = 0; i < _height; ++i)
				{
					memcpy(_matrix[i], ref._matrix[i], _width * sizeof(T));
				}
			}
			else
			{
				memcpy(_data, ref._data, bsize());
			}
		}
		return *this;
	}
	void own_data()
	{
		if (_subset)expand(_width, _height);
	}
	operator T** const () const
	{
		return _matrix;
	}
	operator T** ()
	{
		return _matrix;
	}
	operator TV2()
	{
#ifdef _DEBUG
		return _width == 2 && _subset == 0 ? (T(*)[2]) _data : NULL;
#else
		return (T(*)[2]) _data;
#endif
	}
	operator TV3()
	{
#ifdef _DEBUG
		return _width == 3 && _subset == 0 ? (T(*)[3]) _data : NULL;
#else
		return (T(*)[3]) _data;
#endif
	}
	operator TV5()
	{
#ifdef _DEBUG
		return _width == 5 && _subset == 0 ? (T(*)[5]) _data : NULL;
#else
		return (T(*)[5]) _data;
#endif
	}
	operator TV128()
	{
#ifdef _DEBUG
		return _width == 128 && _subset == 0 ? (T(*)[128]) _data : NULL;
#else
		return (T(*)[128]) _data;
#endif
	}
	int checkdim(int dim)
	{
		if (dim >= _width) expand(_width * 2, _height);
		return _width;
	}

	T* getpt(int i)
	{
		return _matrix[i];
	}
	T* getpt(int i)const
	{
		return _matrix[i];
	}
	void fill(T v)
	{
		int i = _width*_height;
		T* temp = _data;
		for (; i>0; i--)*temp++ = v;
	}

	void reorder(const int* index)
	{
		//
		if (0 == _width && 0 == _height) return;
		T* newdata, **newmatrix, *temp;
		int i;
		newmatrix = (T**)malloc(sizeof(T)*_width*_height + sizeof(T*)*_height);
		temp = newdata = (T*)(newmatrix + _height);

		for (i = 0; i < _height; i++)
		{
			newmatrix[i] = temp;
			temp += _width;
		}
		for (i = 0; i< _height; i++)
		{
			memcpy(newmatrix[i], _matrix[index[i]], _width*sizeof(T));
		}
		release();
		_subset = 0;
		_matrix = newmatrix;
		_data = newdata;

	}

};


//triangle matrix
template <class T>
class PointsT
{
protected:
	T **	_matrix;
	int		_width;
private:
	T** allocate(int width)
	{
		if (width >0)
		{
			T** mat = (T**)malloc(sizeof(T*) * width);
			for (int i = 0; i < _width; ++i)
			{
				mat[i] = ((T*)malloc(sizeof(T) * (width - i))) - i;
			}
			return mat;
		}
		else
		{
			return NULL;
		}
	}
public:
	PointsT()
	{
		_matrix = NULL;
		_width = 0;
	}
	PointsT(int width, int height)
	{
		_width = width > height ? width : height;
		_matrix = allocate(_width);
	}

	void release()
	{
		if (_matrix && _width > 0)
		{
			for (int i = 0; i < _width; ++i)
			{
				free(_matrix[i] + i);
			}
			free(_matrix);
			_matrix = NULL;
			_width = 0;
		}
	}
public:
	void expand(int width, int height, T v)
	{
		expand(width > height ? width : height, v);
	}
	void expand(int width, T v)
	{
		//make sure size are 
		if (width <= _width) return;

		T** newmatrix = (T**)malloc(sizeof(T*)*width);
		int i = 0, dw = width - _width;
		if (dw <= _width)
		{
			for (; i < dw; ++i)
			{
				newmatrix[i] = ((T*)malloc(sizeof(T) * (width - i))) - i;
				memcpy(newmatrix[i] + i, _matrix[i] + i, (_width - i) * sizeof(T));
				//memset(newmatrix[i] + _width, 0, dw * sizeof(T));
				for (int j = _width; j < width; ++j)newmatrix[i][j] = v;
			}
			for (; i < _width; ++i)
			{
				newmatrix[i] = _matrix[i - dw] - dw;
				memcpy(newmatrix[i] + i, _matrix[i] + i, (_width - i) * sizeof(T));
				//memset(newmatrix[i] + _width, 0, dw * sizeof(T));
				for (int j = _width; j < width; ++j)newmatrix[i][j] = v;
			}
		}
		else
		{
			for (; i < _width; ++i)
			{
				newmatrix[i] = ((T*)malloc(sizeof(T) * (width - i))) - i;
				memcpy(newmatrix[i] + i, _matrix[i] + i, (_width - i) * sizeof(T));
				//memset(newmatrix[i] + _width, 0, dw * sizeof(T));
				for (int j = _width; j < width; ++j)newmatrix[i][j] = v;
			}
			for (; i < dw; ++i)
			{
				newmatrix[i] = ((T*)malloc(sizeof(T) * (width - i))) - i;
				//memset(newmatrix[i] + _width, 0, dw * sizeof(T));
				for (int j = i; j < width; ++j)newmatrix[i][j] = v;
			}
		}

		for (; i < width; ++i)
		{
			newmatrix[i] = _matrix[i - dw] - dw;
			for (int j = i; j < width; ++j) newmatrix[i][j] = v;
		}

		free(_matrix);
		_matrix = newmatrix;
		_width = width;
	}

	~PointsT()
	{
		release();
	}

	int invalid()
	{
		return _width == 0;
	}
	int width() const
	{
		return _width;
	}

	int ndim() const
	{
		return _width;
	}

	T* operator[](int i)
	{
		return _matrix[i];
	}
	T* operator[](int i) const
	{
		return _matrix[i];
	}
	operator T** const () const
	{
		return _matrix;
	}
	operator T** ()
	{
		return _matrix;
	}

	void fill(T v)
	{
		for (int i = 0; i < _width; ++i)
		{
			for (int j = i; j < _width; ++j)
			{
				_matrix[i][j] = v;
			}
		}
	}
};

#endif 