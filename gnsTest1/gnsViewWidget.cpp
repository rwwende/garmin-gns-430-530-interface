#include <QtGui>
#include <QtOpenGL>

#include "gnsViewWidget.h"
#include "gnsx30proxy.h"

#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_CLAMP_TO_EDGE 0x812F 
#define GL_CLAMP_TO_BORDER 0x812D 

#define DEFAULT_W   320
#define DEFAULT_H   200


GnsViewWidget::GnsViewWidget( QWidget *parent, const QGLWidget * shareWidget )
:QGLWidget( parent, shareWidget)
{
    m_pGNSx30Proxy = NULL;

    //setMouseTracking(true);

}

GnsViewWidget::~GnsViewWidget()
{

}


void GnsViewWidget::setGNSx30Proxy(GNSx30Proxy* pGNSx30Proxy)
{
    m_pGNSx30Proxy = pGNSx30Proxy;
}


int GnsViewWidget::getW()
{

    if(NULL != m_pGNSx30Proxy)    
    {
        GNSIntf* pGNSIntf = m_pGNSx30Proxy->getInterface();
        return pGNSIntf->bezel_width;
    }else
    {
        return DEFAULT_W;
    }
}

int GnsViewWidget::getH()
{
    if(NULL != m_pGNSx30Proxy)    
    {
        GNSIntf* pGNSIntf = m_pGNSx30Proxy->getInterface();
        return pGNSIntf->bezel_height;
    }else
    {
        return DEFAULT_H;
    }

}






void GnsViewWidget::initializeGL()
{


    glClearColor(0,0,0, 1);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_BLEND);


    //Create the bezel texture
    glGenTextures(1, &m_bezelTextureID);
    glBindTexture(GL_TEXTURE_2D, m_bezelTextureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_BGR, 10, 10, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 10, 10, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


    glGenTextures(1, &m_lcdTextureID);
    glBindTexture(GL_TEXTURE_2D, m_lcdTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGR, 10, 10, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);



    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
    m_updateTimer.setInterval(50);
    m_updateTimer.setSingleShot(true);
    m_updateTimer.start();

}






void GnsViewWidget::paintGL()
{

    glViewport(0,0, width(), height());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,1,1,0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0,0,0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if(NULL != m_pGNSx30Proxy)
    {
        GNSIntf* pGNSIntf = m_pGNSx30Proxy->getInterface();


        if(pGNSIntf->LCDUpdated)
        {

            glBindTexture(GL_TEXTURE_2D, m_lcdTextureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pGNSIntf->lcd_width, pGNSIntf->lcd_height, 0, GL_BGR, GL_UNSIGNED_BYTE, pGNSIntf->LCD_data);

            pGNSIntf->LCDUpdated = false;

        }

        if(pGNSIntf->BezelUpdated)
        {

            glBindTexture(GL_TEXTURE_2D, m_bezelTextureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pGNSIntf->bezel_width, pGNSIntf->bezel_height, 0, GL_BGR, GL_UNSIGNED_BYTE, pGNSIntf->Bezel_data);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pGNSIntf->bezel_width, pGNSIntf->bezel_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pGNSIntf->Bezel_data);

            pGNSIntf->BezelUpdated = false;

        }



        if(NULL != m_pGNSx30Proxy)
        {
            GNSIntf* pGNSIntf = m_pGNSx30Proxy->getInterface();

            glBindTexture(GL_TEXTURE_2D, m_lcdTextureID);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            glBegin(GL_QUADS);
            glColor3f(1,1,1);

            float left = (float)pGNSIntf->bezel_lcd_left/(float)pGNSIntf->bezel_width;
            float right = (float)(pGNSIntf->bezel_lcd_left+pGNSIntf->bezel_lcd_width)/(float)pGNSIntf->bezel_width;
            float top = (float)pGNSIntf->bezel_lcd_top/(float)pGNSIntf->bezel_height;
            float bottom = (float)(pGNSIntf->bezel_lcd_top+pGNSIntf->bezel_lcd_height)/(float)pGNSIntf->bezel_height;

            glTexCoord2f(0, 1); 
            glVertex2f(left, top);

            glTexCoord2f(1 ,1); 
            glVertex2f(right, top);

            glTexCoord2f(1, 0); 
            glVertex2f(right, bottom);

            glTexCoord2f(0, 0); 
            glVertex2f(left, bottom);

            glEnd();
        }


        glBindTexture(GL_TEXTURE_2D, m_bezelTextureID);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA );


        glBegin(GL_QUADS);
        glColor3f(1,1,1);

        glTexCoord2f(0, 1); 
        glVertex2f(0, 0);

        glTexCoord2f(1 ,1); 
        glVertex2f(1, 0);

        glTexCoord2f(1, 0); 
        glVertex2f(1, 1);

        glTexCoord2f(0, 0); 
        glVertex2f(0, 1);

        glEnd();


    
    }




    //Restart the timer because it is in single step mode
    m_updateTimer.start();

    
}


void GnsViewWidget::resizeGL(int width, int height)
{

    glViewport(0,0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    
}



void GnsViewWidget::onUpdateTimer()
{
    updateGL();
}

void GnsViewWidget::mousePressEvent ( QMouseEvent * event )
{
    if(NULL != m_pGNSx30Proxy)    
    {
        GNSIntf* pGNSIntf = m_pGNSx30Proxy->getInterface();
        //return pGNSIntf->bezel_width;
        float x = (float)event->x()/(float)width();
        float y = (float)event->y()/(float)height();
        
        m_pGNSx30Proxy->sendMsg(0, x*pGNSIntf->bezel_width, y*pGNSIntf->bezel_height);
        for(int d=0; d <10; d++)
        {
            Sleep(10);
            qApp->processEvents();
        }
        m_pGNSx30Proxy->sendMsg(1, x*pGNSIntf->bezel_width, y*pGNSIntf->bezel_height);

    }
}














