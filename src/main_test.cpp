#include "main_test.h"

int total_str_num = 0;
int total_panel_num = 0;
int total_cha_num = 0;

#define STAGE_1

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
	/////////////////////////*load classifiers*/////////////////////////////////////////////

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

#pragma region cc_detect
		
		Panel p = panels_total[i];
		vector<Region> regions;

		Mat src = imread(p.imgFile);
		Mat panel(src, p.box);

		Mat gth_regions = panel.clone();
		showPanelGth(gth_regions, p);

		Mat panel_c = panel.clone();
		regionDetect(panel_c, regions);               //连通部件提取

		//vector<Region> regions_copy(regions);

		Mat detect_regions = panel.clone();
		drawRects(detect_regions, regions);

		int recall_c = 0;
		int gth_c = 0;
		recallMatch(regions, p, recall_c, gth_c);
		total_matched += recall_c;
		total_regions += regions.size();
		//if (gth_c)
		//{
		//	cout << i << "  :  ";
		//	cout << (double)recall_c / gth_c << endl;
		//	//cout << p.imgFile << endl;
		//}
		waitKey(0);
#pragma endregion

		///////////////////////////*粗分*///////////////////////////
#pragma region stage_1
		vector<Region> non_texts;
		filter_stage_1_boost(regions, boost_stage_1, -1, non_texts);

		recall_c = 0;
		gth_c = 0;
		recallMatch(regions, p, recall_c, gth_c);
		total_matched_s1 += recall_c;
		total_regions_s1 += regions.size();

		Mat show_stage_1 = panel.clone();
		drawRects(show_stage_1, regions);
#pragma endregion

		///////////////////////////*第二级过滤*////////////////////////////////////////
#pragma region stage_2
		vector<Region> re_texts;
		vector<Region> regions_for_recall;
		filter_stage_2_boost(regions, boost_stage_2, -1, 0, re_texts, regions_for_recall);
		non_texts.insert(non_texts.end(), regions_for_recall.begin(), regions_for_recall.end());
		Mat show_stage_2 = panel.clone();
		drawRects(show_stage_2, regions);

		//filerContain(regions);
		/*Mat show_filterCon = panel.clone();
		drawRects(show_filterCon, regions);*/

		recall_c = 0;
		gth_c = 0;
		recallMatch(regions, p, recall_c, gth_c);
		total_matched_s2 += recall_c;
		total_regions_s2 += regions.size();
		waitKey(0);
#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
#pragma region texts_detection
		Mat show_string_pre = panel.clone();
		double init_w_1[6] = { -5.25453, 4.44628, 7.83862, 6.39484, 5.57244, 2.47514 };
		double init_w_2[6] = { -9.56632, 2.43555, 12.0159, 5.73109, 12.4715, 7.12804 };
		double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得

#pragma region stroke_merge     //笔画合并
		vector<double> w(init_w_3, init_w_3 + 6);
		vector<Str_Chinese> texts;

		Mat show_re_texts = panel.clone();
		drawRects(show_re_texts, re_texts);

		filerContainByScore(re_texts, 0.8);
		textDetect(re_texts, texts, w);
		for (auto &t : texts)
		{
			sort(t.str.begin(), t.str.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
		}

		for (int i = 0; i < texts.size(); ++i)
		{
			if (texts[i].str.size() < 1) continue;
			int b = theRNG().uniform(0, 255);
			int g = theRNG().uniform(0, 255);
			int r = theRNG().uniform(0, 255);

			Scalar color(b, g, r);
			rectangle(show_string_pre, texts[i].box, Scalar(b, g, r), 2);
		}
		waitKey(0);

		////////////////////////////////////////////////////////////////////////////
		Mat show_non_texts = panel.clone();
		//vector<vector<Region>> re_adds;
		vector<Region_add> re_adds;
		findChaAdd(texts, non_texts, re_adds, show_non_texts);
		Mat show_adds = panel.clone();
		for (auto add : re_adds)
		{
			drawRects(show_adds, add.re_add);
		}

		vector<Region> merge;
		for (auto add : re_adds)
		{
			//filerContain(add.re_add,0.8);
			//filerContainByAreaV(re_add, 0.9);
			Mat show_add = panel.clone();
			vector<Region> temp;
			drawRects(show_add, add.re_add);
			CC_Merge(add.re_add, temp, 0.5, 4, add.area_per_character, panel);
			show_add = panel.clone();
			drawRects(show_add, temp);

			filter_stage_1_boost(temp, boost_stage_1, -1);
			filter_stage_2_boost(temp, boost_stage_2, -1, regions_for_recall);

			drawRects(show_add, temp, Scalar(0, 255, 255));
			if (!temp.empty())
			{
				regions.insert(regions.end(), temp.begin(), temp.end());
				merge.insert(merge.end(), temp.begin(), temp.end());
			}
		}

		Mat show_merge = panel.clone();
		drawRects(show_merge, merge);

#pragma endregion

#pragma region merge direct
		/*vector<Region> temp;
		CC_Merge(non_texts, temp, 0.5, 3, panel);
		filter_stage_1_boost(temp, boost_stage_1, -1);
		filter_stage_2_boost(temp, boost_stage_2, -1);
		regions.insert(regions.end(), temp.begin(), temp.end());*/
#pragma endregion
		filerContainByScore(regions, 0.8);
		recall_c = 0;
		gth_c = 0;
		recallMatch(regions, p, recall_c, gth_c);
		total_matched_find += recall_c;
		total_regions_find += regions.size();


		Mat show_regions = panel.clone();
		drawRects(show_regions, regions);
		if (gth_c)
		{
			cout << i << "  :  ";
			cout << (double)recall_c / gth_c << endl;
			//cout << p.imgFile << endl;
		}

		waitKey(0);
		////////////////////////////////////////////////////////////////////////
		texts.clear();
		textDetect(regions, texts, w);

		Mat ftl = panel.clone();
		filterTextlines(texts, boost_stage_3, ftl);  //第三级过滤，笔画合并后过滤一次

		regionsFilterFromTexts(texts, non_texts);
		/*尝试从文本行中召回丢失的文字，低阈值极容易造成误检，在交通面板内部十分普遍,所以这一步慎重！和场景文字检测的区别*/
	    Mat show_recall = panel.clone();
		retrieveCharacter(texts, non_texts, regions,show_recall);
		
		refineTexts(texts);
		
		//filerContainByScore(regions, 0.6);
		Mat show_newRegions = panel.clone();
		drawRects(show_newRegions, regions);

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
		waitKey(0);

		string folderName = "regions_full";
		string image_name = getFileName(p.imgFile);

		char srcToSaved[50];
		sprintf(srcToSaved, "%s/%d_%s.png", folderName.c_str(), i,image_name.c_str());
		//imwrite(srcToSaved, show_regions);

		folderName = "string_full";
		sprintf(srcToSaved, "%s/%d_%s.png", folderName.c_str(), i, image_name.c_str());
		//imwrite(srcToSaved, show_string);

#pragma endregion

#pragma region Sign Detection
		regionsFilterFromTextsForSign(texts, non_texts);
		for (auto &re : non_texts) { re.score = 0; }
		
		mergeForCompleteSign(non_texts, panel);
		Mat show_sign_0 = panel.clone();
		drawRects(show_sign_0, non_texts);

		
		sign_filter_stage_1_boost(non_texts, sign_boost_stage_1, -1);
		Mat show_sign_1 = panel.clone();
		drawRects(show_sign_1, non_texts);

		filter_stage_2_boost(non_texts, sign_boost_stage_2, -1);

		filerContainByScore(non_texts, 0.6);
		Mat show_sign_2 = panel.clone();
		drawRects(show_string, non_texts);

		
		folderName = "result_full";
		sprintf(srcToSaved, "%s/%d_%s_res.png", folderName.c_str(), i,image_name.c_str());
	//	imwrite(srcToSaved, show_string);
		waitKey(0);

#pragma endregion

	}

#pragma region print out the results

	double recall = (double)total_matched / total_cha_num;
	cout << "recall:  " << recall << endl;
	double precision = (double)total_matched / total_regions;
	cout << "precision:  " << total_matched << "  /  " << total_regions << " = " << precision << endl;

	cout << "after stage1" << endl;
	recall = (double)total_matched_s1 / total_cha_num;
	cout << "recall:  " << recall << endl;
	precision = (double)total_matched_s1 / total_regions_s1;
	cout << "precision:  " << total_matched_s1 << "  /  " << total_regions_s1 << " = " << precision << endl;

	cout << "after stage2" << endl;
	recall = (double)total_matched_s2 / total_cha_num;
	cout << "recall:  " << recall << endl;
	precision = (double)total_matched_s2 / total_regions_s2;
	cout << "precision:  " << total_matched_s2 << "  /  " << total_regions_s2 << " = " << precision << endl;

	cout << "after Merge" << endl;
	recall = (double)total_matched_find / total_cha_num;
	cout << "recall:  " << recall << endl;
	precision = (double)total_matched_find / total_regions_find;
	cout << "precision:  " << total_matched_find << "  /  " << total_regions_find << " = " << precision << endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma endregion

	
	return EXIT_SUCCESS;
}
