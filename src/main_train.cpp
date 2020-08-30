#include "main_test.h"

int total_str_num = 0;
int total_panel_num = 0;
int total_cha_num = 0;

int image_use = 1000;
int stage = 1;
bool pre_samples = true;

int main(void)
{
	
#pragma region prepare data
		//////////////////////////////////////*read path.txt*///////////////////////////
		string imageFile = "../image_path/images_train_path.txt";
	//	string imageFile = "../image_path/images_test_path.txt";
		vector<string> image_path;

		string str_dataFile = "../image_path/str_train_datas_path.txt";
	//	string str_dataFile = "../image_path/str_test_datas_path.txt";
		vector<string> str_data_path;

		string cha_dataFile = "../image_path/cha_train_datas_path.txt";
	//	string cha_dataFile = "../image_path/cha_test_datas_path.txt";
		vector<string> cha_data_path;


		loadFile(image_path, str_data_path, cha_data_path, imageFile, str_dataFile, cha_dataFile);

		int imageCount = image_path.size();
		int str_dataCount = str_data_path.size();
		int cha_dataCount = cha_data_path.size();
		if (imageCount != str_dataCount || imageCount != cha_dataCount || str_dataCount != cha_dataCount)
		{
			std::cout << "imageCount != dataCount, Something wrong !" << endl;
			return -1;
		}
		////////////////////////////*prepare the samples*////////////////////////////
		vector<Panel> panels_total;
		for (int i = 0; i != min(image_use, (int)image_path.size()); ++i)
		{
			std::cout << i + 1 << ">" << endl;
			string tempImg = image_path[i];
			Mat imgRGB = imread(tempImg);
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

#pragma region samples
	vector<Region> pos_total;
	vector<Region> neg_total;
	if (pre_samples)
	{
		/////////////////////////*prepare the samples*/////////////////////////////////////////////
		cout << "prepare for the samples" << endl;
		for (int i = 0; i < min(10000,(int)panels_total.size()); ++i)
		{
			cout << "i:   "<<i <<"\r";
			Panel p = panels_total[i];

			Mat src = imread(p.imgFile);
			Mat panel(src, p.box);
			Mat show_regions = panel.clone();
			Mat detect_regions = panel.clone();
			Mat show_pos = panel.clone();
			Mat show_neg = panel.clone();

			vector<Region> regions;
			regionDetect(panel, regions);
			drawRects(detect_regions, regions);
			showPanelGth(show_regions, p);

			vector<Region> pos, neg;
			gth_match(regions, p, pos, neg, src);  // prepare for the pos and neg
			drawRects(show_pos, pos);
			drawRects(show_neg, neg);

			pos_total.insert(pos_total.end(), pos.begin(), pos.end());
			if (neg_total.size() <= 500000) neg_total.insert(neg_total.end(), neg.begin(), neg.end());
		}
	}
	cout << "\n";
	cout << "panels_total: " << panels_total.size() << endl;
	cout << "total_str_num: " << total_str_num << endl;
	cout << "total_cha_num: " << total_cha_num << endl;
	cout << "pos_total: " << pos_total.size() << endl;
	cout << "neg_total: " << neg_total.size() << endl;

	/*ofstream pos_fill("pos_fill.txt");
	ofstream neg_fill("neg_fill.txt");
	for (auto pos : pos_total)
	{
		double r = pos.component_area / pos.r.area();
		if(r) pos_fill << r << endl;
	}
	pos_fill.close();

	for (int i = 0; i < min(pos_total.size(), neg_total.size()); ++i)
	{
		Region n = neg_total[i];
		double r = n.component_area / n.r.area();
		neg_fill << r << endl;
	}
	neg_fill.close();*/

	//rectsSaveAsImage(pos_total, string("pos_gth"), Size(24, 24), string("pos"));
	//rectsSaveAsBinaryImage(pos_total, string("pos"), Size(24, 24), string("pos"));
	//rectsSaveAsImage(neg_total, string("neg"), Size(24, 24), string("neg"));
	//rectsSaveAsBinaryImage(neg_total, string("neg"), Size(24, 24), string("neg"));

#pragma endregion

	/////////////////////////*calculate the features to XML*/////////////////////////////////////////////
#pragma region stage_1
	if (stage == 1)
	{
		cout << "stage_1" << endl;
		cout << "calculate the features" << endl;
		//int neg_use = min(500000, (int)neg_total.size());
		int neg_use = min(pos_total.size()/2,neg_total.size());
		string featureFile = "connected_component_feature_5_1000_05_revised.xml";
		FileStorage fs_feature(featureFile, FileStorage::WRITE);
		int imageNum = pos_total.size() + neg_use;
		int featureDim = 5;
		fs_feature << "imageNum" << imageNum;
		fs_feature << "featureDim" << featureDim;
		fs_feature << "lable_feature" << "[";
		/*positive*/
		for (int i = 0; i != pos_total.size(); ++i)
		{
			vector<double> feature;
			calRegionFeature(pos_total[i], feature);
			//calRegionFeatureFix(pos_total[i], feature);
			fs_feature << 1 << feature;
		}
		/*negative*/
		for (int i = 0; i != neg_use; ++i)
		{
			vector<double> feature;
			calRegionFeature(neg_total[i], feature);
			//calRegionFeatureFix(neg_total[i], feature);
			fs_feature << -1 << feature;
		}
		fs_feature << "]";
	}
#pragma endregion

#pragma region stage_2_CLBP
	if (stage == 2)
	{
		cout << "calculate CLBP feature" << endl;
		string pos_image_folder = "pos_after_1_fix";
		string neg_image_folder = "neg_after_1_fix";
		vector<string> pos_path;
		vector<string> neg_path;
		getFiles(pos_image_folder, pos_path);
		getFiles(neg_image_folder, neg_path);

		int neg_use = min(pos_path.size(), neg_path.size());

		LBP_Option lbp_opt;
		lbp_opt.type = LBP_MAPPING_HF;
		lbp_opt.radius.push_back(1); lbp_opt.radius.push_back(3); lbp_opt.radius.push_back(5);
		lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8);
		lbp_opt.normSize = Size(24, 24);

		string featureFile = "connected_component_feature_lbpHFSMC_ex.xml";
		FileStorage fs_feature(featureFile, FileStorage::WRITE);
		int imageNum = pos_path.size() + neg_use;
		int featureDim = 456;
		fs_feature << "imageNum" << imageNum;
		fs_feature << "featureDim" << featureDim;
		fs_feature << "lable_feature" << "[";
		/*positive*/
		for (int i = 0; i != pos_path.size(); ++i)
		{
			cout << "pos  i:   " << i << "\r";
			string temp_path = pos_path[i];
			Mat src = imread(pos_path[i]);
			vector<double> hist;
			if (src.channels() != 1) cvtColor(src, src, CV_BGR2GRAY);

			//calLBPMultiScale(src, hist, lbp_opt);
			//calLBPHFSM_MultiScale(src, hist, lbp_opt);
			//calCLBP_MultiScale(src, hist, lbp_opt);
			calLBPHFSMC_MultiScale(src, hist, lbp_opt);

			fs_feature << 1 << hist;
		}
		/*negative*/
		cout << "\n";
		for (int i = 0; i != neg_use; ++i)
		{
			cout << "neg  i:   " << i << "\r";
			string temp_path = neg_path[i];
			Mat src = imread(neg_path[i]);

			vector<double> hist;
			if (src.channels() != 1) cvtColor(src, src, CV_BGR2GRAY);

			//calLBPMultiScale(src, hist, lbp_opt);
			//calLBPHFSM_MultiScale(src, hist, lbp_opt);
			//calCLBP_MultiScale(src, hist, lbp_opt);
			calLBPHFSMC_MultiScale(src, hist, lbp_opt);
			fs_feature << -1 << hist;
		}
		fs_feature << "]";

		cout << "\n";
		cout << featureFile << "   saved" << endl;

	}
#pragma endregion

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region stage_3_HOG
	bool read_from_file = true;

	if (stage == 3)
	{
		cout << "stage_3_hog" << endl;

#pragma region read_from_file
		if (read_from_file)
		{
			string pos_image_folder = "pos_after_1_fix";
			string neg_image_folder = "neg_after_1_fix";
			vector<string> pos_path;
			vector<string> neg_path;
			getFiles(pos_image_folder, pos_path);
			getFiles(neg_image_folder, neg_path);
			/////////////////////////////////////////////
			CvBoost  boost_stage_2;
			string boost_filename_2 = "connected_component_feature_hog_800_ex_fix-boost.xml";
			cout << "Boost file: " << boost_filename_2 << endl;
			boost_stage_2.load(boost_filename_2.c_str());

			vector<String> fake;
			/*cout << "pos: " << pos_path.size()<<endl;
			filter_stage_2_boost(pos_path, boost_stage_2, -1, fake);
			cout << "pos_total_after filter stage 2:" << pos_path.size() << endl;*/
			//saveErrImg(neg_path, string("neg_fix_1"));

			cout << "neg: " << neg_path.size() << endl;
			filter_stage_2_boost(neg_path, boost_stage_2, -1, fake);
			cout << "pos_total_after filter stage 2:" << neg_path.size() << endl;

			saveErrImg(fake, string("neg_fix_2"));
			neg_path = fake;
////////////////////////////////////////////////////////////////////////////////////////////
			int neg_use = min(pos_path.size()/2, neg_path.size());

			string featureFile = "connected_component_feature_hog_800_ex_pos_pos.xml";
			FileStorage fs_feature(featureFile, FileStorage::WRITE);
			int imageNum = pos_path.size() + neg_use;
			int featureDim = 800;
			fs_feature << "imageNum" << imageNum;
			fs_feature << "featureDim" << featureDim;
			fs_feature << "lable_feature" << "[";
			/*positive*/
			for (int i = 0; i != pos_path.size(); ++i)
			{
				cout << "pos  i:   " << i<<"\r";
				string temp_path = pos_path[i];
				Mat src = imread(pos_path[i]);
				vector<float> feature;
				calculateHOG(src, feature);
				fs_feature << 1 << feature;
			}
			/*negative*/
			cout << "\n\n";
			for (int i = 0; i != neg_use; ++i)
			{
				cout << "neg  i:   " << i<<"\r";
				Mat src = imread(neg_path[i]);
				vector<float> feature;
				calculateHOG(src, feature);
				fs_feature << -1 << feature;
			}
			fs_feature << "]";
			cout << "\n"<<featureFile << "   saved" << endl;
		}
#pragma endregion
		else
		{
			CvBoost  boost_stage_1;
			string boost_filename = "connected_component_feature_5_1000_05-boost.xml";
			cout << "Boost file: " << boost_filename << endl;
			boost_stage_1.load(boost_filename.c_str());

			cout << "neg_total:  " << neg_total.size() << endl;
			filter_stage_1_boost(neg_total, boost_stage_1, -1);

			cout << "neg_total_after filter stage 1:" << neg_total.size() << endl;

			cout << "pos_total:  " << pos_total.size() << endl;
			filter_stage_1_boost(pos_total, boost_stage_1, -1);

			cout << "pos_total_after filter stage 1:" << pos_total.size() << endl;

///////////////////评估分类器在训练集上的性能/////////////////////
			CvBoost  boost_stage_2;
			string boost_filename_2 = "connected_component_feature_hog_800_ex_fix-boost.xml";
			cout << "Boost file: " << boost_filename_2 << endl;
			boost_stage_2.load(boost_filename_2.c_str());

			vector<Region> fake;
			filter_stage_2_boost(pos_total, boost_stage_2, -1,fake);
			cout << "pos_total_after filter stage 2:" << pos_total.size() << endl;

			vector<Region> ambiguity;
			filter_stage_2_boost(neg_total, boost_stage_2, 0, ambiguity);
			cout << "neg_total_after filter stage 2:" << neg_total.size() << endl;
			//rectsSaveAsImage(pos_total, string("pos_after_1"), Size(24, 24), string("pos"));
			//rectsSaveAsImage(neg_total, string("neg_after_1"), Size(24, 24), string("neg"));
			//rectsSaveAsImage(ambiguity, string("neg_after_1_1"), Size(24, 24), string("neg"));
			//rectsSaveAsImage(neg_total, string("neg_after_1_2"), Size(24, 24), string("neg"));

			neg_total.clear();
			neg_total = ambiguity;
			int neg_use = min(pos_total.size(), neg_total.size());

			string featureFile = "connected_component_feature_hog_800_ex_repeat.xml";
			FileStorage fs_feature(featureFile, FileStorage::WRITE);
			int imageNum = pos_total.size() + neg_use;
			int featureDim = 800;
			fs_feature << "imageNum" << imageNum;
			fs_feature << "featureDim" << featureDim;
			fs_feature << "lable_feature" << "[";
			/*positive*/
			for (int i = 0; i != pos_total.size(); ++i)
			{
				cout << "pos  i:   " << i << "\r";
				vector<float> feature;
				calculateHOG(pos_total[i].image, feature);
				fs_feature << 1 << feature;
			}
			/*negative*/
			cout << "\n";
			for (int i = 0; i != neg_use; ++i)
			{
				cout << "neg  i:   " << i << "\r";
				vector<float> feature;
				calculateHOG(neg_total[i].image, feature);
				fs_feature << -1 << feature;
			}
			fs_feature << "]";
			cout << featureFile << "   saved" << endl;
		}


	}
#pragma endregion

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region neg_merge
	if (stage == 4)
	{
		cout << "prepare for the samples" << endl;
		vector<Region> pos_total;
		vector<Region> neg_total;
		/////////////////////////*preparation*/////////////////////////////////////////////
		CvBoost  boost_stage_1;
		string boost_filename_1 = "connected_component_feature_5_1000_05_fix-boost.xml";
		cout << "Boost file: " << boost_filename_1 << endl;
		boost_stage_1.load(boost_filename_1.c_str());

		CvBoost  boost_stage_2;
		string boost_filename_2 = "connected_component_feature_hog_800_ex_fix-boost.xml";
		cout << "Boost file: " << boost_filename_2 << endl;
		boost_stage_2.load(boost_filename_2.c_str());

		for (int i = 0; i < min((size_t)50000, panels_total.size()); ++i)
		{
			cout << i << '\r';
#pragma region cc_detect

			Panel p = panels_total[i];

			Mat src = imread(p.imgFile);
			Mat panel(src, p.box);

		
			vector<Region> regions;
			Mat panel_c = panel.clone();
			regionDetect(panel_c, regions);
#pragma endregion

			///////////////////////////////////////////////////////////////////////////////
#pragma region stage_1
			vector<Region> non_texts;
			filter_stage_1_boost(regions, boost_stage_1, -1, non_texts);
#pragma endregion

			///////////////////////////////////////////////////////////////////////////////
#pragma region stage_2
			vector<Region> re_texts;
			filter_stage_2_boost(regions, boost_stage_2, -1, 0, re_texts, non_texts);

#pragma endregion
			//////////////////////////////////////////////////////////////////////////////
#pragma region texts_detection
			Mat show_string = panel.clone();
			double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得

			vector<double> w(init_w_3, init_w_3 + 6);
			vector<Str_Chinese> texts;
			textDetect(re_texts, texts, w);
			for (auto &t : texts)
			{
				sort(t.str.begin(), t.str.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
			}
			////////////////////////////////////////////////////////////////////////////
			vector<Region_add> re_adds;
			Mat show_non_texts = panel.clone();
			findChaAdd(texts, non_texts, re_adds, show_non_texts);
			
			
			for (auto add : re_adds)
			{
				
				vector<Region> temp;
				
				CC_Merge(add.re_add, temp, 0.5, 3, add.area_per_character, panel);
			

				filter_stage_1_boost(temp, boost_stage_1, -1);
				filter_stage_2_boost(temp, boost_stage_2, -1);
				
				if (!temp.empty())
				{
					regions.insert(regions.end(), temp.begin(), temp.end());
				}
			}

			vector<Region> pos, neg;
			gth_match(regions, p, pos, neg, src);  // prepare for the pos and neg

			Mat show_pos_extra = panel.clone();
			Mat show_neg_extra = panel.clone();
			drawRects(show_pos_extra, pos);
			drawRects(show_neg_extra, neg);

			pos_total.insert(pos_total.end(), pos.begin(), pos.end());
			if (neg_total.size() <= 500000) neg_total.insert(neg_total.end(), neg.begin(), neg.end());
#pragma endregion
		}
#pragma region calculate the HOG feature

		cout << "pos: " << pos_total.size() << endl;
		cout << "neg: " << neg_total.size() << endl;
	//	rectsSaveAsImage(pos_total, string("pos_last"), Size(24, 24), string("pos"));
	//	rectsSaveAsImage(neg_total, string("neg_last"), Size(24, 24), string("neg"));

		int neg_use = min(pos_total.size(), neg_total.size());
		bool HOG = false;
		if (HOG)
		{
			string featureFile = "connected_component_feature_hog_800_ex_last_stage.xml";
			FileStorage fs_feature(featureFile, FileStorage::WRITE);
			int imageNum = pos_total.size() + neg_use;
			int featureDim = 800;
			fs_feature << "imageNum" << imageNum;
			fs_feature << "featureDim" << featureDim;
			fs_feature << "lable_feature" << "[";
			/*positive*/
			for (int i = 0; i != pos_total.size(); ++i)
			{
				vector<float> feature;
				calculateHOG(pos_total[i].image, feature);
				//calculateColorHOG(pos_total[i].image, feature);
				fs_feature << 1 << feature;
			}
			/*negative*/
			for (int i = 0; i != neg_use; ++i)
			{
				vector<float> feature;
				calculateHOG(neg_total[i].image, feature);
				//calculateColorHOG(neg_total[i].image, feature);
				fs_feature << -1 << feature;
			}
			fs_feature << "]";
			cout << featureFile << "   saved" << endl;
		}

#pragma endregion
	}

#pragma endregion
	//////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region neg_text_line

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

	if (stage == 5)
	{
		vector<Region> neg_text;
		for (int i = 0; i < panels_total.size(); ++i)
		{
			Panel p = panels_total[i];
		
			Mat src = imread(p.imgFile);
			Mat panel(src, p.box);

			vector<Str_Chinese> texts;
			textlineDetection(panel, texts, boost_stage_1,boost_stage_2);

			Mat ftl = panel.clone();
			filterTextlines(texts, boost_stage_3, ftl);
			refineTexts(texts);

			vector<Str_Chinese> neg;
			textLines_gth_match(texts, p, neg);

			vector<Region> neg_one;
			for (auto n : neg)
			{
				neg_one.push_back(n.str[0]);
				neg_text.push_back(n.str[0]);
			}
			
			Mat show = panel.clone();
			Mat show_gth = panel.clone();
			drawRects(show, neg_one);
			showPanelGth(show_gth, p);

			string image_name = getFileName(p.imgFile);
			rectsSaveAsImage(neg_one, string("neg_texts_test"), Size(24, 24), image_name,i);

			waitKey(0);
		}

		cout << "neg: " << neg_text.size() << endl;
		//rectsSaveAsImage(neg_text, string("neg_texts_test"), Size(24, 24), string("neg"));
	}
#pragma endregion

	//////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region feature for the images 独立，从文件夹中读取图片并提取特征
	if (stage == 6)
	{
		/*string imageFile = "pos_last_path.txt";
		vector<string> pos_image_path;
		int posImageCount = readImageFile(imageFile, pos_image_path);

		imageFile = "neg_last_path.txt";
		vector<string> neg_image_path;
		int negImageCount = readImageFile(imageFile, neg_image_path);*/

		string pos_image_folder = "pos_after_1_fix";
		string neg_image_folder = "neg_texts_test";
		vector<string> pos_image_path;
		vector<string> neg_image_path;
		getFiles(pos_image_folder, pos_image_path);
		getFiles(neg_image_folder, neg_image_path);

		CvBoost  boost_stage_3;
		string boost_filename_3 = "connected_component_feature_hog_800_ex_last_stage-boost.xml";
		cout << "Boost file: " << boost_filename_3 << endl;
		boost_stage_3.load(boost_filename_3.c_str());

		vector<String> fake;
		cout << "pos: " << pos_image_path.size() << endl;
		filter_stage_2_boost(pos_image_path, boost_stage_3, 0, fake);
		cout << "pos_total_after filter stage 3:" << pos_image_path.size() << endl;
		//saveErrImg(neg_path, string("neg_fix_1"));

		cout << "neg: " << neg_image_path.size() << endl;
		filter_stage_2_boost(neg_image_path, boost_stage_3, 0, fake);
		cout << "neg_total_after filter stage 3:" << neg_image_path.size() << endl;
		saveErrImg(neg_image_path, string("neg_test_filter"));

		string featureFile = "connected_component_feature_hog_800_ex_last_stage.xml";
		FileStorage fs_feature(featureFile, FileStorage::WRITE);
		int imageNum = pos_image_path.size() + neg_image_path.size();
		int featureDim = 800;
		fs_feature << "imageNum" << imageNum;
		fs_feature << "featureDim" << featureDim;
		fs_feature << "lable_feature" << "[";
		/*positive*/
		cout << "\npositive\n";
		for (int i = 0; i != pos_image_path.size(); ++i)
		{
			cout << "i :  " << i << '\r';
			Mat img = imread(pos_image_path[i]);
			vector<float> feature;
			calculateHOG(img, feature);
			//calculateColorHOG(img, feature);
			fs_feature << 1 << feature;
		}
		/*negative*/
		cout << "\n\nnegtive\n";
		for (int i = 0; i != neg_image_path.size(); ++i)
		{
			cout << "i :  " << i << '\r';
			Mat img = imread(neg_image_path[i]);
			vector<float> feature;
			calculateHOG(img, feature);
			//calculateColorHOG(img, feature);
			fs_feature << -1 << feature;
		}
		fs_feature << "]";
		cout << "\n";
		cout << featureFile << "   saved" << endl;

	}
#pragma endregion

	return EXIT_SUCCESS;
}
