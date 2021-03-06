#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ml.hpp"
#include "opencv2/objdetect.hpp"

#include <iostream>
#include <time.h>
#include <fstream>

using namespace cv;
using namespace cv::ml;
using namespace std;

vector< float > get_svm_detector( const Ptr< SVM >& svm )
{
    // get the support vectors
    Mat sv = svm->getSupportVectors();
    const int sv_total = sv.rows;
    // get the decision function
    Mat alpha, svidx;
    double rho = svm->getDecisionFunction( 0, alpha, svidx );

    CV_Assert( alpha.total() == 1 && svidx.total() == 1 && sv_total == 1 );
    CV_Assert( (alpha.type() == CV_64F && alpha.at<double>(0) == 1.) ||
               (alpha.type() == CV_32F && alpha.at<float>(0) == 1.f) );
    CV_Assert( sv.type() == CV_32F );

    vector< float > hog_detector( sv.cols + 1 );
    memcpy( &hog_detector[0], sv.ptr(), sv.cols*sizeof( hog_detector[0] ) );
    hog_detector[sv.cols] = (float)-rho;
    return hog_detector;
}

/*
* Convert training/testing set to be used by OpenCV Machine Learning algorithms.
* TrainData is a matrix of size (#samples x max(#cols,#rows) per samples), in 32FC1.
* Transposition of samples are made if needed.
*/
void convert_to_ml( const vector< Mat > & train_samples, Mat& trainData )
{
    //--Convert data
    const int rows = (int)train_samples.size();
    const int cols = (int)std::max( train_samples[0].cols, train_samples[0].rows );
    Mat tmp( 1, cols, CV_32FC1 ); //< used for transposition if needed
    trainData = Mat( rows, cols, CV_32FC1 );

    for( size_t i = 0 ; i < train_samples.size(); ++i )
    {
        CV_Assert( train_samples[i].cols == 1 || train_samples[i].rows == 1 );

        if( train_samples[i].cols == 1 )
        {
            transpose( train_samples[i], tmp );
            tmp.copyTo( trainData.row( (int)i ) );
        }
        else if( train_samples[i].rows == 1 )
        {
            train_samples[i].copyTo( trainData.row( (int)i ) );
        }
    }
}

void do_stuff_for_neg( const String & dirname, Size size, vector< Mat > & gradient_lst, bool use_flip, int num_samples)
{
    vector< String > files;
    glob( dirname, files );
    //---
    Rect box;
    box.width = size.width;
    box.height = size.height;

    const int size_x = box.width;
    const int size_y = box.height;

    srand( (unsigned int)time( NULL ) );

    //----

    HOGDescriptor hog;
    hog.winSize = size;
    Mat gray;
    vector< float > descriptors;

    //===========

    cout << "\n";

    for ( size_t i = 0; i < files.size() && i < num_samples; ++i )
    {
        if (i % 100 == 0)
        {
            cout << i << " out of " << min(int(files.size()), num_samples) << "\n";
        }
        Mat img = imread( files[i] ); // load the image
        if ( img.empty() )            // invalid image, skip it.
        {
            cout << files[i] << " is invalid!" << endl;
            continue;
        }

        //=======================

        if ( img.cols > box.width && img.rows > box.height )
        {
            box.x = rand() % ( img.cols - size_x );
            box.y = rand() % ( img.rows - size_y );
            Mat roi = img( box );
            img = roi;
        } else
        {
            cout << files[i] << "   !!has wrong size" << "\n";
            continue;
        }

        //=======================


        if ( img.cols >= size.width && img.rows >= size.height )
        {
            Rect r = Rect(( img.cols - size.width ) / 2,
                          ( img.rows - size.height ) / 2,
                          size.width,
                          size.height);
            cvtColor( img(r), gray, COLOR_BGR2GRAY );
            hog.compute( gray, descriptors, Size( 8, 8 ), Size( 0, 0 ) );
            gradient_lst.push_back( Mat( descriptors ).clone() );
            if ( use_flip )
            {
                flip( gray, gray, 1 );
                hog.compute( gray, descriptors, Size( 8, 8 ), Size( 0, 0 ) );
                gradient_lst.push_back( Mat( descriptors ).clone() );
            }
        }
    }

}

void load_images( const String & dirname, vector< Mat > & img_lst, bool showImages = false )
{
    vector< String > files;
    glob( dirname, files );

    cout << "\n";

    for ( size_t i = 0; i < files.size(); ++i )
    {
        if (i % 100 == 0)
        {
            cout << i << " out of " << files.size() << "\n";
        }
        Mat img = imread( files[i] ); // load the image
        if ( img.empty() )            // invalid image, skip it.
        {
            cout << files[i] << " is invalid!" << endl;
            continue;
        }

        if ( showImages )
        {
            imshow( "image", img );
            waitKey( 1 );
        }
        img_lst.push_back( img );
    }
}

void computeHOGs( const Size wsize, const vector< Mat > & img_lst, vector< Mat > & gradient_lst, bool use_flip )
{
    HOGDescriptor hog;
    hog.winSize = wsize;
    Mat gray;
    vector< float > descriptors;

    for( size_t i = 0 ; i < img_lst.size(); i++ )
    {
        if ( img_lst[i].cols >= wsize.width && img_lst[i].rows >= wsize.height )
        {
            Rect r = Rect(( img_lst[i].cols - wsize.width ) / 2,
                          ( img_lst[i].rows - wsize.height ) / 2,
                          wsize.width,
                          wsize.height);
            cvtColor( img_lst[i](r), gray, COLOR_BGR2GRAY );
            hog.compute( gray, descriptors, Size( 8, 8 ), Size( 0, 0 ) );
            gradient_lst.push_back( Mat( descriptors ).clone() );
            if ( use_flip )
            {
                flip( gray, gray, 1 );
                hog.compute( gray, descriptors, Size( 8, 8 ), Size( 0, 0 ) );
                gradient_lst.push_back( Mat( descriptors ).clone() );
            }
        }
    }
}

int main( int argc, char** argv )
{
    /*const char* keys =
    {
        "{help h|     | show help message}"
        "{pd    |     | path of directory contains positive images}"
        "{nd    |     | path of directory contains negative images}"
        "{td    |     | path of directory contains test images}"
        "{tv    |     | test video file name}"
        "{dw    |     | width of the detector}"
        "{dh    |     | height of the detector}"
        "{f     |false| indicates if the program will generate and use mirrored samples or not}"
        "{d     |false| train twice}"
        "{t     |false| test a trained detector}"
        "{v     |false| visualize training steps}"
        "{fn    |detector.yml| file name of trained SVM}"
    };

    CommandLineParser parser( argc, argv, keys );

    if ( parser.has( "help" ) )
    {
        parser.printMessage();
        exit( 0 );
    }*/

    String pos_dir, neg_dir, obj_det_filename;
    int detector_width = 200;
    int detector_height = 200;
    bool flip_samples = false;
    neg_dir = "../HAAR/bad_not_my";
    detector_height = 200;
    detector_width = 200;

    obj_det_filename = "detector_l.yml";
    cout << "Enter positive sample foulder: ";
    cin >> pos_dir;
    int num_samples = 0;
    cout << "Enter num of negative samples: ";
    cin >> num_samples;

    vector< Mat > pos_lst, full_neg_lst, neg_lst, gradient_lst;
    vector< int > labels;

    clog << "Positive images are being loaded..." ;
    load_images( pos_dir, pos_lst, false );
    if ( pos_lst.size() > 0 )
    {
        clog << "...[done]" << endl;
    } else
    {
        clog << "no image in " << pos_dir <<endl;
        return 1;
    }

    Size pos_image_size = pos_lst[0].size();

    if ( detector_width && detector_height )
    {
        pos_image_size = Size( detector_width, detector_height );
    }
    else
    {
        for ( size_t i = 0; i < pos_lst.size(); ++i )
        {
            if( pos_lst[i].size() != pos_image_size )
            {
                cout << "All positive images should be same size!" << endl;
                exit( 1 );
            }
        }
        pos_image_size = pos_image_size / 8 * 8;
    }

    clog << "Histogram of Gradients are being calculated for positive images...";
    //computeHOGs( pos_image_size, pos_lst, gradient_lst, flip_samples );
    computeHOGs( pos_image_size, pos_lst, gradient_lst, false );
    size_t positive_count = gradient_lst.size();
    labels.assign( positive_count, +1 );
    clog << "\n...[done] ( positive count : " << positive_count << " )" << endl;

    clog << "Negative images loading && Histogram of Gradients calculation...";
    do_stuff_for_neg(neg_dir, pos_image_size, gradient_lst, flip_samples, num_samples);
    size_t negative_count = gradient_lst.size() - positive_count;
    labels.insert( labels.end(), negative_count, -1 );
    CV_Assert( positive_count < labels.size() );
    clog << "\n...[done] ( negative count : " << negative_count << " )" << endl;

    Mat train_data;
    convert_to_ml( gradient_lst, train_data );

    clog << "Training SVM...";
    int64 t = getTickCount();

    Ptr< SVM > svm = SVM::create();
    /* Default values to train SVM */
    svm->setCoef0( 0.0 );
    svm->setDegree( 3 );
    svm->setTermCriteria( TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 1e-3 ) );
    svm->setGamma( 0 );
    svm->setKernel( SVM::LINEAR );
    svm->setNu( 0.5 );
    svm->setP( 0.1 ); // for EPSILON_SVR, epsilon in loss function?
    svm->setC( 0.01 ); // From paper, soft classifier
    svm->setType( SVM::EPS_SVR ); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
    svm->train( train_data, ROW_SAMPLE, labels );

    t = getTickCount() - t;
    clog << "\n...[done] ( took: " << t / getTickFrequency() << " s )" << endl;

    HOGDescriptor hog;
    hog.winSize = pos_image_size;
    hog.setSVMDetector( get_svm_detector( svm ) );
    hog.save( obj_det_filename );

    return 0;
}
