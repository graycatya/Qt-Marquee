#include "marquee.h"
#include <QDebug>

Marquee::Marquee(QWidget *parent) :
    QWidget(parent)
{
    this->resize(400, 50);
    /*初始化 跑马灯*/
    m_vMarquee_Texts.clear();
    m_qMarqueeStackedWidgets.clear();
    m_qMarqueeStackedWidget.clear();
    m_vMarquee_Texts.push_back("hello word! hello word! hello word! hello word! hello word! hello word! hello word! hello word!");
    m_qColor = "gray";
    m_qFont.setFamily("Microsoft YaHei");
    m_qFont.setBold(true);
    m_qFont.setPointSize(30);
    m_nIndexMarquee = 0;
    m_nIndexMarqueeSeat = 0;
    m_nMarqueeSpeen = 45;

    //初始化 Ui
    m_pHorizontalLayout = new QHBoxLayout(this);
    m_pHorizontalLayout->setSpacing(0);
    m_pHorizontalLayout->setContentsMargins(0, 0, 0, 0);
    m_pStackedWidget= new QStackedWidget(this);
    m_pStackedWidget->setStyleSheet("background:transparent");
    m_pHorizontalLayout->addWidget(m_pStackedWidget);
    //初始化 - 刷新线程
    m_pThreadPool = new ThreadPool(1);
    m_bThreadStart = false;
    connect(this, SIGNAL(Update_Marquee_Signal()), this, SLOT(Update_Marquee_Slot()));

}

Marquee::~Marquee()
{
    m_bThreadStart = false;
    m_qThreadCondition.notify_all();
    if(m_pThreadPool != nullptr)
    {
        delete m_pThreadPool;
        m_pThreadPool = nullptr;
    }
    Delete_Marquee();
}

void Marquee::SetFont(QFont font)
{
    m_qFont = font;
}

void Marquee::SetColor(QString Color)
{
    m_qColor = Color;
}

void Marquee::AddText(QString text)
{
    static bool bit = false;
    if(bit == false)
    {
        bit = true;
       Delete_Marquee();
       m_vMarquee_Texts.clear();
       m_qMarqueeStackedWidgets.clear();
       m_qMarqueeStackedWidget.clear();
    }
    m_vMarquee_Texts.push_back(text);
    Update_Marquee(text);
}

void Marquee::SetTexts(QVector<QString> texts)
{
    m_vMarquee_Texts = texts;
}

void Marquee::SetSpeed(Marquee::SPEED speed)
{
    switch(speed)
    {
        case SPEED::Quick:
            m_nMarqueeSpeen = 15;
            break;
        case SPEED::Secondary:
            m_nMarqueeSpeen = 25;
            break;
        case SPEED::Slow:
            m_nMarqueeSpeen = 45;
            break;
    }
}

void Marquee::Update_Marquee(QString str)
{
    QFontMetrics fm(m_qFont);
    //先把多个字符串分别存放
    QRect rec = fm.boundingRect(str);
    int m_nWidget = rec.width();
    int m_nHeight = rec.height();
    Marquee_StackedWidgets *stackedwidgets = new Marquee_StackedWidgets;
    stackedwidgets->MarqueeWidget = new QWidget();
    stackedwidgets->VerticalLayout = new QVBoxLayout(stackedwidgets->MarqueeWidget);
    stackedwidgets->VerticalLayout->setSpacing(0);
    stackedwidgets->VerticalLayout->setContentsMargins(0, 0, 0, 0);
    stackedwidgets->StackedWidget = new QStackedWidget(stackedwidgets->MarqueeWidget);
    stackedwidgets->VerticalLayout->addWidget(stackedwidgets->StackedWidget);
    m_pStackedWidget->addWidget(stackedwidgets->MarqueeWidget);

    //储存分配的地址
    m_qMarqueeStackedWidgets.insert(str, stackedwidgets);

    //提前分配跑马灯刷新数据
    for(int j = this->width(); j > -m_nWidget; j = j - 1)
    {
        Marquee_StackedWidget *stackedWidget = new Marquee_StackedWidget;
        stackedWidget->Widget = new QWidget();
        stackedWidget->Label = new QLabel(stackedWidget->Widget);
        stackedWidget->Widget->resize(m_nWidget, m_nHeight);
        stackedWidget->Label->setFont(m_qFont);
        stackedWidget->Label->setText(str);
        stackedWidget->Label->setGeometry(j, (this->height() - m_nHeight) / 2, m_nWidget+5, m_nHeight);

        stackedWidget->Widget->setStyleSheet("background:transparent");
        QString color = QString("background:transparent;"
                                "color:%1").arg(this->m_qColor);
        stackedWidget->Label->setStyleSheet(color);

        m_qMarqueeStackedWidget[str].push_back(stackedWidget);
        stackedwidgets->StackedWidget->addWidget(stackedWidget->Widget);
    }
}

void Marquee::Delete_Marquee()
{

    for(int i = 0; i < m_qMarqueeStackedWidget.size(); i++)
    {
        for(int j = 0; j < m_qMarqueeStackedWidget[m_vMarquee_Texts[i]].size(); j++)
        {
            delete m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j]->Widget;
            delete m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j]->Label;
            delete m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j];
            m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j]->Widget = nullptr;
            m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j]->Label = nullptr;
            m_qMarqueeStackedWidget[m_vMarquee_Texts[i]][j] = nullptr;
        }
    }
    for(int i = 0; i < m_qMarqueeStackedWidgets.size(); i++)
    {
        delete m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->VerticalLayout;
        delete m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->MarqueeWidget;
        delete m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->StackedWidget;
        delete m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]];
        m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->VerticalLayout = nullptr;
        m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->MarqueeWidget = nullptr;
        m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]]->StackedWidget = nullptr;
        m_qMarqueeStackedWidgets[m_vMarquee_Texts[i]] = nullptr;
    }
}

void Marquee::Start( int index )
{
    if(index > m_vMarquee_Texts.size())
    {
        throw "index greater than Marquee_Texts size";
    } else {
        m_bThreadStart = true;
        m_nIndexMarquee = index;
        m_pStackedWidget->setCurrentIndex(m_nIndexMarquee);
        m_pThreadPool->enqueue([this]{
            while(m_bThreadStart)
            {
                QThread::msleep(static_cast<unsigned long>(m_nMarqueeSpeen));
                emit Update_Marquee_Signal();
                m_qThreadMutex.lock();
                m_qThreadCondition.wait(&m_qThreadMutex);
                m_qThreadMutex.unlock();
                if(m_bThreadStart == false)
                    break;
            }
        });
    }
}

void Marquee::Stop()
{
    if(m_bThreadStart == true)
    {
        throw "No boot update";
    } else {
        m_bThreadStart = false;
        m_nIndexMarqueeSeat = 0;
        m_qThreadCondition.notify_one();
    }
}

void Marquee::Update_Marquee_Slot()
{
    m_qMarqueeStackedWidgets[m_vMarquee_Texts[m_nIndexMarquee]]->StackedWidget->setCurrentIndex(m_nIndexMarqueeSeat);
    m_nIndexMarqueeSeat++;
    if(m_nIndexMarqueeSeat > m_qMarqueeStackedWidget[m_vMarquee_Texts[m_nIndexMarquee]].size())
    {
        m_nIndexMarqueeSeat = 0;
    }
    m_qThreadCondition.notify_one();
}

