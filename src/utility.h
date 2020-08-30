#pragma once
#include <iostream>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\ml\ml.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\objdetect\objdetect.hpp"

#include <set>


#include "../src/LBP/feature.h"
#include "../src/LBP/LBP.hpp"

using namespace std;
using namespace cv;

struct Region
{
	Rect r;
	int p_w, p_h;
	int cen_x, cen_y;
	int label;
	double var;
	Mat image;
	Mat pointImg;
	int sub_count;

	double component_area;
	double score;
	Region(Rect rin)
	{
		r = rin;
		p_w = 0, p_h = 0;
		cen_x = (r.x + r.x + r.width - 1) / 2;
		cen_y = (r.y + r.y + r.height - 1) / 2;
		label = 0;
		var = 0.01;
		sub_count = 1;
		score = 0;
		component_area = 0;
	}

	Region(Rect rin, double v)
	{
		r = rin;
		p_w = 0, p_h = 0;
		cen_x = (r.x + r.x + r.width - 1) / 2;
		cen_y = (r.y + r.y + r.height - 1) / 2;
		label = 0;
		var = v;
		sub_count = 1;
		score = 0;
		component_area = 0;
	}
};

struct Str_Chinese
{
	Rect box;
	vector<Region> str;
	Str_Chinese(){};
	Str_Chinese(Rect rin)
	{
		box = rin;
	}
};

struct Panel
{
	string imgFile;
	Rect box;
	vector<Str_Chinese> strs;
	Panel(Rect rin)
	{
		box = rin;
	}
};

struct Region_TH
{
	double ratio_min;
	double ratio_max;
	double area_r_min;
	double area_r_max;
	Region_TH(){
		ratio_min = 1000;
		ratio_max = 0;
		area_r_min = 1000;
		area_r_max = 0;
	}
};

string getFileName(string str);
void getRangeFromTxt(string tempObject, int *xmin, int *xmax, int *ymin, int *ymax);

void loadFile(vector<string> &image_path, vector<string> &data_path, string imageFile, string dataFile);
void loadFile(vector<string> &image_path, vector<string> &str_data_path, vector<string> &cha_data_path, string imageFile, string str_dataFile, string cha_dataFile);


void normSize(Mat &recImage, double length);

float overlap(const cv::Rect &a, const cv::Rect &b);

int overlap_object(Rect r, int *xmin, int *ymin, int *xmax, int *ymax, int n, double th);

void processPanelData(vector<string> &rects_str, vector<string> &panelBox, vector<string> &stringBox);
void processChaData(vector<string> &rects_cha, vector<string> &chaBox);

void rect_extend(vector<Rect> &rects, double r, int width, int height);

int similarity(Rect r, int xmin, int ymin, int xmax, int ymax, double th);

void getPanelStrChaData(string tempStrData, string tempChaData, vector<Panel> &panels,int &strNum,int &chaNum);
void getPanelStrChaData(string tempImg, string tempStrData, string tempChaData, vector<Panel> &panels, int &stringNum, int &chaNum);

void showPanelData(Mat &img, vector<Panel> &panels);
void prepaereSamples(vector<Panel> &panels, vector<pair<Region, Region>> &pos_samples, vector<pair<Region, Region>> &neg_samples);
void calRegionPairFeature(pair<Region, Region> &cc, vector<double> &feature);
void updateRegionTh(vector<Panel> &panels, Region_TH &region_th);

double calRegionPairDist(Region r1, Region r2, vector<double> &w);


void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist);
void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<pair<Region, Region>> &M, vector<pair<Region, Region>> &C);
void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature);

void updateW(double apha,vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature);
void updateW(double apha, vector<double> &w, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature);

double cost_fuction(vector<vector<double>> &M_feature, vector<vector<double>> &C_feature, vector<double> &w);

void drawRects(Mat &src, vector<Region> &regionBoxes, Scalar color=Scalar(0,0,255));
bool contain(Region &r1, Region &r2, double th);
void rect_extend(vector<Region> &regions, double r, int width, int height);
double iou(Region &r1, Region &r2);
void showPanelGth(Mat &panel_image, Panel &panel);

void calRegionFeature(Region &cc, vector<double> &feature);
float classify_one_svm(Region &cc, CvSVM &svm);
void filter_stage_1(vector<Region> &regions, CvSVM &svm, double s_th);
void calculateHOG(Mat &src, vector<float> &descriptors);
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double s_th);
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double s_th, vector<Region> &re_other);
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double low_th, double high_th, vector<Region> &re_text, vector<Region> &re_other);

void filter_stage_2_clbp(vector<Region> &regions, CvSVM &svm, double s_th);
void filter_stage_2_clbp(vector<Region> &regions, CvSVM &svm, double s_th, vector<Region> &re_out);
void filter_stage_2_clbp(vector<Region> &regions, CvBoost &boost, double low_th, double high_th, vector<Region> &re_text, vector<Region> &re_other);

void rect_extend(Rect &rec, double r, int width, int height);
bool containTwo(Region &r1, Region &r2, double th);
void filter_stage_1(vector<Region> &re_left, CvSVM &svm, double s_th, vector<Region> &re_out);
void recallMatch(vector<Region> &regions, Panel &p, int &c_match, int &gth_c);
void filerContain(vector<Region> &regions);
void gth_match(vector<Region> &regions, Panel &p, vector<Region> &pos, vector<Region> &neg, Mat &src);
void calculateColorHOG(Mat &src, vector<float> &descriptors);

void filter_stage_hogC(vector<Region> &regions, CvSVM &svm, double s_th);
void filerContainByScore(vector<Region> &regions);

void filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th);
void filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th, vector<Region> &re_out);

void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double s_th);
void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double s_th, vector<Region> &other);
void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double low_th, double high_th, vector<Region> &re_text, vector<Region> &re_other);
void filter_stage_2_boost(vector<String> &img_path, CvBoost &boost, double s_th, vector<String> &other);

void filerContain(vector<Region> &regions, double th);
void filerContainByScore(vector<Region> &regions, double th);
void filerContainByAreaV(vector<Region> &regions, double th);

void sign_filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th);
void calculateHOG_binary(Mat &src, vector<float> &descriptors);
void filterTextlines(vector<Str_Chinese> &texts, CvBoost &boost, Mat &ftl);


void calRegionFeatureFix(Region &cc, vector<double> &feature);
