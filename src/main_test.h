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

#include "../src/LBP/feature.h"
#include "../src/LBP/LBP.hpp"

using namespace cv;
using namespace std;