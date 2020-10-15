#pragma once

#include <vector>
#include <cmath>

using namespace std;

class color
{
public:
	color()
	{}
	color(float r, float g, float b)
	{
		red = r, green = g, blue = b;
	}
	float red = 0, green = 0, blue = 0;
};

class point
{
public:
	point()
	{}
	point(float x, float y, float z)
	{
		x_pos = x, y_pos = y, z_pos = z;
	}

	float  x_pos = 0, y_pos = 0, z_pos = 0;
	double depth = 0;
	color pcolor;

	void color_point(float r, float g, float b)
	{
		pcolor.red = r, pcolor.green = g, pcolor.blue = b;
	}
};

class light: public color
{
public:
	light()
	{}
	light(float r, float g, float b, float x, float y, float z): color(r, g, b)
	{
		red = r, green = g, blue = b, x_pos = x, y_pos = y, z_pos = z;
	}
	float  x_pos = 0, y_pos = 0, z_pos = 0;
	point light_vec;
};

extern vector <point> coloringPlanes;
class line
{
public:
	line(float x1, float y1, float x2, float y2)
	{
		x_start = x1, y_start = y1, x_end = x2, y_end = y2;
		if(x1!=x2)
			slope = (y2-y1)/(x2-x1);
		else
			vertical = true;
	}
	float x_start = 0, y_start = 0, x_end = 0, y_end = 0, slope = 0;
	bool vertical = false;

	void drawTheLine(double depth1, double depth2)
	{
		point addingPoint(0, 0, 0);
		//vertical
		if(vertical)
		{
			if(y_start>y_end)
			{
				swap(y_start, y_end);
				swap(depth1, depth2);
			}
			for(float plotY = y_start+1; plotY<=y_end; plotY++)
			{
				addingPoint.x_pos = x_start;
				addingPoint.y_pos = plotY;
				addingPoint.depth = (depth2-depth1) * static_cast<double> (plotY-y_start)/static_cast<double> (y_end-y_start)+depth1;
				coloringPlanes.push_back(addingPoint);
			}
			return;
		}
		//not vertical
		// wrong direction
		if((slope>0&&x_start>x_end)||(slope<0&&x_end > x_start))
		{
			swap(x_start, x_end);
			swap(y_start, y_end);
			swap(depth1, depth2);
		}
		if(slope<0)
		{
			x_start *= -1;
			x_end *= -1;
		}
		if(abs(slope)>1)
		{
			swap(x_start, y_start);
			swap(x_end, y_end);
		}
		// start to draw
		float constA = y_end-y_start;
		float constB = x_start-x_end;
		float dLine = 2*constA+constB;
		float plotY = y_start;
		for(float plotX = x_start+1; plotX<x_end; plotX++)
		{
			plotY += (dLine<=0) ? 0 : 1;
			dLine += (dLine<=0) ? 2*constA : 2*(constA+constB);
			//draw the line depend on input slope
			if(abs(slope)>1)
				swap(addingPoint.x_pos, addingPoint.y_pos);
			if(slope<0)
				addingPoint.x_pos *= -1;
			addingPoint.depth = (depth2-depth1) * static_cast <double> (plotX-x_start)/static_cast<double> (x_end-x_start)+depth1;
			coloringPlanes.push_back(addingPoint);
		}
	}
};

class asc
{
public:
	asc(float r, float g, float b, float kd, float ks, float n)
	{
		ascColor.red = r, ascColor.green = g, ascColor.blue = b, ascKd = kd, ascKs = ks, ascN = n;
	}
	color ascColor;
	float ascKd = 0, ascKs = 0, ascN = 0;
	vector<vector <float>> ascMatrix;
	vector <vector<float>> originMatrix;
	vector <vector<int>> ascPlane;
	vector <vector<int>> originPlane;
};

vector <vector <float>> matrixMultiplication(vector <vector <float>>matrixA, vector <vector <float>>matrixB);
vector <float> readFunc(string input, int amount, int& pos);
void normalization(point p);
bool sortPoints(point p1, point p2);
bool sortPointsX(point p1, point p2);
bool sortPointsDepth(point p1, point p2);
