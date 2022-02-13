/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTC_APP_H
#define QTC_APP_H

#if defined(_WIN32)
#  if defined(EXPORTING_QTCBASE)
#    define DECLSPEC __declspec(dllexport)
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC
#endif

namespace qtc {

class AppImpl;

class DECLSPEC App
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

#endif // QTC_APP_H
