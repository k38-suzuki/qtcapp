#ifndef QTC_EXPORTDECL_H_INCLUDED
# define QTC_EXPORTDECL_H_INCLUDED

# if defined _WIN32 || defined __CYGWIN__
#  define QTC_DLLIMPORT __declspec(dllimport)
#  define QTC_DLLEXPORT __declspec(dllexport)
#  define QTC_DLLLOCAL
# else
#  if __GNUC__ >= 4
#   define QTC_DLLIMPORT __attribute__ ((visibility("default")))
#   define QTC_DLLEXPORT __attribute__ ((visibility("default")))
#   define QTC_DLLLOCAL  __attribute__ ((visibility("hidden")))
#  else
#   define QTC_DLLIMPORT
#   define QTC_DLLEXPORT
#   define QTC_DLLLOCAL
#  endif
# endif

# ifdef QTC_STATIC
#  define QTC_DLLAPI
#  define QTC_LOCAL
# else
#  ifdef Qtcapp_EXPORTS
#   define QTC_DLLAPI QTC_DLLEXPORT
#  else
#   define QTC_DLLAPI QTC_DLLIMPORT
#  endif
#  define QTC_LOCAL QTC_DLLLOCAL
# endif

#endif

#ifdef QTC_EXPORT
# undef QTC_EXPORT
#endif
#define QTC_EXPORT QTC_DLLAPI
