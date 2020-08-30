#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <iostream>
#include <fstream>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\ml\ml.hpp"

#include "../src/imageRead.h"
#include "../src/utility.h"
#include "../src/textDetect.h"


using namespace cv;
using namespace std;

int total_str_num = 0;
int total_panel_num = 0;
int total_cha_num = 0;

int main(void)
{
#pragma region prepareData
	//////////////////////////////////////*read path.txt*///////////////////////////
	string imageFile = "../image_path/images_test_path.txt";
	vector<string> image_path;

	string str_dataFile = "../image_path/str_test_datas_path.txt";
	vector<string> str_data_path;

	string cha_dataFile = "../image_path/cha_test_datas_path.txt";
	vector<string> cha_data_path;

	loadFile(image_path, str_data_path, cha_data_path, imageFile, str_dataFile, cha_dataFile);

	int imageCount = image_path.size();
	int str_dataCount = str_data_path.size();
	int cha_dataCount = cha_data_path.size();
	if (imageCount != str_dataCount || imageCount != cha_dataCount || str_dataCount != cha_dataCount)
	{
		std::cout << "imageCount != dataCount,Something wrong !" << endl;
		return -1;
	}
	////////////////////////////*prepare the samples*////////////////////////////
	vector<Panel> panels_total;
	for (int i = 0; i != min(1000, (int)image_path.size()); ++i)
	{
		std::cout << i + 1 << ">" << endl;
		string tempImg = image_path[i];
		Mat imgRGB = imread(tempImg);
		//imgRGB = imread("1.jpeg");
		Mat imgGray;
		cvtColor(imgRGB, imgGray, CV_BGR2GRAY);
		Mat image_rects = imgRGB.clone();
		/////////////////////////////*get the panels and strings and connected regions*//////////////////////////////////
		vector<Panel> panels;

		string tempStrData = str_data_path[i];
		string tempChaData = cha_data_path[i];
		int stringNum = 0, chaNum = 0;
		getPanelStrChaData(tempImg, tempStrData, tempChaData, panels, stringNum, chaNum);
		showPanelData(image_rects, panels);

		panels_total.insert(panels_total.end(), panels.begin(), panels.end());
		total_str_num += stringNum;
		total_cha_num += chaNum;
	}
	total_panel_num = panels_total.size();

#pragma endregion

#pragma region prepare classifier
	/////////////////////////*preparation*/////////////////////////////////////////////

	CvBoost  boost_stage_1;
	string boost_filename_1 = "connected_component_feature_5_1000_05_fix-boost.xml";
	cout << "Boost file: " << boost_filename_1 << endl;
	boost_stage_1.load(boost_filename_1.c_str());

	CvBoost  boost_stage_2;
	string boost_filename_2 = "connected_component_feature_hog_800_ex_fix-boost.xml";
	cout << "Boost file: " << boost_filename_2 << endl;
	boost_stage_2.load(boost_filename_2.c_str());

	CvBoost  boost_stage_3;
	string boost_filename_3 = "connected_component_feature_hog_800_ex_last_stage-boost.xml";
	cout << "Boost file: " << boost_filename_3 << endl;
	boost_stage_3.load(boost_filename_3.c_str());

	CvBoost  sign_boost_stage_1;
	string sign_boost_filename_1 = "sign_feature_9_1000_10_fix-boost.xml";
	cout << "Boost file: " << sign_boost_filename_1 << endl;
	sign_boost_stage_1.load(sign_boost_filename_1.c_str());

	CvBoost  sign_boost_stage_2;
	string sign_boost_filename_2 = "sign_feature_hog_800_ex_1000-boost.xml";
	cout << "Boost file: " << sign_boost_filename_2 << endl;
	sign_boost_stage_2.load(sign_boost_filename_2.c_str());


	int total_matched = 0;
	int total_regions = 0;

	int total_matched_s1 = 0;
	int total_regions_s1 = 0;

	int total_matched_s2 = 0;
	int total_regions_s2 = 0;

	int total_matched_find = 0;
	int total_regions_find = 0;

#pragma endregion

	for (int i = 0; i < panels_total.size(); ++i)
	{
		cout << "i:   " << i << '\r';
		Panel p = panels_total[i];

		Mat src = imread(p.imgFile);
		Mat panel(src, p.box);

		vector<Str_Chinese> texts;
		textlineDetection(panel, texts, boost_stage_1, boost_stage_2);

		Mat ftl = panel.clone();
		filterTextlines(texts, boost_stage_3, ftl);
		refineTexts(texts);

		Mat show_string = panel.clone();
		for (int i = 0; i < texts.size(); ++i)
		{
			if (texts[i].str.size() < 1) continue;
			int b = theRNG().uniform(0, 255);
			int g = theRNG().uniform(0, 255);
			int r = theRNG().uniform(0, 255);

			Scalar color(b, g, r);
			rectangle(show_string, texts[i].box, Scalar(b, g, r), 2);
		}

		string image_name = getFileName(p.imgFile);
		string folderName = "string_full";
		char srcToSaved[50];
		sprintf(srcToSaved, "%s/%d_%s.png", folderName.c_str(), i, image_name.c_str());
		imwrite(srcToSaved, show_string);
		waitKey(0);

#pragma endregion

//#pragma region Sign Detection
//		regionsFilterFromTexts(texts, non_texts);
//		for (auto &re : non_texts) { re.score = 0; }
//		
//		mergeForCompleteSign(non_texts, panel);
//		Mat show_sign_0 = panel.clone();
//		drawRects(show_sign_0, non_texts);
//
//		
//		sign_filter_stage_1_boost(non_texts, sign_boost_stage_1, -1);
//		Mat show_sign_1 = panel.clone();
//		drawRects(show_sign_1, non_texts);
//
//		filter_stage_2_boost(non_texts, sign_boost_stage_2, -1);
//
//		filerContainByScore(non_texts, 0.6);
//		Mat show_sign_2 = panel.clone();
//		drawRects(show_string, non_texts);
//
//		
//		folderName = "result_full";
//		sprintf(srcToSaved, "%s/%d_%s_res.png", folderName.c_str(), i,image_name.c_str());
//		imwrite(srcToSaved, show_string);
//		waitKey(0);
//
//#pragma endregion

	}

	return EXIT_SUCCESS;
}
