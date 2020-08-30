#pragma once


#include <iostream>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\features2d\features2d.hpp"

#include "utility.h"

using namespace std;
using namespace cv;


struct Region_add
{
	vector<Region> re_add;
	int area_per_character;
	Region_add()
	{
		area_per_character = 0;
	}
};

void textDetect(vector<Region> &regions, vector<Str_Chinese> &texts, vector<double> &w);
void textDetect(vector<Region> &regions, vector<vector<Region>> &groups, vector<double> &w, Mat &show);
void regionDetect(Mat &src, vector<Region> &regionBoxes);
void supressNMS(vector<Region> &regionBoxes, double r);
void CC_Merge(vector<Region> &rm_in, vector<Region> &groups, double d_r, int c_th, Mat &image);
void findCha(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<vector<Region>> &re_adds,Mat &image);
void findChaAdd(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<vector<Region>> &re_adds, Mat &image);
void findChaRe(vector<Region> &regions, vector<Region> &non_texts, vector<vector<Region>> &re_adds, Mat &image);
double overlapHeight(double yu, double yd, Region r);
void mergeCC(vector<Region> &rm_in, int x, int y, double d_r, int c_th, double area_th, vector<Region> &groups, Mat &image);
void CC_Merge(vector<Region> &rm_in, vector<Region> &groups, double d_r, int c_th, int area_th, Mat &image);
void findChaAdd(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<Region_add> &re_adds, Mat &image);

void findChaRecall(vector<Str_Chinese> &texts, vector<Region> &non_texts,Mat &image);
void textlineDetection(Mat &panel, vector<Str_Chinese> &texts_res);
void regionsFilterFromTexts(vector<Str_Chinese> &texts, vector<Region> &regions);

void mergeForCompleteSign(vector<Region> &non_texts, const Mat &image);
void refineTexts(vector<Str_Chinese> &texts);

void textLines_gth_match(vector<Str_Chinese> &texts, Panel &p, vector<Str_Chinese> &neg);
void textlineDetection(Mat &panel, vector<Str_Chinese> &texts_res, CvBoost  &boost_stage_1, CvBoost  &boost_stage_2);
void retrieveCharacter(vector<Str_Chinese> &texts, vector<Region> &non_texts, Mat &image);
void retrieveCharacter(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<Region> &new_cha, Mat &image);

void regionsFilterFromTextsForSign(vector<Str_Chinese> &texts, vector<Region> &regions);

