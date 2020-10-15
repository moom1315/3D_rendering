#include "basic.h"
#include <string>

vector <vector <float>> matrixMultiplication(vector <vector <float>>matrixA, vector <vector <float>>matrixB)
{
	vector <vector <float>> matrix2D;
	vector <float> matrix1D;
	if(matrixA.size()==0||matrixB.size()==0)
		return matrix2D;
	int length = matrixA.size();
	int width = matrixB[0].size();
	int mul = matrixA[0].size();

	for(int i = 0; i<length; i++)
	{
		for(int j = 0; j<width; j++)
		{
			float total = 0;
			for(int k = 0; k<mul; k++)
			{
				total += matrixA[i][k]*matrixB[k][j];
			}
			matrix1D.push_back(total);
		}
		matrix2D.push_back(matrix1D);
		matrix1D.clear();
	}
	return matrix2D;
}

vector <float> readFunc(string input, int amount, int& pos)
{
	vector <float> result(amount);
	for(int i = 0; i<amount; i++)
	{
		//ignore space
		while(input[pos]==' ')
			pos++;
		result[i] = stof(input.substr(pos, input.find(" ", pos)));
		pos = input.find(" ", pos);
	}
	return result;
}

void normalization(point& p)
{
	float total = sqrt(pow(p.x_pos, 2)+pow(p.y_pos, 2)+pow(p.z_pos, 2));
	p.x_pos /= total;
	p.y_pos /= total;
	p.z_pos /= total;
}

bool sortPoints(point p1, point p2)
{
	return p1.y_pos>p2.y_pos;
}

bool sortPointsX(point p1, point p2)
{
	return p1.x_pos>p2.x_pos;
}

bool sortPointsDepth(point p1, point p2)
{
	return p1.depth>p2.depth;
}
