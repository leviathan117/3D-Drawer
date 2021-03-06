#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <GL/glut.h>
#include <bits/stdc++.h>


using namespace cv;
using namespace std;

fstream in, out;

//stand_green
int h1 = 40;
int s1 = 79;
int v1 = 16;
int h2 = 87;
int s2 = 255;
int v2 = 255;

char opencv_window_key = 0;

double firstcamx;
double secondcamx;
double firstcamz;
double a;
double b;
double c;

string new_file_name;
string old_file_name;

bool start_window_work = true;
bool create_new_file = false;
bool file_open = false;

vector <vector <double> > x_points;
vector <vector <double> > y_points;
vector <vector <double> > z_points;
int size = 0;

int show_camera_windows = 1;

int it = 0;
vector <int> detect_time(10, 0);
bool state = 0;
bool detect_active = true;
bool image_resize_to_cascade = false;
double image_resize = 0.5;

//===============================================================

double angle = M_PI / 4 + M_PI, angle2 = M_PI / 16;
double lx = 0, ly = 0, lz = 0;
double deltaAngle = 0, deltaAngle2 = 0;
double angle_speed = 0;
int xOrigin = -1;
int yOrigin = -1;

bool center_object = 0;
bool draw_lines = 1;

long long time_now, timebase;
int frames = 0;

int line_x_length = 20;//RED
int line_y_length = 20;//GREEN
int line_z_length = 20;//BLUE

Mat colorFilter(const Mat& src);
void imageProssesing();
void changeWindSize(int w, int h);
void drawSphere(double x_pos, double y_pos, double z_pos, double rad, double qual, double r_col, double g_col, double b_col);
void drawLine(double x1_pos, double y1_pos, double z1_pos, double x2_pos, double y2_pos, double z2_pos, double r_col, double g_col, double b_col);
void printFPS();
double calIndent (vector <vector <double> > arr, int arr_size);
void renderScene();
void save_file();
void pressKey(unsigned char key, int unkown_param_1, int unkown_param_2);
void mouseMove(int mouse_x_now, int mouse_y_now);
void mouseButton(int button, int state, int mouse_x_now, int mouse_y_now);
void launchGLUT();

class Detector
{
    HOGDescriptor hog;
public:
    Detector() : hog()
    {
        hog.load("hog/detector_l.yml");
    }
    pair<int, int> detect(Mat input_img, Mat &output_img, bool draw_detection)
    {
        Mat inner_img;
        resize(input_img, inner_img, cv::Size(), image_resize, image_resize);
        vector<Rect> found;
        vector<double> values;
        if (image_resize_to_cascade)
        {
            hog.detectMultiScale(inner_img, found, values, 0, Size(8, 8), Size(16, 16), 1.05, 2, false);
        } else
        {
            hog.detectMultiScale(inner_img, found, values, 0, Size(8, 8), Size(16, 16), 1.00, 2, false);
        }
        double max_value = -1;
		double max_i = 0;
		for (int i = 0; i < found.size(); i++)
		{
			if (values[i] > max_value)
			{
				max_value = values[i];
				max_i = i;
			}
		}

		output_img = inner_img;

		pair <int, int> out (-1, -1);

		if (max_value != -1)
		{
			Rect r = found[max_i];
			out.first = r.x + r.width / 2;
			out.second = r.y + r.height / 2;
			if (draw_detection)
			{
				Rect r2 = r;
				if (image_resize_to_cascade)
				{
					adjustRect(r);
				}
				r2.x += r2.width / 4;
				r2.y += r2.height / 4;
				r2.width *= 0.5;
				r2.height *= 0.5;
				rectangle(output_img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
				rectangle(output_img, r2.tl(), r2.br(), cv::Scalar(0, 0, 255), 2);
			}
		}

        return out;

    }
    void adjustRect(Rect & r) const
    {
        // this is because of image resize to 1.05 in detectMultiScale
        r.x += cvRound(r.width * 0.1);
        r.width = cvRound(r.width * 0.8);
        r.y += cvRound(r.height * 0.07);
        r.height = cvRound(r.height * 0.8);
    }
};

static Scalar randomColor(RNG rng)
{
    int icolor = (unsigned)rng;
    return Scalar(icolor&255, (icolor>>8)&255, (icolor>>16)&255);
}


/*

boldness = [1, 9]
*/
void draw_button(Mat &image, int X, int Y, string text, int size)
{
	int lineType = LINE_AA;
	Point back_rectangle[1][4];
		back_rectangle[0][0]  = Point(X, Y + 5);
		back_rectangle[0][1]  = Point(X, Y - size * 22);
		back_rectangle[0][2]  = Point(X + text.size() * size * 18.75, Y - size * 22);
		back_rectangle[0][3]  = Point(X + text.size() * size * 18.75, Y + 5);
		const Point* ppt[1] = { back_rectangle[0] };
		int npt[] = { 4 };
		fillPoly(image, ppt, npt, 1, Scalar(0, 255, 0), lineType );
		putText(image, text, Point(X, Y), 0, size, Scalar(0, 0, 255), size, lineType, false);
	rectangle(image, Point(X, Y + 5), Point(X + text.size() * size * 18.75, Y - size * 22), Scalar(255, 0, 0), size, lineType);

}
class button
{
    int X, Y;
    string text;
    double size;
    int lineType;
    int rect_len;

public:
    button(int x_in = 0, int y_in = 0, string text_in = "NO_TEXT!", double size_in = 2.5, int lineType_in = LINE_AA, int rect_len_in = 100)
    {
        X = x_in;
        Y = y_in;
		text = text_in;
		size = size_in;
		lineType = lineType_in;
		rect_len = rect_len_in;
    };
    void draw(Mat &image)
    {
		Point back_rectangle[1][4];
		back_rectangle[0][0]  = Point(X, Y + 25);
		back_rectangle[0][1]  = Point(X, Y - size * 22 - 5);
		back_rectangle[0][2]  = Point(X + rect_len, Y - size * 22 - 5);
		back_rectangle[0][3]  = Point(X + rect_len, Y + 25);
		const Point* ppt[1] = { back_rectangle[0] };
		int npt[] = { 4 };
		fillPoly(image, ppt, npt, 1, Scalar(0, 255, 0), lineType );
		putText(image, text, Point(X, Y), 0, size, Scalar(0, 0, 255), size, lineType, false);
		rectangle(image, Point(X, Y + 25), Point(X + rect_len, Y - size * 22 - 5), Scalar(255, 0, 0), size, lineType);
    }
    int is_pressed(int x_in, int y_in)
    {
		if (X <= x_in && x_in <= X + rect_len)
		{
			if (Y - size * 22 - 5 <= y_in && y_in <= Y + 25)
			{
				return 1;
			}
		}
		return 0;
    }
};

int mouse_state = 0;
int mouse_x = 0, mouse_y = 0;

void mouse_handle( int event_in, int x_in, int y_in, int, void*)
{
    switch(event_in)
    {
    case EVENT_LBUTTONDOWN:
		mouse_x = x_in;
		mouse_y = y_in;
		mouse_state = 1;
        break;
    case EVENT_LBUTTONUP:
		mouse_x = x_in;
		mouse_y = y_in;
		mouse_state = 0;
        break;
    }
}

string filename;

int main(int argc, char **argv)
{
	char wndname[] = "3Δ Drawer";
    const int NUMBER = 100;
    const int DELAY = 5;
    int lineType = LINE_AA; // change it to LINE_8 to see non-antialiased graphics
    int i, width = 1280, height = 720;
    int x1 = -width/2, x2 = width * 3 / 2, y1 = -height/2, y2 = height*3/2;
    RNG rng(0xFFFFFFFF);

    Mat image = Mat::zeros(height, width, CV_8UC3);
    //imshow(wndname, image);
    waitKey(DELAY);

    char key = ' ';

    button b1(width / 2, 100, "Create new file", 2.5, LINE_AA, 600);
    button b2(width / 2, 200, "Open file", 2.5, LINE_AA, 600);

    namedWindow(wndname);
    setMouseCallback(wndname, mouse_handle);

    int new_file = 0;

    while (key != 27)
    {
		b1.draw(image);
		b2.draw(image);

        if (b1.is_pressed(mouse_x, mouse_y) && mouse_state)
        {
			new_file = 1;
			break;
        }
        if (b2.is_pressed(mouse_x, mouse_y) && mouse_state)
        {
			new_file = 0;
			break;
        }

        imshow(wndname, image);
		key = waitKey(DELAY);
    }


    if (key == 27)
    {
		exit(0);
    }
    waitKey(100);
    destroyAllWindows();

    cout << "Enter filename:";
	cin >> filename;

    if (new_file == 0)
    {
		in.open("files/" + filename + ".3dd", ios::in);

		int number_of_lines;
		in >> number_of_lines;
		for (int i = 0; i < number_of_lines; i++)
		{
			int number_of_points;
			in >> number_of_points;
			x_points.push_back(vector <double> ());
			y_points.push_back(vector <double> ());
			z_points.push_back(vector <double> ());

			int total_lines_now = x_points.size();

			for (int j = 0; j < number_of_points; j++)
			{
				double x_tmp, y_tmp, z_tmp;
				in >> x_tmp >> y_tmp >> z_tmp;
				x_points[total_lines_now - 1].push_back(x_tmp);
				y_points[total_lines_now - 1].push_back(y_tmp);
				z_points[total_lines_now - 1].push_back(z_tmp);
			}
		}

		in.close();
    }

    x_points.push_back(vector <double> ());
	y_points.push_back(vector <double> ());
	z_points.push_back(vector <double> ());

	//Get GLUT ready
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(300,300);
	glutInitWindowSize(1280,720);
	glutCreateWindow("3D Drawer");

	thread f1(imageProssesing);
	thread f2(launchGLUT);
	f1.join();
	f2.join();

	return 1;
}


Mat colorFilter(const Mat& src)
{
    assert(src.type() == CV_8UC3);

    Mat filtered;
    inRange(src, Scalar(h1, s1, v1), Scalar(h2, s2, v2), filtered);

    return filtered;
}

int cam1_finish = 0;
int cam2_finish = 0;

void imageProssesing()
{
	//detect_active = false;//TMP!!
    VideoCapture camera_capture1;
	VideoCapture camera_capture2;
	Mat frame1;
	Mat frame2;
    if (camera_capture1.open(4) == false)
    {
        exit(EXIT_FAILURE);
    }
    if (camera_capture2.open(2) == false)
    {
        exit(EXIT_FAILURE);
    }


    camera_capture1.set(CAP_PROP_FRAME_WIDTH,  1280);
    camera_capture1.set(CAP_PROP_FRAME_HEIGHT, 720);
    camera_capture1.set(CAP_PROP_FPS,          60);

    camera_capture2.set(CAP_PROP_FRAME_WIDTH,  1280);
    camera_capture2.set(CAP_PROP_FRAME_HEIGHT, 720);
    camera_capture2.set(CAP_PROP_FPS,          60);

	int lock_detect_time = 0;


    Detector detector;
	waitKey(100);
    while (true)
    {
        opencv_window_key = waitKey(1);
        //=============================================================================
        //      CAMERA 1

        if (camera_capture1.read(frame1) == false)
        {
            exit(EXIT_FAILURE);
        }
        if (camera_capture2.read(frame2) == false)
        {
            exit(EXIT_FAILURE);
        }

        Point2f point1;
        if (detect_active)
        {
			Mat detect_frame = frame1;
			pair <int, int> detection_cords;

			int64 t = getTickCount();
			detection_cords = detector.detect(frame1, detect_frame, true);
			t = getTickCount() - t;

			// show the window
			{
				ostringstream buf;
				buf << "FPS: " << fixed << setprecision(2) << (getTickFrequency() / (double)t);
				putText(detect_frame, buf.str(), Point(10, 30), FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255), 2, LINE_AA);
			}

			if (show_camera_windows)
			{
				imshow("camera_capture1 - detection", detect_frame);
			}

			point1.x = detection_cords.first;
			point1.y = detection_cords.second;
        }


        if (opencv_window_key == 'q')
            h1++;
        else if (opencv_window_key == 'a')
            h1--;
        else if (opencv_window_key == 'w')
            s1++;
        else if (opencv_window_key == 's')
            s1--;
        else if (opencv_window_key == 'e')
            v1++;
        else if (opencv_window_key == 'd')
            v1--;
        else if (opencv_window_key == 'r')
            h2++;
        else if (opencv_window_key == 'f')
            h2--;
        else if (opencv_window_key == 't')
            s2++;
        else if (opencv_window_key == 'g')
            s2--;
        else if (opencv_window_key == 'y')
            v2++;
        else if (opencv_window_key == 'h')
            v2--;

        //cout << h1 << " " << s1 << " " << v1 << " " << h2 << " " << s2 << " " << v2 << " --- ";


        //=============================================================================
        //      CAMERA 2

        Point2f point2;
        if (detect_active)
        {
			Mat detect_frame = frame2;
			pair <int, int> detection_cords;

			int64 t = getTickCount();
			detection_cords = detector.detect(frame2, detect_frame, true);
			t = getTickCount() - t;
			// show the window
			{
				ostringstream buf;
				buf << "FPS: " << fixed << setprecision(2) << (getTickFrequency() / (double)t);
				putText(detect_frame, buf.str(), Point(10, 30), FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255), 2, LINE_AA);
			}

			if (show_camera_windows)
			{
				imshow("camera_capture2 - detection", detect_frame);
			}

			point2.x = detection_cords.first;
			point2.y = detection_cords.second;
        }


        //=============================================================================
        //      IMAGE PROSESSING

        if (point1.x != -1 && point2.x != -1)
        {
			firstcamx=(point2.x - 1280 / 2) / 40;
			secondcamx=(point1.x - 1280 / 2) / 40;
			firstcamz=(720 / 2 - point1.y) / 40;
			firstcamx=(point2.x - 1280 / 2 * image_resize) / 40;
			secondcamx=(point1.x - 1280 / 2 * image_resize) / 40;
			firstcamz=(720 / 2 * image_resize - point1.y) / 40;
			double kinda23 = 30.8724;
			if (firstcamx != 0)
			{
				a = kinda23 / (firstcamx);
			}
			if (secondcamx != 0)
			{
				b = kinda23 / (secondcamx);
			}
			if (firstcamz != 0)
			{
				c = kinda23 / (firstcamz);
			}

			double x_fill, y_fill, z_fill;

			if (firstcamx != 0 && secondcamx != 0 && firstcamz != 0)
			{
				x_fill = (a + b) / (a - b);
				y_fill = (2 * a * b) / (a - b);
				z_fill = y_fill / c;
			}
			if (firstcamz == 0 && firstcamx != 0 && secondcamx != 0)
			{
				x_fill = (a + b) / (a - b);
				y_fill = (2 * a * b) / (a - b);
				z_fill = 0;
			}
			if (firstcamx == 0 && firstcamz != 0)
			{
				x_fill = 1;
				y_fill = 2 * b;
				z_fill = y_fill / c;
			}
			if (secondcamx == 0 && firstcamz != 0)
			{
				x_fill = -1;
				y_fill = -2 * a;
				z_fill = y_fill / c;
			}
			if (firstcamx == 0 && firstcamz == 0)
			{
				x_fill = 1;
				y_fill = 2 * b;
				z_fill = 0;
			}
			if (secondcamx == 0 && firstcamz == 0)
			{
				x_fill = 1;
				y_fill = -2 * a;
				z_fill = 0;
			}

			if (y_fill <= 60 && x_fill <= 60 && z_fill <= 60 && x_fill >= -60 && y_fill >= -60 && y_fill >= 0 && abs(point1.y - point2.y) < 100)// && state == 1)
			{
				int line_it = x_points.size() - 1;
				x_points[line_it].push_back(-x_fill);
				y_points[line_it].push_back(y_fill / 4);//HERE!!!!!!!!!!!!
				z_points[line_it].push_back(z_fill);
				detect_time[it] = 1;
			} else
			{
				detect_time[it] = 0;
			}
        } else
		{
			detect_time[it] = 0;
		}

		it++;
		it %= 10;


		if (accumulate(detect_time.begin(), detect_time.end(), 0ll) > 8)
		{
			lock_detect_time = 1;
		}

		if (lock_detect_time && accumulate(detect_time.begin(), detect_time.end(), 0ll) < 5)
		{
			x_points.push_back(vector <double> ());
			y_points.push_back(vector <double> ());
			z_points.push_back(vector <double> ());
			fill(detect_time.begin(), detect_time.end(), 0);
			lock_detect_time = 0;
		}

        printFPS();
    }
}


void changeWindSize(int w, int h)
{
	// предотвращение деления на ноль
	if (h == 0)
	{
		h = 1;
	}
	double ratio =  w * 1.0 / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0, ratio, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void drawSphere(double x_pos, double y_pos, double z_pos, double rad, double qual, double r_col, double g_col, double b_col)
{
	double R_col = r_col / 255;
	double G_col = g_col / 255;
	double B_col = b_col / 255;
	glColor3f(R_col, G_col, B_col);
	glPushMatrix();
	glTranslatef(x_pos, y_pos, z_pos);
 	glutSolidSphere(rad, qual, qual);
	glPopMatrix();
}

void drawLine(double x1_pos, double y1_pos, double z1_pos, double x2_pos, double y2_pos, double z2_pos, double r_col, double g_col, double b_col)
{
	double R_col = r_col / 255;
	double G_col = g_col / 255;
	double B_col = b_col / 255;
	glColor3f(R_col, G_col, B_col);
	glBegin(GL_LINES);
	glVertex3f(x1_pos, y1_pos, z1_pos);
	glVertex3f(x2_pos, y2_pos, z2_pos);
	glEnd();
}

void printFPS()
{
	frames++;
	time_now=glutGet(GLUT_ELAPSED_TIME);
	if (time_now - timebase > 1000)
	{
		cout << frames * 1000.0 / (time_now - timebase) << "\n";
		cout.flush();
		timebase = time_now;
		frames = 0;
	}
}

double calIndent (vector <vector <double> > arr)
{
    if (arr.size() == 0 || arr[0].size() == 0)
    {
		return 0;
    }
    double count = 0;
    double num_elements = 0;
    for (int i = 0; i < arr.size(); i++)
    {
		for (int j = 0; j < arr[i].size(); j++)
		{
			count += arr[i][j];
			num_elements++;
		}
    }
    return count / num_elements;
}

void renderScene()
{
	angle += angle_speed;
	lx = sin(angle + deltaAngle);
	lz = -cos(angle + deltaAngle);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(lx * 10 + calIndent(x_points), ly * 10 + calIndent(z_points), lz * 10 + calIndent(y_points), calIndent(x_points), calIndent(z_points),  calIndent(y_points), 0, 1, 0);
	//gluLookAt(lx * 15, ly * 15, lz * 15, 0, 0, 0, 0, 1, 0);

	drawLine(0, 0, 0, line_x_length, 0, 0, 255, 0, 0);
	for (int i = 0; i <= line_x_length; i++)
	{
		drawSphere(i, 0, 0, 0.05, 10, 255, 0, 0);
	}
	drawLine(0, 0, 0, -line_x_length, 0, 0, 50, 0, 0);
	for (int i = 0; i <= line_x_length; i++)
	{
		drawSphere(-i, 0, 0, 0.05, 10, 50, 0, 0);
	}
	//-----------------
	drawLine(0, 0, 0, 0, line_y_length, 0, 0, 255, 0);
	for (int i = 0; i <= line_y_length; i++)
	{
		drawSphere(0, i, 0, 0.05, 10, 0, 255, 0);
	}
	drawLine(0, 0, 0, 0, -line_y_length, 0, 0, 50, 0);
	for (int i = 0; i <= line_y_length; i++)
	{
		drawSphere(0, -i, 0, 0.05, 10, 0, 50, 0);
	}
	//-----------------

	drawLine(0, 0, 0, 0, 0, line_z_length, 0, 0, 255);
	for (int i = 0; i <= line_z_length; i++)
	{
		drawSphere(0, 0, i, 0.05, 10, 0, 0, 255);
	}
	drawLine(0, 0, 0, 0, 0, -line_z_length, 0, 0, 50);
	for (int i = 0; i <= line_z_length; i++)
	{
		drawSphere(0, 0, -i, 0.05, 10, 0, 0, 50);
	}

	drawSphere(0, 0, 0, 0.1, 10, 0, 255, 255);

	//MAYBE MAKE BOURDERS
	/*drawLine(0, 0, MAX_Y / 2 - calIndent(y_points, size), 0, 0, MIN_Y - calIndent(y_points, size), 0, 0, 100);
	for (int i = MIN_Y; i <= MAX_Y / 2; i++)
	{
		drawSphere(0, 0, i - calIndent(y_points, size), 0.05, 10, 0, 0, 100);
	}
	drawLine(0, 0, MAX_Y / 2 - calIndent(y_points, size), 0, 0, MAX_Y - calIndent(y_points, size), 0, 0, 255);
	for (int i = 0; i <= MAX_Y / 2; i++)
	{
		drawSphere(0, 0, MAX_Y / 2 + i - calIndent(y_points, size), 0.05, 10, 0, 0, 255);
	}*/
	double drawSize = 1;
	/*for (int i = 0; i < size; i++)
	{
		if (center_object)
		{
			drawSphere(x_points[i] * drawSize - calIndent(x_points, size),   z_points[i] * drawSize - calIndent(z_points, size),   y_points[i] * drawSize - calIndent(y_points, size), 0.03, 10, 255, 255, 255);
		} else
		{
			drawSphere(x_points[i] * drawSize,   z_points[i] * drawSize,   y_points[i] * drawSize, 0.03, 10, 255, 255, 255);
		}

	}*/
	for (int i = 0; i < x_points.size(); i++)
		{
            for (int j = 0; j < x_points[i].size(); j++)
            {
				drawSphere(x_points[i][j] * drawSize,   z_points[i][j] * drawSize,   y_points[i][j] * drawSize, 0.03, 5, 255, 255, 255);
            }
		}
	if (draw_lines)
	{
		/*for (int i = 1; i < size; i++)
		{
			if (center_object)
			{
				drawLine(x_points[i] * drawSize - calIndent(x_points, size),   z_points[i] * drawSize - calIndent(z_points, size),   y_points[i] * drawSize - calIndent(y_points, size), x_points[i - 1] * drawSize - calIndent(x_points, size),   z_points[i - 1] * drawSize - calIndent(z_points, size),   y_points[i - 1] * drawSize - calIndent(y_points, size), 0, 255, 0);
			} else
			{
				drawLine(x_points[i] * drawSize,   z_points[i] * drawSize,   y_points[i] * drawSize, x_points[i - 1] * drawSize,   z_points[i - 1] * drawSize,   y_points[i - 1] * drawSize, 0, 255, 0);
			}
		}*/
		for (int i = 0; i < x_points.size(); i++)
		{
            for (int j = 1; j < x_points[i].size(); j++)
            {
				drawLine(x_points[i][j] * drawSize,   z_points[i][j] * drawSize,   y_points[i][j] * drawSize, x_points[i][j - 1] * drawSize,   z_points[i][j - 1] * drawSize,   y_points[i][j - 1] * drawSize, 0, 255, 0);
            }
		}
	}

	glutSwapBuffers();
}

void save_file()
{
	out.open("files/" + filename + ".3dd", ios::out);

	int number_of_lines = x_points.size();
	out << number_of_lines << "\n";
	for (int i = 0; i < number_of_lines; i++)
	{
		int number_of_points = x_points[i].size();
		out << number_of_points << "\n";

		for (int j = 0; j < number_of_points; j++)
		{
			double x_tmp = x_points[i][j];
			double y_tmp = y_points[i][j];
			double z_tmp = z_points[i][j];

			out << x_tmp << " " << y_tmp << " " << z_tmp << "\n";
		}
		out << "\n";
	}
	out.close();
}

void pressKey(unsigned char key, int unkown_param_1, int unkown_param_2)
{
    if (key == 27)
    {
		exit(0);
	} else if (key == 'd' || key == 'D')
	{
		angle_speed = -0.01;
	} else if (key == 'a' || key == 'A')
	{
		angle_speed = 0.01;
	} else if (key == ' ')
	{
		x_points.clear();
		y_points.clear();
		z_points.clear();
		x_points.push_back(vector <double> ());
		y_points.push_back(vector <double> ());
			z_points.push_back(vector <double> ());
	} else if (key == 'l' || key == 'L')
	{
		draw_lines = !draw_lines;
	} else if (key == 'p')
	{
		for (int i = 0; i < 10; i++)
		{
			detect_time[i] = 1;
		}
	} else if (key == 's')
	{
		save_file();
	} else
	{
		angle_speed = 0;
	}
}

void mouseMove(int mouse_x_now, int mouse_y_now)
{
	if (xOrigin >= 0)
	{
		deltaAngle = (mouse_x_now - xOrigin) * 0.001f;

		lx = sin(angle + deltaAngle);
		lz = -cos(angle + deltaAngle);
	}
	if (yOrigin >= 0)
	{
		deltaAngle2 = (mouse_y_now - yOrigin) * 0.001f;

		if (angle2 + deltaAngle2 >= M_PI / 2 - 0.1)
		{
			deltaAngle2 = M_PI / 2 - angle2 - 0.1;
		}
		if (angle2 + deltaAngle2 <= 0.1 - M_PI / 2)
		{
			deltaAngle2 = -M_PI / 2 - angle2 + 0.1;
		}

		ly = sin(angle2 + deltaAngle2);
	}
}

void mouseButton(int button, int state, int mouse_x_now, int mouse_y_now)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			angle += deltaAngle;
			deltaAngle = 0;
			angle2 += deltaAngle2;
			deltaAngle2 = 0;
			xOrigin = -1;
		} else
		{
			xOrigin = mouse_x_now;
			yOrigin = mouse_y_now;
		}
	}
}

void launchGLUT()
{

	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeWindSize);
	glutIdleFunc(renderScene);

	lx = sin(angle);
	lz = -cos(angle);
	ly = sin(angle2);

	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(pressKey);

	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
}
