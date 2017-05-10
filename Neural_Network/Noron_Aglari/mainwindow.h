#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include "pixelbasedadaptivesegmenter.h"
#include "framedifferencebgs.h"

using namespace cv;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Mat Kontrol(Mat);
    Mat findConnectedComponents(Mat, int);
    QImage Mat2QImage(cv::Mat const& src);
    QTimer *timer;
    IplImage *frame_aux,*frame;
    IBGS *bgs;
    FrameDifferenceBGS *bgs2;
    Mat img_mask, img_maskF, img_bkgmodel, img_input2,sonuc,grayyy;
    void Veri_Cek();
    void Egitim(int times);
    double Test(double []);
    float Min_Alx(Point2f [],int );
    float Min_Al(Point2f [], int );
    double sigmoid(double x);
    string Arac_dondur(double );


private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

public slots:
    void Basla();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
