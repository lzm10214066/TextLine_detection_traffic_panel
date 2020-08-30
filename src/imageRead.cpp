#include "imageRead.h"

void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

int readImageFile(string &imageFile, vector<string> &pathOfImage)
{
	string buf;
	int imageCount = 0;

	ifstream img_list(imageFile);

	if (img_list)
	{
		cout << "file is : " << imageFile << endl;

	}
	else
	{
		cout << "cannot open the imagelist file." << endl;
	}

	while (img_list)
	{
		if (getline(img_list, buf))
		{
			pathOfImage.push_back(buf);
			imageCount++;
		}
	}
	img_list.close();
	cout << imageCount << " things have been read" << endl;

	return imageCount;
}

void rectsSaveAsImage(Mat &image, vector<Rect> recBag, string imagePath, string &folder)
{
	int m = imagePath.find_last_of('\\');
	int n = imagePath.find_last_of('.');

	string image_name = imagePath.substr(m + 1, n - m - 1);

	for (int k = 0; k != recBag.size(); ++k)
	{
		Rect rec = recBag[k];
		Mat recImage(image, rec);
		char recToSaved[50];
		sprintf(recToSaved, "%s/%s_%d_rec.png", folder.c_str(), image_name.c_str(), k);
		imwrite(recToSaved, recImage);
	}
}

void saveErrImg(vector<string> &error_images, string &folder)
{
	for (int i = 0; i < error_images.size(); ++i)
	{
		Mat img = imread(error_images[i]);
		int m = error_images[i].find_last_of('\\');
		string imgName = error_images[i].substr(m + 1);
		char recToSaved[50];
		sprintf(recToSaved, "%s/%s", folder.c_str(), imgName.c_str());
		imwrite(recToSaved, img);
	}
}

void rectsSaveAsImage(vector<Region> &regions, string &folder, Size &normSize,string type)
{
	for (int k = 0; k != regions.size(); ++k)
	{
		Region r = regions[k];
		Mat img = r.image;
		Mat normImg(normSize, CV_8UC3);
		resize(img, normImg, normSize);

		char recToSaved[50];
	
		sprintf(recToSaved, "%s/%s_%d.png", folder.c_str(), type.c_str(),k);
		imwrite(recToSaved, normImg);
	}
}

void rectsSaveAsImage(vector<Region> &regions, string &folder, Size &normSize, string &image_name,int i)
{
	for (int k = 0; k != regions.size(); ++k)
	{
		Region r = regions[k];
		Mat img = r.image;
		Mat normImg(normSize, CV_8UC3);
		resize(img, normImg, normSize);

		char recToSaved[50];

		sprintf(recToSaved, "%s/%s_%d_%d.png", folder.c_str(), image_name.c_str(), i, k);
		imwrite(recToSaved, normImg);
	}
}

void rectsSaveAsBinaryImage(vector<Region> &regions, string &folder, Size &normSize, string type)
{
	for (int k = 0; k != regions.size(); ++k)
	{
		Region r = regions[k];
		Mat img = r.pointImg;
		if (!img.data) continue;
		Mat normImg(normSize, CV_8UC1);
		//resize(img, normImg, normSize);
		//threshold(normImg, normImg, 1, 255, THRESH_BINARY);

		char recToSaved[50];
		sprintf(recToSaved, "%s/%s_%d_bin.png", folder.c_str(), type.c_str(), k);
		imwrite(recToSaved, img);
	}
}