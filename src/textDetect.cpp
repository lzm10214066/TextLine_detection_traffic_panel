#include "textDetect.h"

struct Pair_Dist
{
	int i, j;
	double dist;
	double color_dist;
	Pair_Dist(int ii, int jj)
	{
		i = ii;
		j = jj;
		dist = 0;
	}
};

bool comp_pair(Pair_Dist p1, Pair_Dist p2)
{
	return p1.dist < p2.dist;
}

//void groupToStr(vector<Region> &regions, Str_Chinese &str)
//{
//	double h_sum = 0;
//	double y_sum = 0;
//	int n = regions.size();
//	if (n == 0) return;
//
//	int x_start = regions[0].r.x, x_end = regions[0].r.x + regions[0].r.width-1;
//
//	for (int i = 0; i < regions.size(); ++i)
//	{
//		h_sum += regions[i].r.height;
//		y_sum += regions[i].r.y;
//		x_start = min(x_start, regions[i].r.x);
//		x_end = max(x_end, regions[i].r.x + regions[i].r.width - 1);
//		str.str.push_back(regions[i]);
//	}
//
//	int h_ave = cvRound(h_sum / n);
//	int y_ave = cvRound(y_sum / n);
//	str.box.x = x_start;
//	str.box.y = y_ave;
//	str.box.width = x_end - x_start + 1;
//	str.box.height = h_ave;
//}

void groupToStr(vector<Region> &regions, Str_Chinese &str)
{
	int n = regions.size();
	if (n == 0) return;

	int x_start = regions[0].r.x, x_end = regions[0].r.x + regions[0].r.width - 1;
	int y_start = regions[0].r.y, y_end = regions[0].r.y + regions[0].r.height - 1;

	for (int i = 0; i < regions.size(); ++i)
	{
		x_start = min(x_start, regions[i].r.x);
		x_end = max(x_end, regions[i].r.x + regions[i].r.width - 1);
		y_start = min(y_start, regions[i].r.y);
		y_end = max(y_end, regions[i].r.y + regions[i].r.height - 1);
		str.str.push_back(regions[i]);
	}

	str.box.x = x_start;
	str.box.y = y_start;
	str.box.width = x_end - x_start + 1;
	str.box.height = y_end - y_start + 1;

	/*for (int i = 1; i < regions.size(); ++i)
	{
		x_start = min(x_start, regions[i].r.x);
		x_end = max(x_end, regions[i].r.x + regions[i].r.width - 1);
		y_start += regions[i].r.y;
		y_end += regions[i].r.y + regions[i].r.height - 1;
		str.str.push_back(regions[i]);
	}

	str.box.x = x_start;
	str.box.y = y_start/(regions.size());
	str.box.width = x_end - x_start + 1;
	str.box.height = (y_end - y_start)/(regions.size()) + 1;*/
}

void textDetect(vector<Region> &regions, vector<Str_Chinese> &texts, vector<double> &w)
{
	vector<Pair_Dist> pair_dist;
	for (int i = 0; i < regions.size(); ++i) { regions[i].label = i; }

	for (int i = 0; i < regions.size(); ++i)
	{
		for (int j = i + 1; j < regions.size(); ++j)
		{
			Pair_Dist pd(i, j);
			pd.dist = calRegionPairDist(regions[i], regions[j], w);
			if (pd.dist <= 0)
			{
				pair_dist.push_back(pd);
			}
		}
	}

	sort(pair_dist.begin(), pair_dist.end(), comp_pair);
	for (int k = 0; k < pair_dist.size(); ++k)
	{
		int i = pair_dist[k].i;
		int j = pair_dist[k].j;

		if (regions[i].label != regions[j].label)
		{
			int t = regions[j].label;
			for (int m = 0; m < regions.size(); ++m)
			{
				if (regions[m].label == t)
				{
					regions[m].label = regions[i].label;
				}
			}
		}
	}

	vector<vector<Region>> groups(regions.size());
	groups.resize(regions.size());
	for (int i = 0; i < regions.size(); ++i)
	{
		groups[regions[i].label].push_back(regions[i]);
	}

	for (vector<vector<Region>> ::iterator it = groups.begin(); it != groups.end();++it)
	{
		if (it->empty()) continue;
		Str_Chinese str;
		groupToStr(*it, str);
		texts.push_back(str);
	}
}

void textDetect(vector<Region> &regions, vector<vector<Region>> &groups, vector<double> &w,Mat &show)
{
	vector<Pair_Dist> pair_dist;
	for (int i = 0; i < regions.size(); ++i) { regions[i].label = i; }

	for (int i = 0; i < regions.size(); ++i)
	{
		for (int j = i + 1; j < regions.size(); ++j)
		{
			Pair_Dist pd(i, j);
			pd.dist = calRegionPairDist(regions[i], regions[j], w);
			if (pd.dist <= 0)
			{
				pair_dist.push_back(pd);
			}
		}
	}

	sort(pair_dist.begin(), pair_dist.end(), comp_pair);
	for (int k = 0; k < pair_dist.size(); ++k)
	{
		int i = pair_dist[k].i;
		int j = pair_dist[k].j;

		//rectangle(show, regions[i].r, Scalar(0, 0, 255));
		//rectangle(show, regions[j].r, Scalar(0, 0, 255));

		if (regions[i].label != regions[j].label)
		{
			int t = regions[j].label;
			for (int m = 0; m < regions.size(); ++m)
			{
				if (regions[m].label == t)
				{
					regions[m].label = regions[i].label;
				}
			}
		}
	}

	groups.resize(regions.size());
	for (int i = 0; i < regions.size(); ++i)
	{
		groups[regions[i].label].push_back(regions[i]);
	}

	for (vector<vector<Region>> ::iterator it = groups.begin(); it != groups.end();)
	{
		if (it->empty())
		{
			it = groups.erase(it);
		}
		else
		{
			++it;
		}
	}
}
void textDetectStr(vector<Region> &regions, vector<Str_Chinese> &texts, vector<double> &w)
{
	vector<Pair_Dist> pair_dist;
	for (int i = 0; i < regions.size(); ++i) { regions[i].label = i; }

	for (int i = 0; i < regions.size(); ++i)
	{
		for (int j = i + 1; j < regions.size(); ++j)
		{
			Pair_Dist pd(i, j);
			pd.dist = calRegionPairDist(regions[i], regions[j], w);
			if (pd.dist <= 0)
			{
				pair_dist.push_back(pd);
			}
		}
	}

	sort(pair_dist.begin(), pair_dist.end(), comp_pair);
	for (int k = 0; k < pair_dist.size(); ++k)
	{
		int i = pair_dist[k].i;
		int j = pair_dist[k].j;

		if (regions[i].label != regions[j].label)
		{
			int t = regions[j].label;
			for (int m = 0; m < regions.size(); ++m)
			{
				if (regions[m].label == t)
				{
					regions[m].label = regions[i].label;
				}
			}
		}
	}

	vector<vector<Region>> groups(regions.size());
	groups.resize(regions.size());
	for (int i = 0; i < regions.size(); ++i)
	{
		groups[regions[i].label].push_back(regions[i]);
	}

	for (vector<vector<Region>> ::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		if (it->empty()) continue;
		Str_Chinese str;
		groupToStr(*it, str);
		texts.push_back(str);
	}
}

void drawPointImg(vector<Point> &points, Mat &srcGray)
{
	CV_Assert(srcGray.channels() == 1 && srcGray.data);
	for (auto p : points)
	{
		uchar *p_src = srcGray.ptr<uchar>(p.y);
		p_src[p.x] = 255;
	}
}

void regionDetect(Mat &src, vector<Region> &regionBoxes)
{
	Mat src_c = src.clone();

	/*vector<Mat> hsv;
	Mat hsv_img;
	cvtColor(src_c, hsv_img, CV_BGR2HSV);
	split(hsv_img, hsv);
	Mat h = hsv[0];
	Mat s = hsv[1];
	Mat v = hsv[2];*/

	if (src.channels() == 3) cvtColor(src, src, CV_BGR2GRAY);

	double areaPanel = src.rows*src.cols;
	double t = (double)getTickCount();
	MSER ms(3, 10, areaPanel*0.5,
		0.3, 0.0,
		200, 1.01,
		0.003, 3);
	//MSER ms;
	vector<vector<Point>> regions;
	vector<double> val;
	ms(src, regions,val);

	/*Mat show = src.clone();
	cvtColor(show, show, CV_GRAY2BGR);*/

	for (int k = 0; k != regions.size(); ++k)
	{
		Rect roirect = boundingRect(regions[k]);
		double var = val[k];
		double ratio = (double)roirect.width / roirect.height;
		double area = roirect.area();

		//rectangle(show, roirect, Scalar(theRNG().uniform(0, 255), theRNG().uniform(0, 255), theRNG().uniform(0, 255)));

		if (ratio >= 1. / 4 && ratio <= 5)
		{
			if (area >= areaPanel*0.001 && area <= areaPanel*0.2)
			{
				Region r(roirect,var);
				r.p_h = src.rows;
				r.p_w = src.cols;
				r.component_area = regions[k].size();

				/*double fill = r.component_area / area;
				if (fill > 0.2) continue;*/  //对分类没有实际作用

				Rect r_ex(roirect);
				rect_extend(r_ex, 1.2, src.cols, src.rows);
				r.image = Mat(src_c, r_ex).clone();

				Mat mask(src.size(), CV_8UC1,Scalar(0));
				drawPointImg(regions[k], mask);
				r.pointImg = Mat(mask, r_ex).clone();

				regionBoxes.push_back(r);
			}
		}
	}

	supressNMS(regionBoxes, 0.7);
}

void supressNMS(vector<Region> &regionBoxes,double r)
{
	sort(regionBoxes.begin(), regionBoxes.end(), [](const Region &a, const Region &b){return a.var < b.var; });
	//sort(regionBoxes.begin(), regionBoxes.end(), [](const Region &a, const Region &b){return a.component_area/a.r.area() >
		//b.component_area/b.r.area(); });
	vector<int> retain(regionBoxes.size(), 1);
	for (int i = 0; i < regionBoxes.size(); ++i)
	{
		if (!retain[i])
			continue;
		for (int j = i + 1; j < regionBoxes.size(); ++j)
		{
			if (!retain[j])
				continue;
			double ain = overlap(regionBoxes[i].r, regionBoxes[j].r);
			double aun = regionBoxes[i].r.area() + regionBoxes[j].r.area() - ain;
			if ((ain / aun) > r)
			{
				retain[j] = 0;
			}
		}
	}

	vector<Region> temp;
	for (int i = 0; i != regionBoxes.size(); ++i)
	{
		if (retain[i])
		{
			temp.push_back(regionBoxes[i]);
		}

	}
	regionBoxes = temp;
}

double calRegionColorDist(Region r1, Region r2)
{
	double dist = 1000;
	Mat r1_img = r1.image;
	Mat r1_points = r1.pointImg;
	Scalar mean_c_1 = mean(r1_img, r1_points);

	Mat r2_img = r2.image;
	Mat r2_points = r2.pointImg;
	Scalar mean_c_2 = mean(r2_img, r2_points);

	dist = abs(mean_c_1[0] - mean_c_2[0]) + abs(mean_c_1[1] - mean_c_2[1]) + abs(mean_c_1[2] - mean_c_2[2])/3;
	return dist;
}

double calRegionDist(Region &r1, Region &r2)
{
	double dx = r2.cen_x - r1.cen_x;
	double dy = r2.cen_y - r1.cen_y;
	double dist = sqrt(dx*dx + dy*dy);

	double a = overlap(r1.r, r2.r);
	if (a > 0) dist = 1000;

	return dist;
}


int find(vector<Region> &rm_in,int x) {
	if (x != rm_in[x].label) rm_in[x].label = find(rm_in, rm_in[x].label);
	return rm_in[x].label;
}
Rect mergeRect(Rect r1, Rect r2)
{
	int xs = min(r1.x, r2.x);
	int ys = min(r1.y, r2.y);
	int xe = max(r1.x + r1.width, r2.x + r2.width);
	int ye = max(r1.y + r1.height, r2.y + r2.height);
	Rect r(xs, ys, xe - xs, ye - ys);
	return r;
}
void mergeCC(vector<Region> &rm_in, int x, int y,double d_r,int c_th,vector<Region> &groups,Mat &image) {

	Mat show = image.clone();

	if ((x = find(rm_in, x)) == (y = find(rm_in, y))) return;

	rectangle(show, rm_in[x].r, Scalar(0, 0, 255));
	rectangle(show, rm_in[y].r, Scalar(255, 0, 255));
	Mat mask1 = rm_in[x].pointImg;
	Mat mask2 = rm_in[y].pointImg;


	if (rm_in[x].sub_count > c_th || rm_in[y].sub_count > c_th) return;
	if (rm_in[x].sub_count + rm_in[y].sub_count > c_th) return;
	
	if (containTwo(rm_in[x], rm_in[y], 0.8)) return;

	Rect n_r = mergeRect(rm_in[x].r, rm_in[y].r);
	double v = min(rm_in[x].var, rm_in[y].var);
	Region res(n_r,v);

	rectangle(show, n_r, Scalar(0, 255, 255));

	double dx = abs(rm_in[x].cen_x - rm_in[y].cen_x);
	double dy = abs(rm_in[x].cen_y - rm_in[y].cen_y);
	if (dx / n_r.width > d_r && dy / n_r.height > d_r) return;

	Rect r_ex(res.r);
	rect_extend(r_ex, 1.2, image.cols, image.rows);
	res.image = Mat(image, r_ex).clone();           // 总是忘记扩展boundingbox，有一些小错误还是可以避免的，经常复习代码也是有好处的嘛

	res.p_h = image.rows;
	res.p_w = image.cols;

	groups.push_back(res);

	if (rm_in[x].sub_count > rm_in[y].sub_count)
	{
		rm_in[y].label = x;
		rm_in[x].r = n_r;
		rm_in[x].sub_count += rm_in[y].sub_count;
		rm_in[x].var = v;
	}
	else {
		rm_in[x].label = y;
		rm_in[y].r=n_r;
		rm_in[y].sub_count += rm_in[x].sub_count;
		rm_in[y].var = v;
	}
}
void mergeCC(vector<Region> &rm_in, int x, int y, double d_r, int c_th,int area_th,vector<Region> &groups, Mat &image) {

	Mat show = image.clone();

	if ((x = find(rm_in, x)) == (y = find(rm_in, y))) return;
	if (rm_in[x].r.area() > area_th) return;
	if (rm_in[y].r.area() > area_th) return;

	rectangle(show, rm_in[x].r, Scalar(0, 0, 255));
	rectangle(show, rm_in[y].r, Scalar(255, 0, 255));
	Mat mask1 = rm_in[x].pointImg;
	Mat mask2 = rm_in[y].pointImg;


	if (rm_in[x].sub_count > c_th || rm_in[y].sub_count > c_th) return;
	if (rm_in[x].sub_count + rm_in[y].sub_count > c_th) return;

	if (containTwo(rm_in[x], rm_in[y], 0.8)) return;

	Rect n_r = mergeRect(rm_in[x].r, rm_in[y].r);
	if (n_r.area() > area_th*1.5) return;

	double v = min(rm_in[x].var, rm_in[y].var);
	Region res(n_r, v);

	rectangle(show, n_r, Scalar(0, 255, 255));

	double dx = abs(rm_in[x].cen_x - rm_in[y].cen_x);
	double dy = abs(rm_in[x].cen_y - rm_in[y].cen_y);
	if (dx / n_r.width > d_r && dy / n_r.height > d_r) return;

	Rect r_ex(res.r);
	rect_extend(r_ex, 1.2, image.cols, image.rows);
	res.image = Mat(image, r_ex).clone();           // 总是忘记扩展boundingbox，有一些小错误还是可以避免的，经常复习代码也是有好处的嘛

	res.p_h = image.rows;
	res.p_w = image.cols;

	groups.push_back(res);

	if (rm_in[x].sub_count > rm_in[y].sub_count)
	{
		rm_in[y].label = x;
		rm_in[x].r = n_r;
		rm_in[x].sub_count += rm_in[y].sub_count;
		rm_in[x].var = v;
	}
	else {
		rm_in[x].label = y;
		rm_in[y].r = n_r;
		rm_in[y].sub_count += rm_in[x].sub_count;
		rm_in[y].var = v;
	}
}

void CC_Merge(vector<Region> &rm_in, vector<Region> &groups, double d_r, int c_th, Mat &image)
{
	vector<Pair_Dist> pair_dist;
	for (int i = 0; i < rm_in.size(); ++i)  { rm_in[i].label = i; }

	double ratio = sqrt(image.rows*image.rows + image.cols*image.cols);
	for (int i = 0; i < rm_in.size(); ++i)
	{
		for (int j = i + 1; j < rm_in.size(); ++j)
		{
			Pair_Dist pd(i, j);
			double dist = calRegionDist(rm_in[i], rm_in[j]);
			double color_dist = calRegionColorDist(rm_in[i], rm_in[j]);
			pd.dist = dist + color_dist*0.5;
			if (dist / ratio<0.2) pair_dist.push_back(pd);
		}
	}

	sort(pair_dist.begin(), pair_dist.end(), comp_pair);
	for (int k = 0; k < pair_dist.size(); ++k)
	{
		int i = pair_dist[k].i;
		int j = pair_dist[k].j;
		
		Mat show = image.clone();
		rectangle(show, rm_in[i].r, Scalar(0, 0, 255));
		rectangle(show, rm_in[j].r, Scalar(0, 0, 255));

		mergeCC(rm_in,i, j,d_r,c_th,groups,image);
	}
}
void CC_Merge(vector<Region> &rm_in, vector<Region> &groups, double d_r, int c_th,int area_th, Mat &image)
{
	vector<Pair_Dist> pair_dist;
	for (int i = 0; i < rm_in.size(); ++i)  { rm_in[i].label = i; }

	double ratio = sqrt(image.rows*image.rows + image.cols*image.cols);
	for (int i = 0; i < rm_in.size(); ++i)
	{
		for (int j = i + 1; j < rm_in.size(); ++j)
		{
		/*	Pair_Dist pd(i, j);
			pd.dist = calRegionDist(rm_in[i], rm_in[j]);
			if (pd.dist / ratio<0.2) pair_dist.push_back(pd);*/
			Pair_Dist pd(i, j);
			double dist = calRegionDist(rm_in[i], rm_in[j]);
			double color_dist = calRegionColorDist(rm_in[i], rm_in[j]);
			pd.dist = dist + color_dist*0.1;
			if (dist / ratio<0.2) pair_dist.push_back(pd);
		}
	}

	sort(pair_dist.begin(), pair_dist.end(), comp_pair);
	for (int k = 0; k < pair_dist.size(); ++k)
	{
		int i = pair_dist[k].i;
		int j = pair_dist[k].j;

		Mat show = image.clone();
		rectangle(show, rm_in[i].r, Scalar(0, 0, 255));
		rectangle(show, rm_in[j].r, Scalar(0, 0, 255));

		mergeCC(rm_in, i, j, d_r, c_th, area_th,groups, image);
	}
}

bool com_y(Region &r1, Region &r2)
{
	return r1.r.y < r2.r.y;
}
bool com_x(Region &r1, Region &r2)
{
	return r1.r.x < r2.r.x;
}
bool com_y_str(Str_Chinese &s1, Str_Chinese &s2)
{
	return s1.box.y < s2.box.y;
}
bool com_x_str(Str_Chinese &s1, Str_Chinese &s2)
{
	return s1.box.x < s2.box.x;
}


bool isOnLine(double yu,double yd, Region r)
{
	double r_yu = r.r.y;
	double r_yd = r.r.y + r.r.height - 1;

	double ys = max(yu, r_yu);
	double ye = min(yd, r_yd);

	double inLen = ye > ys ? ye - ys+1 : 0;
	double Len = r.r.height;
	if (inLen / Len > 0.6)
		return true;
	return false;
}

double overlapHeight(double yu, double yd, Region r)
{
	double r_yu = r.r.y;
	double r_yd = r.r.y + r.r.height - 1;

	double ys = max(yu, r_yu);
	double ye = min(yd, r_yd);

	double inLen = ye > ys ? ye - ys + 1 : 0;
	double Len = r.r.height;
	return inLen / Len;
}

void findChaAdd(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<vector<Region>> &re_adds, Mat &image)
{
	sort(texts.begin(), texts.end(), com_x_str);
	sort(non_texts.begin(), non_texts.end(), com_y);
	vector<bool> visited(non_texts.size(), false);
	Mat mask(image.size(), CV_8UC1, Scalar(255));
	for (auto t : texts)
	{
		rectangle(mask, t.box, Scalar(0), -1);
	}
	for (auto t : texts)
	{
		double y_u = t.box.y;
		double y_d = t.box.y + t.box.height - 1;
		int x_left = t.box.x;
		int x_right = t.box.x + t.box.width - 1;

		int c_area = t.box.height*t.box.height;
		vector<Region> re_add_left;
		vector<Region> re_add_right;
		Mat show = image.clone();
		rectangle(show, t.box, Scalar(0, 0, 255));

		for (int i = 0; i != non_texts.size(); ++i)
		{
			
			if (visited[i]) continue;
			if (mask.at<uchar>(non_texts[i].cen_y, non_texts[i].r.x) == 0) continue;
			if (non_texts[i].r.area() > c_area) continue;
			if (non_texts[i].r.y >= y_d) continue;
			if (non_texts[i].r.y + non_texts[i].r.height <= y_u) continue;
			double overlap_r = overlapHeight(y_u, y_d, non_texts[i]);

			Mat re_show = image.clone();
			rectangle(show, non_texts[i].r, Scalar(255, 255, 255));

			if (overlap_r>0.6)
			{
				if(overlap_r>0.8) visited[i] = true;
				if (non_texts[i].r.x <= x_left) re_add_left.push_back(non_texts[i]);
				else re_add_right.push_back(non_texts[i]);
				rectangle(show, non_texts[i].r, Scalar(0, 255, 255));
				waitKey(0);
			}
		}

		re_add_left.push_back(t.str[0]);
		re_add_right.push_back(t.str[t.str.size() - 1]);
		if (re_add_left.size()>1)  re_adds.push_back(re_add_left);
		if (re_add_right.size()>1)  re_adds.push_back(re_add_right);
	}
}
void findChaAdd(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<Region_add> &re_adds, Mat &image)
{
	sort(texts.begin(), texts.end(), com_x_str);
	sort(non_texts.begin(), non_texts.end(), com_y);
	vector<bool> visited(non_texts.size(), false);
	Mat mask(image.size(), CV_8UC1, Scalar(255));
	for (auto t : texts)
	{
		rectangle(mask, t.box, Scalar(0), -1);
	}
	for (auto t : texts)
	{
		double y_u = t.box.y;
		double y_d = t.box.y + t.box.height - 1;
		int x_left = t.box.x;
		int x_right = t.box.x + t.box.width - 1;

		int c_area = t.box.height*t.box.height;
		vector<Region> re_add_left;
		vector<Region> re_add_right;
		Region_add temp_left;
		Region_add temp_right;
		Mat show = image.clone();
		rectangle(show, t.box, Scalar(0, 0, 255));

		for (int i = 0; i != non_texts.size(); ++i)
		{

			if (visited[i]) continue;
			if (mask.at<uchar>(non_texts[i].cen_y, non_texts[i].r.x) == 0) continue;
			if (non_texts[i].r.area() > c_area) continue;
			if (non_texts[i].r.y >= y_d) continue;
			if (non_texts[i].r.y + non_texts[i].r.height <= y_u) continue;
			double overlap_r = overlapHeight(y_u, y_d, non_texts[i]);

			Mat re_show = image.clone();
			rectangle(show, non_texts[i].r, Scalar(255, 255, 255));

			if (overlap_r>0.6)
			{
				if (overlap_r>0.8) visited[i] = true;
				if (non_texts[i].r.x <= x_left) re_add_left.push_back(non_texts[i]);
				else re_add_right.push_back(non_texts[i]);
				rectangle(show, non_texts[i].r, Scalar(0, 255, 255));
				waitKey(0);
			}
		}

		if(t.str[0].r.area()<c_area*0.6) re_add_left.push_back(t.str[0]);
		if (t.str[t.str.size() - 1].r.area()<c_area*0.6) re_add_right.push_back(t.str[t.str.size() - 1]);
		
		temp_left.re_add = re_add_left;
		temp_right.re_add = re_add_right;
		temp_left.area_per_character = temp_right.area_per_character = c_area;

		if(temp_left.re_add.size()>1) re_adds.push_back(temp_left);
		if(temp_right.re_add.size()>1) re_adds.push_back(temp_right);

	}
}


void findChaRecall(vector<Str_Chinese> &texts, vector<Region> &non_texts, Mat &image)
{
	sort(texts.begin(), texts.end(), com_x_str);
	sort(non_texts.begin(), non_texts.end(), com_y);
	vector<bool> visited(non_texts.size(), false);

	double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得
	vector<double> w(init_w_3, init_w_3 + 6);

	for (auto &t : texts)
	{
		sort(t.str.begin(), t.str.end(), com_x);
		double y_u = t.box.y;
		double y_d = t.box.y + t.box.height - 1;
		int x_left = t.box.x;
		int x_right = t.box.x + t.box.width - 1;

		int c_area = t.box.height*t.box.height;
	
		Mat show = image.clone();
		rectangle(show, t.box, Scalar(0, 0, 255));

		vector<Region> re_add_left;
		vector<Region> re_add_right;

		for (int i = 0; i != non_texts.size(); ++i)
		{
			Mat re_show = image.clone();
			rectangle(re_show, non_texts[i].r, Scalar(255, 255, 255));

			if (visited[i]) continue;
			if (non_texts[i].r.area() < c_area*0.5 || non_texts[i].r.area() > c_area*1.5) continue;
			if (non_texts[i].r.y >= y_d) continue;
			if (non_texts[i].r.y + non_texts[i].r.height <= y_u) continue;
			if (overlap(non_texts[i].r, t.box) > 0)
			{
				visited[i] = true;
				continue;
			}

			rectangle(re_show, non_texts[i].r, Scalar(0, 255, 255));

			double overlap_r = overlapHeight(y_u, y_d, non_texts[i]);
			if (overlap_r>0.6)
			{
				visited[i] = true;
				if (non_texts[i].r.x <= x_left) re_add_left.push_back(non_texts[i]);
				else re_add_right.push_back(non_texts[i]);
				rectangle(show, non_texts[i].r, Scalar(0, 255, 255));
				waitKey(0);
			}
		}

		sort(re_add_left.begin(), re_add_left.end(), [](const Region &a, const Region &b){return a.r.x > b.r.x; });
		for (auto re : re_add_left)
		{
			double d = calRegionPairDist(re, t.str[0],w);
			if (d < 0)
			{
				t.str.insert(t.str.begin(), re);
				t.box.width = t.box.x + t.box.width - re.r.x;
				t.box.x = re.r.x;
			}
		}

		sort(re_add_right.begin(), re_add_right.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
		for (auto re : re_add_right)
		{
			double d = calRegionPairDist(re, t.str[t.str.size()-1], w);
			if (d < 0)
			{
				t.str.insert(t.str.end(), re);
				t.box.width = re.r.x+re.r.width-t.box.x;
			}
		}
	}
}

void textlineDetection(Mat &panel, vector<Str_Chinese> &texts_res, CvBoost  &boost_stage_1, CvBoost  &boost_stage_2)
{
//	CvBoost  boost_stage_1;
//	string boost_filename_1 = "connected_component_feature_5_1000_05-boost.xml";
////	cout << "Boost file: " << boost_filename_1 << endl;
//	boost_stage_1.load(boost_filename_1.c_str());
//
//	CvBoost  boost_stage_2;
//	string boost_filename_2 = "connected_component_feature_hog_800_ex_fix-boost.xml";
////	cout << "Boost file: " << boost_filename_2 << endl;
//	boost_stage_2.load(boost_filename_2.c_str());
	/////////////////////////////////////////////////////////////////////////////////////////
	// cc_detect

	Mat panel_c = panel.clone();
	vector<Region> regions;
	regionDetect(panel_c, regions);

	///////////////////////////////////////////////////////////////////////////////
	vector<Region> non_texts;
	// stage_1

	filter_stage_1_boost(regions, boost_stage_1, -1, non_texts);

	// stage_2
	vector<Region> re_texts;
	filter_stage_2_boost(regions, boost_stage_2, -1, 0, re_texts, non_texts);

	/////////////////////////////////////////////////////////////////////////////////////////
	//text detection
	double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得
	vector<double> w(init_w_3, init_w_3 + 6);
	vector<Str_Chinese> texts;
	textDetect(re_texts, texts, w);
	for (auto &t : texts)
	{
		sort(t.str.begin(), t.str.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
	}
	////////////////////////////////////////////////////////////////////////////
	Mat show_non_texts = panel.clone();
	vector<Region_add> re_adds;
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
	filerContainByScore(regions, 0.6);
	////////////////////////////////////////////////////////////////////////
	texts.clear();
	textDetect(regions, texts, w);

	texts_res = texts;
}


void regionsFilterFromTextsForSign(vector<Str_Chinese> &texts, vector<Region> &regions)
{
	if (regions.empty()) return;
	vector<bool> pass(regions.size(), false);

	int panel_w = regions[0].p_w;
	int panel_h = regions[0].p_h;

	for (int i = 0; i != regions.size(); ++i)
	{
		Region re = regions[i];
		for (auto t : texts)
		{
			double in_area = overlap(re.r, t.box);
			double ratio = in_area / re.r.area();
			if (re.cen_x > panel_w / 3 && re.cen_x < panel_w * 2 / 3)
			{
				if (ratio > 0.7) pass[i] = true;
			}
			else if (ratio > 0.5) pass[i] = true;
		}
	}

	vector<Region> tmp;

	/*int left_bound = 2;
	int right_bound = panel_w - left_bound;
	int top_bound = 2;
	int bottom_bound = panel_h - top_bound;*/

	for (int i = 0; i < regions.size(); ++i)
	{
		Region re = regions[i];
		/*if (re.r.x<left_bound || re.r.x + re.r.width - 1>right_bound || re.r.y<top_bound
		|| re.r.y + re.r.height - 1>bottom_bound) continue;*/
		if (pass[i] == false) tmp.push_back(regions[i]);

	}
	regions.clear();
	regions = tmp;
}

void regionsFilterFromTexts(vector<Str_Chinese> &texts, vector<Region> &regions)
{
	if (regions.empty()) return;
	vector<bool> pass(regions.size(), false);

	int panel_w = regions[0].p_w;
	int panel_h = regions[0].p_h;

	for (int i = 0; i != regions.size(); ++i)
	{
		Region re = regions[i];
		for (auto t : texts)
		{
			if (t.str.size() < 2) continue;
			double in_area = overlap(re.r, t.box);
			double ratio = in_area / re.r.area();
			if (ratio > 0.1) pass[i] = true;
		}
	}

	vector<Region> tmp;
	for (int i = 0; i < regions.size(); ++i)
	{
		Region re = regions[i];
		if (pass[i] == false) tmp.push_back(regions[i]);
	}
	regions.clear();
	regions = tmp;
}


void refineTexts(vector<Str_Chinese> &texts)
{
	if (texts.empty()) return;
	vector<Str_Chinese> texts_2, texts_1;
	for (auto &t : texts)
	{
		if (t.str.size()>1) texts_2.push_back(t);
		else texts_1.push_back(t);
	}

	vector<bool> pass(texts_1.size(), false);

	for (int i = 0; i != texts_1.size(); ++i)
	{
		for (int j = 0; j != texts_2.size();++j)
		{
			double in_area = overlap(texts_1[i].box, texts_2[j].box);
			if (in_area>0)
			{
				pass[i] = true;
			}
		}
	}
	texts = texts_2;
	for (int i = 0; i < texts_1.size(); ++i)
	{
		if(pass[i]==false) texts.push_back(texts_1[i]);
	}
	
}


void textLines_gth_match(vector<Str_Chinese> &texts, Panel &p, vector<Str_Chinese> &neg)
{
	/*prepare for neg*/
	
	for (auto text : texts)
	{
		if (text.str.size()>1) continue;
		bool isTextline = false;
		for (auto s : p.strs)
		{
			s.box.x -= p.box.x;
			s.box.y -= p.box.y;

			double in_area = overlap(text.box, s.box);
			double r = in_area / text.box.area();
			if (r > 0.5)
			{
				isTextline = true;
				break;
			}
		}
		if (isTextline == false) neg.push_back(text);
	}
}


/*for symbol detection*/
bool adjacent(Region r1, Region r2)
{
	double y1 = r1.r.y;
	double y11 = r1.r.y + r1.r.height - 1;
	double y2 = r2.r.y;
	double y22 = r2.r.y + r2.r.height - 1;

	double ys = max(y1, y2);
	double ye = min(y11, y22);

	double inLen = ye > ys ? ye - ys : 0;
	if (inLen / r1.r.height > 0.9 || inLen / r2.r.height > 0.9)
	{
		/*interval*/
		double inter = 0;
		if (r1.r.x < r2.r.x)
		{
			double dx = r2.r.x - r1.r.x - r1.r.width;
			if (dx>0 && dx<5) return true;
		}
		else
		{
			double dx = r1.r.x - r2.r.x - r2.r.width;
			if (dx>0 && dx<5) return true;
		}
	}
	return false;
}

void mergeForCompleteSign(vector<Region> &non_texts,const Mat &image)
{
	vector<Region> tmp;
	Mat show_regions = image.clone();
	drawRects(show_regions, non_texts);
	for (int i = 0; i < non_texts.size(); ++i)
	{
		for (int j = 0; j < non_texts.size(); ++j)
		{
			if ((adjacent(non_texts[i], non_texts[j])))
			{
				Rect n_r = mergeRect(non_texts[i].r, non_texts[j].r);

				double v = min(non_texts[i].var, non_texts[j].var);
				Region res(n_r, v);

			/*	Mat show = image.clone();
				rectangle(show, n_r, Scalar(0, 255, 255));*/

				Rect r_ex(res.r);
				rect_extend(r_ex, 1.2, image.cols, image.rows);
				res.image = Mat(image, r_ex).clone();           // 总是忘记扩展boundingbox，有一些小错误还是可以避免的，经常复习代码也是有好处的嘛

				res.p_h = image.rows;
				res.p_w = image.cols;
				tmp.push_back(res);
			}
		}
	}

	Mat show = image.clone();
	drawRects(show, tmp);
	non_texts.insert(non_texts.end(), tmp.begin(), tmp.end());
}

void retrieveCharacter(vector<Str_Chinese> &texts, vector<Region> &non_texts, Mat &image)
{
	sort(non_texts.begin(), non_texts.end(), com_y);
	vector<bool> visited(non_texts.size(), false);

	double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得
	vector<double> w(init_w_3, init_w_3 + 6);

	for (auto &t : texts)
	{
		sort(t.str.begin(), t.str.end(), com_x);
		double y_u = t.box.y;
		double y_d = t.box.y + t.box.height - 1;
		int x_left = t.box.x;
		int x_right = t.box.x + t.box.width - 1;

		int c_area = t.box.height*t.box.height;

		Mat show = image.clone();
		rectangle(show, t.box, Scalar(0, 0, 255));

		vector<Region> re_add_left;
		vector<Region> re_add_right;

		for (int i = 0; i != non_texts.size(); ++i)
		{
			Mat re_show = image.clone();
			rectangle(re_show, non_texts[i].r, Scalar(255, 255, 255));

			if (visited[i]) continue;
			if (non_texts[i].r.y + non_texts[i].r.height <= y_u) continue;
			if (non_texts[i].r.y >= y_d) break;

			if (non_texts[i].r.x > x_left && non_texts[i].r.x < x_right) continue;
			if (non_texts[i].r.x + non_texts[i].r.width > x_left && non_texts[i].r.x+non_texts[i].r.width < x_right) continue;

			if (non_texts[i].r.area() < c_area*0.4 || non_texts[i].r.area() > c_area*1.1) continue;
			
			if (overlap(non_texts[i].r, t.box) > 0)
			{
				visited[i] = true;
				continue;
			}

			rectangle(re_show, non_texts[i].r, Scalar(0, 255, 255));

			double overlap_r = overlapHeight(y_u, y_d, non_texts[i]);
			if (overlap_r>0.6)
			{
				//visited[i] = true;
				if (non_texts[i].r.x <= x_left) re_add_left.push_back(non_texts[i]);
				else re_add_right.push_back(non_texts[i]);
				rectangle(show, non_texts[i].r, Scalar(0, 255, 255));
				waitKey(0);
			}
		}

		sort(re_add_left.begin(), re_add_left.end(), [](const Region &a, const Region &b){return a.r.x > b.r.x; });
		for (auto re : re_add_left)
		{
			double d = calRegionPairDist(re, t.str[0], w);
			double dx = t.box.x - re.r.x - re.r.width;
			double r = dx / re.r.width;
			if (d < 0 && r<0.5)
			{
				t.str.insert(t.str.begin(), re);
				t.box.width = t.box.x + t.box.width - re.r.x;
				t.box.x = re.r.x;
			}
		}

		sort(re_add_right.begin(), re_add_right.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
		for (auto re : re_add_right)
		{
			double d = calRegionPairDist(re, t.str[t.str.size() - 1], w);
			double dx = re.r.x-t.box.x-t.box.width;
			double r = dx / re.r.width;
			if (d < 0 && r<0.5)
			{
				t.str.insert(t.str.end(), re);
				t.box.width = re.r.x + re.r.width - t.box.x;
			}
		}
	}
}
void retrieveCharacter(vector<Str_Chinese> &texts, vector<Region> &non_texts, vector<Region> &new_cha, Mat &image)
{
	sort(non_texts.begin(), non_texts.end(), com_y);
	vector<bool> visited(non_texts.size(), false);

	double init_w_3[6] = { -9.59974, 2.47251, 12.0097, 5.7961, 12.9216, 6.42144 };   //文本行聚合的参数，通过距离度量学习获得
	vector<double> w(init_w_3, init_w_3 + 6);

	for (auto &t : texts)
	{
		sort(t.str.begin(), t.str.end(), com_x);
		double y_u = t.box.y;
		double y_d = t.box.y + t.box.height - 1;
		int x_left = t.box.x;
		int x_right = t.box.x + t.box.width - 1;

		//int c_area = t.box.height*t.box.height;
		int c_area = t.str[0].r.area();

		Mat show = image.clone();
		rectangle(show, t.box, Scalar(0, 0, 255));

		vector<Region> re_add_left;
		vector<Region> re_add_right;

		for (int i = 0; i != non_texts.size(); ++i)
		{
			Mat re_show = image.clone();
			rectangle(re_show, non_texts[i].r, Scalar(255, 255, 255));

			if (visited[i]) continue;
			if (non_texts[i].r.y + non_texts[i].r.height <= y_u) continue;
			if (non_texts[i].r.y >= y_d) break;

			if (non_texts[i].r.x > x_left && non_texts[i].r.x < x_right) continue;
			if (non_texts[i].r.x + non_texts[i].r.width > x_left && non_texts[i].r.x + non_texts[i].r.width < x_right) continue;

			if (non_texts[i].r.area() < c_area*0.4 || non_texts[i].r.area() > c_area*1.4) continue;

			if (overlap(non_texts[i].r, t.box) > 0)
			{
				visited[i] = true;
				continue;
			}

			rectangle(re_show, non_texts[i].r, Scalar(0, 255, 255));

			double overlap_r = overlapHeight(y_u, y_d, non_texts[i]);
			if (overlap_r>0.7)
			{
				//visited[i] = true;
				if (non_texts[i].r.x <= x_left) re_add_left.push_back(non_texts[i]);
				else re_add_right.push_back(non_texts[i]);
				rectangle(show, non_texts[i].r, Scalar(0, 255, 255));
				waitKey(0);
			}
		}

		sort(re_add_left.begin(), re_add_left.end(), [](const Region &a, const Region &b){return a.r.x > b.r.x; });
		for (auto re : re_add_left)
		{
			double d = calRegionPairDist(re, t.str[0], w);
			double dx = t.box.x - re.r.x - re.r.width;
			double r = dx / re.r.width;
			if (d < 0 && r<0.5)
			{
				new_cha.push_back(re);
				t.str.insert(t.str.begin(), re);
				t.box.width = t.box.x + t.box.width - re.r.x;
				t.box.x = re.r.x;
			}
		}

		sort(re_add_right.begin(), re_add_right.end(), [](const Region &a, const Region &b){return a.r.x < b.r.x; });
		for (auto re : re_add_right)
		{
			double d = calRegionPairDist(re, t.str[t.str.size() - 1], w);
			double dx = re.r.x - t.box.x - t.box.width;
			double r = dx / re.r.width;
			if (d < 0 && r<0.5)
			{
				new_cha.push_back(re);
				t.str.insert(t.str.end(), re);
				t.box.width = re.r.x + re.r.width - t.box.x;
			}
		}
	}
}
