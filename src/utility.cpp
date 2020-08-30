#include "utility.h"
#include "imageRead.h"

string getFileName(string str)
{
	int m = str.find_last_of('\\');
	int n = str.find_last_of('.');

	return str.substr(m + 1, n - m - 1);
}
void getRangeFromTxt(string tempObject, int *xmin, int *xmax, int *ymin, int *ymax)
{
	int posi[8];
	int k = 0;
	posi[0] = 0;
	for (int i = 0; i != tempObject.size(); ++i)
	{
		if (tempObject[i] == ',')
		{
			posi[++k] = i;
		}
	}

	int xy[8];

	xy[0] = atoi(tempObject.substr(0, posi[1]).c_str());
	for (k = 1; k < 8; k++)
	{
		xy[k] = atoi(tempObject.substr(posi[k] + 1, posi[k + 1] - posi[k] - 1).c_str());
	}

	/*check premeters*/
	{
		xy[0] = xy[0] < 0 ? 0 : xy[0]; xy[0] = xy[0] > 479 ? 479 : xy[0];
		xy[1] = xy[1] < 0 ? 0 : xy[1]; xy[1] = xy[1] > 359 ? 359 : xy[1];

		xy[4] = xy[4] < 0 ? 0 : xy[4]; xy[4] = xy[4] > 479 ? 479 : xy[4];
		xy[5] = xy[5] < 0 ? 0 : xy[5]; xy[5] = xy[5] > 359 ? 359 : xy[5];
	}

	*xmin = xy[0];
	*ymin = xy[1];
	*xmax = xy[4];
	*ymax = xy[5];
}

void loadFile(vector<string> &image_path, vector<string> &data_path, string imageFile, string dataFile)
{
	readImageFile(imageFile, image_path);
	readImageFile(dataFile, data_path);
}
void loadFile(vector<string> &image_path, vector<string> &str_data_path, vector<string> &cha_data_path, string imageFile, string str_dataFile, string cha_dataFile)
{
	readImageFile(imageFile, image_path);
	readImageFile(str_dataFile, str_data_path);
	readImageFile(cha_dataFile, cha_data_path);
}

void normSize(Mat &recImage, double length)
{
	double w = recImage.cols;
	double h = recImage.rows;

	if (max(w, h) == length)
		return;

	if (w > h)
	{
		double s = length / w;
		resize(recImage, recImage, Size(), s, s);
	}
	else
	{
		double s = length / h;
		resize(recImage, recImage, Size(), s, s);
	}
}

float overlap(const cv::Rect &a, const cv::Rect &b)
{
	int w = std::min(a.x + a.width, b.x + b.width) - std::max(a.x, b.x);
	int h = std::min(a.y + a.height, b.y + b.height) - std::max(a.y, b.y);

	return (w < 0 || h < 0) ? 0.f : (float)(w * h);
}
int overlap_object(Rect r, int *xmin, int *ymin, int *xmax, int *ymax, int n, double th)
{
	double over_area_total = 0;
	for (int i = 0; i < n; ++i)
	{
		int sx = max(xmin[i], r.x);
		int sy = max(ymin[i], r.y);

		int ex = min(xmax[i], r.x + r.width);
		int ey = min(ymax[i], r.y + r.height);

		int over_w = ex - sx > 0 ? ex - sx : 0;
		int over_h = ey - sy > 0 ? ey - sy : 0;

		double over_area = over_w*over_h;
		over_area_total += over_area;
	}
	double ra = over_area_total / r.area();
	if (ra > th)
		return 1;
	return 0;
}

void rect_extend(vector<Rect> &rects, double r, int width, int height)
{
	for (int i = 0; i < rects.size(); ++i)
	{
		Rect pr = rects[i];
		int dx = cvRound(pr.width*(r - 1) / 2.0);
		int dy = cvRound(pr.height*(r - 1) / 2.0);
		int w = cvRound(pr.width*r);
		int h = cvRound(pr.height*r);
		int x = pr.x - dx;
		int y = pr.y - dy;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x + w>width - 1)
			w = width - 1 - x;
		if (y + h>height - 1)
			h = height - 1 - y;

		rects[i].x = x;
		rects[i].y = y;
		rects[i].width = w;
		rects[i].height = h;
	}
}

int similarity(Rect r, int xmin, int ymin, int xmax, int ymax, double th)
{
	int sx = max(xmin, r.x);
	int sy = max(ymin, r.y);

	int ex = min(xmax, r.x + r.width);
	int ey = min(ymax, r.y + r.height);

	int over_w = ex - sx > 0 ? ex - sx : 0;
	int over_h = ey - sy > 0 ? ey - sy : 0;

	double over_area = over_w*over_h;
	double un_area = r.area() + (xmax - xmin)*(ymax - ymin) - over_area;

	double r_a = over_area / un_area;
	if (r_a > th)
		return 1;

	return 0;
}

void processPanelData(vector<string> &rects_str, vector<string> &panelBox, vector<string> &stringBox)
{
	for (int i = 0; i < rects_str.size();)
	{
		string tempN = rects_str[i++];
		string tempP = rects_str[i++];
		i++;

		if (!tempP.compare("b\r"))
			panelBox.push_back(tempN);
		else if (!tempP.compare("c\r"))
			stringBox.push_back(tempN);
	}
}
void processChaData(vector<string> &rects_cha, vector<string> &chaBox)
{
	for (int i = 0; i < rects_cha.size();)
	{
		string tempN = rects_cha[i++];
		string tempP = rects_cha[i++];
		i++;

		if (!tempP.compare("c\r"))
			chaBox.push_back(tempN);
	}
}

bool com_str(Str_Chinese r1, Str_Chinese r2)
{
	return r1.box.y < r2.box.y;
}
bool com_cha(Region r1, Region r2)
{
	return r1.cen_x < r2.cen_x;
}

void getPanelStrChaData(string tempStrData, string tempChaData, vector<Panel> &panels, int &stringNum, int &chaNum)
{
	vector<string> rects_str;
	readImageFile(tempStrData, rects_str);
	vector<string> panelBox, stringBox;
	processPanelData(rects_str, panelBox, stringBox);

	int panelNum = panelBox.size();
	int xmin_b = 0;
	int xmax_b = 0;
	int ymin_b = 0;
	int ymax_b = 0;

	stringNum = stringBox.size();
	int xmin_s = 0;
	int xmax_s = 0;
	int ymin_s = 0;
	int ymax_s = 0;

	vector<string> rects_cha;
	readImageFile(tempChaData, rects_cha);
	vector<string> chaBox;
	processChaData(rects_cha, chaBox);

	chaNum = chaBox.size();
	int xmin_c = 0;
	int xmax_c = 0;
	int ymin_c = 0;
	int ymax_c = 0;

	for (int i = 0; i < panelNum; ++i)
	{
		string tempObject = panelBox[i];
		getRangeFromTxt(tempObject, &xmin_b, &xmax_b, &ymin_b, &ymax_b);
		Rect r_panel(xmin_b, ymin_b, xmax_b - xmin_b + 1, ymax_b - ymin_b + 1);
		Panel panel(r_panel);
		panels.push_back(panel);
	}

	for (int i = 0; i < stringNum; ++i)
	{
		string tempObject = stringBox[i];
		getRangeFromTxt(tempObject, &xmin_s, &xmax_s, &ymin_s, &ymax_s);
		Rect r_str(xmin_s, ymin_s, xmax_s - xmin_s + 1, ymax_s - ymin_s + 1);
		for (int j = 0; j < panels.size(); ++j)
		{
			double o = overlap(r_str, panels[j].box);
			double a = r_str.area();

			if (o / a > 0.7)
			{
				panels[j].strs.push_back(Str_Chinese(r_str));
				break;
			}
		}
	}

	for (int i = 0; i < chaNum; ++i)
	{
		string tempObject = chaBox[i];
		getRangeFromTxt(tempObject, &xmin_c, &xmax_c, &ymin_c, &ymax_c);
		Rect r_cha(xmin_c, ymin_c, xmax_c - xmin_c + 1, ymax_c - ymin_c + 1);
		bool find = false;

		for (int j = 0; j < panels.size(); ++j)
		{
			for (int k = 0; k < panels[j].strs.size(); ++k)
			{
				double o = overlap(r_cha, panels[j].strs[k].box);
				double a = r_cha.area();

				if (o / a > 0.7)
				{
					panels[j].strs[k].str.push_back(Region(r_cha));
					find = true;
					break;
				}
			}
			if (find) break;
		}
	}

	for (int i = 0; i < panels.size(); ++i)
	{
		sort(panels[i].strs.begin(), panels[i].strs.end(), com_str);
		for (int j = 0; j < panels[i].strs.size(); ++j)
		{
			sort(panels[i].strs[j].str.begin(), panels[i].strs[j].str.end(), com_cha);
		}
	}
}
void getPanelStrChaData(string tempImg,string tempStrData, string tempChaData, vector<Panel> &panels, int &stringNum, int &chaNum)
{
	vector<string> rects_str;
	readImageFile(tempStrData, rects_str);
	vector<string> panelBox, stringBox;
	processPanelData(rects_str, panelBox, stringBox);

	int panelNum = panelBox.size();
	int xmin_b = 0;
	int xmax_b = 0;
	int ymin_b = 0;
	int ymax_b = 0;

	stringNum = stringBox.size();
	int xmin_s = 0;
	int xmax_s = 0;
	int ymin_s = 0;
	int ymax_s = 0;

	vector<string> rects_cha;
	readImageFile(tempChaData, rects_cha);
	vector<string> chaBox;
	processChaData(rects_cha, chaBox);

	chaNum = chaBox.size();
	int xmin_c = 0;
	int xmax_c = 0;
	int ymin_c = 0;
	int ymax_c = 0;

	for (int i = 0; i < panelNum; ++i)
	{
		string tempObject = panelBox[i];
		getRangeFromTxt(tempObject, &xmin_b, &xmax_b, &ymin_b, &ymax_b);
		Rect r_panel(xmin_b, ymin_b, xmax_b - xmin_b + 1, ymax_b - ymin_b + 1);
		Panel panel(r_panel);
		panel.imgFile = tempImg;
		panels.push_back(panel);
	}

	for (int i = 0; i < stringNum; ++i)
	{
		string tempObject = stringBox[i];
		getRangeFromTxt(tempObject, &xmin_s, &xmax_s, &ymin_s, &ymax_s);
		Rect r_str(xmin_s, ymin_s, xmax_s - xmin_s + 1, ymax_s - ymin_s + 1);
		for (int j = 0; j < panels.size(); ++j)
		{
			double o = overlap(r_str, panels[j].box);
			double a = r_str.area();

			if (o / a > 0.7)
			{
				panels[j].strs.push_back(Str_Chinese(r_str));
				break;
			}
		}
	}

	for (int i = 0; i < chaNum; ++i)
	{
		string tempObject = chaBox[i];
		getRangeFromTxt(tempObject, &xmin_c, &xmax_c, &ymin_c, &ymax_c);
		Rect r_cha(xmin_c, ymin_c, xmax_c - xmin_c + 1, ymax_c - ymin_c + 1);
		bool find = false;

		for (int j = 0; j < panels.size(); ++j)
		{
			for (int k = 0; k < panels[j].strs.size(); ++k)
			{
				double o = overlap(r_cha, panels[j].strs[k].box);
				double a = r_cha.area();

				if (o / a > 0.7)
				{
					panels[j].strs[k].str.push_back(Region(r_cha));
					find = true;
					break;
				}
			}
			if (find) break;
		}
	}

	for (int i = 0; i < panels.size(); ++i)
	{
		sort(panels[i].strs.begin(), panels[i].strs.end(), com_str);
		for (int j = 0; j < panels[i].strs.size(); ++j)
		{
			sort(panels[i].strs[j].str.begin(), panels[i].strs[j].str.end(), com_cha);
		}
	}
}

void showPanelData(Mat &img, vector<Panel> &panels)
{
	for (int i = 0; i < panels.size(); ++i)
	{
		rectangle(img, panels[i].box, Scalar(0, 255, 255));

		for (int j = 0; j < panels[i].strs.size(); ++j)
		{
			rectangle(img, panels[i].strs[j].box, Scalar(0, 255, 0));

			for (int k = 0; k < panels[i].strs[j].str.size(); ++k)
			{
				rectangle(img, panels[i].strs[j].str[k].r, Scalar(0, 0, 255));
			}
		}
	}
}

bool isOnLine(Region r1, Region r2)
{
	/*double dx = abs(r1.cen_x - r2.cen_x);
	double dy = abs(r1.cen_y - r2.cen_y);
	double the = atan(dy / dx)*180/CV_PI;
	if (the < 10)
	return true;*/
	/*int up = r1.r.y;
	int down = r1.r.y + r1.r.height - 1;
	if (r2.cen_y >= up && r2.cen_y <= down)
	return true;*/
	double y1 = r1.r.y;
	double y11 = r1.r.y + r1.r.height - 1;
	double y2 = r2.r.y;
	double y22 = r2.r.y + r2.r.height - 1;

	double ys = max(y1, y2);
	double ye = min(y11, y22);

	double inLen = ye > ys ? ye - ys : 0;
	double unLen = r1.r.height + r2.r.height - inLen;
	if (inLen / unLen > 0.3)
		return true;
	return false;
}

void prepaereSamples(vector<Panel> &panels, vector<pair<Region, Region>> &pos_samples, vector<pair<Region, Region>> &neg_samples)
{
	if (panels.empty()) return;

	/******************for positive samples****************/
	for (int i = 0; i < panels.size(); ++i)
	{
		if (panels[i].strs.empty()) continue;
		for (int j = 0; j < panels[i].strs.size(); ++j)
		{
			vector<Region> temp_str = panels[i].strs[j].str;
			int region_num = temp_str.size();
			if (region_num < 2) continue;

			for (int m = 0; m < region_num - 1; ++m)
			{
				Region r1 = temp_str[m];
				Region r2 = temp_str[m + 1];
				if (isOnLine(r1, r2))
				{
					pos_samples.push_back(make_pair(r1, r2));
				}
			}
		}
	}

	/******************for negative samples****************/
	vector<pair<Str_Chinese, Str_Chinese>> closeStrs;

	for (int i = 0; i < panels.size(); ++i)
	{
		if (panels[i].strs.size() < 2) continue;
		vector<Str_Chinese> strs = panels[i].strs;

		for (int j = 0; j < strs.size() - 1; ++j)
		{
			Str_Chinese str_j = strs[j];
			int minDist = INT_MAX;
			int p = 0;
			for (int k = j + 1; k < strs.size(); ++k)
			{
				Str_Chinese str_k = strs[k];
				double dist = (str_j.box.x - str_k.box.x)*(str_j.box.x - str_k.box.x) + (str_j.box.y - str_k.box.y)*(str_j.box.y - str_k.box.y);
				if (dist < minDist)
				{
					minDist = dist;
					p = k;
				}
			}
			Str_Chinese str_p = strs[p];
			closeStrs.push_back(make_pair(str_j, str_p));
		}
	}

	for (int i = 0; i < closeStrs.size(); ++i)
	{
		vector<Region> str1 = closeStrs[i].first.str;
		vector<Region> str2 = closeStrs[i].second.str;
		if (str1.empty() || str2.empty()) continue;
		for (int j = 0; j < str1.size(); ++j)
		{
			Region rj = str1[j];
			int minDist = INT_MAX;
			int p = 0;
			for (int k = 0; k < str2.size(); ++k)
			{
				Region rk = str2[k];
				double dist = (rj.cen_x - rk.cen_x)*(rj.cen_x - rk.cen_x) + (rj.cen_y - rk.cen_y)*(rj.cen_y - rk.cen_y);
				if (dist < minDist)
				{
					minDist = dist;
					p = k;
				}
			}
			Region rp = str2[p];
			neg_samples.push_back(make_pair(rj, rp));
		}
	}
}

double energy(Region &cc)
{
	Mat src = cc.image;
	Mat src_gray;
	if (src.channels() != 1) cvtColor(src, src_gray, CV_RGB2GRAY);
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	/// Gradient X
	//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
	Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
	/// Gradient Y
	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);
	/// Total Gradient (approximate)
	Mat grad;
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

	Scalar s = mean(grad);
	return s(0);
}

void calRegionFeature(Region &cc, vector<double> &feature)
{
	double area = cc.p_h*cc.p_w;

	double w_h_r = (double)cc.r.width / cc.r.height;
	double area_r = (double)cc.r.area() / area;

	double w_r = (double)cc.r.width / cc.p_w;
	double h_r = (double)cc.r.height / cc.p_h;

	feature.push_back(w_h_r);

	feature.push_back(area_r);
	feature.push_back(w_r);
	feature.push_back(h_r);

	feature.push_back(cc.var);

	//feature.push_back(cc.r.x);
	//feature.push_back(cc.r.y);
	//feature.push_back(cc.p_w - cc.r.x - cc.r.width);
	//feature.push_back(cc.p_h - cc.r.y - cc.r.height);
	/*double e = energy(cc);
	feature.push_back(e);*/
}

void calRegionFeatureFix(Region &cc, vector<double> &feature)
{
	double min_length = min(cc.p_h, cc.p_w);

	double area = min_length*min_length;

	double w_h_r = (double)cc.r.width / cc.r.height;
	double area_r = (double)cc.r.area() / area;

	double w_r = (double)cc.r.width / min_length;
	double h_r = (double)cc.r.height / min_length;

	feature.push_back(w_h_r);

	feature.push_back(area_r);
	feature.push_back(w_r);
	feature.push_back(h_r);

	feature.push_back(cc.var);
}

void updateRegionTh(vector<Panel> &panels, Region_TH &region_th)
{
	for (int i = 0; i < panels.size(); ++i)
	{
		double area_p = panels[i].box.area();
		for (int j = 0; j < panels[i].strs.size(); ++j)
		{
			for (int k = 0; k < panels[i].strs[j].str.size(); ++k)
			{
				Region r = panels[i].strs[j].str[k];
				double w_h_r = (double)r.r.width / r.r.height;
				double area_r = (double)r.r.area() / area_p;

				region_th.area_r_max = max(region_th.area_r_max, area_r);
				region_th.area_r_min = min(region_th.area_r_min, area_r);
				region_th.ratio_max = max(region_th.ratio_max, w_h_r);
				region_th.ratio_min = min(region_th.ratio_min, w_h_r);
			}
		}
	}
}
//void prepaereSamples(vector<Panel> &panels, vector<pair<Region, Region>> &pos_samples, vector<pair<Region, Region>> &neg_samples)
//{
//	if (panels.empty()) return;
//
//	/******************for positive samples****************/
//	for (int i = 0; i < panels.size(); ++i)
//	{
//		if (panels[i].strs.empty()) continue;
//		for (int j = 0; j < panels[i].strs.size(); ++j)
//		{
//			vector<Region> temp_str = panels[i].strs[j].str;
//			int region_num = temp_str.size();
//			if (region_num < 2) continue;
//
//			for (int m = 0; m < region_num - 1; ++m)
//			{
//				Region r1 = temp_str[m];
//				int minDist = INT_MAX;
//				int k = 0;
//				for (int n = m + 1; n < region_num; ++n)
//				{
//					Region r2 = temp_str[n];
//					if (isOnLine(r1, r2))
//					{
//						int dist = abs(r1.cen_x - r2.cen_x);
//						if (dist < minDist)
//						{
//							minDist = dist;
//							k = n;
//						}
//					}
//				}
//				Region rk = temp_str[k];
//				pos_samples.push_back(make_pair(r1, rk));
//			}
//		}
//	}
//
//	/******************for negative samples****************/
//	vector<pair<Str_Chinese, Str_Chinese>> closeStrs;
//
//	for (int i = 0; i < panels.size(); ++i)
//	{
//		if (panels[i].strs.size()<2) continue;
//		vector<Str_Chinese> strs = panels[i].strs;
//		for (int j = 0; j < strs.size() - 1; ++j)
//		{
//			Str_Chinese str_j = strs[j];
//			int minDist = INT_MAX;
//			int p = 0;
//			for (int k = j + 1; k < strs.size(); ++k)
//			{
//				Str_Chinese str_k = strs[k];
//				double dist = (str_j.box.x - str_k.box.x)*(str_j.box.x - str_k.box.x) + (str_j.box.y - str_k.box.y)*(str_j.box.y - str_k.box.y);
//				if (dist < minDist)
//				{
//					minDist = dist;
//					p = k;
//				}
//			}
//			Str_Chinese str_p = strs[p];
//			closeStrs.push_back(make_pair(str_j, str_p));
//		}
//	}
//
//	for (int i = 0; i < closeStrs.size(); ++i)
//	{
//		vector<Region> str1 = closeStrs[i].first.str;
//		vector<Region> str2 = closeStrs[i].second.str;
//
//		for (int j = 0; j < str1.size(); ++j)
//		{
//			Region rj = str1[j];
//			int minDist = INT_MAX;
//			int p = 0;
//			for (int k = 0; k < str2.size(); ++k)
//			{
//				Region rk = str2[k];
//				double dist = (rj.cen_x - rk.cen_x)*(rj.cen_x - rk.cen_x) + (rj.cen_y - rk.cen_y)*(rj.cen_y - rk.cen_y);
//				if (dist < minDist)
//				{
//					minDist = dist;
//					p = k;
//				}
//			}
//			Region rp = str2[p];
//			neg_samples.push_back(make_pair(rj, rp));
//		}
//	}
//}

double calRegionPairDist(Region r1,Region r2,vector<double> &w)
{
	vector<double> feature;

	feature.push_back(1);
	/*interval*/
	double inter = 0;
	if (r1.r.x < r2.r.x)
	{
		double dx = r2.r.x - r1.r.x - r1.r.width;
		inter = double(abs(dx)) / max(r1.r.width, r2.r.width);
		if (dx<0 && inter>0.2) inter = 100;
	}
	else
	{
		double dx = r1.r.x - r2.r.x - r2.r.width;
		inter = double(abs(dx)) / max(r1.r.width, r2.r.width);
		if (dx<0 && inter>0.2) inter = 100;
	}
	feature.push_back(inter);

	/*width and height difference*/
	double ws = double(abs(r1.r.width - r2.r.width)) / max(r1.r.width, r2.r.width);
	double hs = double(abs(r1.r.height - r2.r.height)) / max(r1.r.height, r2.r.height);
	feature.push_back(ws), feature.push_back(hs);

	/*top and bottom alignments*/
	//double up = abs(r1.r.y - r2.r.y);
	//double k = abs(r1.r.x + r1.r.width / 2 - r2.r.x - r2.r.width / 2);
	//double t = atan2(up, k);
	//feature.push_back(t);
	//double down = abs(r1.r.y + r1.r.height - r2.r.y - r2.r.height);
	//t = atan2(down, k);
	//feature.push_back(t);

	double up = abs(r1.r.y - r2.r.y);
	double t = up / max(r1.r.height, r2.r.height);
	feature.push_back(t);
	double down = abs(r1.r.y + r1.r.height - r2.r.y - r2.r.height);
	t = down / max(r1.r.height, r2.r.height);
	feature.push_back(t);

	/*center alignments*/
	//	double dx = abs(r1.cen_x - r2.cen_x);
	//double dy = abs(r1.cen_y - r2.cen_y) / max(r1.r.height, r2.r.height);
	//feature.push_back(dy);

	double dist = 0;
	for (int i = 0; i < feature.size(); ++i)
	{
		dist += feature[i] * w[i];
	}
	return dist;
}
double calRegionPairDist(Region r1, Region r2, vector<double> &w,vector<double> &feature)
{
	feature.push_back(1);
	/*interval*/
	double inter = 0;
	if (r1.r.x < r2.r.x)
	{
		inter = double(abs(r2.r.x - r1.r.x - r1.r.width)) / max(r1.r.width, r2.r.width);
	}
	else
	{
		inter = double(abs(r1.r.x - r2.r.x - r2.r.width)) / max(r1.r.width, r2.r.width);
	}
	feature.push_back(inter);

	/*width and height difference*/
	double ws = double(abs(r1.r.width - r2.r.width)) / max(r1.r.width, r2.r.width);
	double hs = double(abs(r1.r.height - r2.r.height)) / max(r1.r.height, r2.r.height);
	feature.push_back(ws), feature.push_back(hs);

	/*top and bottom alignments*/
	//double up = abs(r1.r.y - r2.r.y);
	//double k = abs(r1.r.x + r1.r.width / 2 - r2.r.x - r2.r.width / 2);
	//double t = atan2(up, k);
	//feature.push_back(t);
	//double down = abs(r1.r.y + r1.r.height - r2.r.y - r2.r.height);
	//t = atan2(down, k);
	//feature.push_back(t);

	double up = abs(r1.r.y - r2.r.y);
	double t = up / max(r1.r.height, r2.r.height);
	feature.push_back(t);
	double down = abs(r1.r.y + r1.r.height - r2.r.y - r2.r.height);
	t = down / max(r1.r.height, r2.r.height);
	feature.push_back(t);

	/*center alignments*/
//	double dx = abs(r1.cen_x - r2.cen_x);
	//double dy = abs(r1.cen_y - r2.cen_y)/max(r1.r.height,r2.r.height);
	//feature.push_back(dy);

	double dist = 0;
	for (int i = 0; i < feature.size(); ++i)
	{
		dist += feature[i] * w[i];
	}
	return dist;
}

double calStringPairDist(Str_Chinese s1, Str_Chinese s2, vector<double> &w)
{
	double strDist = 100000000000;
	if (s1.str.empty() || s2.str.empty()) return strDist;
	for (int i = 0; i < s1.str.size(); ++i)
	{
		for (int j = 0; j < s2.str.size(); ++j)
		{
			double dist = calRegionPairDist(s1.str[i], s2.str[j],w);
			strDist= min(strDist, dist);
		}
	}
	return strDist;
}
double calStringPairDist(Str_Chinese s1, Str_Chinese s2, vector<double> &w,int &m,int &n)
{
	double strDist = 100000000000;
	if (s1.str.empty() || s2.str.empty()) return strDist;

	for (int i = 0; i < s1.str.size(); ++i)
	{
		for (int j = 0; j < s2.str.size(); ++j)
		{
			double dist = calRegionPairDist(s1.str[i], s2.str[j], w);
			if (dist < strDist)
			{
				strDist = dist;
				m = i;
				n = j;
			}
		}
	}
	return strDist;
}

void sampleSeletionPanel(Panel &panel, vector<double> &w,vector<double> &M_dist, vector<double> &C_dist)
{
	if (panel.strs.size()<1) return;

	/******************for M samples****************/
	for (int i = 0; i < panel.strs.size(); ++i)
	{
		if (panel.strs[i].str.size() < 2) continue;

		double maxDist = -10000000;
		for (int j = 0; j < panel.strs[i].str.size(); ++j)
		{
			double minDist = 1000000;
			for (int k = 0; k < panel.strs[i].str.size(); ++k)
			{
				if (j == k) continue;
				double dist = calRegionPairDist(panel.strs[i].str[j], panel.strs[i].str[k], w);
				minDist = min(minDist, dist);
			}
			maxDist = max(maxDist, minDist);
		}
		M_dist.push_back(maxDist);
	}
	/******************for C samples****************/
	if (panel.strs.size()<2) return;

	//vector<vector<int>> visited(panel.strs.size(), vector<int>(panel.strs.size(), 0));
	for (int i = 0; i < panel.strs.size(); ++i)
	{
		if (panel.strs[i].str.empty()) continue;
		int valid = 0;
		double minDist = 100000000000;
		int pp = 0;
		for (int j=0; j < panel.strs.size(); ++j)
		{
			if (i == j || panel.strs[j].str.empty()) continue;
			valid = 1;
			double strDist = calStringPairDist(panel.strs[i], panel.strs[j],w);
			if (strDist < minDist)
			{
				minDist = strDist;
				pp = j;
			}
		}
		if (valid)
		{
			C_dist.push_back(minDist);
			//visited[i][pp] = visited[pp][i] = 1;
		}
	}
}
void sampleSeletionPanel(Panel &panel, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist,vector<pair<Region,Region>> &M,vector<pair<Region,Region>> &C)
{
	if (panel.strs.size()<1) return;
	/******************for M samples****************/
	for (int i = 0; i < panel.strs.size(); ++i)
	{
		if (panel.strs[i].str.size() < 2) continue;

		double maxDist = -10000000;
		pair<int, int> tt;

		for (int j = 0; j < panel.strs[i].str.size(); ++j)
		{
			double minDist = 1000000;
			int m = 0, n = 0;
			for (int k = 0; k < panel.strs[i].str.size(); ++k)
			{
				if (j == k) continue;
				double dist = calRegionPairDist(panel.strs[i].str[j], panel.strs[i].str[k], w);
				if (dist < minDist)
				{
					minDist = dist;
					m = j;
					n = k;
				}
			}
			if (minDist > maxDist)
			{
				maxDist = minDist;
				tt.first = m;
				tt.second = n;
			}
		}
		M_dist.push_back(maxDist);
		M.push_back(make_pair(panel.strs[i].str[tt.first], panel.strs[i].str[tt.second]));
	}
	/******************for C samples****************/
	if (panel.strs.size()<2) return;
	//vector<vector<int>> visited(panel.strs.size(), vector<int>(panel.strs.size(), 0));

	for (int i = 0; i < panel.strs.size(); ++i)
	{
		double minDist = 100000000000;
		int m = 0, n = 0;
		pair<int, int> tt;
		int pp = 0;
		if (panel.strs[i].str.empty()) continue;

		int valid = 0;
		for (int j = 0; j < panel.strs.size(); ++j)
		{
			if (i == j || panel.strs[j].str.empty()) continue;
			valid = 1;
			double strDist = calStringPairDist(panel.strs[i], panel.strs[j], w,m,n);
			if (strDist < minDist)
			{
				minDist = strDist;
				tt.first = m;
				tt.second = n;
				pp = j;
			}
		}
		if (valid)
		{
			C_dist.push_back(minDist);
			C.push_back(make_pair(panel.strs[i].str[tt.first], panel.strs[pp].str[tt.second]));
			//visited[i][pp] = visited[pp][i] = 1;
		}
	}
}
void sampleSeletionPanel(Panel &panel, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature)
{
	if (panel.strs.size()<1) return;
	/******************for M samples****************/
	for (int i = 0; i < panel.strs.size(); ++i)
	{
		if (panel.strs[i].str.size() < 2) continue;

		vector<double> feature;
		double maxDist = INT_MIN;
	
		for (int j = 0; j < panel.strs[i].str.size(); ++j)
		{
			double minDist = INT_MAX;
			int m = 0, n = 0;
			vector<double> min_feature;
			for (int k = 0; k < panel.strs[i].str.size(); ++k)
			{
				if (j == k) continue;
				vector<double> temp_feature;
				double dist = calRegionPairDist(panel.strs[i].str[j], panel.strs[i].str[k], w,temp_feature);
				if (dist < minDist)
				{
					minDist = dist;
					min_feature.assign(temp_feature.begin(), temp_feature.end());
				}
			}
			if (minDist > maxDist)
			{
				maxDist = minDist;
				feature.assign(min_feature.begin(), min_feature.end());
			}
		}
		M_dist.push_back(maxDist);
		M_feature.push_back(feature);
	}
	/******************for C samples****************/
	if (panel.strs.size()<2) return;

	for (int i = 0; i < panel.strs.size(); ++i)
	{
		double minDist = INT_MAX;
		int m = 0, n = 0;
		pair<int, int> tt;
		int pp = 0;
		if (panel.strs[i].str.empty()) continue;

		int valid = 0;
		for (int j = 0; j < panel.strs.size(); ++j)
		{
			if (i == j || panel.strs[j].str.empty()) continue;
			valid = 1;
			double strDist = calStringPairDist(panel.strs[i], panel.strs[j], w, m, n);
			if (strDist < minDist)
			{
				minDist = strDist;
				tt.first = m;
				tt.second = n;
				pp = j;
			}
		}
		if (valid)
		{
			C_dist.push_back(minDist);
			vector<double> temp_feature;
			calRegionPairDist(panel.strs[i].str[tt.first], panel.strs[pp].str[tt.second], w, temp_feature);
			C_feature.push_back(temp_feature);
		}
	}
}

void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist)
{
	for (int i = 0; i < panels.size(); ++i)
	{
		sampleSeletionPanel(panels[i], w, M_dist, C_dist);
	}
}
void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<pair<Region, Region>> &M, vector<pair<Region, Region>> &C)
{
	for (int i = 0; i < panels.size(); ++i)
	{
		sampleSeletionPanel(panels[i], w, M_dist, C_dist,M,C);
	}
}
void sampleSeletion(vector<Panel> panels, vector<double> &w, vector<double> &M_dist, vector<double> &C_dist, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature)
{
	for (int i = 0; i < panels.size(); ++i)
	{
		sampleSeletionPanel(panels[i], w, M_dist, C_dist, M_feature, C_feature);
	}
}

double hypothesis(vector<double> &feature, vector<double>&w){

	double sum = 0.0;
	for (int i = 0; i < feature.size(); i++)
	{
		sum += w[i] * feature[i];
	}
	return 1 / (1 + exp(-sum));
}

double cost_fuction(vector<vector<double>> &M_feature, vector<vector<double>> &C_feature, vector<double> &w){

	double sum = 0.0;
	for (int i = 0; i < C_feature.size(); i++)
	{
		sum += -log(hypothesis(C_feature[i], w));
	}
	for (int i = 0; i < M_feature.size(); i++)
	{
		sum += -log(1 - hypothesis(M_feature[i], w));
	}
	double train_num = M_feature.size() + C_feature.size();
	return sum / train_num;
}

void updateW(double apha, vector<double> &w, vector<vector<double>> &M_feature, vector<vector<double>> &C_feature)
{
	double m = M_feature.size() + C_feature.size();
	for (int i = 0; i < w.size(); ++i)
	{
		double sum_M = 0;
		for (int j = 0; j < M_feature.size(); ++j)
		{
			double h = hypothesis(M_feature[j], w);
			sum_M += h*M_feature[j][i];
		}
		double sum_C = 0;
		for (int k = 0; k < C_feature.size(); ++k)
		{
			double h = hypothesis(C_feature[k], w);
			sum_C += (h - 1)*C_feature[k][i];
		}
		double tmp = w[i] - apha*(sum_M + sum_C) / m;
		if (i > 0)
		{
			w[i] = tmp > 0 ? tmp : w[i];
		}
		else w[i] = tmp;
	}
}

void drawRects(Mat &src, vector<Region> &regionBoxes,Scalar color)
{
	for (int i = 0; i < regionBoxes.size(); ++i)
	{
		rectangle(src, regionBoxes[i].r, color);
	}
}

double iou(Region &r1,Region &r2)
{
	int in_w = min(r1.r.x+r1.r.width, r2.r.x + r2.r.width) - max(r1.r.x, r2.r.x);
	int in_h = min(r1.r.y+r1.r.height, r2.r.y + r2.r.height) - max(r1.r.y, r2.r.y);

	double in_area = (in_w <= 0 || in_h <= 0) ? 0.f : in_w*in_h;
	double un_area = r1.r.area() + r2.r.area() - in_area;

	return	in_area / un_area;
}

void rect_extend(vector<Region> &regions, double r, int width, int height)
{
	for (int i = 0; i !=regions.size(); ++i)
	{
		Rect pr = regions[i].r;
		int dx = cvRound(pr.width*(r - 1) / 2.0);
		int dy = cvRound(pr.height*(r - 1) / 2.0);

		int w = cvRound(pr.width*r);
		int h = cvRound(pr.height*r);

		int x = pr.x - dx;
		int y = pr.y - dy;

		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x + w>width - 1) w = width - 1 - x;
		if (y + h>height - 1) h = height - 1 - y;
			
		pr.x = x;
		pr.y = y;
		pr.width = w;
		pr.height = h;

		regions[i].r = pr;
	}
}

void rect_extend(Rect &rec, double r, int width, int height)
{
	
	Rect pr =rec;
	int dx = cvRound(pr.width*(r - 1) / 2.0);
	int dy = cvRound(pr.height*(r - 1) / 2.0);

	int w = cvRound(pr.width*r);
	int h = cvRound(pr.height*r);

	int x = pr.x - dx;
	int y = pr.y - dy;

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x + w>width - 1) w = width - 1 - x;
	if (y + h>height - 1) h = height - 1 - y;

	pr.x = x;
	pr.y = y;
	pr.width = w;
	pr.height = h;

	rec = pr;
	
}



bool contain(Region &r1,Region &r2,double th)
{

	int in_w = min(r1.r.x + r1.r.width, r2.r.x + r2.r.width) - max(r1.r.x, r2.r.x);
	int in_h = min(r1.r.y + r1.r.height, r2.r.y + r2.r.height) - max(r1.r.y, r2.r.y);

	double in_area = (in_w <= 0 || in_h <= 0) ? 0.f : in_w*in_h;

	//return in_area / r1.r.area() > th || in_area / r2.r.area() > th;
	return in_area / r1.r.area() > th;

}

bool containTwo(Region &r1, Region &r2, double th)
{

	int in_w = min(r1.r.x + r1.r.width, r2.r.x + r2.r.width) - max(r1.r.x, r2.r.x);
	int in_h = min(r1.r.y + r1.r.height, r2.r.y + r2.r.height) - max(r1.r.y, r2.r.y);

	double in_area = (in_w <= 0 || in_h <= 0) ? 0.f : in_w*in_h;

	return in_area / r1.r.area() > th || in_area / r2.r.area() > th;

}

void showPanelGth(Mat &panel_image, Panel &panel)
{
	for (auto s:panel.strs)
	{
		s.box.x -= panel.box.x;
		s.box.y-=panel.box.y;

		rectangle(panel_image, s.box, Scalar(0, 255, 255), 1);
		for (auto re : s.str)
		{
			re.r.x -= panel.box.x;
			re.r.y -= panel.box.y;

			rectangle(panel_image, re.r, Scalar(0, 0, 255), 1);
		}
			
	}
}


float classify_one_svm(Region &cc, CvSVM &svm)
{
	vector<double> hist;
	calRegionFeature(cc, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat, true));
}
void filter_stage_1(vector<Region> &regions, CvSVM &svm, double s_th)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		float s = -classify_one_svm(r, svm);
		r.score = s;
		if (s > s_th) temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}
void filter_stage_1(vector<Region> &re_left, CvSVM &svm, double s_th, vector<Region> &re_out)
{
	vector<Region> t1,t2;
	for (auto r : re_left)
	{
		float s = -classify_one_svm(r, svm);
		r.score = s;
		if (s > s_th) t1.push_back(r);
		else re_out.push_back(r);
	}
	re_left.clear();
	re_left = t1;
}

float classify_one_boost(Region &cc, CvBoost &boost)
{
	vector<double> hist;
	calRegionFeature(cc, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(boost.predict(hist_mat, Mat(), Range::all(), false, true));
}
void filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th)
{
	vector<Region> t;
	for (auto r : re_left)
	{
		float s = classify_one_boost(r, boost);
		r.score = s;
		if (s > s_th) t.push_back(r);
	}
	re_left.clear();
	re_left = t;
}

void filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th, vector<Region> &re_out)
{
	vector<Region> t1, t2;
	for (auto r : re_left)
	{
		float s = classify_one_boost(r, boost);
		r.score += s;
		if (s > s_th) t1.push_back(r);
		else re_out.push_back(r);
	}
	re_left.clear();
	re_left = t1;
}

double hog_classify_one_boost(Mat &img, CvBoost &boost)
{
	/*vector<double> edgeF;
	calEdge(img, edgeF);*/
	vector<float> hist;
	calculateHOG(img, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(boost.predict(hist_mat, Mat(), Range::all(), false, true));
}

void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double s_th)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = hog_classify_one_boost(img, boost);
		r.score += s;
		if (s > s_th) temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}

void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double s_th, vector<Region> &other)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = hog_classify_one_boost(img, boost);
		r.score += s;
		if (s > s_th) temp.push_back(r);
		else other.push_back(r);
	}
	regions.clear();
	regions = temp;
}

void filter_stage_2_boost(vector<string> &img_path, CvBoost &boost, double s_th, vector<string> &other)
{
	vector<string> temp;
	for (auto p : img_path)
	{
		Mat img = imread(p);
		float s = hog_classify_one_boost(img, boost);
		if (s > s_th) temp.push_back(p);
		else other.push_back(p);
	}
	img_path.clear();
	img_path = temp;
}


void filter_stage_2_boost(vector<Region> &regions, CvBoost &boost, double low_th, double high_th, vector<Region> &re_text, vector<Region> &re_other)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = hog_classify_one_boost(img, boost);
		r.score += s;
		if (s > high_th) re_text.push_back(r);
		if (s<low_th) re_other.push_back(r);
		else temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}

void calculateHOG(Mat &src, vector<float> &descriptors)
{
	int nbins = 8;
	Size blockSize(8, 8);
	Size blockStride(4, 4);
	Size cellSize(4, 4);
	Size NormSize(24, 24);

	bool gammaCorrection = false;
	double winSigma = 4;
	double L2HysThreshold = 0.2;

	int featureDim = nbins * (blockSize.width / cellSize.width*blockSize.height / cellSize.height) *
		((NormSize.width - blockSize.width) / blockStride.width + 1) * ((NormSize.height - blockSize.height) / blockStride.height + 1);

	HOGDescriptor *hog = new cv::HOGDescriptor(NormSize, blockSize, blockStride, cellSize, nbins, 1, winSigma,
		HOGDescriptor::L2Hys, L2HysThreshold, gammaCorrection);

	if(src.channels()!=1) cvtColor(src, src, CV_BGR2GRAY);

	Mat imgNorm(NormSize, CV_8UC1);
	resize(src, imgNorm, NormSize);

	hog->compute(imgNorm, descriptors, Size(8, 8), Size(0, 0));
}
void calculateColorHOG(Mat &src, vector<float> &descriptors)
{
	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);
	vector<Mat> hsv;
	split(src_hsv, hsv);
	Mat h = hsv.at(0);
	Mat s = hsv.at(1);
	Mat v = hsv.at(2);

	vector<float> h_d, s_d, v_d;
	calculateHOG(h, h_d);
	calculateHOG(s, s_d);
	calculateHOG(v, v_d);

	for (int i = 0; i < 25; ++i)
	{
		vector<float> temp;
		for (int j = 0; j < 32; ++j)
		{
			temp.push_back(h_d[i * 32 + j]);
			temp.push_back(s_d[i * 32 + j]);
			temp.push_back(v_d[i * 32 + j]);
		}
		//normalize(temp, temp, 1, 0, NORM_L2);
		descriptors.insert(descriptors.end(), temp.begin(), temp.end());
	}
}
double hog_classify_one_svm(Mat &img, CvSVM &svm)
{
	/*vector<double> edgeF;
	calEdge(img, edgeF);*/
	vector<float> hist;
	calculateHOG(img, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat, true));
}
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double s_th)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = -hog_classify_one_svm(img, svm);
		r.score += s;
		if (s > s_th) temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double s_th, vector<Region> &re_other)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		float s = -hog_classify_one_svm(r.image, svm);
		r.score += s;
		if (s > s_th) temp.push_back(r);
		else re_other.push_back(r);
	}
	regions.clear();
	regions = temp;
}
void filter_stage_2(vector<Region> &regions, CvSVM &svm, double low_th, double high_th, vector<Region> &re_text,vector<Region> &re_other)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = -hog_classify_one_svm(img, svm);
		r.score += s;
		if (s > high_th) re_text.push_back(r);
		if (s<low_th) re_other.push_back(r);
		else temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}


double hog_color_classify_one_svm(Mat &img, CvSVM &svm)
{
	/*vector<double> edgeF;
	calEdge(img, edgeF);*/
	vector<float> hist;
	calculateColorHOG(img, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat, true));
}
void filter_stage_hogC(vector<Region> &regions, CvSVM &svm, double s_th)
{
	vector<Region> temp;
	for (auto r : regions)
	{
		Mat img = r.image.clone();
		float s = -hog_color_classify_one_svm(img, svm);
		r.score += s;
		if (s > s_th) temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}

double lbp_classify_one_boost(Mat &img, CvBoost &boost, LBP_Option lbp_opt)
{
	vector<double> hist;
	if (img.channels() != 1) cvtColor(img, img, CV_BGR2GRAY);

	//calLBPMultiScale(img, hist, lbp_opt);
	//calLBPHFSM_MultiScale(img, hist, lbp_opt); 
	//calCLBP_MultiScale(img, hist, lbp_opt);
	calLBPHFSMC_MultiScale(img, hist, lbp_opt);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(boost.predict(hist_mat, Mat(), Range::all(), false, true));
}

void filter_stage_2_clbp(vector<Region> &regions, CvBoost &boost, double s_th)
{
	LBP_Option lbp_opt;
	lbp_opt.type = LBP_MAPPING_RIU2;
	lbp_opt.radius.push_back(1); lbp_opt.radius.push_back(3); lbp_opt.radius.push_back(5);
	lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8);
	lbp_opt.normSize = Size(24, 24);
	vector<Region> temp;
	for (auto r : regions)
	{
		float s = lbp_classify_one_boost(r.image, boost,lbp_opt);
		if (s > s_th) temp.push_back(r);
	}
	regions.clear();
	regions = temp;
}

void filter_stage_2_clbp(vector<Region> &regions, CvBoost &boost, double low_th, double high_th, vector<Region> &re_text, vector<Region> &re_other)
{
	LBP_Option lbp_opt;
	lbp_opt.type = LBP_MAPPING_HF;
	lbp_opt.radius.push_back(1); lbp_opt.radius.push_back(3); lbp_opt.radius.push_back(5);
	lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8);
	lbp_opt.normSize = Size(24, 24);
	vector<Region> temp;
	for (auto r : regions)
	{
		float s = lbp_classify_one_boost(r.image, boost, lbp_opt);
		if (s > high_th) re_text.push_back(r);
		if (s > low_th) temp.push_back(r);
		else re_other.push_back(r);
	}
	regions.clear();
	regions = temp;
}

void recallMatch(vector<Region> &regions, Panel &p, int &c_match,int &gth_c)
{
	for (auto s : p.strs)
	{
		gth_c += s.str.size();
	}
	vector<bool> visited(regions.size(), false);
	for (auto s : p.strs)
	{
		for (auto gTh_region : s.str)
		{
			gTh_region.r.x -= p.box.x;
			gTh_region.r.y -= p.box.y;
			for (int i = 0; i != regions.size();++i)
			{
				if (visited[i]) continue;
				if (iou(regions[i], gTh_region) > 0.5)
				{
					c_match++;
					visited[i] = true;
					break;
				}
			}
		}
	}
	c_match = min(c_match, gth_c);
}

bool com_area(Region &r1, Region &r2)
{
	return r1.r.area() > r2.r.area();
}
bool com_s(Region &r1, Region &r2)
{
	return r1.score > r2.score;
}
void filerContain(vector<Region> &regions,double th)
{
	sort(regions.begin(), regions.end(),com_area);
	vector<bool> pass(regions.size(), false);
	for (int i = 0; i != regions.size(); ++i)
	{
		if (pass[i]) continue;
		for (int j = i + 1; j != regions.size(); ++j)
		{
			if(pass[j]) continue;
			if (contain(regions[j], regions[i],th))
			{
				pass[j] = true;
			}
		}
	}

	vector<Region> temp;
	for (int i = 0; i != regions.size(); ++i)
	{
		if (!pass[i]) temp.push_back(regions[i]);
	}
	regions = temp;
}

void filerContainByScore(vector<Region> &regions,double th)
{
	sort(regions.begin(), regions.end(), com_s);
	vector<bool> pass(regions.size(), false);
	for (int i = 0; i != regions.size(); ++i)
	{
		if (pass[i]) continue;
		for (int j = i + 1; j != regions.size(); ++j)
		{
			if (pass[j]) continue;
			if (containTwo(regions[j], regions[i], th))
			{
				pass[j] = true;
			}
		}
	}

	vector<Region> temp;
	for (int i = 0; i != regions.size(); ++i)
	{
		if (!pass[i]) temp.push_back(regions[i]);
	}
	regions = temp;
}

void filerContainByAreaV(vector<Region> &regions, double th)
{
	sort(regions.begin(), regions.end(), [](const Region &a, const Region &b){return a.var < b.var; });
	vector<bool> pass(regions.size(), false);
	for (int i = 0; i != regions.size(); ++i)
	{
		if (pass[i]) continue;
		for (int j = i + 1; j != regions.size(); ++j)
		{
			if (pass[j]) continue;
			if (contain(regions[i], regions[j], th))
			{
				pass[j] = true;
			}
		}
	}

	vector<Region> temp;
	for (int i = 0; i != regions.size(); ++i)
	{
		if (!pass[i]) temp.push_back(regions[i]);
	}
	regions = temp;
}

void gth_match(vector<Region> &regions,Panel &p, vector<Region> &pos, vector<Region> &neg,Mat &src)
{
	/*prepare for positive */
	vector<bool> visited(regions.size(), false);
	for (auto s : p.strs)
	{
		for (auto gTh_region : s.str)
		{
			Rect r_ex(gTh_region.r);
			rect_extend(r_ex, 1.2, src.cols, src.rows);
			gTh_region.image = Mat(src, r_ex).clone();

			gTh_region.r.x -= p.box.x;
			gTh_region.r.y -= p.box.y;
			gTh_region.p_h = p.box.height;
			gTh_region.p_w = p.box.width;

			int find_pos = 0;
			for (int i = 0; i != regions.size(); ++i)
			{
				if (visited[i]) continue;
				Region region = regions[i];
				if (iou(region, gTh_region) > 0.5)
				{
					if (region.r.area() < gTh_region.r.area()) pos.push_back(region);
					else pos.push_back(gTh_region);

					visited[i] = true;
					find_pos = 1;
					break;
				}
			}
		    if (find_pos == 0) pos.push_back(gTh_region);
		}
	}

	for (int i = 0; i != regions.size(); ++i)
	{
		if (visited[i] == true) continue;
		int is_not_neg = 0;
		Region region = regions[i];
		for (auto s : p.strs)
		{
			if (is_not_neg) break;
			for (auto gTh_region : s.str)
			{
				if (is_not_neg) break;
				gTh_region.r.x -= p.box.x;
				gTh_region.r.y -= p.box.y;
				if (iou(region, gTh_region) < 1 && contain(region, gTh_region, 0.5))
				{
					is_not_neg = 1;
				}
			}
		}
		if (!is_not_neg) neg.push_back(region);
	}
}

void calSignFeature(Region &cc, vector<double> &feature)
{
	double area = cc.p_h*cc.p_w;

	double w_h_r = (double)cc.r.width / cc.r.height;
	double area_r = (double)cc.r.area() / area;

	double w_r = (double)cc.r.width / cc.p_w;
	double h_r = (double)cc.r.height / cc.p_h;

	feature.push_back(w_h_r);

	feature.push_back(area_r);
	feature.push_back(w_r);
	feature.push_back(h_r);

	feature.push_back(cc.var);

	feature.push_back(cc.r.x);
	feature.push_back(cc.r.y);
	feature.push_back(cc.p_w - cc.r.x - cc.r.width);
	feature.push_back(cc.p_h - cc.r.y - cc.r.height);

	/*double e = energy(cc);
	feature.push_back(e);*/
}


float sign_classify_one_boost(Region &cc, CvBoost &boost)
{
	vector<double> hist;
	calSignFeature(cc, hist);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(boost.predict(hist_mat, Mat(), Range::all(), false, true));
}
void sign_filter_stage_1_boost(vector<Region> &re_left, CvBoost &boost, double s_th)
{
	vector<Region> t;
	for (auto r : re_left)
	{
		float s = sign_classify_one_boost(r, boost);
		r.score = s;
		if (s > s_th) t.push_back(r);
	}
	re_left.clear();
	re_left = t;
}


void calculateHOG_binary(Mat &src, vector<float> &descriptors)
{
	int nbins = 8;
	Size blockSize(8, 8);
	Size blockStride(4, 4);
	Size cellSize(4, 4);
	Size NormSize(24, 24);

	bool gammaCorrection = false;
	double winSigma = 4;
	double L2HysThreshold = 0.2;

	int featureDim = nbins * (blockSize.width / cellSize.width*blockSize.height / cellSize.height) *
		((NormSize.width - blockSize.width) / blockStride.width + 1) * ((NormSize.height - blockSize.height) / blockStride.height + 1);

	HOGDescriptor *hog = new cv::HOGDescriptor(NormSize, blockSize, blockStride, cellSize, nbins, 1, winSigma,
		HOGDescriptor::L2Hys, L2HysThreshold, gammaCorrection);

	if (src.channels() != 1) cvtColor(src, src, CV_BGR2GRAY);

	Mat imgNorm(NormSize, CV_8UC1);
	resize(src, imgNorm, NormSize);

	Mat bin;
	threshold(imgNorm, bin, 100, 255, THRESH_OTSU);
	hog->compute(bin, descriptors, Size(8, 8), Size(0, 0));
}

void filterTextlines(vector<Str_Chinese> &texts, CvBoost &boost, Mat &ftl)
{
	vector<Str_Chinese> tmp;
	for (auto &t : texts)
	{
		int n = t.str.size();
		if (n > 1)
		{
			tmp.push_back(t);
			continue;
		}

		Mat show = ftl.clone();
		rectangle(ftl, t.box, Scalar(0, 255, 255), 2);
		filter_stage_2_boost(t.str, boost, 0);
		if (t.str.size() == n) tmp.push_back(t);
	}
	texts = tmp;
}