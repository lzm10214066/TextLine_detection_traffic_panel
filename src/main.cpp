#include "main.h"

int total_str_num = 0;
int total_panel_num = 0;
int total_cha_num = 0;

int main(void)
{
	//////////////////////////////////////*read path.txt*///////////////////////////
	string imageFile = "../image_path/panel_train_path.txt";
	//imageFile = "../image_path/images_textline_path.txt";
	vector<string> image_path;

	string str_dataFile = "../image_path/str_train_datas_path.txt";
	//str_dataFile = "../image_path/string_textline_path.txt";
	vector<string> str_data_path;

	string cha_dataFile = "../image_path/cha_train_datas_path.txt";
//	cha_dataFile = "../image_path/cha_texteline_path.txt";
	vector<string> cha_data_path;

	loadFile(image_path, str_data_path, cha_data_path,imageFile, str_dataFile,cha_dataFile);

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
	for (int i = 0; i != min(2000,(int)image_path.size()); ++i)
	{
		std:: cout << i + 1 << ">" << endl;
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
		getPanelStrChaData(tempImg,tempStrData, tempChaData, panels,stringNum,chaNum);
		showPanelData(image_rects, panels);

	/*	vector<pair<Region, Region>> M;
		vector<pair<Region, Region>> C;
		vector<double> M_dist;
		vector<double> C_dist;
		double init_w[] = { -0.05, 0.15, 0.15, 0.15, 0.15, 0.15 };
		vector<double> w(init_w, init_w + 6);
		sampleSeletion(panels, w, M_dist, C_dist, M, C);

		Mat image_M = imgRGB.clone();
		for (int i = 0; i < M.size(); ++i)
		{
			Rect r1 = M[i].first.r;
			Rect r2 = M[i].second.r;

			int b = theRNG().uniform(0, 255);
			int g = theRNG().uniform(0, 255);
			int r = theRNG().uniform(0, 255);

			Scalar color(b, g, r);
			rectangle(image_M, r1, color);
			rectangle(image_M, r2, color);

			int t = theRNG().uniform(0, 255);
		}

		Mat image_C = imgRGB.clone();
		for (int i = 0; i < C.size(); ++i)
		{
			Rect r1 = C[i].first.r;
			Rect r2 = C[i].second.r;

			int b = theRNG().uniform(0, 255);
			int g = theRNG().uniform(0, 255);
			int r = theRNG().uniform(0, 255);

			Scalar color(b, g, r);
			rectangle(image_C, r1, color);
			rectangle(image_C, r2, color);

			int t = theRNG().uniform(0, 255);
		}
		cout << "M: " << M.size() << "  C: " << C.size() << endl;
		total_str_num += M.size();
		total_cha_num += C.size();*/

		panels_total.insert(panels_total.end(), panels.begin(), panels.end());
		total_str_num += stringNum;
		total_cha_num += chaNum;
	}
	total_panel_num = panels_total.size();
	/////////////////////////*Logistic Regression*/////////////////////////////////////////////
	int it_num = 500;
	double init_w[] = { -1, 1, 1,1,1,1};
	vector<double> w(init_w,init_w+6);
	vector<double> cost(it_num);
	int k = 0;
	while (it_num--)
	{
		vector<double> M_dist;
		vector<vector<double>> M_feature;
		vector<double> C_dist;
		vector<vector<double>> C_feature;
		sampleSeletion(panels_total, w, M_dist, C_dist,M_feature,C_feature);

		int d = it_num;
		updateW(10,w, M_dist, C_dist, M_feature, C_feature);
		cost[k] = cost_fuction(M_feature, C_feature, w);
		cout <<cost[k]<< endl;
		++k;
	}
	cout << "\n\nw: " << endl;
	for (int i = 0; i < w.size(); ++i)
	{
		cout << w[i] << "   ";
	}
	ofstream costData("costData.txt");
	for (int i = 0; i < k; ++i)
	{
		costData << cost[i] << endl;
	}
	costData.close();
	cout << "\n\nupdate w down" << endl;
	cout << "panel_num_Total : " << total_panel_num << endl;
	cout << "string_num_Total : " << total_str_num<< endl;
	cout << "cha_num_Total : " << total_cha_num << endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	return EXIT_SUCCESS;
}
