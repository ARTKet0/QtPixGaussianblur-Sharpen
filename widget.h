#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QImage>
#include<QtMath>
#include<QPainter>
#include<QVector>
#include<QString>
#include<QFileDialog>
#include<QMessageBox>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    bool eventFilter(QObject *watched, QEvent *event);
    void imgPaint();

    void displaySampleImg();

    //Function
    double Gaussian1D(int r,double v);
    QRgb Lapacian2D(int &x,int &y,QImage &img);

    void gassianblur();
    void sharpen();
    void setMonochrome();



private:
    Ui::Widget *ui;

    QImage img;
    QImage constimg;
    QImage originimg;

    QImage img_select;
    QImage img_display;

    qreal radius;
    qreal variance;
    qreal sharp_rate;

    bool is_gaussianblur;
    bool is_monochrome;
    bool is_sharpen;


private slots:
    void radiusChanged();
    void varianceChanged();
    void sharprateChanged();

    void dataReset();

    void ChangePixmap();
    void SelectPixmap();
    void enableGaussianBlur();
    void enableMonochrome();
    void enableSharpen();



};


#endif // WIDGET_H
