/**
   \file
   \author Kenta Suzuki
*/

#ifndef NETEM_NETEM_WIDGET_H
#define NETEM_NETEM_WIDGET_H

#include <QWidget>
#include "exportdecl.h"

namespace netem {

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

#endif // NETEM_NETEM_WIDGET_H
