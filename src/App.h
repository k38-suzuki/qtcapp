/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTCAPP_APP_H
#define QTCAPP_APP_H

namespace qtc {

class AppImpl;

class App
{
public:
    App(int argc, char** argv);
    virtual ~App();

    int exec();

private:
    AppImpl* impl;
    friend class AppImpl;
};

}

#endif // QTCAPP_APP_H
