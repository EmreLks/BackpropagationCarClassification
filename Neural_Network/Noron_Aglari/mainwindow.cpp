#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <iostream>
#include <opencv2/opencv.hpp>

#include "pixelbasedadaptivesegmenter.h"
#include "framedifferencebgs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QtCore>
#include <math.h>
#include <fstream>
#include <iomanip>

#define InputN 7
#define HN 4
#define OutN 1
#define datanum 1000
#define testnum 423

double sigmoid(double);
double w[InputN][HN];
double v[HN][OutN];

double w_islem[InputN][HN];
double v_islem[HN][OutN];

struct{
    double input[InputN];
    double teach[OutN];
}datagiris[datanum],test_islem[testnum];

double x_out[InputN];
double hn_out[HN];
double y_out[OutN];

double x_goster[InputN];

double y_akson[OutN];

using namespace std;
using namespace cv;

int resize_factor = 20;
CvCapture *capture = 0;

ofstream agirik_kaydet,son_agirlik;

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    capture = cvCaptureFromAVI("/home/lksey/Documents/bitirme_dataset/dataset2.avi");
    if (!capture){
        std::cerr << "Video Acilmadi!" << std::endl;
    }
    bgs = new PixelBasedAdaptiveSegmenter;
    bgs2 = new FrameDifferenceBGS;
    QPixmap pix("/home/lksey/Documents/Noron_Aglari/Noron_Aglari/NORON.png");
    ui->label_6->setPixmap(pix);


}

MainWindow::~MainWindow()
{
    delete ui;
}
Mat MainWindow::Kontrol(Mat sonuc)
{
    Mat grayyy;
    if (sonuc.channels() == 3)
    {
        cvtColor(sonuc, grayyy, CV_RGB2GRAY);
    }
    else
    {
        grayyy = sonuc;
    }
    return grayyy;
}
Mat MainWindow::findConnectedComponents(Mat inputImage, int deger)
{
    Mat grayscale;
    if (inputImage.channels() == 3)
    {
        cvtColor(inputImage, grayscale, CV_BGR2GRAY);
    }
    else
    {
        grayscale = inputImage;
    }

    cv::threshold(grayscale, grayscale, 128, 255, CV_THRESH_BINARY);


    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;


    findContours(grayscale, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    vector<vector<Point> > contours_polyg(contours.size());
    vector<Point2f>centerg(contours.size());
    vector<float>radiusg(contours.size());


    for (int i = 0; i < contours.size(); i++)
    {

        approxPolyDP(Mat(contours[i]), contours_polyg[i], 3, true);
        minEnclosingCircle((Mat)contours_polyg[i], centerg[i], radiusg[i]);
        if (radiusg[i] < deger)
        {
            contours[i].clear();
        }

    }

    Mat dst = Mat::zeros(inputImage.size(), CV_8UC1);
    if (!contours.empty() && !hierarchy.empty())
    {

        for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
        {
            Scalar color(255, 255, 255);
            drawContours(dst, contours, idx, color, CV_FILLED, 8, hierarchy, 0, cv::Point());
        }
    }


    return dst;
}


void MainWindow::Basla()
{


    frame_aux = cvQueryFrame(capture);
    frame = cvCreateImage(cvSize((int)((frame_aux->width*resize_factor) / 100), (int)((frame_aux->height*resize_factor) / 100)), frame_aux->depth, frame_aux->nChannels);
    cvResize(frame_aux, frame);




   // int key = 0;

    //while (key != 'q')
    //{
        //frame_aux = cvQueryFrame(capture);
        //if (!frame_aux) break;
        //cvResize(frame_aux, frame);
        Mat img_input(frame), son(frame);
        QImage qimg2;
        //imshow("Giris", img_input);
        ////////////////////////////////////////////////////
        img_input2 = img_input;
        bgs->process(img_input, img_mask);
        bgs2->process(img_input2, img_maskF, img_bkgmodel);

        if (!img_maskF.empty())
        {

            sonuc = max(img_mask, img_maskF);
            sonuc = findConnectedComponents(sonuc, 15);
            cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4), cv::Point(-1, -1));
            cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2), cv::Point(-1, -1));
            cv::morphologyEx(sonuc, sonuc, cv::MORPH_OPEN, element, cv::Point(-1, -1), 4);
            cv::morphologyEx(sonuc, sonuc, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 1);
            cv::morphologyEx(sonuc, sonuc, cv::MORPH_ERODE, element2, cv::Point(-1, -1), 1);
            cv::medianBlur(sonuc, sonuc, 5);


            //cv::imshow("Maske", sonuc);
            ////////////////////////////////////////////////////////////////////////////////////
            grayyy = Kontrol(sonuc);
            /////////////////////////////////
            Mat gecici;
            if(!grayyy.empty())
            {
                cvtColor(grayyy, gecici, CV_GRAY2BGR);
                qimg2 = Mat2QImage(gecici);
                //cv::imshow("Maske", gecici);
                ui->label_7->setPixmap(QPixmap::fromImage(qimg2));
            }
            //////////////////////////
            cv::threshold(grayyy, grayyy, 128, 255, CV_THRESH_BINARY);
            vector<vector<Point> > contours, contoursAs;
            vector<Vec4i> hierarchyAs;
            findContours(grayyy, contoursAs, hierarchyAs, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
            ///////////////////////////////////////////////////////////////////////////////////
            vector<vector<Point> > contours_polyg(contoursAs.size());
            vector<Point2f>centerg(contoursAs.size());
            vector<float>radiusg(contoursAs.size());

            for (int i = 0; i < contoursAs.size(); i++)
            {
                approxPolyDP(Mat(contoursAs[i]), contours_polyg[i], 3, true);
                minEnclosingCircle((Mat)contours_polyg[i], centerg[i], radiusg[i]);
                if (radiusg[i] > 10 && contoursAs[i].size()>60)
                {
                    contours.push_back(contoursAs[i]);
                }
            }
            if (contours.size() > 0)
            {
                vector<RotatedRect> minRect(contours.size());
                vector<RotatedRect> minEllipse(contours.size());
                for (int i = 0; i < contours.size(); i++)
                {
                    minRect[i] = minAreaRect(Mat(contours[i]));
                    if (contours[i].size() > 5)
                    {
                        minEllipse[i] = fitEllipse(Mat(contours[i]));
                    }
                }


                float en = 0, boy = 0, imge_alan = 0, ellipse_major = 0, ellipse_minor = 0, tum = img_input.cols*img_input.rows;

                for (int i = 0; i < contours.size(); i++)
                {
                    Point2f rect_points[4];
                    minRect[i].points(rect_points);

                    float kontrol_et = rect_points[0].y;


                    if ((kontrol_et > img_input.rows - 27 &&  kontrol_et < img_input.rows - 5 ) && ((abs(rect_points[0].x - rect_points[3].x) == abs(rect_points[1].x - rect_points[2].x)) && (abs(rect_points[0].y - rect_points[1].y) == abs(rect_points[2].y - rect_points[3].y))))
                    {
                        en = minRect[i].size.width;
                        boy = minRect[i].size.height;
                        imge_alan = contourArea(contours[i]);

                        if (minEllipse[i].size.width > minEllipse[i].size.height)
                        {
                            ellipse_major = minEllipse[i].size.width;
                            ellipse_minor = minEllipse[i].size.height;
                        }
                        else
                        {
                            ellipse_major = minEllipse[i].size.height;
                            ellipse_minor = minEllipse[i].size.width;
                        }


                        /*
                        cout << i << "." << " bolgenin alan alan orani" << imge_alan/tum << endl;
                        cout << i << "." << " bolgenin dikdortgen alan orani-->>" << " en oraný: "<<en/tum<<" boy: "<<boy/tum<<endl;
                        cout <<i<< ". dikdorgen/imge_alan:" << ((en*boy) / imge_alan)<<endl;
                        cout << i << " . Beyaz/Siyah: " << ((imge_alan) / ((en*boy) - imge_alan)) << endl;
                        cout << i << ". Major alan orani" << ellipse_major/tum<<endl;
                        cout << i << " . Minor alan orani" << ellipse_minor/tum<<endl;
                        cout << "*********************************************************************" << endl;
                        */
                        //////////////////////////////

                        /*fprintf(dataset, "%f,", imge_alan / tum);
                        fprintf(dataset, "%f,", en / tum);
                        fprintf(dataset, "%f,", boy / tum);
                        fprintf(dataset, "%f,", ((en*boy) / imge_alan));
                        fprintf(dataset, "%f,", ellipse_minor / tum);
                        fprintf(dataset, "%f,", ellipse_major / tum);
                        fprintf(dataset, "%f\n", ((imge_alan) / ((en*boy) - imge_alan)));
                        */

                        Scalar color = Scalar(255, 0, 0);
                        x_goster[0] = imge_alan / tum;
                        x_goster[1] = en / tum;
                        x_goster[2] = boy / tum;
                        x_goster[3] = ((en*boy) / imge_alan);
                        x_goster[4] = ellipse_minor / tum;
                        x_goster[5] = ellipse_major / tum;
                        x_goster[6] = ((imge_alan) / ((en*boy) - imge_alan));


                        ///////////////////////////////

                        double sonuc = Test(x_goster);


                        string yazdirr = Arac_dondur(sonuc);

                        string yazdirr2 = std::to_string(sonuc);

                        String yazdir3 = yazdirr + " -> " +yazdirr2;

                        QString yaz = QString::fromStdString(yazdir3);

                        ui->lineEdit_2->setText(yaz);





                        putText(son,yazdirr, cv::Point(Min_Alx(rect_points, 4), Min_Al(rect_points,4) - 10), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 0, 255), 1);


                        for (int j = 0; j < 4; j++)
                            cv::line(son, rect_points[j], rect_points[(j + 1) % 4], color, 2, 8);


                    }
                }


            }
            //imshow("Siniflama Sonuc", son);
            QImage qimg;

            qimg = Mat2QImage(son);

            ui->label->setPixmap(QPixmap::fromImage(qimg));



        }
    //}
    //fclose(dataset);
/*
    delete bgs;

    cvDestroyAllWindows();
    cvReleaseCapture(&capture);*/

}
QImage MainWindow::Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp; // make the same cv::Mat
     cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
     QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     dest.bits(); // enforce deep copy, see documentation
     // of QImage::QImage ( const uchar * data, int width, int height, Format format )
     return dest;
}

void MainWindow::on_pushButton_clicked()
{
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(Basla()));
    timer->start(20);

}

///////////////////////////////BP ALGORİTHM//////////////////////////////////////////////
void MainWindow::Egitim(int times)
{
        char result[200] = "";
        char buffer[200];


        double deltaw[InputN][HN];
        double deltav[HN][OutN];

        double hn_delta[HN];
        double y_delta[OutN];
        double error=0.0;
        double errlimit = 0.001;
        double learning_rate = 0.05;
        int loop = 0;
        int i, j, m;

        double sumtemp;
        double errtemp;

        agirik_kaydet.open("agirlik.txt", std::ios_base::app);
        agirik_kaydet<<"Rastgele Atanan Agirliklar "<<endl;
        agirik_kaydet<<"Giris Katmani Agirliklari"<<endl;


         srand(time(NULL));
        for(i=0; i<InputN; i++){
            for(j=0; j<HN; j++){

                w[i][j] = ((double)rand()/2147483647.0)*2-1;
                agirik_kaydet<<w[i][j]<<"    ";
                deltaw[i][j] = 0.0;
            }
            agirik_kaydet<<endl;
        }
        for(i=0; i<HN; i++){
            for(j=0; j<OutN; j++){
                v[i][j] = ((double)rand()/2147483647.0)*2-1;
                agirik_kaydet<<v[i][j]<<"    ";
                deltav[i][j] = 0.0;
            }
            agirik_kaydet<<endl;
        }
        agirik_kaydet.close();


        while(loop < times)
        {
            loop++;
            error = 0.0;

            for(m=0; m<datanum ; m++)
            {

                for(i=0; i<InputN; i++){
                    x_out[i] = datagiris[m].input[i];
                }

                for(i=0; i<OutN ; i++){

                    y_akson[i] = datagiris[m].teach[i];
                    y_akson[i] = (y_akson[i] - 1)/2.0;
                }

                for(i=0; i<HN; i++)
                {
                    sumtemp = 0.0;

                    for(j=0; j<InputN; j++)
                    {
                        sumtemp += w[j][i] * x_out[j];
                    }

                    hn_out[i] = sigmoid(sumtemp);

                }

                for(i=0; i<OutN; i++)
                {
                    sumtemp = 0.0;

                    for(j=0; j<HN; j++)
                    {
                        sumtemp += v[j][i] * hn_out[j];
                    }
                    y_out[i] = sigmoid(sumtemp);

                }

                for(i=0; i<OutN; i++)
                {
                    errtemp = y_akson[i] - y_out[i];
                    y_delta[i] = errtemp * y_out[i] * (1.0 - y_out[i]); //Sensivity Cikis
                    error += pow(errtemp,2);
                }

                for(i=0; i<HN; i++)
                {
                    errtemp = 0.0;
                    for(j=0; j<OutN; j++)
                    {
                        errtemp += y_delta[j] * v[i][j];
                    }
                    hn_delta[i] = errtemp * (1.0 - hn_out[i]) * hn_out[i];//Sensivity Giris
                }

                for(i=0; i<OutN; i++)
                {
                    for(j=0; j<HN; j++)
                    {

                        deltav[j][i] = learning_rate * y_delta[i] * hn_out[j];
                        v[j][i] += deltav[j][i];
                    }
                }

                for(i=0; i<HN; i++)
                {
                    for(j=0; j<InputN; j++)
                    {
                        deltaw[j][i] = learning_rate * hn_delta[i] * x_out[j];
                        w[j][i] += deltaw[j][i];
                    }
                }
            }


            error = error / 2.0;
            if(loop%1000==0){
                strcpy(result ,"Global Error = ");
                sprintf(buffer, "%lf", error);
                strcat(result , buffer);
                strcat(result , "\r\n");
            }
            if(error < errlimit)
                break;


            printf("%d -> Egitim  Hata: %lf\n", loop, error);
        }

        printf("\nSonuc: %s\n",result);
}
double MainWindow::sigmoid(double x){
    return(1.0 / (1.0 + exp(-x)));
}
void MainWindow::Veri_Cek()
{
    int i=0;

    ifstream giris_dosyasi;

    giris_dosyasi.open("/home/lksey/Documents/Noron_Aglari/Noron_Aglari/train.txt");



    while(!giris_dosyasi.eof())
    {
        giris_dosyasi>>datagiris[i].input[0]>>datagiris[i].input[1]>>datagiris[i].input[2]>>datagiris[i].input[3]>>datagiris[i].input[4]>>datagiris[i].input[5]>>datagiris[i].input[6]>>datagiris[i].teach[0];
        i++;
    }

    giris_dosyasi.close();


}

//Egitim Baslat
void MainWindow::on_pushButton_2_clicked()
{
    Veri_Cek();
    QString iterasyon = ui->lineEdit->text();
    int ite = iterasyon.toInt();
    Egitim(ite);


    agirik_kaydet.open("agirlik.txt", std::ios_base::app);
    agirik_kaydet<<"\n";
    agirik_kaydet<<"Ierasyon "<<ite<<endl;
    agirik_kaydet<<"Giris Katmani Agirliklari"<<endl;

    for(int i=0; i<InputN; i++){
        for(int j=0; j<HN; j++){
            agirik_kaydet<<w[i][j]<<"   ";
        }
        agirik_kaydet<<endl;
    }
    agirik_kaydet<<endl<<"Gizli Katmani Agirliklari"<<endl;
    for(int i=0; i<HN; i++){
        for(int j=0; j<OutN; j++){
            agirik_kaydet<<v[i][j]<<"    ";
        }
        agirik_kaydet<<endl;
    }

    agirik_kaydet.close();
    /////////////////////////////////////////////////////
    son_agirlik.open("son_agirlik.txt");
    for(int i=0; i<InputN; i++){
        for(int j=0; j<HN; j++){
            son_agirlik<<w[i][j]<<"   ";
        }
        son_agirlik<<endl;
    }
    for(int i=0; i<HN; i++){
        for(int j=0; j<OutN; j++){
            son_agirlik<<v[i][j]<<"    ";
        }
        son_agirlik<<endl;
    }
    son_agirlik.close();



}

void MainWindow::on_pushButton_4_clicked()
{
    ifstream islem_agirlik;

    islem_agirlik.open("son_agirlik.txt");

    for(int i=0; i<InputN; i++){
        for(int j=0; j<HN; j++){
            islem_agirlik>>w_islem[i][j];
            //cout<<w_islem[i][j];
        }
        //cout<<endl;
    }

    printf("\n\n");
    for(int i=0; i<HN; i++){
        for(int j=0; j<OutN; j++){
            islem_agirlik>>v_islem[i][j];
            //cout<<v_islem[i][j];
        }
        //cout<<endl;
    }

    islem_agirlik.close();
    ui->lineEdit_2->setText("Agirliklar Yüklendi");
    cout<<"Agirliklar Yüklendi";
}
double MainWindow::Test(double x_out[])
{
    double max, min;
    double sumtemp;
    double errtemp;

    for(int i=0; i<HN; i++)
    {
        sumtemp = 0.0;
        for(int j=0; j<InputN; j++)
        {
            sumtemp += w_islem[j][i] * x_out[j];
        }
         hn_out[i] = sigmoid(sumtemp);
     }

     for(int i=0; i<OutN; i++)
     {
        sumtemp = 0.0;
        for(int j=0; j<HN; j++)
        {
             sumtemp += v_islem[j][i] * hn_out[j];
        }
        y_out[i] = sigmoid(sumtemp);
     }

    return y_out[0];

}

float MainWindow::Min_Alx(Point2f giris[],int deger)
{
    float max = giris[0].x;
    for (int i = 0; i<deger; i++)
    {
        if (giris[i].x<max)
            max = giris[i].x;
    }
    return max;
}
float MainWindow::Min_Al(Point2f giris[], int deger)
{
    float max = giris[0].y;
    for (int i = 0; i<deger; i++)
    {
        if (giris[i].y<max)
            max = giris[i].y;
    }
    return max;
}
string MainWindow::Arac_dondur(double giris)
{
    string sonuc = "Belli Degil";

    if( abs(giris-0.0) < abs(giris-0.5) && abs(giris-0.0) < abs(giris-1.0)  )
    {
        sonuc = "buyuk";
    }
    else if ( abs(giris-0.5) < abs(giris-0.0) && abs(giris-0.5) < abs(giris-1.0) )
    {
        sonuc = "orta";
    }
    else if( abs(giris-1.0) < abs(giris-0.0) && abs(giris-1.0) < abs(giris-0.5) )
    {
        sonuc = "kucuk";
    }
    return sonuc;
}
