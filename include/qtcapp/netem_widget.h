/**
   \file
   \author Kenta Suzuki
*/

#ifndef RQT_NETEM_NETEM_WIDGET_H
#define RQT_NETEM_NETEM_WIDGET_H

#include <QWidget>
#include "exportdecl.h"

namespace rqt_netem {

class NetEmWidgetImpl;

class DECLSPEC NetEmWidget : public QWidget
{
    Q_OBJECT
public:
    NetEmWidget(QWidget* parent = 0);
    virtual ~NetEmWidget();

private:
    NetEmWidgetImpl* impl;
    friend class NetEmWidgetImpl;
};

}

#endif // RQT_NETEM_NETEM_WIDGET_H
