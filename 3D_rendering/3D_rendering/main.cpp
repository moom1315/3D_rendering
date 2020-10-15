#include <gl/glut.h>
#include <iostream>
#include <fstream>
#include <string>
#include "basic.h"

//global variable
ifstream file;
int windowWidth = 0, windowHeight = 0;
float xRatio = 0, yRatio = 0;
vector <float> readNums;
vector <float> viewportVertex(4);

const float pi = 3.1415926/180.0f;

//background value
//color
color ambient;
color BGColor;
//points
point eyePosition;
point COIPosition;

//objects value
//point
vector <point> coloringPlanes;
//light
vector <light> lights;
//ASC
vector <asc> asc_obj;

//matrices
//TM
vector <vector <float>> TM = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
//EM
vector <vector <float>> mirrorM = {{-1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
vector <vector <float>> GRM = vector<vector<float>>(4, vector<float>(4));
vector <vector <float>> tiltM = vector<vector<float>>(4, vector<float>(4));
vector <vector <float>> EM = vector<vector<float>>(4, vector<float>(4));
vector <vector <float>> eyePositionM = vector<vector<float>>(4, vector<float>(4));
//PM
vector <vector <float>> PM = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

void initial()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gluOrtho2D(-windowWidth/2, windowWidth/2, -windowHeight/2, windowHeight/2);
}

void display()
{
	//initialize
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnd();
	glFlush();
	//instuctions start
	string showLine;
	int counting = 0;
	while(getline(file, showLine))
	{
		if(showLine[0]!='\#')
		{
			//read instruction
			counting = showLine.find(" \0");
			string instruction = showLine.substr(0, counting);
			//execute by instruction
			if(instruction=="ambient")
			{
				readNums = readFunc(showLine, 3, counting);
				ambient.red = readNums[0], ambient.green = readNums[1], ambient.blue = readNums[2];
				readNums.clear();
			}
			else if(instruction=="background")
			{
				readNums = readFunc(showLine, 3, counting);
				BGColor.red = readNums[0], BGColor.green = readNums[1], BGColor.blue = readNums[2];
				readNums.clear();
			}
			else if(instruction=="light")
			{
				readNums = readFunc(showLine, 7, counting);
				light addingLight(readNums[1], readNums[2], readNums[3], readNums[4], readNums[5], readNums[6]);
				if(lights.size()>readNums[0]-1)
					lights[static_cast<int>(readNums[0])-1] = addingLight;
				else
				{
					lights.resize(readNums[0]-1);
					lights.push_back(addingLight);
				}
				readNums.clear();
			}
			else if(instruction=="scale")
			{
				readNums = readFunc(showLine, 3, counting);
				vector <vector <float>> scaleMatrix = {{readNums[0], 0, 0, 0}, {0, readNums[1], 0, 0}, {0, 0, readNums[2], 0}, {0, 0, 0, 1}};
				readNums.clear();
				TM = matrixMultiplication(scaleMatrix, TM);
			}
			else if(instruction=="rotate")
			{
				readNums = readFunc(showLine, 3, counting);
				//x
				vector <vector <float>> rotateMatrix = {{1, 0, 0, 0},
				{0, static_cast <float> (cos(readNums[0]*pi)), static_cast <float> (-1*sin(readNums[0]*pi)), 0},
				{0, static_cast <float> (sin(readNums[0]*pi)), static_cast <float> (cos(readNums[0]*pi)), 0},
				{0, 0, 0, 1}};
				TM = matrixMultiplication(rotateMatrix, TM);
				//y
				rotateMatrix = {{static_cast <float> (cos(readNums[1]*pi)), 0, static_cast <float> (sin(readNums[1]*pi)), 0},
				{0, 1, 0, 0},
				{static_cast <float> ((-1*sin(readNums[1]*pi))), 0, static_cast <float> (cos(readNums[1]*pi)), 0},
				{0, 0, 0, 1}};
				TM = matrixMultiplication(rotateMatrix, TM);
				//z
				rotateMatrix = {{static_cast <float> (cos(readNums[2]*pi)), static_cast <float> (-1*sin(readNums[2]*pi)), 0, 0},
				{static_cast <float> (sin(readNums[2]*pi)), static_cast <float> (cos(readNums[2]*pi)), 0, 0},
				{0, 0, 1, 0},
				{0, 0, 0, 1}};
				TM = matrixMultiplication(rotateMatrix, TM);
				readNums.clear();
			}
			else if(instruction=="translate")
			{
				readNums = readFunc(showLine, 3, counting);
				vector <vector <float>> translateMatrix = {{1, 0, 0, readNums[0]},
				{0, 1, 0, readNums[1]},
				{0, 0, 1, readNums[2]},
				{0, 0, 0, 1}};
				readNums.clear();
				TM = matrixMultiplication(translateMatrix, TM);
			}
			else if(instruction=="object")
			{
				//========================= read file name =========================
				string ascName = "";
				while(showLine[counting]==' ')
					counting++;
				ascName = showLine.substr(counting, showLine.find(" ", counting));
				counting = showLine.find(" ", counting);
				//==================== read object color and kd ks N ====================
				readNums = readFunc(showLine, 6, counting);
				asc addingASC(readNums[0], readNums[1], readNums[2], readNums[3], readNums[4], readNums[5]);
				readNums.clear();
				//========================= open asc file =========================
				ifstream ascFile(ascName);
				showLine = "";
				counting = 0;
				//==================== read vertex and plane amount ====================
				getline(ascFile, showLine);
				while(showLine[counting]=='\0')
					getline(ascFile, showLine);
				readNums = readFunc(showLine, 2, counting);
				int vertexAmount = readNums[0];
				int planeAmount = readNums[1];
				readNums.clear();
				addingASC.ascMatrix = vector<vector<float>>(4, vector<float>(vertexAmount));
				addingASC.originMatrix = addingASC.ascMatrix;
				//========================= read vertices =========================
				for(int i = 0; i<vertexAmount; i++)
				{
					getline(ascFile, showLine);
					counting = 0;
					readNums = readFunc(showLine, 3, counting);
					addingASC.ascMatrix[0][i] = readNums[0];
					addingASC.ascMatrix[1][i] = readNums[1];
					addingASC.ascMatrix[2][i] = readNums[2];
					addingASC.ascMatrix[3][i] = 1.0f;
					readNums.clear();
				}
				// TM convertion
				addingASC.ascMatrix = matrixMultiplication(TM, addingASC.ascMatrix);
				//========================= read planes =========================
				for(int i = 0; i<planeAmount; i++)
				{
					getline(ascFile, showLine);
					counting = 0;
					readNums = readFunc(showLine, 1, counting);
					int vertex_amount = readNums[0];
					readNums = readFunc(showLine, vertex_amount, counting);
					for(int j = 0;j<vertex_amount; j++)
						addingASC.ascPlane[i][j] = static_cast<int>(readNums[j])-1;
				}
				addingASC.originPlane = addingASC.ascPlane;
				asc_obj.push_back(addingASC);
			}
			else if(instruction=="observer")
			{
				readNums = readFunc(showLine, 10, counting);
				eyePosition.x_pos = readNums[0], eyePosition.y_pos = readNums[1], eyePosition.z_pos = readNums[2];
				COIPosition.x_pos = readNums[3], COIPosition.y_pos = readNums[4], COIPosition.z_pos = readNums[5];
				//EM
				eyePositionM = {{1, 0, 0, -1*eyePosition.x_pos},
				{0, 1, 0, -1*eyePosition.y_pos},
				{0, 0, 1, -1*eyePosition.z_pos},
				{0, 0, 0, 1}};
				point vec3(COIPosition.x_pos-eyePosition.x_pos, COIPosition.y_pos-eyePosition.y_pos, COIPosition.z_pos-eyePosition.z_pos);
				point vec1(vec3.z_pos, 0, -1*vec3.x_pos);
				point vec2(vec3.y_pos*vec1.z_pos-vec3.z_pos*vec1.y_pos,
						   vec3.z_pos*vec1.x_pos-vec3.x_pos*vec1.z_pos,
						   vec3.x_pos*vec1.y_pos-vec3.y_pos*vec1.x_pos);
				// |v| = 1
				normalization(vec3);
				normalization(vec1);
				normalization(vec2);
				GRM = {{vec1.x_pos, vec1.y_pos, vec1.z_pos, 0},
				{vec2.x_pos, vec2.y_pos, vec2.z_pos, 0},
				{vec3.x_pos, vec3.y_pos, vec3.z_pos, 0},
				{0, 0, 0, 1}};
				tiltM = {{static_cast <float> (cos(readNums[6]*pi)), static_cast <float> (sin(readNums[6]*pi)), 0, 0},
				{-1*static_cast <float> (sin(readNums[6]*pi)), static_cast <float> (cos(readNums[6]*pi)), 0, 0},
				{0, 0, 1, 0},
				{0, 0, 0, 1}};
				EM = matrixMultiplication(GRM, eyePositionM);
				EM = matrixMultiplication(mirrorM, EM);
				EM = matrixMultiplication(tiltM, EM);
				//PM
				PM = {{1, 0, 0, 0},
				{0, 1, 0, 0},
				{0, 0, (readNums[8]/(readNums[8]-readNums[7]))*static_cast <float> (tan(readNums[9]*pi)), ((readNums[7]*readNums[8])/(readNums[7]-readNums[8]))*static_cast <float> (tan(readNums[9]*pi))},
				{0, 0, static_cast <float> (tan(readNums[9]*pi)), 0}};
				readNums.clear();
			}
			else if(instruction=="viewport")
			{
				viewportVertex.clear();
				//read port values
				viewportVertex = readFunc(showLine, 4, counting);
				PM[1][1] = (viewportVertex[1]-viewportVertex[0])/(viewportVertex[3]-viewportVertex[2]);
				viewportVertex[0] *= windowWidth/2;
				viewportVertex[1] *= windowWidth/2;
				viewportVertex[2] *= windowHeight/2;
				viewportVertex[3] *= windowHeight/2;
				xRatio = (viewportVertex[1]-viewportVertex[0])/2;
				yRatio = (viewportVertex[3]-viewportVertex[2])/2;
			}
			else if(instruction=="display")
			{
				vector<point>points;
				points.clear();
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				glBegin(GL_POINTS);
				//========================= draw viewport lines =========================
				glColor3f(BGColor.red, BGColor.green, BGColor.blue);
				for(int i = viewportVertex[0]; i<=viewportVertex[1]; i++)
					for(int j = viewportVertex[2]; j<=viewportVertex[3]; j++)
						glVertex3f(i, j, 1.0f);
				//========================= asc matrix calculation =========================
				for(int i = 0; i<asc_obj.size(); i++)
				{
					asc_obj[i].ascMatrix = asc_obj[i].originMatrix;
					asc_obj[i].ascPlane = asc_obj[i].originPlane;
					asc_obj[i].ascMatrix = matrixMultiplication(EM, asc_obj[i].ascMatrix);
					asc_obj[i].ascMatrix = matrixMultiplication(PM, asc_obj[i].ascMatrix);
				}
				//========================= draw the asc =========================
				for(int i = 0; i<asc_obj.size(); i++)
				{
					for(int j = 0; j<asc_obj[i].ascPlane.size(); j++)
					{
						//========================= normal vector =========================
						// use these two to calculate normal vector
						//number i asc_obj's x(0) value of number j plane's vertax2-1
						point vec1(asc_obj[i].originMatrix[0][asc_obj[i].originPlane[j][2]]-asc_obj[i].originMatrix[0][asc_obj[i].originPlane[j][1]],
								   asc_obj[i].originMatrix[1][asc_obj[i].originPlane[j][2]]-asc_obj[i].originMatrix[1][asc_obj[i].originPlane[j][1]],
								   asc_obj[i].originMatrix[2][asc_obj[i].originPlane[j][2]]-asc_obj[i].originMatrix[2][asc_obj[i].originPlane[j][1]]);
						point vec2(asc_obj[i].originMatrix[0][asc_obj[i].originPlane[j][0]]-asc_obj[i].originMatrix[0][asc_obj[i].originPlane[j][1]],
								   asc_obj[i].originMatrix[1][asc_obj[i].originPlane[j][0]]-asc_obj[i].originMatrix[1][asc_obj[i].originPlane[j][1]],
								   asc_obj[i].originMatrix[2][asc_obj[i].originPlane[j][0]]-asc_obj[i].originMatrix[2][asc_obj[i].originPlane[j][1]]);
					   //2 cross 1
						point normal_vec(vec2.y_pos*vec1.z_pos-vec2.z_pos*vec2.y_pos,
										 vec2.z_pos*vec1.x_pos-vec2.x_pos*vec2.z_pos,
										 vec2.x_pos*vec1.y_pos-vec2.y_pos*vec2.x_pos);
						// |v| = 1
						normalization(normal_vec);
						//==================== calculate central point  ====================
						point central(0, 0, 0);
						for(int k = 0; k<asc_obj[i].originPlane[j].size(); k++)
						{
							central.x_pos += asc_obj[i].originMatrix[0][asc_obj[i].originPlane[j][k]];
							central.y_pos += asc_obj[i].originMatrix[1][asc_obj[i].originPlane[j][k]];
							central.z_pos += asc_obj[i].originMatrix[2][asc_obj[i].originPlane[j][k]];
						}
						//central point
						central.x_pos /= static_cast <float> (asc_obj[i].originPlane[j].size());
						central.y_pos /= static_cast <float> (asc_obj[i].originPlane[j].size());
						central.z_pos /= static_cast <float> (asc_obj[i].originPlane[j].size());
						//==================== light vector ====================
						color diffuseLight;
						color specularLight;
						for(int k = 0; k<lights.size(); k++)
						{
							lights[k].light_vec.x_pos = lights[k].x_pos-central.x_pos;
							lights[k].light_vec.y_pos = lights[k].y_pos-central.y_pos;
							lights[k].light_vec.z_pos = lights[k].z_pos-central.z_pos;
							// |v| = 1
							normalization(lights[k].light_vec);
							float N_dot_L = lights[k].light_vec.x_pos*normal_vec.x_pos+
								lights[k].light_vec.y_pos*normal_vec.y_pos+
								lights[k].light_vec.z_pos*normal_vec.z_pos;
							if(N_dot_L>=0)
							{
								diffuseLight.red += asc_obj[i].ascKd*lights[k].red * N_dot_L;
								diffuseLight.green += asc_obj[i].ascKd*lights[k].green * N_dot_L;
								diffuseLight.blue += asc_obj[i].ascKd*lights[k].blue * N_dot_L;
							}
							point R_vec(N_dot_L*normal_vec.x_pos * 2.0f-lights[k].light_vec.x_pos,
										N_dot_L*normal_vec.y_pos * 2.0f-lights[k].light_vec.y_pos,
										N_dot_L*normal_vec.z_pos * 2.0f-lights[k].light_vec.z_pos);
							point V_vec(eyePosition.x_pos-central.x_pos,
										eyePosition.y_pos-central.y_pos,
										eyePosition.z_pos-central.z_pos);
							normalization(V_vec);
							float cosB = R_vec.x_pos*V_vec.x_pos+R_vec.y_pos*V_vec.y_pos+R_vec.z_pos*V_vec.z_pos;
							if(cosB>0)
							{
								specularLight.red = specularLight.red+(asc_obj[i].ascKs*lights[k].red * pow(cosB, asc_obj[i].ascN));
								specularLight.green = specularLight.green+(asc_obj[i].ascKs*lights[k].green * pow(cosB, asc_obj[i].ascN));
								specularLight.blue = specularLight.blue+(asc_obj[i].ascKs*lights[k].blue * pow(cosB, asc_obj[i].ascN));
							}
						}
						//========================================here===================================
						///////////////////////////////  processing plane ///////////////////////////////////////////////
						for(int k = 0; k<ascPlanes[i][j].size(); k++)
						{
							//check clipping
							bool clipping = false;
							bool trivial = false;
							bool turn = false;
							float clippingC1 = 0;
							float clippingC2 = 0;
							int second = 0;
							int posneg = -1;
							for(int m = 0; m<3; m++)
							{
								for(int n = 0; n<2; n++)
								{
									posneg *= -1;
									if(k==ascPlanes[i][j].size()-1)
									{
										second = 0;
									}
									else
									{
										second = k+1;
									}
									// Z && W-Z
									if(m==2)
									{
										float C1 = showMatrices[i][3][ascPlanes[i][j][k]]*n+posneg*showMatrices[i][m][ascPlanes[i][j][k]];
										float C2 = showMatrices[i][3][ascPlanes[i][j][second]]*n+posneg*showMatrices[i][m][ascPlanes[i][j][second]];
										if(C1>=0&&C2>=0)
										{
											continue;
										}
										else if(C1<0&&C2<0)
										{
											trivial = true;
											break;
										}
										else
										{
											clippingC1 = C1;
											clippingC2 = C2;
											if(C2>=0)
											{
												turn = true;
											}
											clipping = true;
										}
									}
									// W+X && W-X && W+Y && W-Y
									else
									{
										float C1 = showMatrices[i][3][ascPlanes[i][j][k]]+posneg*showMatrices[i][m][ascPlanes[i][j][k]];
										float C2 = showMatrices[i][3][ascPlanes[i][j][second]]+posneg*showMatrices[i][m][ascPlanes[i][j][second]];
										if(C1>=0&&C2>=0)
										{
											continue;
										}
										else if(C1<0&&C2<0)
										{
											trivial = true;
											break;
										}
										else
										{
											clippingC1 = C1;
											clippingC2 = C2;
											if(C2>=0)
											{
												turn = true;
											}
											clipping = true;
										}
									}
								}
								if(trivial)
								{
									break;
								}
							}
							if(!trivial)
							{
								double pointDepth1 = 0;
								double pointDepth2 = 0;
								if(clipping)
								{
									float clippingX = 0;
									float clippingY = 0;
									float clippingZ = 0;
									float clippingW = 0;
									float t = clippingC1/(clippingC1-clippingC2);
									if(k==ascPlanes[i][j].size()-1)
									{
										clippingX = showMatrices[i][0][ascPlanes[i][j][k]]+t*(showMatrices[i][0][ascPlanes[i][j][0]]-showMatrices[i][0][ascPlanes[i][j][k]]);
										clippingY = showMatrices[i][1][ascPlanes[i][j][k]]+t*(showMatrices[i][1][ascPlanes[i][j][0]]-showMatrices[i][1][ascPlanes[i][j][k]]);
										clippingZ = showMatrices[i][2][ascPlanes[i][j][k]]+t*(showMatrices[i][2][ascPlanes[i][j][0]]-showMatrices[i][2][ascPlanes[i][j][k]]);
										clippingW = showMatrices[i][3][ascPlanes[i][j][k]]+t*(showMatrices[i][3][ascPlanes[i][j][0]]-showMatrices[i][3][ascPlanes[i][j][k]]);
									}
									else
									{
										clippingX = showMatrices[i][0][ascPlanes[i][j][k]]+t*(showMatrices[i][0][ascPlanes[i][j][k+1]]-showMatrices[i][0][ascPlanes[i][j][k]]);
										clippingY = showMatrices[i][1][ascPlanes[i][j][k]]+t*(showMatrices[i][1][ascPlanes[i][j][k+1]]-showMatrices[i][1][ascPlanes[i][j][k]]);
										clippingZ = showMatrices[i][2][ascPlanes[i][j][k]]+t*(showMatrices[i][2][ascPlanes[i][j][k+1]]-showMatrices[i][2][ascPlanes[i][j][k]]);
										clippingW = showMatrices[i][3][ascPlanes[i][j][k]]+t*(showMatrices[i][3][ascPlanes[i][j][k+1]]-showMatrices[i][3][ascPlanes[i][j][k]]);
									}
									//perspective divide
									clippingX /= clippingW;
									clippingY /= clippingW;
									//C2 >= 0
									if(turn)
									{
										//start of line
										addingLine.x1 = (clippingX+1)*xRatio+viewportVertex[0];
										addingLine.y1 = (clippingY+1)*yRatio+viewportVertex[2];
										pointDepth1 = (double) clippingZ/(double) clippingW;
										//end of line
										if(k==ascPlanes[i][j].size()-1)
										{
											addingLine.x2 = (showMatrices[i][0][ascPlanes[i][j][0]]/showMatrices[i][3][ascPlanes[i][j][0]]+1)*xRatio+viewportVertex[0];
											addingLine.y2 = (showMatrices[i][1][ascPlanes[i][j][0]]/showMatrices[i][3][ascPlanes[i][j][0]]+1)*yRatio+viewportVertex[2];
											pointDepth2 = (double) showMatrices[i][2][ascPlanes[i][j][0]]/(double) showMatrices[i][3][ascPlanes[i][j][0]];
										}
										else
										{
											addingLine.x2 = (showMatrices[i][0][ascPlanes[i][j][k+1]]/showMatrices[i][3][ascPlanes[i][j][k+1]]+1)*xRatio+viewportVertex[0];
											addingLine.y2 = (showMatrices[i][1][ascPlanes[i][j][k+1]]/showMatrices[i][3][ascPlanes[i][j][k+1]]+1)*yRatio+viewportVertex[2];
											pointDepth2 = (double) showMatrices[i][2][ascPlanes[i][j][k+1]]/(double) showMatrices[i][3][ascPlanes[i][j][k+1]];
										}
									}
									//C1 >= 0
									else
									{
										//start of line
										addingLine.x1 = (showMatrices[i][0][ascPlanes[i][j][k]]/showMatrices[i][3][ascPlanes[i][j][k]]+1)*xRatio+viewportVertex[0];
										addingLine.y1 = (showMatrices[i][1][ascPlanes[i][j][k]]/showMatrices[i][3][ascPlanes[i][j][k]]+1)*yRatio+viewportVertex[2];
										pointDepth1 = (double) showMatrices[i][2][ascPlanes[i][j][k]]/(double) showMatrices[i][3][ascPlanes[i][j][k]];
										//end of line
										addingLine.x2 = (clippingX+1)*xRatio+viewportVertex[0];
										addingLine.y2 = (clippingY+1)*yRatio+viewportVertex[2];
										pointDepth2 = (double) clippingZ/(double) clippingW;
									}
								}
								else
								{
									//start of line
									addingLine.x1 = (showMatrices[i][0][ascPlanes[i][j][k]]/showMatrices[i][3][ascPlanes[i][j][k]]+1)*xRatio+viewportVertex[0];
									addingLine.y1 = (showMatrices[i][1][ascPlanes[i][j][k]]/showMatrices[i][3][ascPlanes[i][j][k]]+1)*yRatio+viewportVertex[2];
									pointDepth1 = (double) showMatrices[i][2][ascPlanes[i][j][k]]/(double) showMatrices[i][3][ascPlanes[i][j][k]];
									//end of line
									if(k==ascPlanes[i][j].size()-1)
									{
										addingLine.x2 = (showMatrices[i][0][ascPlanes[i][j][0]]/showMatrices[i][3][ascPlanes[i][j][0]]+1)*xRatio+viewportVertex[0];
										addingLine.y2 = (showMatrices[i][1][ascPlanes[i][j][0]]/showMatrices[i][3][ascPlanes[i][j][0]]+1)*yRatio+viewportVertex[2];
										pointDepth2 = (double) showMatrices[i][2][ascPlanes[i][j][0]]/(double) showMatrices[i][3][ascPlanes[i][j][0]];
									}
									else
									{
										addingLine.x2 = (showMatrices[i][0][ascPlanes[i][j][k+1]]/showMatrices[i][3][ascPlanes[i][j][k+1]]+1)*xRatio+viewportVertex[0];
										addingLine.y2 = (showMatrices[i][1][ascPlanes[i][j][k+1]]/showMatrices[i][3][ascPlanes[i][j][k+1]]+1)*yRatio+viewportVertex[2];
										pointDepth2 = (double) showMatrices[i][2][ascPlanes[i][j][k+1]]/(double) showMatrices[i][3][ascPlanes[i][j][k+1]];
									}
								}
								//draw start & end points
								addingLine.x1 = roundf(addingLine.x1);
								addingLine.y1 = roundf(addingLine.y1);
								addingLine.x2 = roundf(addingLine.x2);
								addingLine.y2 = roundf(addingLine.y2);
								addingPoint.x = addingLine.x1;
								addingPoint.y = addingLine.y1;
								addingPoint.depth = pointDepth1;
								coloringPlanes.push_back(addingPoint);
								addingPoint.x = addingLine.x2;
								addingPoint.y = addingLine.y2;
								addingPoint.depth = pointDepth2;
								coloringPlanes.push_back(addingPoint);
								//determine slope
								if(addingLine.x1!=addingLine.x2)
								{
									addingLine.slope = (addingLine.y2-addingLine.y1)/(addingLine.x2-addingLine.x1);
									//draw a line based on slope
									if(addingLine.slope>=0&&addingLine.slope<=1)
									{
										if(addingLine.x1>addingLine.x2)
										{
											swap(addingLine.x1, addingLine.x2);
											swap(addingLine.y1, addingLine.y2);
											swap(pointDepth1, pointDepth2);
										}
										drawTheLine(addingLine, 0, pointDepth1, pointDepth2);
									}
									else if(addingLine.slope>=-1&&addingLine.slope<0)
									{
										if(addingLine.x2>addingLine.x1)
										{
											swap(addingLine.x1, addingLine.x2);
											swap(addingLine.y1, addingLine.y2);
											swap(pointDepth1, pointDepth2);
										}
										addingLine.x1 *= -1;
										addingLine.x2 *= -1;
										drawTheLine(addingLine, 1, pointDepth1, pointDepth2);
									}
									else if(addingLine.slope>1)
									{
										if(addingLine.x1>addingLine.x2)
										{
											swap(addingLine.x1, addingLine.x2);
											swap(addingLine.y1, addingLine.y2);
											swap(pointDepth1, pointDepth2);
										}
										swap(addingLine.x1, addingLine.y1);
										swap(addingLine.x2, addingLine.y2);
										drawTheLine(addingLine, 2, pointDepth1, pointDepth2);
									}
									else
									{
										if(addingLine.x2>addingLine.x1)
										{
											swap(addingLine.x1, addingLine.x2);
											swap(addingLine.y1, addingLine.y2);
											swap(pointDepth1, pointDepth2);
										}
										addingLine.x1 *= -1;
										addingLine.x2 *= -1;
										swap(addingLine.x1, addingLine.y1);
										swap(addingLine.x2, addingLine.y2);
										drawTheLine(addingLine, 3, pointDepth1, pointDepth2);
									}
								}
								//vertical line
								else
								{
									if(addingLine.y1>addingLine.y2)
									{
										swap(addingLine.y1, addingLine.y2);
										swap(pointDepth1, pointDepth2);
									}
									for(float plotY = addingLine.y1+1; plotY<=addingLine.y2; plotY++)
									{
										addingPoint.x = addingLine.x1;
										addingPoint.y = plotY;
										addingPoint.depth = (pointDepth2-pointDepth1) * (double) (plotY-addingLine.y1)/(double) (addingLine.y2-addingLine.y1)+pointDepth1;
										coloringPlanes.push_back(addingPoint);
									}
								}
							}
						}
						///////////////////////////////////////////////////////////////////////////////////////////////
						//add viewport lines
						int leftFirst = viewportVertex[3];
						int leftSecond = viewportVertex[2];
						int rightFirst = viewportVertex[3];
						int rightSecond = viewportVertex[2];
						int upFirst = viewportVertex[1];
						int upSecond = viewportVertex[0];
						int downFirst = viewportVertex[1];
						int downSecond = viewportVertex[0];
						double depthFirst = 0;
						double depthSecond = 0;
						bool left = false;
						bool right = false;
						bool down = false;
						bool up = false;
						for(int a = 0; a<coloringPlanes.size(); a++)
						{
							if(coloringPlanes[a].x==viewportVertex[0])
							{
								left = true;
								if(coloringPlanes[a].y>leftSecond)
								{
									leftSecond = coloringPlanes[a].y;
									depthSecond = coloringPlanes[a].depth;
								}
								else if(coloringPlanes[a].y<leftFirst)
								{
									leftFirst = coloringPlanes[a].y;
									depthFirst = coloringPlanes[a].depth;
								}
							}
							else if(coloringPlanes[a].x==viewportVertex[1])
							{
								right = true;
								if(coloringPlanes[a].y>rightSecond)
								{
									rightSecond = coloringPlanes[a].y;
									depthSecond = coloringPlanes[a].depth;
								}
								else if(coloringPlanes[a].y<rightFirst)
								{
									rightFirst = coloringPlanes[a].y;
									depthFirst = coloringPlanes[a].depth;
								}
							}
							else if(coloringPlanes[a].y==viewportVertex[2])
							{
								down = true;
								if(coloringPlanes[a].y>downSecond)
								{
									downSecond = coloringPlanes[a].x;
									depthSecond = coloringPlanes[a].depth;
								}
								else if(coloringPlanes[a].y<downFirst)
								{
									downFirst = coloringPlanes[a].x;
									depthFirst = coloringPlanes[a].depth;
								}
							}
							else if(coloringPlanes[a].y==viewportVertex[3])
							{
								up = true;
								if(coloringPlanes[a].y>upSecond)
								{
									upSecond = coloringPlanes[a].x;
									depthSecond = coloringPlanes[a].depth;
								}
								else if(coloringPlanes[a].y<upFirst)
								{
									upFirst = coloringPlanes[a].x;
									depthFirst = coloringPlanes[a].depth;
								}
							}
						}
						if(left)
						{
							for(int a = leftFirst; a<=leftSecond; a++)
							{
								addingPoint.x = viewportVertex[0];
								addingPoint.y = a;
								addingPoint.depth = (depthFirst-depthSecond) * (double) (a-leftFirst)/(double) (leftSecond-leftFirst)+depthSecond;
								coloringPlanes.push_back(addingPoint);
							}
						}
						else if(right)
						{
							for(int a = rightFirst; a<=rightSecond; a++)
							{
								addingPoint.x = viewportVertex[1];
								addingPoint.y = a;
								addingPoint.depth = (depthFirst-depthSecond) * (double) (a-leftFirst)/(double) (leftSecond-leftFirst)+depthSecond;
								coloringPlanes.push_back(addingPoint);
							}
						}
						else if(down)
						{
							for(int a = downFirst; a<=downSecond; a++)
							{
								addingPoint.x = a;
								addingPoint.y = viewportVertex[2];
								addingPoint.depth = (depthFirst-depthSecond) * (double) (a-leftFirst)/(double) (leftSecond-leftFirst)+depthSecond;
								coloringPlanes.push_back(addingPoint);
							}
						}
						else if(up)
						{
							for(int a = upFirst; a<=upSecond; a++)
							{
								addingPoint.x = a;
								addingPoint.y = viewportVertex[3];
								addingPoint.depth = (depthFirst-depthSecond) * (double) (a-leftFirst)/(double) (leftSecond-leftFirst)+depthSecond;
								coloringPlanes.push_back(addingPoint);
							}
						}
						/////////////////////////////////////////////////////////
						//fill the plane
						sort(coloringPlanes.begin(), coloringPlanes.end(), sortPoints);
						int size = coloringPlanes.size();
						float xLeft = coloringPlanes[0].x;
						float xRight = coloringPlanes[0].x;
						double depthLeft = coloringPlanes[0].depth;
						double depthRight = coloringPlanes[0].depth;
						for(int m = 1; m<size; m++)
						{
							if(coloringPlanes[m].y!=coloringPlanes[m-1].y)
							{
								for(int n = roundf(xLeft)+1; n<roundf(xRight); n++)
								{
									addingPoint.x = n;
									addingPoint.y = roundf(coloringPlanes[m-1].y);
									addingPoint.depth = (depthRight-depthLeft) * (double) (n-xLeft)/(double) (xRight-xLeft)+(double) depthLeft;
									coloringPlanes.push_back(addingPoint);
								}
								xLeft = coloringPlanes[m].x;
								xRight = coloringPlanes[m].x;
								depthLeft = coloringPlanes[m].depth;
								depthRight = coloringPlanes[m].depth;
							}
							else
							{
								if(coloringPlanes[m].x<xLeft)
								{
									xLeft = coloringPlanes[m].x;
									depthLeft = coloringPlanes[m].depth;
								}
								else if(coloringPlanes[m].x>xRight)
								{
									xRight = coloringPlanes[m].x;
									depthRight = coloringPlanes[m].depth;
								}
							}
						}
						//////////////////////////////   determine color  /////////////////////////////
						for(float a = 0; a<coloringPlanes.size(); a++)
						{
							coloringPlanes[a].pointColor.red = (ambient.red+diffuseLight.red)*ascColor[i].red+specularLight.red;
							coloringPlanes[a].pointColor.green = (ambient.green+diffuseLight.green)*ascColor[i].green+specularLight.green;
							coloringPlanes[a].pointColor.blue = (ambient.blue+diffuseLight.blue)*ascColor[i].blue+specularLight.blue;
							if(coloringPlanes[a].pointColor.red>1)
							{
								coloringPlanes[a].pointColor.red = 1;
							}
							if(coloringPlanes[a].pointColor.green>1)
							{
								coloringPlanes[a].pointColor.green = 1;
							}
							if(coloringPlanes[a].pointColor.blue>1)
							{
								coloringPlanes[a].pointColor.blue = 1;
							}
							points.push_back(coloringPlanes[a]);
						}
						allColors.push_back(coloringPlanes[0].pointColor);
						coloringPlanes.clear();
					}
				}
				//draw shape depend on position and depth
				sort(points.begin(), points.end(), sortPointsDepth);
				vector <point> sameDepthPoints;
				vector <point> sameYZPoints;
				vector <point> samePosPoints;
				for(int i = 0; i<points.size(); i++)
				{
					if(i!=points.size()-1&&points[i].depth==points[i+1].depth)
					{
						sameDepthPoints.push_back(points[i]);
					}
					else
					{
						sameDepthPoints.push_back(points[i]);
						sort(sameDepthPoints.begin(), sameDepthPoints.end(), sortPoints);
						for(int j = 0; j<sameDepthPoints.size(); j++)
						{
							if(j!=sameDepthPoints.size()-1&&sameDepthPoints[j].y==sameDepthPoints[j+1].y)
							{
								sameYZPoints.push_back(sameDepthPoints[j]);
							}
							else
							{
								sameYZPoints.push_back(sameDepthPoints[j]);
								sort(sameYZPoints.begin(), sameYZPoints.end(), sortPointsX);
								for(int k = 0; k<sameYZPoints.size(); k++)
								{
									if(k!=sameYZPoints.size()-1&&sameYZPoints[k].x==sameYZPoints[k+1].x)
									{
										samePosPoints.push_back(sameYZPoints[k]);
									}
									else
									{
										samePosPoints.push_back(sameYZPoints[k]);
										float brightest = 0;
										int brightestOne = 0;
										for(int l = 0; l<samePosPoints.size(); l++)
										{
											if(brightest<samePosPoints[l].pointColor.red+samePosPoints[l].pointColor.green+samePosPoints[l].pointColor.blue)
											{
												brightest = samePosPoints[l].pointColor.red+samePosPoints[l].pointColor.green+samePosPoints[l].pointColor.blue;
												brightestOne = l;
											}
										}
										//cut extra part
										if(samePosPoints[brightestOne].x>=viewportVertex[0]&&samePosPoints[brightestOne].x<=viewportVertex[1]&&samePosPoints[brightestOne].y>=viewportVertex[2]&&samePosPoints[brightestOne].y<=viewportVertex[3])
										{
											glColor3f(samePosPoints[brightestOne].pointColor.red, samePosPoints[brightestOne].pointColor.green, samePosPoints[brightestOne].pointColor.blue);
											glVertex3f(samePosPoints[brightestOne].x, samePosPoints[brightestOne].y, 1.0f);
										}
										samePosPoints.clear();
									}
								}
								sameYZPoints.clear();
							}
						}
						sameDepthPoints.clear();
					}
				}
				glEnd();
				glFlush();
				std::system("pause");
			}
			else if(instruction=="reset")
			{
				TM = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
			}
			else if(instruction=="end")
			{
				file.close();
				exit(0);
			}
		}
	}
}

int main(int argc, char * argv[])
{
	cin.get();
	//string fileName = argv [1];
	string fileName = "test.in";
	file.open(fileName);
	//==================== read window width & height ====================
	string showLine;
	getline(file, showLine);
	int split = 0;
	windowWidth = stoi(showLine.substr(split, showLine.find(" ", split)));
	split = showLine.find(" ")+1;
	windowHeight = stoi(showLine.substr(split, showLine.find(" ", split)));
	//========================= settings =========================
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(800, 100);
	glutCreateWindow("CG");
	glutDisplayFunc(&display);
	initial();
	glutMainLoop();
	return 0;
}