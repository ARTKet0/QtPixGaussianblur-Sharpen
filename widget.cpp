#include "widget.h"
#include "ui_widget.h"

#include<QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->widget_imgPaint->installEventFilter(this);
    ui->label_DisplayImg->installEventFilter(this); //display sample img

//Comb add Items

    ui->comboBox_SelectPixmap->addItem("OriginImage",0);
    ui->comboBox_SelectPixmap->addItem("1_Image",1);


//Init
    img.load(":/Image/img_0.jpg");
    constimg=img;
    originimg=img;

    radius=0;
    variance=0.01;
    sharp_rate=0;

    img_select=img;

    is_gaussianblur=false;
    is_monochrome=false;
    is_sharpen=false;

 //Connect
    //radius changed
    connect(ui->HorlSlider_radius,SIGNAL(valueChanged(int)),this,SLOT(radiusChanged()));

    //variance changed
    connect(ui->HorSlider_variance,SIGNAL(valueChanged(int)),this,SLOT(varianceChanged()));

    //sharp_rate changed
    connect(ui->HorlSlider_sharprate,&QSlider::valueChanged,this,&Widget::sharprateChanged);

    //Data Reset
    connect(ui->pushButton_ResetData,SIGNAL(clicked()),this,SLOT(dataReset()));

    //Change Pixmap
    connect(ui->pushButton_ChangePixmap,&QPushButton::clicked,this,&Widget::ChangePixmap);

    //Select Pixmap
    connect(ui->comboBox_SelectPixmap,&QComboBox::currentTextChanged,this,&Widget::SelectPixmap);

    //GaussianBlur Pixmap
    connect(ui->radioButton_GaussianBlur_enable,&QRadioButton::clicked,this,&Widget::enableGaussianBlur);

    //Sharp Pixmap
    connect(ui->radioButton_Sharp_enable,&QRadioButton::clicked,this,&Widget::enableSharpen);

    //Monochrome Pixmap
    connect(ui->radioButton_monochrome,&QRadioButton::clicked,this,&Widget::enableMonochrome);



}
Widget::~Widget(){  delete ui;}


bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::Paint&&watched==ui->widget_imgPaint)
    {

        img=constimg;
        imgPaint();

    }
    if(event->type()==QEvent::Paint&&watched==ui->label_DisplayImg)
    {
        img_display=img;
        displaySampleImg();
    }
    return QWidget::eventFilter(watched,event);
}

void Widget::imgPaint()
{
    QPainter painter(ui->widget_imgPaint);

    img=img.scaled(QSize(ui->widget_imgPaint->width(),ui->widget_imgPaint->height()),Qt::KeepAspectRatio);
    int drawpointx=(ui->widget_imgPaint->width()-img.width())/2;
    int drawpointy=(ui->widget_imgPaint->height()-img.height())/2;
    constimg=img;
    originimg=img;

    //Monochrome
    if(is_monochrome)
    {
        setMonochrome();
        originimg=img;
    }


    //GaussianBlus
    if(is_gaussianblur)
    {
        gassianblur();
        originimg=img;
    }

    //Sharpen
    if(is_sharpen)
    {
        sharpen();
    }


    painter.drawImage(drawpointx,drawpointy,img);

}

void Widget::displaySampleImg()
{
    QPainter painter(ui->label_DisplayImg);

    img_display=img_select.scaled(QSize(ui->label_DisplayImg->width(),ui->label_DisplayImg->height()),Qt::KeepAspectRatio);
    int drawpointx=(ui->label_DisplayImg->width()-img_display.width())/2;
    int drawpointy=(ui->label_DisplayImg->height()-img_display.height())/2;

    painter.drawImage(drawpointx,drawpointy,img_display);
}

double Widget::Gaussian1D(int r,double v)
{
    return std::exp(-pow(r,2)/((2*pow(v,2))))/(std::pow(v,2));
}
QRgb Widget::Lapacian2D(int &x,int &y,QImage &_img)
{
    int w=_img.width();
    int h=_img.height();
    QColor color=_img.pixelColor(x,y);
    int ar[3][3]={0};
    int n;
    //int k;
    int add_rgb;

    ar[0][1]=(y==0)?0:qGray(_img.pixelColor(x,y-1).rgb());
    ar[1][0]=(x==0)?0:qGray(_img.pixelColor(x-1,y).rgb());
    ar[1][2]=(x==w-1)?0:qGray(_img.pixelColor(x+1,y).rgb());
    ar[2][1]=(y==h-1)?0:qGray(_img.pixelColor(x,y+1).rgb());
/*
    // 4 和 8  差别不大
    ar[0][0]=(y==0||x==0)?0:qGray(_img.pixelColor(x-1,y-1).rgb());
    ar[0][2]=(y==0||x==w-1)?0:qGray(_img.pixelColor(x+1,y-1).rgb());
    ar[2][0]=(y==h-1||x==0)?0:qGray(_img.pixelColor(x-1,y+1).rgb());
    ar[2][2]=(y==h-1||x==w-1)?0:qGray(_img.pixelColor(x+1,y+1).rgb());
*/
    ar[2][2]=qGray(_img.pixelColor(x,y).rgb());

    n=ar[0][1]+ar[2][1]+ar[1][0]+ar[1][2]-4*ar[2][2];
            //+ar[0][0]+ar[0][2]+ar[2][0]+ar[2][2]-4*ar[2][2];

    add_rgb=n*sharp_rate;

    int R=color.red()-add_rgb;
    int G=color.green()-add_rgb;
    int B=color.blue()-add_rgb;

    R=R>255?255:(R<0?0:R);
    G=G>255?255:(G<0?0:G);
    B=B>255?255:(B<0?0:B);


    return qRgb(R,G,B);

}

void Widget::gassianblur()
{

    int d=radius*2;
    int gausssize=radius*2+1;
    int r=radius;
    int w=img.width();
    int h=img.height();

    qreal gaussw[gausssize]={0};
    qreal sum_gaussw=0;
    //高斯核
    for(int a=0;a<gausssize;a++)
    {
        gaussw[a]=Gaussian1D(a-r,variance);

        sum_gaussw+=gaussw[a];
    }
    //sum To one 计算权值
    for(auto& n : gaussw)
    {
        n/=sum_gaussw;
    }

    //Get GaussianBlusColor - X
    for(int y=0;y<h;y++)
    {

        qreal gaussR[w+gausssize]={0};
        qreal gaussG[w+gausssize]={0};
        qreal gaussB[w+gausssize]={0};

        qreal sumgaussR=0;
        qreal sumgaussG=0;
        qreal sumgaussB=0;
        for(int x=0;x<w;x++)
        {
            int n=x % gausssize;

            if(x<r)  //图片边界重复利用（边界模糊）左侧
            {
                QColor color=img.pixelColor(x,y);
                gaussR[x]=color.red();
                gaussG[x]=color.green();
                gaussB[x]=color.blue();

                gaussR[x]*=gaussw[n];
                gaussG[x]*=gaussw[n];
                gaussB[x]*=gaussw[n];
            }
            if(x>=r)
            {
                QColor color=img.pixelColor(x-r,y);
                gaussR[x]=color.red();
                gaussG[x]=color.green();
                gaussB[x]=color.blue();

                gaussR[x]*=gaussw[n];
                gaussG[x]*=gaussw[n];
                gaussB[x]*=gaussw[n];
            }
            if(x==w-1)
            {
                for(int _x=0;_x<r;_x++)
                {
                    n=(w+_x) % gausssize;

                    QColor color=img.pixelColor(w-r+_x,y);
                    gaussR[w+_x]=color.red();
                    gaussG[w+_x]=color.green();
                    gaussB[w+_x]=color.blue();

                    gaussR[w+_x]*=gaussw[n];
                    gaussG[w+_x]*=gaussw[n];
                    gaussB[w+_x]*=gaussw[n];
                }
            }

            if(x==gausssize)  //图片边界重复利用（边界模糊）右侧
            {
                for(int _x=r;_x<d;_x++)
                {
                    QColor color=img.pixelColor(w-d+_x,y);
                    n=(w+_x)%gausssize;

                    gaussR[w+_x]=color.red();
                    gaussG[w+_x]=color.green();
                    gaussB[w+_x]=color.blue();

                    gaussR[w+_x]*=gaussw[n];
                    gaussG[w+_x]*=gaussw[n];
                    gaussB[w+_x]*=gaussw[n];
                }
            }


        }
        //Set GaussianBlurColor - X
        for(int x=0;x<w;x++)
        {
            if(x==0)   //Get sumgaussRGB
            {
                for(int a=0;a<gausssize;a++)
                {
                    sumgaussR+=gaussR[a];
                    sumgaussG+=gaussG[a];
                    sumgaussB+=gaussB[a];
                }
            }
            else   //add endpoint_gaussRGB
            {
                sumgaussR+=gaussR[x+d];
                sumgaussG+=gaussG[x+d];
                sumgaussB+=gaussB[x+d];
            }

            img.setPixel(x,y,qRgb(sumgaussR,sumgaussG,sumgaussB));

            //reduce beginpoint_gaussRGB
            sumgaussR-=gaussR[x];
            sumgaussG-=gaussG[x];
            sumgaussB-=gaussB[x];
        }
    }


    //Get GaussianBlurColor - Y
    for(int x=0;x<w;x++)
    {
        qreal gaussR[w+gausssize]={0};
        qreal gaussG[w+gausssize]={0};
        qreal gaussB[w+gausssize]={0};

        qreal sumgaussR=0;
        qreal sumgaussG=0;
        qreal sumgaussB=0;
        for(int y=0;y<h;y++)
        {
            int n=y % gausssize;
            int pointy=0;

            if(y<r)  //图片边界重复利用（边界模糊）上侧
            {
                QColor color=img.pixelColor(x,y);
                gaussR[y]=color.red();
                gaussG[y]=color.green();
                gaussB[y]=color.blue();

                gaussR[y]*=gaussw[n];
                gaussG[y]*=gaussw[n];
                gaussB[y]*=gaussw[n];
            }
            if(y>=r&&y<h)
            {
                pointy=y-r;
                QColor color=img.pixelColor(x,pointy);
                gaussR[y]=color.red();
                gaussG[y]=color.green();
                gaussB[y]=color.blue();

                gaussR[y]*=gaussw[n];
                gaussG[y]*=gaussw[n];
                gaussB[y]*=gaussw[n];
            }

            if(y==h-1)
            {
                for(int _y=0;_y<r;_y++)
                {
                    n=(h+_y) % gausssize;

                    QColor color=img.pixelColor(x,h+_y-r);
                    gaussR[h+_y]=color.red();
                    gaussG[h+_y]=color.green();
                    gaussB[h+_y]=color.blue();

                    gaussR[h+_y]*=gaussw[n];
                    gaussG[h+_y]*=gaussw[n];
                    gaussB[h+_y]*=gaussw[n];
                }
            }
            if(y==gausssize)   //图片边界重复利用（边界模糊）下侧
            {
                for(int _y=r;_y<d;_y++)
                {
                    QColor color=img.pixelColor(x,h-d+_y);
                    n=(h+_y)%gausssize;

                    gaussR[h+_y]=color.red();
                    gaussG[h+_y]=color.green();
                    gaussB[h+_y]=color.blue();

                    gaussR[h+_y]*=gaussw[n];
                    gaussG[h+_y]*=gaussw[n];
                    gaussB[h+_y]*=gaussw[n];
                }
            }
        }

        //Set GaussianBlurColor - Y
        for(int y=0;y<h;y++)
        {
            if(y==0)  //Get sumgaussRGB
            {
                for(int a=0;a<gausssize;a++)
                {
                    sumgaussR+=gaussR[a];
                    sumgaussG+=gaussG[a];
                    sumgaussB+=gaussB[a];
                }
            }
            else   //add endpoint_gaussRGB
            {
                sumgaussR+=gaussR[y+d];
                sumgaussG+=gaussG[y+d];
                sumgaussB+=gaussB[y+d];
            }
            img.setPixel(x,y,qRgb(sumgaussR,sumgaussG,sumgaussB));

            //reduce beginpoint_gaussRGB
            sumgaussR-=gaussR[y];
            sumgaussG-=gaussG[y];
            sumgaussB-=gaussB[y];
        }
    }
}

void Widget::sharpen()
{
    int w=img.width();
    int h=img.height();
    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
        {
            QRgb rgb=Lapacian2D(x,y,originimg);
            img.setPixel(x,y,rgb);
        }
    }

}

void Widget::setMonochrome()
{
    int w=img.width();
    int h=img.height();
    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
        {
            int gray=qGray(img.pixelColor(x,y).rgb());
            img.setPixel(x,y,qRgb(gray,gray,gray));
        }
    }
}

void Widget::radiusChanged()
{
    radius=(ui->HorlSlider_radius->value())/10;
    ui->labRadiusValue->setText(QString::number(radius));

    ui->widget_imgPaint->update();
}

void Widget::varianceChanged()
{
    variance=qreal(ui->HorSlider_variance->value())/100;

    ui->labVarianceValue->setText(QString::number(variance));
    ui->widget_imgPaint->update();
}

void Widget::sharprateChanged()
{
    sharp_rate=qreal(ui->HorlSlider_sharprate->value())/100;

    ui->labSharpRateValue->setText(QString::number(sharp_rate));
    ui->widget_imgPaint->update();
}

void Widget::dataReset()
{
    radius=0;
    variance=0.01;
    is_monochrome=false;
    is_gaussianblur=false;
    is_sharpen=false;

    ui->HorlSlider_radius->setValue(radius*10);
    ui->HorSlider_variance->setValue(variance*100);
    ui->radioButton_monochrome->setChecked(false);
    ui->radioButton_GaussianBlur_enable->setChecked(false);
    ui->radioButton_Sharp_enable->setChecked(false);

    radiusChanged();
    varianceChanged();
    sharprateChanged();
}

void Widget::ChangePixmap()
{

    if(img_select.isNull())
    {
        qDebug()<<"ChangePixmap - img_select is Null";
    }
    else
    {
        img=img_select;
        constimg=img_select;
    }

    ui->widget_imgPaint->update();
}

void Widget::SelectPixmap()
{
    int imgnum=ui->comboBox_SelectPixmap->currentIndex();
    qDebug()<<"SelectPixmap : "<<imgnum;
    img_select.load(":/Image/img_"+QString::number(imgnum)+".jpg");

    ui->label_DisplayImg->update();

    if(img_select.isNull())
    {
        qDebug()<<"img->"<<":/Image/img_"+QString::number(imgnum)+".jpg"<<" is Null";
    }

}

void Widget::enableGaussianBlur()
{
    is_gaussianblur=ui->radioButton_GaussianBlur_enable->isChecked();
    if(is_gaussianblur)
        qDebug()<<"is_gaussianblur_true";
    else
        qDebug()<<"is_gaussianblur_false";

    ui->widget_imgPaint->update();
}

void Widget::enableMonochrome()
{
    is_monochrome=ui->radioButton_monochrome->isChecked();
    if(is_monochrome)
        qDebug()<<"is_monochrome_true";
    else
        qDebug()<<"is_monochrome_false";

    ui->widget_imgPaint->update();

}

void Widget::enableSharpen()
{
    is_sharpen=ui->radioButton_Sharp_enable->isChecked();
        if(is_sharpen)
            qDebug()<<"is_sharpen_true";
        else
            qDebug()<<"is_sharpen_false";

        ui->widget_imgPaint->update();
}



