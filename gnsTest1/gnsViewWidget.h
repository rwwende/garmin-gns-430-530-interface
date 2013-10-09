#ifndef __GNS_VIEW_WIDGTET_H__
#define __GNS_VIEW_WIDGTET_H__
#include <QtGui>
#include <QGLWidget>

class GNSx30Proxy;

class GnsViewWidget : public QGLWidget
{
    Q_OBJECT

public:
    GnsViewWidget(QWidget *parent = 0, const QGLWidget * shareWidget = 0);
    ~GnsViewWidget();

    void    setGNSx30Proxy(GNSx30Proxy*    pGNSx30Proxy);
    int     getW();
    int     getH();

public slots:
    void onUpdateTimer();


protected:

    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    virtual void mousePressEvent ( QMouseEvent * event );

private:
    QTimer          m_updateTimer;
    GNSx30Proxy*    m_pGNSx30Proxy;

    GLuint          m_bezelTextureID;
    GLuint          m_lcdTextureID;

};

#endif //MANUALCALIBRATIONGLWIDGET_H
